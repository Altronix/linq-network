#ifndef WIRE_EVENT_H
#define WIRE_EVENT_H

#include "wire.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void wire_event(wire_s* wire, void* ctx, E_WIRE_EVENT e, ...);

#ifdef __cplusplus
}
#endif
#endif /* WIRE_EVENT_H */
