
#include "http_users.h"
#include "http_auth.h"
#include "json.h"
#include "log.h"
#include "sys.h"

#include "openssl/sha.h"

void
login(http_request_s* r, HTTP_METHOD meth, uint32_t l, const char* body)
{
    database_s* db = ((http_s*)http_request_context(r))->db;
    char user_str[USER_MAX_LEN];
    char pass_str[PASS_MAX_LEN];
    char* token_str = NULL;
    char response[2048];
    int count, err;
    jsmn_token_encode_s token;
    json_parser p;
    jsontok t[20];
    const jsontok *user, *pass;
    json_init(&p);
    count = json_parse(&p, body, l, t, 20);
    if (count > 0 && (user = json_get_member(body, t, "user")) &&
        (pass = json_get_member(body, t, "pass"))) {
        snprintf(
            user_str,
            sizeof(user_str),
            "%.*s",
            user->end - user->start,
            &body[user->start]);
        snprintf(
            pass_str,
            sizeof(pass_str),
            "%.*s",
            pass->end - pass->start,
            &body[pass->start]);
        err = http_auth_login(db, user_str, pass_str, &token);
        if (!err) {
            l = snprintf(
                response,
                sizeof(response),
                "{\"token\":\"%.*s\"}",
                token.len,
                token.b);
            // Send token
            http_printf_json(r->connection, 200, response);
            linq_network_free(token_str);
        } else {
            // Unauthorized
            http_printf_json(r->connection, 503, JERROR_503);
        }
    } else {
        // Bad arguments
        http_printf_json(r->connection, 400, JERROR_400);
    }
}

void
users(http_request_s* ctx, HTTP_METHOD meth, uint32_t l, const char* body)
{}

static void
process_create_admin(http_request_s* r, uint32_t l, const char* body)
{
    database_s* db = ((http_s*)http_request_context(r))->db;
    char k[128], v[256], h[HASH_LEN], s[SALT_LEN], uid[UUID_MAX_LEN];
    uint32_t klen, vlen, ulen, plen;
    int count, err;
    json_parser parser;
    jsontok t[20];
    const jsontok *u, *p;
    const char *user, *pass;
    json_init(&parser);
    count = json_parse(&parser, body, l, t, 20);
    if (count > 0 && (u = json_get_member(body, t, "user")) &&
        (p = json_get_member(body, t, "pass"))) {
        (user = &body[u->start], ulen = u->end - u->start);
        (pass = &body[p->start], plen = p->end - p->start);
        log_info("(DATA) creating admin account %.*s", ulen, user);
        http_auth_generate_uuid(uid);
        err = http_auth_generate_password_hash(h, s, (char*)pass, plen);
        if (!err) {
            // clang-format off
            klen = snprintf(k, sizeof(k), "%s", "user_id,user,pass,salt,role");
            vlen = snprintf(
                v,
                sizeof(v),
                "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%d",
                UUID_MAX_LEN,
                uid,
                ulen,
                user,
                HASH_LEN,
                h,
                SALT_LEN,
                s,
                0);
            // clang-format on
            err = database_insert_raw_n(db, "users", k, klen, v, vlen);
            if (!err) {
                http_printf_json(http_request_connection(r), 200, JERROR_200);
            } else {
                http_printf_json(http_request_connection(r), 500, JERROR_500);
            }
        } else {
            http_printf_json(http_request_connection(r), 400, JERROR_400);
        }
    } else {
        http_printf_json(http_request_connection(r), 400, JERROR_400);
    }
}

void
has_admin(http_request_s* r, HTTP_METHOD meth, uint32_t l, const char* body)
{
    database_s* db = ((http_s*)http_request_context(r))->db;
    int ret = database_count(db, "users") ? 1 : 0;
    http_printf_json(http_request_connection(r), 200, "%d", ret);
}

void
create_admin(http_request_s* r, HTTP_METHOD meth, uint32_t l, const char* body)
{
    database_s* db = ((http_s*)http_request_context(r))->db;
    if (database_count(db, "users")) {
        http_printf_json(http_request_connection(r), 503, JERROR_503);
    } else {
        process_create_admin(r, l, body);
    }
}
