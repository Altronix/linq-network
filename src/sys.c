#include "linq_io_internal.h"

bool
sys_running()
{
    return !zsys_interrupted;
}
