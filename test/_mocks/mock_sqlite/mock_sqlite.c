#include "mock_sqlite.h"
#include "containers.h"

typedef struct on_step_return
{
    int ret;
} on_step_return;

LIST_INIT_W_FREE(statements, outgoing_statement);
statements_list_s* outgoing_statements = NULL;

LIST_INIT_W_FREE(on_step, on_step_return);
on_step_list_s* on_step = NULL;
on_step_return on_step_default = { .ret = SQLITE_DONE };

void
sqlite_spy_init()
{
    outgoing_statements = statements_list_create();
    on_step = on_step_list_create();
}

void
sqlite_spy_deinit()
{
    if (outgoing_statements) statements_list_destroy(&outgoing_statements);
    if (on_step) on_step_list_destroy(&on_step);
}

outgoing_statement*
sqlite_spy_outgoing_statement_pop()
{
    return statements_list_pop(outgoing_statements);
}

void
sqlite_spy_outgoing_statement_push(const char* sql, uint32_t len)
{
    outgoing_statement* stmt =
        linq_netw_malloc(sizeof(outgoing_statement) + len + 1);
    linq_netw_assert(stmt);
    stmt->len = len;
    memcpy(stmt->data, sql, len);
    statements_list_push(outgoing_statements, &stmt);
}

void
sqlite_spy_step_return_push(int ret)
{
    on_step_return* s = linq_netw_malloc(sizeof(on_step_return));
    linq_netw_assert(s);
    s->ret = ret;
    on_step_list_push(on_step, &s);
}

int
__wrap_sqlite3_open_v2(
    const char* filename,
    sqlite3** ppdb,
    int flags,
    const char* zvfs)
{
    ((void)filename);
    ((void)ppdb);
    ((void)flags);
    ((void)zvfs);
    return SQLITE_OK;
}

int
__wrap_sqlite3_close(sqlite3* db)
{
    ((void)db);
    return SQLITE_OK;
}

int
__wrap_sqlite3_prepare_v2(
    sqlite3* db,
    const char* zsql,
    int nbyte,
    sqlite3_stmt** ppstmt,
    const char** pztail)
{
    ((void)db);
    ((void)ppstmt);
    ((void)pztail);
    sqlite_spy_outgoing_statement_push(zsql, nbyte > 0 ? nbyte : strlen(zsql));
    return SQLITE_OK;
}

int
__wrap_sqlite3_prepare_v3(
    sqlite3* db,
    const char* zsql,
    int nbyte,
    unsigned int prepflags,
    sqlite3_stmt** ppstmt,
    const char** pztail)
{
    ((void)db);
    ((void)prepflags);
    ((void)ppstmt);
    ((void)pztail);
    sqlite_spy_outgoing_statement_push(zsql, nbyte ? nbyte : strlen(zsql));
    return SQLITE_OK;
}

int
__wrap_sqlite3_step(sqlite3_stmt* pstmt)
{
    ((void)pstmt);
    int ret;
    on_step_return* step = on_step_list_pop(on_step);
    if (step) {
        ret = step->ret;
        linq_netw_free(step);
    } else {
        ret = on_step_default.ret;
    }
    return ret;
}

int
__wrap_sqlite3_finalize(sqlite3_stmt* pstmt)
{
    ((void)pstmt);
    return SQLITE_OK;
}

int
__wrap_sqlite3_exec(
    sqlite3* db,
    const char* sql,
    int (*callback)(void*, int, char**, char**),
    void* ctx,
    char** errmsg)
{
    ((void)db);
    ((void)sql);
    ((void)callback);
    ((void)ctx);
    ((void)errmsg);
    return SQLITE_OK;
}
