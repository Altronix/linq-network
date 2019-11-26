#include "atxclient.h"
#include "helpers.h"

typedef struct
{
    atx_api_s api;
    atxclient__s* client;
    unet* netw;
    uint32_t port;
} fixture_context;

static int
cb_504(item_s* item, item_cb_s* cb)
{
    ((void)item);
    ((void)cb);
    static int count = 0;
    if (count++ < 3) {
        return 504;
    } else {
        count = 0;
        return 0;
    }
}

fixture_context*
fixture_create(const char* sid, uint32_t port)
{
    fixture_context* f = malloc(sizeof(fixture_context));
    if (!f) return f;

    f->port = port;

    // Create a network context
    f->netw = unet_create(1);
    if (!f->netw) {
        free(f);
        return NULL;
    }

    // Create the atxclient context
    f->client = atxclient_create(f->netw, &f->api, sid, "product");
    if (!f->client) {
        unet_destroy(&f->netw);
        free(f);
        return NULL;
    }

    // Create API root
    f->api.links = NULL;
    f->api.doc = item_create_root("ATX");
    if (!f->api.doc) {
        atxclient_destroy(&f->client);
        unet_destroy(&f->netw);
        free(f);
        return NULL;
    }

    // Create API (continued)
    if (!(item_create_set_str_path(
              f->api.doc, "hello", 5, "world", NULL, NULL) &&
          item_create_set_num_path(f->api.doc, "shutdown", 0, NULL, NULL) &&
          item_create_set_num_path(f->api.doc, "result", -1, NULL, NULL) &&
          item_create_set_num_path(f->api.doc, "alert", 0, NULL, NULL) &&
          item_create_set_num_path(f->api.doc, "test_504", 0, cb_504, NULL))) {
        item_destroy(&f->api.doc);
        atxclient_destroy(&f->client);
        unet_destroy(&f->netw);
        free(f);
        return NULL;
    }
    return f;
}

void
fixture_destroy(fixture_context** f_p)
{
    fixture_context* f = *f_p;
    *f_p = NULL;
    item_destroy(&f->api.doc);
    atxclient_destroy(&f->client);
    unet_destroy(&f->netw);
    free(f);
}
void
fixture_poll(fixture_context* f)
{
    static int hb_sent = 0, alert_sent = 0;
    int err;

    atxclient_poll(f->client);
    unet_poll(f->netw, 10);
    if (atxclient_connected(f->client)) {
        if (atxclient_ready(f->client)) {
            if (!hb_sent) {
                err = atxclient_heartbeat(f->client);
                if (!err) {
                    printf("[D] heartbeat sent\n");
                    hb_sent = 1;
                }
            }
            if (!alert_sent) {
                err = atxclient_alert(
                    f->client,
                    TEST_ALERT,
                    TEST_ALERT_LEN,
                    TEST_EMAIL,
                    TEST_EMAIL_LEN);
                if (!err) {
                    printf("[D] alert sent\n");
                    alert_sent = 1;
                }
            }
        }
    } else if (!(atxclient_connecting(f->client))) {
        // Connect to test fixture
        err = atxclient_connect(f->client, "127.0.0.1", f->port);
        if (err) {
            printf("[D] Connect error\n");
        } else {
            printf("[D] Connected\n");
        }
    }
}

