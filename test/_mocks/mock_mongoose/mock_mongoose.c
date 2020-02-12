// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mock_mongoose.h"
#include "base64.h"
#include "containers.h"
#include "http_parser.h"
#include "parse_http.h"

#include "cmocka.h"

#define HEADERS                                                                \
    "GET %s HTTP/1.1\r\n"                                                      \
    "Host: 0.0.0.0:33000\r\n"                                                  \
    "Authorization: Bearer %s\r\n"                                             \
    "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) "             \
    "Gecko/2008061015 Firefox/3.0\r\n"                                         \
    "Accept: "                                                                 \
    "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"      \
    "Accept-Language: en-us,en;q=0.5\r\n"                                      \
    "Accept-Encoding: gzip,deflate\r\n"                                        \
    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"                       \
    "Keep-Alive: 300\r\n"                                                      \
    "Connection: keep-alive\r\n"                                               \
    "\r\n"

#define HEADERS_DATA                                                           \
    "POST %s HTTP/1.1\r\n"                                                     \
    "Host: 0.0.0.0:33000\r\n"                                                  \
    "Authorization: Bearer %s\r\n"                                             \
    "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) "             \
    "Gecko/2008061015 Firefox/3.0\r\n"                                         \
    "Accept: "                                                                 \
    "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"      \
    "Accept-Language: en-us,en;q=0.5\r\n"                                      \
    "Accept-Encoding: gzip,deflate\r\n"                                        \
    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"                       \
    "Keep-Alive: 300\r\n"                                                      \
    "Content-Length: %ld\r\n"                                                  \
    "Connection: keep-alive\r\n"                                               \
    "\r\n"                                                                     \
    "%s"

// Free mock event
static void
mock_mongoose_event_destroy(mock_mongoose_event** ev_p)
{
    mock_mongoose_event* event = *ev_p;
    *ev_p = NULL;
    linq_network_free(event);
}

// Linked list of mock events
LIST_INIT(event, mock_mongoose_event, mock_mongoose_event_destroy);

// Free outgoing event
void
mock_mongoose_outgoing_data_destroy(mock_mongoose_outgoing_data** data_p)
{
    mock_mongoose_outgoing_data* data = *data_p;
    *data_p = NULL;
    linq_network_free(data);
}
// Linked list of outgoing data
LIST_INIT(
    outgoing_data,
    mock_mongoose_outgoing_data,
    mock_mongoose_outgoing_data_destroy);

// TODO eventually we want to map events and responses to a handle
// Right now this only supports testing single connection
static event_list_s* incoming_events = NULL;
static outgoing_data_list_s* outgoing_data = NULL;

// Callers event handler
typedef void(ev_handler)(struct mg_connection* nc, int, void*, void*);
static ev_handler* test_event_handler = NULL;
static void* test_user_data = NULL;

// Call before your test
void
mongoose_spy_init()
{
    incoming_events = event_list_create();
    outgoing_data = outgoing_data_list_create();
}

// Call after your test
void
mongoose_spy_deinit()
{
    if (incoming_events) event_list_destroy(&incoming_events);
    if (outgoing_data) outgoing_data_list_destroy(&outgoing_data);
}
void
mongoose_spy_incoming_events_flush()
{
    mock_mongoose_event* ev = NULL;
    while ((ev = event_list_pop(incoming_events))) {
        mock_mongoose_event_destroy(&ev);
    }
}

void
mongoose_spy_outgoing_data_flush()
{
    mock_mongoose_outgoing_data* data = NULL;
    while ((data = outgoing_data_list_pop(outgoing_data))) {
        mock_mongoose_outgoing_data_destroy(&data);
    }
}

void
mock_mongoose_response_destroy(mock_mongoose_response** resp_p)
{
    mock_mongoose_response* resp = *resp_p;
    *resp_p = NULL;
    linq_network_free(resp);
}

static mock_mongoose_event*
event_copy(mock_mongoose_event* src, int ev)
{
    mock_mongoose_event* dst = linq_network_malloc(sizeof(mock_mongoose_event));
    linq_network_assert(dst);
    memcpy(dst, src, sizeof(mock_mongoose_event));
    dst->ev = ev;
    return dst;
}

void
mongoose_spy_event_request_push(
    const char* auth,
    const char* meth,
    const char* path,
    const char* data)
{
    // Basic auth string
    size_t l;
    http_parser parser;

    // Init the event context
    mock_mongoose_event* r = linq_network_malloc(sizeof(mock_mongoose_event));
    linq_network_assert(r);
    memset(r, 0, sizeof(mock_mongoose_event));
    mongoose_parser_init(&parser, r, HTTP_REQUEST);

    // Populate the request
    switch (meth[0]) {
        case 'G':
        case 'D':
            l = snprintf(r->request, sizeof(r->request), HEADERS, path, auth);
            mongoose_parser_parse(&parser, r->request, l);
            break;
        case 'P':
            l = snprintf(
                r->request,
                sizeof(r->request),
                HEADERS_DATA,
                path,
                auth,
                strlen(data),
                data);
            mongoose_parser_parse(&parser, r->request, l);
            break;
    }
    r->ev = MG_EV_HTTP_REQUEST;
    mock_mongoose_event* accept = event_copy(r, MG_EV_ACCEPT);
    mock_mongoose_event* recv = event_copy(r, MG_EV_ACCEPT);
    mock_mongoose_event* chunk = event_copy(r, MG_EV_ACCEPT);
    event_list_push(incoming_events, &accept);
    event_list_push(incoming_events, &recv);
    event_list_push(incoming_events, &chunk);
    event_list_push(incoming_events, &r);
}

void
mongoose_spy_event_close_push(int handle)
{
    ((void)handle);
    mock_mongoose_event* event =
        linq_network_malloc(sizeof(mock_mongoose_event));
    linq_network_assert(event);
    memset(event, 0, sizeof(mock_mongoose_event));
    event->ev = MG_EV_CLOSE;
    event_list_push(incoming_events, &event);
}

mock_mongoose_outgoing_data*
mongoose_spy_outgoing_data_pop(int i)
{
    mock_mongoose_outgoing_data *n, *d = outgoing_data_list_pop(outgoing_data);
    if (i == 1) {
        return d;
    } else {
        uint32_t spot = d->l;
        i--;
        while (i--) {
            n = outgoing_data_list_pop(outgoing_data);
            linq_network_assert(spot + n->l < sizeof(d->mem));
            snprintf(&d->mem[spot], sizeof(d->mem) - spot, n->mem, n->l);
            spot += n->l;
            d->l += n->l;
            mock_mongoose_outgoing_data_destroy(&n);
        }
        return d;
    }
}

mongoose_parser_context*
mongoose_spy_response_pop()
{
    http_parser parser;

    // Allocate return
    mongoose_parser_context* ctx =
        linq_network_malloc(sizeof(mongoose_parser_context));
    linq_network_assert(ctx);

    // Prepare to parse outgoing data
    mongoose_parser_init(&parser, ctx, HTTP_RESPONSE);
    mock_mongoose_outgoing_data* d = mongoose_spy_outgoing_data_pop(2);
    linq_network_assert(d);
    mongoose_parser_parse(&parser, d->mem, d->l);
    mock_mongoose_outgoing_data_destroy(&d);
    return ctx;
}

void
__wrap_mg_mgr_init(struct mg_mgr* m, void* user_data)
{
    ((void)m);
    ((void)user_data);
}

void
__wrap_mg_mgr_free(struct mg_mgr* m)
{
    ((void)m);
}

void
__wrap_mg_set_protocol_http_websocket(struct mg_connection* nc)
{
    ((void)nc);
}

struct mg_connection*
__wrap_mg_bind(
    struct mg_mgr* srv,
    const char* address,
    mg_event_handler_t event_handler,
    void* user_data)
{
    ((void)srv);
    ((void)address);
    ((void)event_handler);
    test_event_handler = event_handler;
    test_user_data = user_data;
    return NULL;
}

int
__wrap_mg_mgr_poll(struct mg_mgr* m, int timeout_ms)
{
    ((void)m);
    ((void)timeout_ms);
    int count = 0;
    mock_mongoose_event* ev = event_list_pop(incoming_events);
    if (ev) {
        count = 1;
        (*test_event_handler)(NULL, ev->ev, &ev->message, test_user_data);
        mock_mongoose_event_destroy(&ev);
    }
    return count;
}

int
__wrap_mg_printf(struct mg_connection* c, const char* fmt, ...)
{
    ((void)c);
    va_list list;

    mock_mongoose_outgoing_data* d =
        linq_network_malloc(sizeof(mock_mongoose_outgoing_data));
    linq_network_assert(d);
    va_start(list, fmt);
    d->l = vsnprintf(d->mem, sizeof(d->mem), fmt, list);
    va_end(list);
    outgoing_data_list_push(outgoing_data, &d);
    return 0;
}

int
__wrap_mg_vprintf(struct mg_connection* c, const char* fmt, va_list list)
{
    ((void)c);
    mock_mongoose_outgoing_data* d =
        linq_network_malloc(sizeof(mock_mongoose_outgoing_data));
    linq_network_assert(d);
    d->l = vsnprintf(d->mem, sizeof(d->mem), fmt, list);
    outgoing_data_list_push(outgoing_data, &d);
    return 0;
}

void
__wrap_mg_send_websocket_frame(
    struct mg_connection* nc,
    int op,
    const void* data,
    size_t len)
{
    va_list list;

    mock_mongoose_outgoing_data* d =
        linq_network_malloc(sizeof(mock_mongoose_outgoing_data));
    linq_network_assert(d);
    linq_network_assert(len < sizeof(d->mem));
    snprintf(d->mem, sizeof(d->mem), "%.*s", (int)len, data);
    outgoing_data_list_push(outgoing_data, &d);
}

void
__wrap_mg_printf_http_chunk(struct mg_connection* nc, const char* fmt, ...)
{
    ((void)nc);
    ((void)fmt);
    assert_string_equal(NULL, "__wrap_mg_printf_chunk() not implemented");
}

void
__wrap_mg_printf_html_escape(struct mg_connection* nc, const char* fmt, ...)
{
    ((void)nc);
    ((void)fmt);
    assert_string_equal(NULL, "__wrap_mg_printf_html_escape() not implemented");
}

