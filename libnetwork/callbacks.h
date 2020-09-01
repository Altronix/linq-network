#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "common.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    netw_callbacks zmtp_callbacks;
    void zmtp_callbacks_init();

#ifdef __cplusplus
}
#endif
#endif /* CALLBACKS_H */
