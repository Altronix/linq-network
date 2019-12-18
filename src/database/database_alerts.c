#include "database_alerts.h"

#define DATABASE                                                               \
    "CREATE TABLE %s("                                                         \
    "alert_id INTEGER PRIMARY KEY,"                                            \
    "who TEXT,"                                                                \
    "what TEXT,"                                                               \
    "site_id TEXT,"                                                            \
    "time INTEGER,"                                                            \
    "mesg TEXT,"                                                               \
    "name TEXT,"                                                               \
    "device_id INTEGER,"                                                       \
    "FOREIGN KEY(device_id) REFERENCES devices(device_id)"                     \
    ");"

void
database_alerts_create_table(database_s* database, const char* name)
{
    char table[512];
    snprintf(table, sizeof(table), DATABASE, name);
    database_assert_command(database, table);
}
