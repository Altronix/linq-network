#include "make_request.h"
#include "http_parser.h"
#include "log.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define FMT_GET                                                                \
    "%s %s HTTP/1.1\r\n"                                                       \
    "host: atx-usbd\r\n"                                                       \
    "\r\n"

#define FMT_POST                                                               \
    "%s %s HTTP/1.1\r\n"                                                       \
    "host: atx-usbd\r\n"                                                       \
    "Content-Length: %d\r\n"                                                   \
    "\r\n"                                                                     \
    "%*.s"

typedef struct
{
    int handle;
    const char* curr_header_ptr;
    uint16_t code;
    uint32_t curr_header_len;
    char server[32];
    char connection[32];
    char cache_control[32];
    char content_type[32];
    char content_encoding[32];
    uint32_t content_length;
    char body[2048];
} parser_context;

// Private
static int
http_response_on_header_field(http_parser* p, const char* at, size_t len)
{
    parser_context* ctx = p->data;
    ctx->curr_header_ptr = at;
    ctx->curr_header_len = len;
    return 0;
}

static int
http_response_on_header_value(http_parser* p, const char* at, size_t len)
{
    int err = 0;
    parser_context* ctx = p->data;
    if (len < 32) {
        if ((ctx->curr_header_len == 14) &&
            (!(memcmp(ctx->curr_header_ptr, "Content-Length", 14)))) {
            ctx->content_length = atoi(at);
        } else if (
            (ctx->curr_header_len == 12) &&
            (!(memcmp(ctx->curr_header_ptr, "Content-Type", 12)))) {
            memcpy(ctx->content_type, at, len);
            ctx->content_type[len] = 0;
        } else if (
            (ctx->curr_header_len == 16) &&
            (!(memcmp(ctx->curr_header_ptr, "Content-Encoding", 16)))) {
            memcpy(ctx->content_encoding, at, len);
            ctx->content_encoding[len] = 0;
        } else if (
            (ctx->curr_header_len == 6) &&
            (!(memcmp(ctx->curr_header_ptr, "Server", 6)))) {
            memcpy(ctx->server, at, len);
            ctx->server[len] = 0;
        } else if (
            (ctx->curr_header_len == 10) &&
            (!(memcmp(ctx->curr_header_ptr, "Connection", 10)))) {
            memcpy(ctx->connection, at, len);
            ctx->connection[len] = 0;
        } else if (
            (ctx->curr_header_len == 13) &&
            (!(memcmp(ctx->curr_header_ptr, "Cache-Control", 13)))) {
            memcpy(ctx->cache_control, at, len);
            ctx->cache_control[len] = 0;
        }
    }
    return err;
}

static int
http_response_on_url(http_parser* p, const char* at, size_t len)
{
    ((void)p);
    ((void)at);
    ((void)len);
    return 0;
}

static int
http_response_on_body(http_parser* p, const char* at, size_t len)
{
    int err = 0;
    parser_context* ctx = p->data;
    if (len <= sizeof(ctx->body)) {
        snprintf(ctx->body, sizeof(ctx->body), "%.*s", (int)len, at);
    }
    return err;
}

static int
http_response_on_status(http_parser* p, const char* at, size_t len)
{
    char buff[6];
    parser_context* ctx = p->data;
    snprintf(buff, sizeof(buff), "%.*s", (int)len, at);
    ctx->code = atoi(buff);
    return 0;
}

http_parser_settings parse_http_settings_response = {
    .on_header_field = http_response_on_header_field,
    .on_header_value = http_response_on_header_value,
    .on_url = http_response_on_url,
    .on_body = http_response_on_body,
    .on_status = http_response_on_status,
    .on_headers_complete = NULL,
    .on_message_begin = NULL,
    .on_message_complete = NULL,
    .on_chunk_header = NULL,
    .on_chunk_complete = NULL
};

static int
req_connect(int* sock_p, const char* host, int port)
{
    int ret = 0;
    struct sockaddr_in addr;
    int* sock = sock_p;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr.s_addr)) {
        log_error("inet_pton error [%d] [%s]", errno, strerror(errno));
        return -1;
    }
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("socket error [%d] [%s]", errno, strerror(errno));
        return -1;
    }
    int rc = connect(*sock, (struct sockaddr*)&addr, sizeof(addr));
    if (rc < 0) {
        if (errno == EINPROGRESS) {
            ret = 0;
            assert(false); // blocking socket
        } else {
            if (*sock >= 0) close(*sock);
            ret = -1;
        }
    } else {
        ret = 1;
    }
    return ret;
}

static int
req_send(int* fd, const char* b, uint32_t len)
{
    ssize_t bytes_sent = 0;
    bytes_sent = send(*fd, (char*)b, len, 0);
    if (bytes_sent < 0) {
        log_warn("(APP) tx [%d] [%s]", bytes_sent, strerror(errno));
        if (errno == EAGAIN || errno == EWOULDBLOCK) { bytes_sent = 0; }
    }
    log_info("(APP) tx [%d]", bytes_sent);
    return bytes_sent;
}

static int
req_recv(int* fd, const char* b, uint32_t len)
{
    ssize_t bytes_read = 0;
    bytes_read = recv(*fd, (char*)b, len, 0);
    if (bytes_read < 0) {
        log_warn("(APP) rx [%d] [%s]", bytes_read, strerror(errno));
        if (errno == EAGAIN || errno == EWOULDBLOCK) { bytes_read = 0; }
    } else if (bytes_read == 0) {
        log_warn("(APP) rx [%d] [%s]", bytes_read, strerror(errno));
        bytes_read = -1;
    }
    log_info("(APP) rx [%d]", bytes_read);
    return bytes_read;
}

int
make_request(
    const char* meth,
    const char* path,
    const char* data,
    uint32_t dlen,
    void (*fn)(void*, uint16_t, const char* body),
    void* ctx)
{
    int sock, rc;
    uint32_t l;
    http_parser parser;
    parser_context context;
    char resp[8192];
    char* req = NULL;
    if (!(memcmp(meth, "GET", 3))) {
        l = snprintf(NULL, 0, FMT_GET, meth, path);
        req = malloc(l + 1);
        assert(req);
        snprintf(req, l + 1, FMT_GET, meth, path);
    } else {
        l = snprintf(NULL, 0, FMT_POST, meth, path, dlen, dlen, data);
        req = malloc(l + 1);
        assert(req);
        snprintf(req, l + 1, FMT_POST, meth, path, dlen, dlen, data);
    }

    rc = req_connect(&sock, "http://127.0.0.1", 33888);
    if (!rc) {
        log_error("(APP) failed to connect!");
        free(req);
        return -1;
    }

    rc = req_send(&sock, req, l);
    if (!(rc == l)) {
        log_error("(APP) failed to send!");
        close(sock);
        free(req);
        return -1;
    }

    l = req_recv(&sock, resp, sizeof(resp));
    if (!(l > 0)) {
        log_error("(APP) failed to recv!");
        close(sock);
        free(req);
        return -1;
    }

    memset(&context, 0, sizeof(parser_context));
    http_parser_init(&parser, HTTP_RESPONSE);
    parser.data = &context;
    rc = http_parser_execute(&parser, &parse_http_settings_response, resp, l);
    if (!(rc == l)) {
        log_error("(APP) failed to parse HTTP response!");
        close(sock);
        free(req);
        return -1;
    }

    fn(ctx, context.code, context.body);

    free(req);
    close(sock);
    return 0;
}
