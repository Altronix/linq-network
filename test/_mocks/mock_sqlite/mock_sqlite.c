#include "mock_sqlite.h"
#include "containers.h"

//
typedef struct on_step_return
{
    int ret;
} on_step_return;

//
typedef struct on_column_text_return
{
    const char* ret;
} on_column_text_return;

//
typedef struct on_column_int_return
{
    int ret;
} on_column_int_return;

//
LIST_INIT_W_FREE(on_step, on_step_return);
LIST_INIT_W_FREE(on_column_text, on_column_text_return);
LIST_INIT_W_FREE(on_column_int, on_column_int_return);
LIST_INIT_W_FREE(statements, outgoing_statement);

on_step_list_s* on_step = NULL;
on_column_text_list_s* on_column_text = NULL;
on_column_int_list_s* on_column_int = NULL;
statements_list_s* outgoing_statements = NULL;

//
on_step_return on_step_default = { .ret = SQLITE_DONE };
on_column_text_return on_column_text_default = { .ret = "column" };
on_column_int_return on_coumn_int_default = { .ret = 0 };

void
sqlite_spy_init()
{
    on_step = on_step_list_create();
    on_column_text = on_column_text_list_create();
    on_column_int = on_column_int_list_create();
    outgoing_statements = statements_list_create();
}

void
sqlite_spy_deinit()
{
    if (on_step) on_step_list_destroy(&on_step);
    if (on_column_text) on_column_text_list_destroy(&on_column_text);
    if (on_column_int) on_column_int_list_destroy(&on_column_int);
    if (outgoing_statements) statements_list_destroy(&outgoing_statements);
}

outgoing_statement*
sqlite_spy_outgoing_statement_pop()
{
    return statements_list_pop(outgoing_statements);
}

void
sqlite_spy_outgoing_statement_flush()
{
    outgoing_statement* statement = NULL;
    while ((statement = statements_list_pop(outgoing_statements))) {
        linq_netw_free(statement);
    }
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

void
sqlite_spy_column_text_return_push(const char* ret)
{
    on_column_text_return* r = linq_netw_malloc(sizeof(on_column_text_return));
    linq_netw_assert(r);
    r->ret = ret;
    on_column_text_list_push(on_column_text, &r);
}

void
sqlite_spy_column_int_return_push(int ret)
{
    on_column_int_return* r = linq_netw_malloc(sizeof(on_column_int_return));
    linq_netw_assert(r);
    r->ret = ret;
    on_column_int_list_push(on_column_int, &r);
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

const unsigned char*
__wrap_sqlite3_column_text(sqlite3_stmt* stmt, int iCol)
{
    ((void)stmt);
    ((void)iCol);
    const char* ret;
    on_column_text_return* r = on_column_text_list_pop(on_column_text);
    if (r) {
        ret = r->ret;
        linq_netw_free(r);
    } else {
        ret = on_column_text_default.ret;
    }
    return (const unsigned char*)ret;
}

int
__wrap_sqlite3_column_int(sqlite3_stmt* stmt, int iCol)
{
    ((void)stmt);
    ((void)iCol);
    int ret;
    on_column_int_return* r = on_column_int_list_pop(on_column_int);
    if (r) {
        ret = r->ret;
        linq_netw_free(r);
    } else {
        ret = 0;
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
