#include "callbacks.h"
#include "log.h"

static void
on_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    app_error("Received Error");
    app_error("Code   : [%d]", e);
    app_error("What   : [%s]", what ? what : netw_strerror(e));
    app_error("Serial : [%s]", serial ? serial : "unknown");
}

static void
on_new(void* ctx, const char* sid)
{
    app_info("Received New Device [%s]", sid);
}

static void
on_heartbeat(void* ctx, const char* s)
{
    app_info("Received Heartbeat [%s]", s);
}

static void
on_alert(void* ctx, const char* serial, netw_alert_s* a, netw_email_s* email)
{
    app_info("Received Alert [%.6s...]", serial);
    app_info("WHO   : [%.*s]", a->who.len, a->who.p);
    app_info("WHAT  : [%.*s]", a->what.len, a->what.p);
    app_info("WHERE : [%.*s]", a->where.len, a->where.p);
    app_info("WHEN  : [%.*s]", a->when.len, a->when.p);
    app_info("JSON  : %s", a->data);
    // ...
}

static void
on_ctrlc(void* ctx)
{
    app_info("Received Shutdown");
}

netw_callbacks callbacks = { .on_err = on_error,
                             .on_new = on_new,
                             .on_heartbeat = on_heartbeat,
                             .on_alert = on_alert,
                             .on_ctrlc = on_ctrlc };
