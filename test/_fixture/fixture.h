#ifndef FIXTURE_H_
#define FIXTURE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct fixture_context fixture_context;
    fixture_context* fixture_create(uint32_t port);
    void fixture_poll(fixture_context* f);
    void fixture_destroy(fixture_context**);

#ifdef __cplusplus
}
#endif

#endif /* FIXTURE_H_ */
