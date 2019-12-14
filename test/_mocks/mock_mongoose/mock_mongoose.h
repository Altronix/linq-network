#ifndef MOCK_MONGOOSE_H
#define MOCK_MONGOOSE_H

#include "mongoose.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void mongoose_spy_init();
    void mongoose_spy_deinit();

    void mongoose_spy_event_request_push(
        const char* auth,
        const char* meth,
        const char* path,
        const char* data);

    void mongoose_spy_event_close_push(int handle);
    void mongoose_spy_set_ev_handler(mg_event_handler_t *ev, void*);

#ifdef __cplusplus
}
#endif
#endif
