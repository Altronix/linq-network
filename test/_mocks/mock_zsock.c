#include "mock_zsock.h"

typedef struct mock_zsock_s
{
    char dummy[100];
} mock_zsock_s;

int
__wrap_zsock_bind(zsock_t* self, const char* format, ...)
{
    ((void)self);
    ((void)format);
    return 0;
}

int
__wrap_zsock_connect(zsock_t* self, const char* format, ...)
{
    ((void)self);
    ((void)format);
    return 0;
}

int
__wrap_zsock_send(void* self, const char* picture, ...)
{
    ((void)self);
    ((void)picture);
    return 0;
}

int
__wrap_zsock_recv(void* self, const char* picture, ...)
{
    ((void)self);
    ((void)picture);
    return 0;
}

void*
__wrap_zsock_resolve(void* self)
{
    return self;
}

zsock_t*
__wrap_zsock_new_router(const char* endpoints)
{
    ((void)endpoints);
    mock_zsock_s* s = malloc(sizeof(mock_zsock_s));
    if (s) {
        memset(s, 0, sizeof(mock_zsock_s));
        snprintf(s->dummy, sizeof(s->dummy), "%s", "ROUTER");
    }
    return (zsock_t*)s;
}

zsock_t*
__wrap_zsock_new_router_checked(
    const char* endpoints,
    const char* filename,
    size_t line_nbr)
{
    ((void)filename);
    ((void)line_nbr);
    return __wrap_zsock_new_router(endpoints);
}

zsock_t*
__wrap_zsock_new_dealer(const char* endpoints)
{
    ((void)endpoints);
    mock_zsock_s* s = malloc(sizeof(mock_zsock_s));
    if (s) {
        memset(s, 0, sizeof(mock_zsock_s));
        snprintf(s->dummy, sizeof(s->dummy), "%s", "DEALER");
    }
    return (zsock_t*)s;
}

zsock_t*
__wrap_zsock_new_dealer_checked(
    const char* endpoints,
    const char* filename,
    size_t line_nbr)
{
    ((void)filename);
    ((void)line_nbr);
    return __wrap_zsock_new_dealer(endpoints);
}

void
__wrap_zsock_destroy(zsock_t** sock_p)
{
    void* sock = *sock_p;
    *sock_p = NULL;
    free(sock);
}

void
__wrap_zsock_destroy_checked(
    zsock_t** self_p,
    const char* filename,
    size_t line_nbr)
{
    ((void)filename);
    ((void)line_nbr);
    return __wrap_zsock_destroy(self_p);
}
