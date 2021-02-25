#ifndef __SYS_H
#define __SYS_H

#if defined _WIN32
#include "win/sys.h"
#else
#include "unix/sys.h"
#endif

#endif
