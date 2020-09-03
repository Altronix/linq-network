#include "mock_file.h"

int __real_fwrite(void* data, size_t size, size_t n, FILE* stream);
int __real_fread(void* data, size_t size, size_t n, FILE* stream);
int __real_vfprintf(FILE*, const char* fmt, va_list list);
int __real_fprintf(FILE*, const char* fmt, ...);
FILE* __real_fopen(const char* path, const char* mode);
int __real_fclose(FILE*);

typedef spy_file_packet_s mock_file_outgoing_s;
typedef spy_file_packet_s mock_file_incoming_s;
LIST_INIT_W_FREE(outgoing, mock_file_outgoing_s);

static bool active = false;
static uint32_t throughput = 0;
static spy_file_packet_s* incoming = NULL;
static outgoing_list_s* outgoing = NULL;
static sys_file* g_mock_file = (void*)1;

static uint32_t
find_lowest(uint32_t n, ...)
{
    uint32_t i = 0, lowest, find[8];
    va_list list;
    va_start(list, n);
    while (n-- && i < sizeof(find)) find[i++] = va_arg(list, int);
    va_end(list);
    lowest = find[0];
    for (n = 0; n < i; n++) {
        if (find[n] < lowest) lowest = find[n];
    }
    return lowest;
}

void
spy_file_init()
{
    spy_file_free();
    outgoing = outgoing_list_create();
    throughput = 0;
    active = true;
}

void
spy_file_free()
{
    if (outgoing) outgoing_list_destroy(&outgoing);
    if (incoming) spy_file_packet_free(&incoming);
    active = false;
    throughput = 0;
}

void
spy_file_set_throughput(uint32_t val)
{
    throughput = val;
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
        if (incoming) spy_file_packet_free(&incoming);
        incoming = packet;
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
spy_file_packet_flush_outgoing()
{
    spy_file_packet_s* packet = outgoing_list_pop(outgoing);
    while (packet) {
        spy_file_packet_free(&packet);
        packet = outgoing_list_pop(outgoing);
    }
}

void
spy_file_packet_free(spy_file_packet_s** p)
{
    spy_file_packet_s* packet = *p;
    *p = NULL;
    free(packet);
}

sys_file*
__wrap_fopen(const char* path, const char* mode)
{
    if (active) {
        return g_mock_file;
    } else {
        return __real_fopen(path, mode);
    }
}

int
__wrap_fclose(sys_file* f)
{
    if (active) {
        return 0;
    } else {
        return __real_fclose(f);
    }
}

int
__wrap_fread(void* data, size_t size, size_t n, FILE* stream)
{
    uint32_t ret = 0;
    if (active) {
        assert(size == 1);
        if (incoming) {
            ret = throughput ? find_lowest(3, incoming->len, n, throughput)
                             : find_lowest(2, incoming->len, n);
            memcpy(data, incoming->bytes, ret);
            if (incoming->len) {
                memmove(
                    incoming->bytes,
                    &incoming->bytes[ret],
                    incoming->len - ret);
                incoming->len -= ret;
            }
            if (!incoming->len) spy_file_packet_free(&incoming);
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
    if (active) {
        assert(size == 1);
        spy_file_push(SPY_FILE_PUSH_OUTGOING, data, n);
        return n;
    } else {
        return __real_fwrite(data, size, n, stream);
    }
}

int
__wrap_vfprintf(FILE* f, const char* fmt, va_list list)
{

    if (active) {
        int len;
        char* buff = malloc(8192);
        assert(buff);
        len = vsnprintf(buff, 8192, fmt, list);
        assert(len < 8192);
        spy_file_push_outgoing(buff, len);
        free(buff);
        return len;
    } else {
        return __real_vfprintf(f, fmt, list);
    }
}

int
__wrap_fprintf(FILE* f, const char* fmt, ...)
{
    int rc;
    va_list list;
    va_start(list, fmt);
    rc = __wrap_vfprintf(f, fmt, list);
    va_end(list);
    return rc;
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

    if (active) {
        va_start(list, request);
        int* param = va_arg(list, int*);
        if (incoming) {
            *param = (throughput && throughput < incoming->len) ? throughput
                                                                : incoming->len;
        } else {
            *param = 0;
        }
        va_end(list);
    }
    return 0;
}

int
__wrap_fileno(FILE* f)
{
    return 0;
}
