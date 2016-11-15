
#include "generic_functions.h"
#include <openssl/bn.h>

/*
 * Private functions
 */
RA_RESULT closest_prime(uint32_t n, uint32_t* out) {

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *big = BN_new();
    BIGNUM *one = BN_new();
    char string[100];
    char *res;

    BN_one(one);
    sprintf(string, "%"PRIu32, n);

    if (n < 2 || BN_dec2bn(&big, string) == 0) return RA_ERROR_GENERIC;


    while (1) {
        if (BN_is_prime_ex(big, BN_prime_checks, ctx, NULL) == 1) {
            res = BN_bn2dec(big);
            sscanf(res, "%"SCNu32, &n);
            break;
        }
        BN_sub(big, big, one);
    }
    BN_CTX_free(ctx);
    *out = n;
    return RA_SUCCESS;
}
