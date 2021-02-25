#ifndef CONFIG_H
#define CONFIG_H

#include "sys/sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    LINQ_EXPORT const char* sys_config_dir(const char* name);

#ifdef __cplusplus
}
#endif
#endif
