#include "atx_net_internal.h"
#include "jsmn_helpers.h"
#include "log.h"
#include "routes.h"

#include "openssl/sha.h"

#define UUID_MAX_LEN 32
#define USER_MAX_LEN 24
#define PASS_MAX_LEN 48
#define PASS_MIN_LEN 12
#define SALT_LEN 16
#define HASH_LEN 65

#ifdef TESTING
static void
gen_salt(char dst[SALT_LEN])
{
    memcpy(dst, "0123456789ABCDEF", SALT_LEN);
}
static void
gen_uuid(char dst[UUID_MAX_LEN])
{
    memset(dst, 0, UUID_MAX_LEN);
    snprintf(dst, UUID_MAX_LEN, "%s", "user_id01234");
}
#else
static void
gen_salt(char dst[SALT_LEN])
{
    // TODO
    memcpy(dst, "0123456789ABCDEF", 16);
}
static void
gen_uuid(char dst[UUID_MAX_LEN])
{
    zuuid_t* uid = zuuid_new();
    snprintf(dst, UUID_MAX_LEN, "%s", zuuid_str(uid));
    zuuid_destroy(&uid);
}
#endif

static void
hash_256(const char* data, uint32_t dlen, char buffer[HASH_LEN])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha;
    SHA256_Init(&sha);
    SHA256_Update(&sha, data, dlen);
    SHA256_Final(hash, &sha);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(buffer + (i * 2), "%02X", hash[i]);
    }
    buffer[64] = 0;
}

static int
gen_password_hash(
    char hash[HASH_LEN],
    char salt[SALT_LEN],
    const char* pass,
    uint32_t len)
{
    char concat[PASS_MAX_LEN + SALT_LEN];
    int l;
#ifdef TESTING
    log_warn("(HTTP) generating unsafe debug hash!");
#endif
    if (!(len < PASS_MAX_LEN && len >= PASS_MIN_LEN)) return -1;
    gen_salt(salt);
    l = snprintf(concat, sizeof(concat), "%.*s%.*s", len, pass, SALT_LEN, salt);
    hash_256(concat, l, hash);
    return 0;
}

void
route_login(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{}

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

    if (!(count == 2)) {
        http_printf_json(ctx->curr_connection, 400, JERROR_400);
    } else {
        log_info("(DATA) creating admin account %.*s", user.len, user.p);
        gen_uuid(u);
        err = gen_password_hash(h, s, pass.p, pass.len);
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
