#include "device.h"
#include "device_map.h"
#include "linq_internal.h"

#define JSMN_HEADER
#include "jsmn/jsmn.h"

// Main class
typedef struct linq
{
    void* context;
    zsock_t* sock;
    device_map* devices;
    linq_callbacks* callbacks;
} linq;

// A version on the wire is a byte
typedef int8_t version;

// Types of protocol messages
typedef enum
{
    heartbeat = 0,
    request = 1,
    response = 2,
    alert = 3
} type;

// parse a token from a json string inside a frame
static uint32_t
parse_token(zframe_t* f, jsmntok_t* t, char** ptr)
{
    uint32_t sz = t->end - t->start;
    *ptr = (char*)&zframe_data(f)[t->start];
    (*ptr)[sz] = 0;
    return sz;
}

// unholy footgun (parse many tokens)
static uint32_t
parse_tokens(
    zframe_t* f,
    uint32_t n_tokens,
    jsmntok_t** tokens_p,
    uint32_t n_tags,
    ...)
{
    char *tag = NULL, *cmp, **str;
    uint32_t taglen = 0;
    uint32_t count = 0;
    jsmntok_t* t = *tokens_p;
    for (uint32_t i = 0; i < n_tokens; i++) {
        if (t[i].type == JSMN_OBJECT || (t[i].type == JSMN_ARRAY)) continue;
        if (!tag) {
            taglen = parse_token(f, &t[i], &tag);
        } else {
            uint32_t c = n_tags << 1;
            va_list list;
            va_start(list, n_tags);
            while (c >= 2) {
                c -= 2;
                cmp = va_arg(list, char*);
                str = va_arg(list, char**);
                if (taglen == strlen(cmp) && !memcmp(tag, cmp, taglen)) {
                    parse_token(f, &t[i], str);
                    count++;
                }
            }
            va_end(list);
            tag = NULL;
        }
    }
    return count;
}

// read incoming zmq frame and test size is equal to expected
static zframe_t*
pop_eq(zmsg_t* msg, uint32_t expect)
{
    zframe_t* frame = zmsg_pop(msg);
    if (zframe_size(frame) == expect) {
        return frame;
    } else {
        zframe_destroy(&frame);
        return NULL;
    }
}

// read incoming zmq frame and test size is less than value
static zframe_t*
pop_lt(zmsg_t* msg, uint32_t lt)
{
    zframe_t* frame = zmsg_pop(msg);
    if (zframe_size(frame) < lt) {
        return frame;
    } else {
        zframe_destroy(&frame);
        return NULL;
    }
}

// read incoming zmq frame and test size is less than value
static zframe_t*
pop_le(zmsg_t* msg, uint32_t le)
{
    zframe_t* frame = zmsg_pop(msg);
    if (zframe_size(frame) <= le) {
        return frame;
    } else {
        zframe_destroy(&frame);
        return NULL;
    }
}

// read incoming zmq frame and test valid json
static zframe_t*
pop_alert(zmsg_t* msg, linq_alert* alert)
{
    memset(alert, 0, sizeof(linq_alert));
    int r, count;
    zframe_t* f = pop_le(msg, 1024);
    jsmntok_t t[30], *tokens = t;
    jsmn_parser p;
    jsmn_init(&p);
    r = jsmn_parse(&p, (char*)zframe_data(f), zframe_size(f), t, 30);
    if (r >= 11) {
        // clang-format off
        count = parse_tokens(
            f,
            r,
            &tokens,
            5,
            "who",   &alert->who,
            "what",  &alert->what,
            "siteId",&alert->where,
            "when",  &alert->when,
            "mesg",  &alert->mesg);
        // clang-format on
        if (!(count == 5)) zframe_destroy(&f);
    } else {
        zframe_destroy(&f);
    }
    return f;
}

static zframe_t*
pop_email(zmsg_t* msg, linq_email* emails)
{
    memset(emails, 0, sizeof(linq_email));
    int r, count;
    zframe_t* f = pop_le(msg, 1024);
    jsmntok_t t[30], *tokens = t;
    jsmn_parser p;
    jsmn_init(&p);
    r = jsmn_parse(&p, (char*)zframe_data(f), zframe_size(f), t, 30);
    if (r >= 11) {
        // clang-format off
        count = parse_tokens(
            f,
            r,
            &tokens,
            5,
            "to0", &emails->to0,
            "to1", &emails->to1,
            "to2", &emails->to2,
            "to3", &emails->to3,
            "to4", &emails->to4);
        // clang-format on
        if (!(count == 5)) zframe_destroy(&f);
    } else {
        zframe_destroy(&f);
    }
    return f;
}

// when we detect an error call the error callback
static void
on_error(linq* l, e_linq_error e, const char* serial)
{
    if (l->callbacks && l->callbacks->err) {
        l->callbacks->err(l->context, e, "", serial);
    }
}

// when we receive a heartbeat call the heartbeat callback
static void
on_heartbeat(linq* l, device** d)
{
    if (l->callbacks && l->callbacks->hb) {
        l->callbacks->hb(l->context, device_serial(*d), d);
    }
}

// When we receive an alert, call the alert callback
static void
on_alert(linq* l, device** d, linq_alert* alert, linq_email* email)
{
    if (l->callbacks && l->callbacks->alert) {
        l->callbacks->alert(l->context, alert, email, d);
    }
}

static int
print_null_terminated(char* c, uint32_t sz, zframe_t* f)
{
    uint32_t fsz = zframe_size(f);
    if (fsz + 1 <= sz) {
        memcpy(c, zframe_data(f), fsz);
        c[fsz] = 0;
        return 0;
    } else {
        return -1;
    }
}

// find a device in our device map and update the router id. insert device if hb
static device**
device_resolve(linq* l, device_map* devices, zframe_t** frames, bool hb)
{
    uint32_t rid_sz = zframe_size(frames[FRAME_RID_IDX]);
    uint8_t* rid = zframe_data(frames[FRAME_RID_IDX]);
    char sid[SID_LEN], pid[PID_LEN];
    print_null_terminated(sid, SID_LEN, frames[FRAME_SID_IDX]);
    print_null_terminated(pid, PID_LEN, frames[FRAME_HB_PID_IDX]);
    device** d = device_map_get(devices, sid);
    if (d) {
        device_heartbeat(*d);
        device_update_router(*d, rid, rid_sz);
    } else {
        if (hb && frames[FRAME_HB_PID_IDX]) {
            d = device_map_insert(l->devices, &l->sock, rid, rid_sz, sid, pid);
        }
    }
    return d;
}

// check the zmq request frames are valid and process the request
static e_linq_error
process_request(linq* l, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    ((void)l);
    ((void)frames);
    return e;
}

// check the zmq response frames are valid and process the response
static e_linq_error
process_response(linq* l, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    ((void)l);
    ((void)frames);
    return e;
}

// check the zmq alert frames are valid and process the alert
static e_linq_error
process_alert(linq* l, zmsg_t** msg, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    linq_alert alert;
    linq_email email;
    if (zmsg_size(*msg) == 3 &&
        (frames[FRAME_ALERT_PID_IDX] = pop_le(*msg, PID_LEN)) &&
        (frames[FRAME_ALERT_DAT_IDX] = pop_alert(*msg, &alert)) &&
        (frames[FRAME_ALERT_DST_IDX] = pop_email(*msg, &email))) {
        device** d = device_resolve(l, l->devices, frames, false);
        if (d) {
            on_alert(l, d, &alert, &email);
            e = e_linq_ok;
        }
    }

    return e;
}

// check the zmq heartbeat frames are valid and process the heartbeat
static e_linq_error
process_heartbeat(linq* l, zmsg_t** msg, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    if (zmsg_size(*msg) == 2 &&
        (frames[FRAME_HB_PID_IDX] = pop_le(*msg, PID_LEN)) &&
        (frames[FRAME_HB_SITE_IDX] = pop_le(*msg, SITE_LEN))) {
        device** d = device_resolve(l, l->devices, frames, true);
        if (d) {
            on_heartbeat(l, d);
            e = e_linq_ok;
        }
    }
    return e;
}

// check the zmq header frames are valid and process the packet
static e_linq_error
process_packet(linq* l, zmsg_t** msg, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    *msg = zmsg_recv(l->sock);
    if (*msg && zmsg_size(*msg) >= 4 &&
        (frames[FRAME_RID_IDX] = pop_le(*msg, RID_LEN)) &&
        (frames[FRAME_VER_IDX] = pop_eq(*msg, 1)) &&
        (frames[FRAME_TYP_IDX] = pop_eq(*msg, 1)) &&
        (frames[FRAME_SID_IDX] = pop_le(*msg, SID_LEN))) {
        switch ((type)zframe_data(frames[FRAME_TYP_IDX])[0]) {
            case heartbeat: e = process_heartbeat(l, msg, frames); break;
            case request: break;
            case response: break;
            case alert: e = process_alert(l, msg, frames); break;
        }
    }
    return e;
}

// read zmq messages
static e_linq_error
process_incoming(linq* l)
{
    int n = 0;
    zframe_t* frames[FRAME_MAX + 1];
    memset(frames, 0, sizeof(frames));
    zmsg_t* msg;
    e_linq_error e = process_packet(l, &msg, frames);
    if (e) on_error(l, e, "");
    zmsg_destroy(&msg);
    while (frames[n]) zframe_destroy(&frames[n++]);
    return e;
}

// Create main context for the caller
linq*
linq_create(linq_callbacks* cb, void* context)
{
    linq* l = linq_malloc(sizeof(linq));
    if (l) {
        memset(l, 0, sizeof(linq));
        l->devices = device_map_create();
        l->callbacks = cb;
        l->context = context;
    }
    return l;
}

// Free main context after use
void
linq_destroy(linq** linq_p)
{
    linq* l = *linq_p;
    *linq_p = NULL;
    device_map_destroy(&l->devices);
    if (l->sock) zsock_destroy(&l->sock);
    linq_free(l);
}

// Listen for incoming device connections on "endpoint"
e_linq_error
linq_listen(linq* l, const char* ep)
{
    if (l->sock) return e_linq_bad_args;
    l->sock = zsock_new_router(ep);
    return l->sock ? e_linq_ok : e_linq_bad_args;
}

// poll network socket file handles
e_linq_error
linq_poll(linq* l)
{
    int err;
    zmq_pollitem_t item = { zsock_resolve(l->sock), 0, ZMQ_POLLIN, 0 };
    err = zmq_poll(&item, 1, 5);
    if (err < 0) return err;
    if (item.revents && ZMQ_POLLIN) err = process_incoming(l);
    return err;
}

// get a device from the device map
device**
linq_device(linq* l, const char* serial)
{
    return device_map_get(l->devices, serial);
}

// return how many devices are connected to linq
uint32_t
linq_device_count(linq* l)
{
    return device_map_size(l->devices);
}
