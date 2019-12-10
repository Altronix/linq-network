#include "zmtp.h"
#include "device.h"

static device_s**
device_get(const zmtp_s* zmtp, const char* serial)
{
    return device_map_get(*zmtp->devices_p, serial);
}

void
zmtp_init(
    zmtp_s* zmtp,
    device_map_s** devices_p,
    node_map_s** nodes_p,
    const zmtp_callbacks* callbacks,
    void* context)
{
    zmtp->devices_p = devices_p;
    zmtp->nodes_p = nodes_p;
    zmtp->callbacks = callbacks;
    zmtp->context = context;
    zmtp->shutdown = false;
    zmtp->routers = socket_map_create();
    zmtp->dealers = socket_map_create();
}

void
zmtp_deinit(zmtp_s* zmtp)
{
    socket_map_destroy(&zmtp->routers);
    socket_map_destroy(&zmtp->dealers);
}

linq_netw_socket
zmtp_listen(zmtp_s* zmtp, const char* ep)
{}

linq_netw_socket
zmtp_connect(zmtp_s* l, const char* ep)
{}

E_LINQ_ERROR
zmtp_close_router(zmtp_s* zmtp, linq_netw_socket sock) {}

E_LINQ_ERROR
zmtp_close_dealer(zmtp_s* zmtp, linq_netw_socket sock) {}

static void
send_error(linq_netw_request_complete_fn fn, void* context, E_LINQ_ERROR e)
{
    char err[32];
    if (fn) {
        snprintf(err, sizeof(err), "{\"error\":%d}", e);
        fn(context, e, err, NULL);
    }
}

E_LINQ_ERROR
zmtp_device_send_get(
    const zmtp_s* zmtp,
    const char* serial,
    const char* path,
    linq_netw_request_complete_fn fn,
    void* context)
{
    device_s** d = device_get(zmtp, serial);
    if (!d) {
        send_error(fn, context, LINQ_ERROR_DEVICE_NOT_FOUND);
        return LINQ_ERROR_DEVICE_NOT_FOUND;
    } else {
        device_send_get(*d, path, fn, context);
        return LINQ_ERROR_OK;
    }
}

E_LINQ_ERROR
zmtp_device_send_post(
    const zmtp_s* zmtp,
    const char* serial,
    const char* path,
    const char* json,
    linq_netw_request_complete_fn fn,
    void* context)
{
    device_s** d = device_get(zmtp, serial);
    if (!d) {
        send_error(fn, context, LINQ_ERROR_DEVICE_NOT_FOUND);
        return LINQ_ERROR_DEVICE_NOT_FOUND;
    } else {
        device_send_post(*d, path, json, fn, context);
        return LINQ_ERROR_OK;
    }
}

E_LINQ_ERROR
zmtp_device_send_delete(
    const zmtp_s* zmtp,
    const char* serial,
    const char* path,
    linq_netw_request_complete_fn fn,
    void* context)
{
    device_s** d = device_get(zmtp, serial);
    if (!d) {
        send_error(fn, context, LINQ_ERROR_DEVICE_NOT_FOUND);
        return LINQ_ERROR_DEVICE_NOT_FOUND;
    } else {
        device_send_delete(*d, path, fn, context);
        return LINQ_ERROR_OK;
    }
}
