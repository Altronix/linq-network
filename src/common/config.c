#include "config.h"
#include "log.h"
#include "sys/sys.h"

LINQ_EXPORT const char*
sys_config_dir(const char* name)
{
#if defined _WIN32
    return NULL;
#else
    static char path[128];
    uint32_t c = 0;
    int rc;
    struct stat s;
    SYS_CONFIG_DIRS(seek);
    while (seek[c]) {
        snprintf(path, sizeof(path), seek[c], name);
        rc = stat(path, &s);
        if (rc == 0 && (s.st_mode & S_IFMT) == S_IFREG) return path;
        c++;
    }
    return NULL;
#endif
}
