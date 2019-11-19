#include "linq_netw_internal.h"

bool
sys_running()
{
    return !zsys_interrupted;
}
