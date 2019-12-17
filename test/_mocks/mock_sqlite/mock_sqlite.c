#include "mock_sqlite.h"

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
}

int
__wrap_sqlite_close(sqlite3* db)
{
    ((void)db);
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
    ((void)zsql);
    ((void)nbyte);
    ((void)ppstmt);
    ((void)pztail);
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
    ((void)zsql);
    ((void)nbyte);
    ((void)prepflags);
    ((void)ppstmt);
    ((void)pztail);
}

int
__wrap_sqlite3_finalize(sqlite3_stmt* pstmt)
{
    ((void)pstmt);
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
}
