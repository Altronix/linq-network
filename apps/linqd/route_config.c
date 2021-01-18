#include "route_config.h"
#include "config.h"
#include "libcommon/config.h"
#include "log.h"
#include "sys.h"

#define app_info(...) log_info("LINQD", __VA_ARGS__)
#define app_warn(...) log_warn("LINQD", __VA_ARGS__)
#define app_debug(...) log_debug("LINQD", __VA_ARGS__)
#define app_trace(...) log_trace("LINQD", __VA_ARGS__)
#define app_error(...) log_error("LINQD", __VA_ARGS__)
#define app_fatal(...) log_fatal("LINQD", __VA_ARGS__)

static void
get(http_request_s* r, uint32_t l, const char* b)
{
    sys_file* f = NULL;
    char buffer[1024];
    uint32_t sz = sizeof(buffer);
    const char* dir = sys_config_dir("linqd");
    if ((dir = sys_config_dir("linqd")) &&
        (f = sys_open(dir, FILE_MODE_READ, FILE_BLOCKING))) {
        sz = sys_read_buffer(f, buffer, &sz);
        buffer[sz] = 0;
        http_printf_json(r->connection, 200, buffer);
        sys_close(&f);
    } else {
        app_error("req 404");
        const char* err404 = "{\"error\":\"Not found\"}";
        http_printf_json(r->connection, 404, err404);
    }
}

static void
put(http_request_s* r, uint32_t l, const char* b)
{
    static const char* errfmt = "{\"error\":\"%s\"}";
    int err, i;
    FILE* f = NULL;
    const char *e = "{\"error\":\"ok\"}", *dir = NULL;
    config_s c;
    err = config_parse(b, l, &c);
    if (err == 0 && (dir = sys_config_dir("linqd")) &&
        (f = sys_open(dir, FILE_MODE_READ_WRITE_CREATE, FILE_BLOCKING))) {
        err = config_fprint(f, &c);
        sys_close(&f);
        if (!(err < 0)) {
            http_printf_json(r->connection, 200, e);
        } else {
            e = strerror(errno);
            app_error("file io error [%s]", r);
            http_printf_json(r->connection, 500, errfmt, r);
        }
    } else {
        e = strerror(errno);
        app_error("file io error [%s]", r);
        http_printf_json(r->connection, 500, errfmt, r);
    }
}

void
route_config(http_request_s* ctx, HTTP_METHOD meth, uint32_t l, const char* b)
{
    switch (meth) {
        case HTTP_METHOD_GET: get(ctx, l, b); break;
        case HTTP_METHOD_PUT:
        case HTTP_METHOD_POST: put(ctx, l, b); break;
        case HTTP_METHOD_DELETE: break;
    }
}
