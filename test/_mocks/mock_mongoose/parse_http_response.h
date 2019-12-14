#ifndef PARSE_HTTP_RESPONSE_H_
#define PARSE_HTTP_RESPONSE_H_

#include "http_parser.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int handle;
        const char* curr_header_ptr;
        uint32_t curr_header_len;
        char server[32];
        char connection[32];
        char cache_control[32];
        char content_type[32];
        char content_encoding[32];
        uint32_t content_length;
        char body[2048];
    } mongoose_parser_context;

    void mongoose_parser_init(http_parser* p, mongoose_parser_context*);
    int mongoose_parser_parse(http_parser* p, const char* data, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif
