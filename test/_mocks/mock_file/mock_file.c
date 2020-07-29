#include "mock_file.h"

typedef spy_file_packet_s mock_file_outgoing_s;
typedef spy_file_packet_s mock_file_incoming_s;
LIST_INIT_W_FREE(outgoing, mock_file_outgoing_s);
LIST_INIT_W_FREE(incoming, mock_file_incoming_s);

static outgoing_list_s* outgoing = NULL;
static incoming_list_s* incoming = NULL;

void
spy_file_init()
{
    spy_file_free();
    outgoing = outgoing_list_create();
    incoming = incoming_list_create();
}

void
spy_file_free()
{
    if (outgoing) outgoing_list_destroy(&outgoing);
    if (incoming) incoming_list_destroy(&incoming);
}

void
spy_file_push_incoming(const char* bytes, uint32_t l)
{
    spy_file_push(SPY_FILE_PUSH_INCOMING, bytes, l);
}

void
spy_file_push_outgoing(const char* bytes, uint32_t l)
{
    spy_file_push(SPY_FILE_PUSH_OUTGOING, bytes, l);
}

spy_file_packet_s*
spy_file_packet_pop_outgoing()
{
    return outgoing_list_pop(outgoing);
}

void
spy_file_packet_free(spy_file_packet_s** p)
{
    spy_file_packet_s* packet = *p;
    *p = NULL;
    free(packet);
}

void
spy_file_push(E_SPY_FILE_PUSH dst, const char* bytes, uint32_t l)
{
    spy_file_packet_s* packet = malloc(l + sizeof(spy_file_packet_s));
    assert(packet);
    packet->len = l;
    memcpy(packet->bytes, bytes, l);
    if (dst == SPY_FILE_PUSH_INCOMING) {
        incoming_list_push(incoming, &packet);
    } else {
        outgoing_list_push(outgoing, &packet);
    }
}

int
__wrap_fopen(const char* path, const char* mode)
{
    return 0;
}

int
__wrap_fread(void* data, size_t size, size_t n, FILE* stream)
{
    uint32_t ret = 0;
    assert(size == 1);
    spy_file_packet_s* p = incoming_list_pop(incoming);
    if (p) {
        ret = n < p->len ? n : p->len;
        memcpy(data, p->bytes, ret);
        free(p);
    } else {
        ret = 0;
    }
    return ret;
}

int
__wrap_fwrite(void* data, size_t size, size_t n, FILE* stream)
{
    assert(size == 1);
    spy_file_push(SPY_FILE_PUSH_OUTGOING, data, n);
    return n;
}

int
__wrap_ioctl(int fd, unsigned long request, ...)
{
    return 0;
}
