#ifndef LINQ_H_
#define LINQ_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct device device;
    typedef struct devices devices;
    typedef struct linq linq;
    typedef void (*linq_error_fn)(void*, int, const char*, const char*);

    typedef enum
    {
        e_linq_ok = 0,
        e_linq_oom = -1,
        e_linq_bad_args = -2,
        e_linq_protocol = -3
    } e_linq_error;

    typedef struct linq_callbacks
    {
        linq_error_fn err;
    } linq_callbacks;

    linq* linq_create(linq_callbacks*, void*);
    void linq_destroy(linq**);
    e_linq_error linq_listen(linq*, const char* ep);
    e_linq_error linq_poll(linq* l);
    device** linq_device(linq*, const char*);
    uint32_t linq_device_count(linq*);

#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
