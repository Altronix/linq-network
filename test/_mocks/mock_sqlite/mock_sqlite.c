#include "mock_sqlite.h"
#include "containers.h"

LIST_INIT_W_FREE(statements, outgoing_statement);
statements_list_s* outgoing_statements = NULL;

void
sqlite_spy_init()
{
    outgoing_statements = statements_list_create();
}

void
sqlite_spy_deinit()
{
    if (outgoing_statements) statements_list_destroy(&outgoing_statements);
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
    return SQLITE_DONE;
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
