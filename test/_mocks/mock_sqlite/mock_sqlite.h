#ifndef MOCK_SQLITE_H
#define MOCK_SQLITE_H

#include "sqlite3.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct outgoing_statement
    {
        uint32_t len;
        char data[];
    } outgoing_statement;
    void sqlite_spy_init();
    void sqlite_spy_deinit();
    outgoing_statement* sqlite_spy_outgoing_statement_pop();
    void sqlite_spy_step_return_push(int ret);

#ifdef __cplusplus
}
#endif
#endif
