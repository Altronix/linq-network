#include "callbacks.h"
#include "common/log.h"
#include "netw.h"
#include "netw_internal.h"

#define zmtp_info(...) log_info("ZMTP", __VA_ARGS__)
#define zmtp_warn(...) log_warn("ZMTP", __VA_ARGS__)
#define zmtp_debug(...) log_debug("ZMTP", __VA_ARGS__)
#define zmtp_trace(...) log_trace("ZMTP", __VA_ARGS__)
#define zmtp_error(...) log_error("ZMTP", __VA_ARGS__)
#define zmtp_fatal(...) log_fatal("ZMTP", __VA_ARGS__)

#define WEBSOCKET_NEW_FMT                                                      \
    "{"                                                                        \
    "\"type\":\"new\","                                                        \
    "\"data\":{"                                                               \
    "\"sid\":\"%.*s\","                                                        \
    "\"product\":\"%.*s\","                                                    \
    "\"prjVersion\":\"%.*s\","                                                 \
    "\"atxVersion\":\"%.*s\""                                                  \
    "}}"

#define WEBSOCKET_HEARTBEAT_FMT                                                \
    "{"                                                                        \
    "\"type\":\"heartbeat\","                                                  \
    "\"data\":{"                                                               \
    "\"sid\":\"%.*s\""                                                         \
    "}}"

#define WEBSOCKET_ALERT_FMT                                                    \
    "{"                                                                        \
    "\"type\":\"alert\","                                                      \
    "\"data\":{"                                                               \
    "\"sid\":\"%.*s\","                                                        \
    "\"who\":\"%.*s\","                                                        \
    "\"what\":\"%.*s\","                                                       \
    "\"siteId\":\"%.*s\","                                                     \
    "\"when\":%.*s,"                                                           \
    "\"mesg\":\"%.*s\""                                                        \
    "}}"

#ifdef BUILD_LINQD
static void
on_zmtp_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    zmtp_error("Event Error [%d]", e);
    netw_s* l = ctx;
    if (l->callbacks && l->callbacks->on_err) {
        l->callbacks->on_err(l->context, e, what, serial);
    }
}

static void
handle_about_response(
    void* ctx,
    const char* serial,
    E_LINQ_ERROR e,
    const char* r)
{
    zmtp_trace("[%.6s...] About data received", r);
    netw_s* l = ctx;
    if (e) {
        zmtp_warn("[%.6s...] (%.3d) About request failed!", serial, e);
    } else {
        database_device_insert_json(&l->database, serial, r, strlen(r));
    }
}

static void
on_zmtp_new(void* ctx, const char* sid)
{
    netw_s* netw = ctx;
    zmtp_info("[%.6s...] Event New Device", sid);
    if (netw->callbacks && netw->callbacks->on_new) {
        netw->callbacks->on_new(netw->context, sid);
    }
    netw_send(
        netw,
        sid,
        "GET",
        "/ATX/about",
        10,
        NULL,
        0,
        handle_about_response,
        netw);
}

static void
on_zmtp_heartbeat(void* ctx, const char* s)
{
    netw_s* l = ctx;
    zmtp_info("[%.6s...] Event Heartbeat", s);
    if (l->callbacks && l->callbacks->on_heartbeat) {
        l->callbacks->on_heartbeat(l->context, s);
    }
    http_broadcast_json(&l->http, 200, WEBSOCKET_HEARTBEAT_FMT, strlen(s), s);
}

static void
on_zmtp_alert(
    void* ctx,
    const char* serial,
    netw_alert_s* a,
    netw_email_s* email)
{
    netw_s* l = ctx;
    int err;
    zmtp_info("[%.6s...] Event Alert", serial);
    char when[32];
    alert_insert_s alert = { .who = { .p = a->who.p, .len = a->who.len },
                             .what = { .p = a->what.p, .len = a->what.len },
                             .site = { .p = a->where.p, .len = a->where.len },
                             .time = { .p = a->when.p, .len = a->when.len },
                             .mesg = { .p = a->mesg.p, .len = a->mesg.len } };

    zmtp_info("[%.6s...] Event Alert", serial);
    if (l->callbacks && l->callbacks->on_alert) {
        l->callbacks->on_alert(l->context, serial, a, email);
    }
    // clang-format off
    http_broadcast_json(
        &l->http,
        200,
        WEBSOCKET_ALERT_FMT,
        strlen(serial),      serial,
        a->who.len,     a->who.p,
        a->what.len,    a->what.p,
        a->where.len,   a->where.p,
        a->when.len,   a->when.p,
        a->mesg.len,    a->mesg.p);
    // clang-format on
    err = database_alert_insert(&l->database, serial, &alert);
}

static void
on_zmtp_ctrlc(void* ctx)
{
    zmtp_info("Received shutdown signal...");
    netw_s* l = ctx;
    if (l->callbacks && l->callbacks->on_ctrlc) {
        l->callbacks->on_ctrlc(l->context);
    }
}
#else  /* BUILD_LINQD */
static void
on_zmtp_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    zmtp_error("Event Error [%d]", e);
    netw_s* l = ctx;
    if (l->callbacks && l->callbacks->on_err) {
        l->callbacks->on_err(l->context, e, what, serial);
    }
}

static void
on_zmtp_new(void* ctx, const char* sid)
{
    netw_s* l = ctx;
    zmtp_info("[%.6s...] Event New Device", sid);
    if (l->callbacks && l->callbacks->on_new) {
        l->callbacks->on_new(l->context, sid);
    }
}

static void
on_zmtp_heartbeat(void* ctx, const char* sid)
{
    netw_s* l = ctx;
    zmtp_info("[%.6s...] Event Heartbeat", sid);
    if (l->callbacks && l->callbacks->on_heartbeat) {
        l->callbacks->on_heartbeat(l->context, sid);
    }
}

static void
on_zmtp_alert(
    void* ctx,
    const char* serial,
    netw_alert_s* a,
    netw_email_s* email)
{
    netw_s* l = ctx;
    zmtp_info("[%.6s...] Event Alert", serial);
    if (l->callbacks && l->callbacks->on_alert) {
        l->callbacks->on_alert(l->context, serial, a, email);
    }
}

static void
on_zmtp_ctrlc(void* ctx)
{
    zmtp_info("Received shutdown signal...");
    netw_s* l = ctx;
    if (l->callbacks && l->callbacks->on_ctrlc) {
        l->callbacks->on_ctrlc(l->context);
    }
}
#endif /* IF BUILD_LINQD ...ELSE */

netw_callbacks zmtp_callbacks = {
    .on_err = on_zmtp_error,
    .on_new = on_zmtp_new,
    .on_heartbeat = on_zmtp_heartbeat,
    .on_alert = on_zmtp_alert,
    .on_ctrlc = on_zmtp_ctrlc,
};

void
zmtp_callbacks_init()
{
    // https://developercommunity.visualstudio.com/content/problem/76456/optimizer-deletes-custom-static-initializer.html
    // developercommunity "problem" 76456 as "Won't fix" because VS removes
    // "Unused Variables".
    zmtp_callbacks.on_err = on_zmtp_error;
    zmtp_callbacks.on_new = on_zmtp_new;
    zmtp_callbacks.on_heartbeat = on_zmtp_heartbeat;
    zmtp_callbacks.on_alert = on_zmtp_alert;
    zmtp_callbacks.on_ctrlc = on_zmtp_ctrlc;
}
