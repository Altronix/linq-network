#ifndef LINQ_H_
#define LINQ_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct device device;
    typedef struct linq linq;

    typedef enum
    {
        e_linq_ok = 0,
        e_linq_oom = -1,
        e_linq_bad_args = -2,
        e_linq_protocol = -3
    } e_linq_error;

    typedef struct linq_callbacks
    {
    } linq_callbacks;

    linq* linq_create(linq_callbacks*, void*);
    void linq_destroy(linq**);
    e_linq_error linq_listen(linq*, const char* ep);
    e_linq_error linq_poll(linq* l);

#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
