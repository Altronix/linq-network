#include "atx_net_internal.h"
#include "http_auth.h"
#include "jsmn/jsmn_helpers.h"
#include "log.h"
#include "routes.h"

#include "openssl/sha.h"

void
route_login(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{
    database_s* db = atx_net_database(ctx->context);
    char user_str[USER_MAX_LEN];
    char pass_str[PASS_MAX_LEN];
    char* token_str = NULL;
    char response[2048];
    int count, err;
    atx_str user, pass;
    jsmntok_t t[20];

    // clang-format off
    count = jsmn_parse_tokens(
        t, 20,
        body, l,
        2,
        "user", &user,
        "pass", &pass);
    // clang-format on
    if (count == 2 && user.len < USER_MAX_LEN && pass.len < PASS_MAX_LEN) {
        snprintf(user_str, sizeof(user_str), "%.*s", user.len, user.p);
        snprintf(pass_str, sizeof(pass_str), "%.*s", pass.len, pass.p);
        /*
        token = http_auth_login(db, user_str, pass_str);
        if (token) {
            if ((token_str = jwt_encode_str(token))) {
                l = snprintf(
                    response,
                    sizeof(response),
                    "{\"token\":\"%s\"}",
                    token_str);
                // Send token
                http_printf_json(ctx->curr_connection, 200, response);
                atx_net_free(token_str);
            } else {
                // Authorized OK but failed to create a token
                http_printf_json(ctx->curr_connection, 500, JERROR_500);
            }
            jwt_free(token);
        } else {
            // Unauthorized
            http_printf_json(ctx->curr_connection, 503, JERROR_503);
        }
        */
    } else {
        // Bad arguments
        http_printf_json(ctx->curr_connection, 400, JERROR_400);
    }
}

void
route_users(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{}

static void
process_create_admin(http_route_context* ctx, uint32_t l, const char* body)
{
    database_s* db = atx_net_database(ctx->context);
    char k[128], v[256], h[HASH_LEN], s[SALT_LEN], u[UUID_MAX_LEN];
    uint32_t klen, vlen;
    int count, err;
    atx_str user, pass;
    jsmntok_t t[20];

    // clang-format off
    count = jsmn_parse_tokens(
        t, 20,
        body, l,
        2,
        "user", &user,
        "pass", &pass);
    // clang-format on

    if (count == 2) {
        log_info("(DATA) creating admin account %.*s", user.len, user.p);
        http_auth_generate_uuid(u);
        err = http_auth_generate_password_hash(h, s, (char*)pass.p, pass.len);
        if (!err) {
            // clang-format off
            klen = snprintf(k, sizeof(k), "%s", "user_id,user,pass,salt,role");
            vlen = snprintf(
                v,
                sizeof(v),
                "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%d",
                UUID_MAX_LEN,
                u,
                user.len,
                user.p,
                HASH_LEN,
                h,
                SALT_LEN,
                s,
                0);
            // clang-format on
            err = database_insert_raw_n(db, "users", k, klen, v, vlen);
            if (!err) {
                http_printf_json(ctx->curr_connection, 200, JERROR_200);
            } else {
                http_printf_json(ctx->curr_connection, 500, JERROR_500);
            }
        } else {
            http_printf_json(ctx->curr_connection, 400, JERROR_400);
        }
    } else {
        http_printf_json(ctx->curr_connection, 400, JERROR_400);
    }
}

void
route_create_admin(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{
    database_s* db = atx_net_database(ctx->context);
    if (database_count(db, "users")) {
        http_printf_json(ctx->curr_connection, 503, JERROR_503);
    } else {
        process_create_admin(ctx, l, body);
    }
}
