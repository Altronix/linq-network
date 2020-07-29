#include "mock_file.h"

int __real_fwrite(void* data, size_t size, size_t n, FILE* stream);
int __real_fread(void* data, size_t size, size_t n, FILE* stream);
FILE* __real_fopen(const char* path, const char* mode);
int __real_fclose(FILE*);

typedef spy_file_packet_s mock_file_outgoing_s;
typedef spy_file_packet_s mock_file_incoming_s;
LIST_INIT_W_FREE(outgoing, mock_file_outgoing_s);
LIST_INIT_W_FREE(incoming, mock_file_incoming_s);
LIST_INIT_W_FREE(on_ioctl, int);

static outgoing_list_s* outgoing = NULL;
static incoming_list_s* incoming = NULL;
static on_ioctl_list_s* on_ioctl = NULL;
static sys_file* g_mock_file = (void*)1;

void
spy_file_init()
{
    spy_file_free();
    outgoing = outgoing_list_create();
    incoming = incoming_list_create();
    on_ioctl = on_ioctl_list_create();
}

void
spy_file_free()
{
    if (outgoing) outgoing_list_destroy(&outgoing);
    if (incoming) incoming_list_destroy(&incoming);
    if (on_ioctl) on_ioctl_list_destroy(&on_ioctl);
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
spy_file_push_ioctl(int param)
{
    int* p = malloc(sizeof(int));
    assert(p);
    *p = param;
    on_ioctl_list_push(on_ioctl, &p);
}

sys_file*
__wrap_fopen(const char* path, const char* mode)
{
    if (outgoing) {
        return g_mock_file;
    } else {
        return __real_fopen(path, mode);
    }
}

int
__wrap_fclose(sys_file* f)
{
    if (outgoing) {
        return 0;
    } else {
        return __real_fclose(f);
    }
}

int
__wrap_fread(void* data, size_t size, size_t n, FILE* stream)
{
    uint32_t ret = 0;
    if (incoming) {
        assert(size == 1);
        spy_file_packet_s* p = incoming_list_pop(incoming);
        if (p) {
            ret = n < p->len ? n : p->len;
            memcpy(data, p->bytes, ret);
            free(p);
        } else {
            ret = 0;
        }
    } else {
        ret = __real_fread(data, size, n, stream);
    }
    return ret;
}

int
__wrap_fwrite(void* data, size_t size, size_t n, FILE* stream)
{
    if (outgoing) {
        assert(size == 1);
        spy_file_push(SPY_FILE_PUSH_OUTGOING, data, n);
        return n;
    } else {
        return __real_fwrite(data, size, n, stream);
    }
}

// This is a simple mock that simply sets the callers param with a control
// Doesn't support much.
// IE:
//   spy_file_push_ioctl(33);
//   ...
//   ioctl(file, param, &result); (result == 33)
int
__wrap_ioctl(int fd, unsigned long request, ...)
{
    va_list list;

    int* set = on_ioctl_list_pop(on_ioctl);

    if (set) {
        va_start(list, request);
        int* param = va_arg(list, int*);
        *param = *set;
        va_end(list);
        free(set);
    }
    return 0;
}

int
__wrap_fileno(FILE* f)
{
    return 0;
}
