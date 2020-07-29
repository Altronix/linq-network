#ifndef MOCK_FILE_H
#define MOCK_FILE_H

#include "containers.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum E_SPY_FILE_PUSH
    {
        SPY_FILE_PUSH_INCOMING,
        SPY_FILE_PUSH_OUTGOING,
    } E_SPY_FILE_PUSH;

    typedef struct spy_file_packet_s
    {
        uint32_t len;
        char bytes[];
    } spy_file_packet_s;

    void spy_file_push(E_SPY_FILE_PUSH, const char* bytes, uint32_t l);
    void spy_file_push_incoming(const char* bytes, uint32_t l);
    void spy_file_push_outgoing(const char* bytes, uint32_t l);
    void spy_file_init();
    void spy_file_free();
    spy_file_packet_s* spy_file_packet_pop_outgoing();
    void spy_file_packet_free(spy_file_packet_s** p);

#ifdef __cplusplus
}
#endif
#endif /* MOCK_FILE_H */
