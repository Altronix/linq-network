#include "http_auth.h"
#include "jsmn/jsmn_tokens.h"
#include "log.h"
#include "sys.h"

#include "http_auth_unsafe.h"

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
    sys_uuid(dst);
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

/// token_ok() - Check that user exists in the database
static bool
token_ok(database_s* db, jsmn_token_decode_s* token)
{

    // NOTE other potential standard tokens are
    // nbf - not before
    // iss - issuer
    // aud - audience
    int err, iat, exp;
    jsmn_value sub;
    if (!jsmn_token_get_claim_int(token, "iat", &iat) &&
        !jsmn_token_get_claim_int(token, "exp", &exp) &&
        !jsmn_token_get_claim_str(token, "sub", &sub) &&
        sys_unix() < exp && // token not expired
        database_row_exists_mem(db, "users", "user", sub.p, sub.len)) {
        return true;
    } else {
        return false;
    }
}

/// http_auth_login() - Check the supplied credentials against the database and
/// issue a token
int
http_auth_login(
    database_s* db,
    const char* user,
    const char* pass,
    jsmn_token_encode_s* token)
{
    int err = -1, l;
    uint32_t now;
    char test[HASH_LEN];
    char concat[PASS_MAX_LEN + SALT_LEN];
    user_s u;

    err = database_user_open(db, &u, user);
    if (!err) {
        err = -1;
        l = snprintf(concat, sizeof(concat), "%s%s", pass, u.salt);
        if (l <= sizeof(concat)) {
            now = sys_unix();
            hash_256(concat, l, test);
            err = memcmp(u.pass, test, HASH_LEN);
            if (!err && !(err = jsmn_token_init(
                              token,
                              JSMN_ALG_HS256,
                              "{\"sub\":\"%s\",\"iat\":%d,\"exp\":\"%d\"}",
                              user,
                              now,
                              now + 600))) {
                char s[SECRET_LEN];
                get_secret(s);
                jsmn_token_sign(token, s, SECRET_LEN);
                memset(s, 0, SECRET_LEN);
            }
        }
        database_user_close(&u);
    }
    return err;
}

/// http_auth_is_authorized() - Check a user exists in the database and has a
/// valid web token.
bool
http_auth_is_authorized(
    database_s* db,
    struct mg_connection* c,
    struct http_message* m)
{
    jsmn_token_decode_s jwt;
    struct mg_str token;
    int err = -1;
    char key[SECRET_LEN];
    bool ret = false;
    get_jwt(m, &token);
    get_secret((char*)key);
    err = jsmn_token_decode(&jwt, key, JSMN_ALG_HS256, token.p, token.len);
    if (!err && token_ok(db, &jwt)) ret = true;
#ifdef DISABLE_PASSWORD
    // TODO TESTING needs refactor in order to support no password case.
    //      So when there is no password we still check the db for now.
    ret = true;
#else
    return ret;
#endif
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
