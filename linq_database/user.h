#ifndef USER_H_
#define USER_H_

#include "database.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int user_open(database_s* db, user_s* u, const char* user);
    void user_close(user_s* u);

#ifdef __cplusplus
}
#endif
#endif
