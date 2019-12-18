#include "database_devices.h"
#include "database.h"

#define DATABASE                                                               \
    "CREATE TABLE %s("                                                         \
    "device_id INTEGER PRIMARY KEY,"                                           \
    "product TEXT,"                                                            \
    "prj_version TEXT,"                                                        \
    "atx_version INTEGER,"                                                     \
    "web_version TEXT,"                                                        \
    "mac TEXT"                                                                 \
    ");"

void
database_devices_create_table(database_s* database, const char* name)
{
    char table[512];
    snprintf(table, sizeof(table), DATABASE, name);
    database_assert_command(database, table);
}
