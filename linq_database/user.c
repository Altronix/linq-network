#include "user.h"
#include "sqlite3.h"

#define QUERY                                                                  \
    "SELECT user_id,user,pass,salt "                                           \
    "FROM users "                                                              \
    "WHERE user=\"%s\" "                                                       \
    "LIMIT 1"

int
user_open(database_s* db, user_s* u, const char* user)
{
    char query[256];
    int l, err;
    memset(u, 0, sizeof(user_s));
    l = snprintf(query, sizeof(query), QUERY, user);
    err = sqlite3_prepare_v2(db->db, query, l + 1, &u->stmt, NULL);
    linq_network_assert(err == SQLITE_OK);
    err = sqlite3_step(u->stmt);
    if (err == SQLITE_ROW) {
        u->id = (const char*)sqlite3_column_text(u->stmt, 0);
        u->user = (const char*)sqlite3_column_text(u->stmt, 1);
        u->pass = (const char*)sqlite3_column_text(u->stmt, 2);
        u->salt = (const char*)sqlite3_column_text(u->stmt, 3);
        u->role = sqlite3_column_int(u->stmt, 4);
        return 0;
    } else {
        sqlite3_finalize(u->stmt);
        return -1;
    }
}

void
user_close(database_s* db, user_s* u)
{
    sqlite3_finalize(u->stmt);
}

