#ifndef MOCK_MONGOOSE_H
#define MOCK_MONGOOSE_H

#include "mongoose.h"
#include "parse_http.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // State of mock event
    typedef struct mock_mongoose_event
    {
        struct mg_connection* c;
        int ev;
        void* p;
        struct http_message message;
        char request[2048];
    } mock_mongoose_event;

    typedef struct mock_mongoose_outgoing_data
    {
        uint32_t l;
        char mem[2048];
    } mock_mongoose_outgoing_data;

    // Any outgoing responses
    typedef mongoose_parser_context mock_mongoose_response;

    void mongoose_spy_init();
    void mongoose_spy_deinit();

    void mongoose_spy_event_request_push(
        const char* auth,
        const char* meth,
        const char* path,
        const char* data);

    void mongoose_spy_event_close_push(int handle);
    mock_mongoose_outgoing_data* mongoose_spy_outgoing_data_pop();
    void mongoose_spy_set_ev_handler(mg_event_handler_t* ev, void*);
    void mock_mongoose_response_destroy(mock_mongoose_response** resp_p);

#ifdef __cplusplus
}
#endif
#endif
