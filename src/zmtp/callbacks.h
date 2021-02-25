#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "common/common.h"
#include "common/sys/sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern netw_callbacks zmtp_callbacks;
    void zmtp_callbacks_init();

#ifdef __cplusplus
}
#endif
#endif /* CALLBACKS_H */
