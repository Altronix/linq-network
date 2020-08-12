#include "daemon.h"
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
    "\"when\":%.*s,"                                                           \
    "\"mesg\":\"%.*s\""                                                        \
    "}}"

static void
on_heartbeat_response(
    void* ctx,
    const char* serial,
    E_LINQ_ERROR e,
    const char* r)
{
    daemon_s* l = ctx;
    if (e) {
        log_warn("(LINQ) [%.6s...] (%.3d) About request failed!", serial, e);
    } else {
        database_device_insert_json(&l->http.db, serial, r, strlen(r));
    }
}

static void
on_hb(void* ctx, const char* s)
{
    // Note - All tests load devices into context by pushing in heartbeats.
    // Therefore tests should also flush out the response, or mock database
    // query to make tests think device doesn't need to be added into database
    // so there will be no request and response to flush through
    daemon_s* l = ctx;
    http_broadcast_json(&l->http, 200, WEBSOCKET_HEARTBEAT_FMT, strlen(s), s);
    if (!database_row_exists_str(&l->http.db, "devices", "device_id", s)) {
        log_info(
            "(LINQ) [%.6s...] "
            "New device connected, requesting about data...",
            s);
        linq_network_send(
            l->netw,
            s,
            "GET",
            "/ATX/about",
            10,
            NULL,
            0,
            on_heartbeat_response,
            l);
    }
}

static void
on_alert(
    void* ctx,
    const char* serial,
    linq_network_alert_s* a,
    linq_network_email_s* email)
{
    int err;
    daemon_s* l = ctx;
    log_info("(LINQ) [%.6s...] Event Alert", serial);
    char when[32];
    alert_insert_s alert = { .who = { .p = a->who.p, .len = a->who.len },
                             .what = { .p = a->what.p, .len = a->what.len },
                             .site = { .p = a->where.p, .len = a->where.len },
                             .time = { .p = a->when.p, .len = a->when.len },
                             .mesg = { .p = a->mesg.p, .len = a->mesg.len } };
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
    err = database_alert_insert(&l->http.db, serial, &alert);
}

static void
on_ctrlc(void* ctx)
{
    daemon_s* l = ctx;
    log_info("(LINQ) Received shutdown signal...");
    l->shutdown = true;
}

static void
on_error(void* context, E_LINQ_ERROR e, const char* sid, const char* what)
{
    log_error("(LINQ) Event Error [%d]", e);
}

static linq_network_callbacks callbacks = { .on_heartbeat = on_hb,
                                            .on_alert = on_alert,
                                            .on_err = on_error,
                                            .on_ctrlc = on_ctrlc };

void
daemon_init(daemon_s* linqd, daemon_config_s* config)
{
    char endpoint[64];
    memset(linqd, 0, sizeof(daemon_s));

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
daemon_free(daemon_s* linqd)
{
    linq_network_destroy(&linqd->netw);
    if (linqd->http.routes) http_deinit(&linqd->http);
    memset(linqd, 0, sizeof(daemon_s));
}

int
daemon_poll(daemon_s* linqd, uint32_t ms)
{
    int err = linq_network_poll(linqd->netw, ms);
    if (!err) err = http_poll(&linqd->http, ms);
    return err;
}
