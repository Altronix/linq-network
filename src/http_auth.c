#include "http_auth.h"
#include "jwt.h"
#include "log.h"

#include "http_auth_unsafe.h"

#define QUERY                                                                  \
    "SELECT user_id,user,pass,salt "                                           \
    "FROM users "                                                              \
    "WHERE user=\"%s\" "                                                       \
    "LIMIT 1"

#include "openssl/sha.h"

/// gen_salt() - Generate randomness to concat with password for hashing
#ifdef TESTING
static void
gen_salt(char dst[SALT_LEN])
{
    /// TESTING generate unsafe salt value for testing
    memcpy(dst, UNSAFE_SALT, SALT_LEN);
}
#else
static void
gen_salt(char dst[SALT_LEN])
{
    /// TODO - Generate randomness to salt the password hash
    memcpy(dst, UNSAFE_SALT, SALT_LEN);
}
#endif

/// gen_uuid() - Generate a unique id for the given user
#ifdef TESTING
static void
gen_uuid(char dst[UUID_MAX_LEN])
{
    /// TESTING - generated a fixed user ID for  testing
    memset(dst, 0, UUID_MAX_LEN);
    snprintf(dst, UUID_MAX_LEN, "%s", UNSAFE_UUID);
}
#else
static void
gen_uuid(char dst[UUID_MAX_LEN])
{
    /// Generate random unique user id
    zuuid_t* uid = zuuid_new();
    snprintf(dst, UUID_MAX_LEN, "%s", zuuid_str(uid));
    zuuid_destroy(&uid);
}
#endif

#ifdef TESTING
static void
get_secret(char secret[SECRET_LEN])
{
    log_warn("(HTTP) validating tokens with debugger Secret!");
    memset(secret, 0, SECRET_LEN);
    snprintf(secret, SECRET_LEN, "%s", UNSAFE_SECRET);
}
#else
static void
get_secret(char secret[SECRET_LEN])
{
    // TODO
    log_warn("(HTTP) TODO - Using UNSAFE secret in production!");
    memset(secret, 0, SECRET_LEN);
    snprintf(secret, SECRET_LEN, "%s", UNSAFE_SECRET);
}
#endif

/// hash_256() - Hash some data, put in buffer
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

/// get_jwt() - Get the Json Web Token from HTTP headers
static inline void
get_jwt(struct http_message* m, struct mg_str* ret)
{
    struct mg_str* token;
    if (((token = mg_get_http_header(m, "Authorization")) ||
         (token = mg_get_http_header(m, "authorization"))) &&
        (token->len > 6) &&
        ((!memcmp(token->p, "Bearer ", 7)) ||
         (!memcmp(token->p, "bearer ", 7)))) {
        ret->p = token->p += 7;
        ret->len = token->len -= 7;
    } else {
        ret->p = NULL;
        ret->len = 0;
    }
}

/// check_token_access() - Check that user exists in the database
static bool
check_token_access(database_s* db, jwt_t* token)
{

    // NOTE other potential standard tokens are
    // nbf - not before
    // iss - issuer
    // aud - audience
    int iat, exp;
    const char* sub;
    iat = jwt_get_grant_int(token, "iat");
    exp = jwt_get_grant_int(token, "exp");
    sub = jwt_get_grant(token, "sub");
    return sys_unix() < exp &&
           database_row_exists_str(db, "users", "user", sub);
}

/// http_auth_login() - Check the supplied credentials against the database and
/// issue a token
jwt_t*
http_auth_login(database_s* db, const char* user, const char* pass)
{
    jwt_t* jwt = NULL;
    sqlite3_stmt* stmt;
    int err = -1, l;
    char query[256];
    char test[HASH_LEN];
    char concat[PASS_MAX_LEN + SALT_LEN];
    const char *uuid, *name, *hash, *salt;

    l = snprintf(query, sizeof(query), QUERY, user);
    atx_net_assert(l < sizeof(query));
    err = sqlite3_prepare_v2(db->db, query, l + 1, &stmt, NULL);
    atx_net_assert(err == SQLITE_OK);
    err = sqlite3_step(stmt);
    if (err == SQLITE_ROW) {
        uuid = (const char*)sqlite3_column_text(stmt, 0),
        name = (const char*)sqlite3_column_text(stmt, 1),
        hash = (const char*)sqlite3_column_text(stmt, 2),
        salt = (const char*)sqlite3_column_text(stmt, 3);
        uint32_t plen = strlen(pass);
        uint32_t slen = strlen(salt);
        uint32_t hlen = strlen(hash);
        atx_net_assert(slen == SALT_LEN);
        atx_net_assert(hlen == HASH_LEN);
        l = snprintf(
            concat, sizeof(concat), "%.*s%.*s", plen, pass, slen, salt);
        if (l <= sizeof(concat)) {
            hash_256(concat, l, test);
            err = memcmp(hash, test, HASH_LEN);
            // TODO issue a token
        }
    }
    sqlite3_finalize(stmt);
    return jwt;
}

/// http_auth_is_authorized() - Check a user exists in the database and has a
/// valid web token.
bool
http_auth_is_authorized(
    database_s* db,
    struct mg_connection* c,
    struct http_message* m)
{
    jwt_t* jwt;
    struct mg_str token;
    int err = -1;
    char t[256];
    unsigned char secret[SECRET_LEN];
    const char* sub;
    int iat, exp;
    get_jwt(m, &token);
    if (token.len && token.len < sizeof(t)) {
        snprintf(t, sizeof(t), "%.*s", (uint32_t)token.len, token.p);
        get_secret((char*)secret);
        // Note we validate alg after decode as per
        // https://github.com/benmcollins/libjwt/issues/58
        err = jwt_decode(&jwt, t, secret, SECRET_LEN);
        if (!err && jwt) {
            err = jwt_get_alg(jwt) == JWT_ALG_HS256 ? 0 : -1;
            if (!err) err = check_token_access(db, jwt) ? 0 : -1;
            jwt_free(jwt);
        }
    }
    return err ? false : true;
}

/// http_auth_generate_uuid() - Generate a unique user id
void
http_auth_generate_uuid(char uuid[UUID_MAX_LEN])
{
    gen_uuid(uuid);
}

/// http_auth_generate_password_hash() - Populate hash and salt of a password
/// (Will erase the password after hash is generated)
int
http_auth_generate_password_hash(
    char hash[HASH_LEN],
    char salt[SALT_LEN],
    char* pass,
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
    memset(pass, 0, len);
    return 0;
}
