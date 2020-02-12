#ifndef HTTP_AUTH_UNSAFE_H
#define HTTP_AUTH_UNSAFE_H

/*
Data generated from https://jwt.io (signed with "unsafe_secret")
{
    "alg": "HS256",
    "typ":"jwt"
}.{
    "sub":"unsafe_user",
    "iat":1580601600,
    "exp":1580602200
}.sig

{
    "alg": "HS256",
    "typ":"jwt"
}.{
    "sub":"unsafe_user",
    "iat":1580601600,
    "exp":1580602200
}.Base64(sig)

*/

// TODO note the case sensitive tests
#define UNSAFE_IAT 1580601600
#define UNSAFE_EXP 1580602200
#define UNSAFE_SECRET "unsafe_secret"
#define UNSAFE_USER "unsafe_user"
#define UNSAFE_UUID "user_id01234"
#define UNSAFE_SALT "0123456789ABCDEF"
#define UNSAFE_HASH                                                            \
    "392C3B8FCEEEDCDC47060BD17DB8E7ACD818D22BBAFF4CBBFBE134E8249DA4DC"
#define UNSAFE_PASS "unsafe_password"
#define UNSAFE_HEADER "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"
#define UNSAFE_PAYLOAD                                                         \
    "eyJzdWIiOiJ1bnNhZmVfdXNlciIsImlhdCI6MTU4MDYwMTYwMCwiZXhwIjoxNTgwNjAyMjAw" \
    "fQ"
#define UNSAFE_SIG "G9qxf3_PQ-H_uabAVeVnHb6lg06uJvC8Wli2m5gCIJQ"
#define UNSAFE_SIG_B64 "qyfM4kzDJ3Nmfw_lKQNqZ4ViCq32PVGmzBNoC17hQvw"
#define UNSAFE_TOKEN UNSAFE_HEADER "." UNSAFE_PAYLOAD "." UNSAFE_SIG

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
#endif
