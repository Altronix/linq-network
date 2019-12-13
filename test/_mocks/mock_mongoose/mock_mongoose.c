#include "mock_mongoose.h"
#include "base64.h"
#include "containers.h"
#include "mongoose.h"

#define HEADERS                                                                \
    "GET %s HTTP/1.1\r\n"                                                      \
    "Host: 0.0.0.0:33000\r\n"                                                  \
    "Authorization: Basic %s\r\n"                                              \
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
    "Authorization: Basic %s\r\n"                                              \
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

// State of mock event
typedef struct mock_mongoose_event
{
    struct mg_connection* c;
    int ev;
    void* p;
    struct http_message message;
    char request[2048];
} mock_mongoose_event;
static void

// Free mock event
mock_mongoose_event_destroy(mock_mongoose_event** ev_p)
{
    mock_mongoose_event* event = *ev_p;
    *ev_p = NULL;
    linq_netw_free(event);
}

// Linked list of mock events
LIST_INIT(event, mock_mongoose_event, mock_mongoose_event_destroy);

static event_list_s* incoming_events = NULL;

// Call before your test
void
mongoose_spy_init()
{
    incoming_events = event_list_create();
}

// Call after your test
void
mongoose_spy_deinit()
{
    event_list_destroy(&incoming_events);
}

static mock_mongoose_event*
event_copy(mock_mongoose_event* src, int ev)
{
    mock_mongoose_event* dst = linq_netw_malloc(sizeof(mock_mongoose_event));
    linq_netw_assert(dst);
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
    uint8_t authb64[96];
    size_t len = sizeof(authb64);
    b64_encode(authb64, &len, (uint8_t*)auth, strlen((char*)auth));

    // Init the event context
    mock_mongoose_event* req = linq_netw_malloc(sizeof(mock_mongoose_event));
    linq_netw_assert(req);
    memset(req, 0, sizeof(mock_mongoose_event));

    // Populate the request
    switch (meth[0]) {
        case 'G':
        case 'D':
            snprintf(req->request, sizeof(req->request), HEADERS, path, auth);
            break;
        case 'P':
            snprintf(
                req->request,
                sizeof(req->request),
                HEADERS_DATA,
                path,
                auth,
                strlen(data),
                data);
            break;
    }
    req->ev = MG_EV_HTTP_REQUEST;
    mock_mongoose_event* accept = event_copy(req, MG_EV_ACCEPT);
    mock_mongoose_event* recv = event_copy(req, MG_EV_ACCEPT);
    mock_mongoose_event* chunk = event_copy(req, MG_EV_ACCEPT);
    event_list_push(incoming_events, &accept);
    event_list_push(incoming_events, &recv);
    event_list_push(incoming_events, &chunk);
    event_list_push(incoming_events, &req);
}

void
mongoose_spy_event_close_push(int handle)
{
    ((void)handle);
    mock_mongoose_event* event = linq_netw_malloc(sizeof(mock_mongoose_event));
    linq_netw_assert(event);
    memset(event, 0, sizeof(mock_mongoose_event));
    event->ev = MG_EV_CLOSE;
    event_list_push(incoming_events, &event);
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

struct mg_connection*
__wrap_mg_bind(
    struct mg_mgr* srv,
    const char* address,
    MG_CB(mg_event_handler_t event_handler, void* user_data))
{
    ((void)srv);
    ((void)address);
    ((void)event_handler);
    return NULL;
}

int
__wrap_mg_mgr_poll(struct mg_mgr* m, int timeout_ms)
{
    ((void)m);
    ((void)timeout_ms);
    return -1;
}

int
__wrap_mg_printf(struct mg_connection* c, const char* fmt, ...)
{
    ((void)c);
    ((void)fmt);
    return -1;
}

int
__wrap_mg_vprintf(struct mg_connection* c, const char* fmt, va_list list)
{
    ((void)c);
    ((void)fmt);
    ((void)list);
    return 0;
}

void
__wrap_mg_printf_http_chunk(struct mg_connection* nc, const char* fmt, ...)
{
    ((void)nc);
    ((void)fmt);
}

void
__wrap_mg_printf_html_escape(struct mg_connection* nc, const char* fmt, ...)
{
    ((void)nc);
    ((void)fmt);
}
