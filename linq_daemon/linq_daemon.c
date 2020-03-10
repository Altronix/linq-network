#include "linq_daemon.h"
#include "database.h"
#include "device.h"
#include "http.h"
#include "linq_network.h"
#include "log.h"

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
    "\"when\":%d,"                                                             \
    "\"mesg\":\"%.*s\""                                                        \
    "}}"

static void
on_heartbeat_response(void* ctx, E_LINQ_ERROR e, const char* r, device_s** d)
{
    linqd_s* l = ctx;
    const char* s = device_serial(*d);
    if (e) {
        log_warn("(LINQ) [%.6s...] (%.3d) About request failed!", s, e);
    } else {
        database_insert_device_from_json(&l->http.db, s, r, strlen(r));
    }
}

static void
on_hb(void* ctx, const char* s, device_s** d)
{
    // Note - All tests load devices into context by pushing in heartbeats.
    // Therefore tests should also flush out the response, or mock database
    // query to make tests think device doesn't need to be added into database
    // so there will be no request and response to flush through
    linqd_s* l = ctx;
    http_broadcast_json(&l->http, 200, WEBSOCKET_HEARTBEAT_FMT, strlen(s), s);
    if (!database_row_exists_str(&l->http.db, "devices", "device_id", s)) {
        log_info(
            "(LINQ) [%.6s...] "
            "New device connected, requesting about data...",
            s);
        device_send_get(*d, "/ATX/about", on_heartbeat_response, l);
    }
}

static void
on_alert(
    void* ctx,
    linq_network_alert_s* a,
    linq_network_email_s* email,
    device_s** d)
{
    int err;
    const char* s = device_serial(*d);
    linqd_s* l = ctx;
    log_info("(LINQ) [%.6s...] Event Alert", s);
    char when[32];
    jsmn_value alert[5] = {
        { .p = a->who.p, .len = a->who.len },
        { .p = a->what.p, .len = a->what.len },
        { .p = a->where.p, .len = a->where.len },
        { .p = a->when.p, .len = a->when.len },
        { .p = a->mesg.p, .len = a->mesg.len },
    };
    snprintf(when, sizeof(when), "%.*s", a->when.len, a->when.p);

    // clang-format off
    http_broadcast_json(
        &l->http,
        200,
        WEBSOCKET_ALERT_FMT,
        strlen(s),      s,
        a->who.len,     a->who.p,
        a->what.len,    a->what.p,
        a->where.len,   a->where.p,
        atoi(when),
        a->mesg.len,    a->mesg.p);
    // clang-format on
    err = database_insert_alert(&l->http.db, s, alert);
}

static void
on_ctrlc(void* ctx)
{
    linqd_s* l = ctx;
    log_info("(LINQ) Received shutdown signal...");
    l->shutdown = true;
}

static void
on_error(void* context, E_LINQ_ERROR e, const char* sid, const char* what)
{
    log_error("(LINQ) Event Error [%d]", e);
}

static linq_network_callbacks callbacks = { .hb = on_hb,
                                            .alert = on_alert,
                                            .err = on_error,
                                            .ctrlc = on_ctrlc };

void
linqd_init(linqd_s* linqd, linqd_config_s* config)
{
    char endpoint[64];
    memset(linqd, 0, sizeof(linqd_s));

    // Create network context
    linqd->netw = linq_network_create(&callbacks, linqd);
    linq_network_assert(linqd->netw);

    // Open zmtp port
    if (config->zmtp) {
        snprintf(endpoint, sizeof(endpoint), "tcp://*:%d", config->zmtp);
        linq_network_listen(linqd->netw, endpoint);
    }

    // Create HTTP server
    http_init(&linqd->http, linqd->netw);

    // Open HTTP port
    if (config->http) {
        snprintf(endpoint, sizeof(endpoint), "%d", config->http);
        http_listen(&linqd->http, endpoint);
    }

    // Open HTTPS port
    if (config->https && config->cert && config->key) {
        snprintf(endpoint, sizeof(endpoint), "%d", config->https);
        http_listen_tls(&linqd->http, endpoint, config->cert, config->key);
    }
}

void
linqd_free(linqd_s* linqd)
{
    linq_network_destroy(&linqd->netw);
    if (linqd->http.routes) http_deinit(&linqd->http);
    memset(linqd, 0, sizeof(linqd_s));
}

int
linqd_poll(linqd_s* linqd, uint32_t ms)
{
    int err = linq_network_poll(linqd->netw, ms);
    if (!err) err = http_poll(&linqd->http, ms);
    return err;
}
