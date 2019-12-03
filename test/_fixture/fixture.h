// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FIXTURE_H_
#define FIXTURE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct fixture_context fixture_context;
    fixture_context* fixture_create(const char* sid, uint32_t port);
    void fixture_poll(fixture_context* f);
    void fixture_destroy(fixture_context**);

#ifdef __cplusplus
}
#endif

#endif /* FIXTURE_H_ */
