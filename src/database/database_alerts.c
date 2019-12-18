#include "database_alerts.h"

int
database_alerts_create_table(database_s* database)
{
    static const char* table =
        "CREATE TABLE alerts("
        "alert_id INTEGER PRIMARY KEY,"
        "who TEXT,"
        "what TEXT,"
        "site_id TEXT,"
        "when INTEGER,"
        "mesg TEXT,"
        "name TEXT,"
        "FOREIGN KEY(device_id) REFERENCES devices(device_id));";
    ((void)database);
    return -1;
}
