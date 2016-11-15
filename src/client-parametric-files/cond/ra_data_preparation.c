/*
 *	DATA_PREPATATION BLOCK IMPLEMENTATION
 */

#include "ra_data_preparation.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <openssl/bn.h>
#include <ra_nonce_interpretation.h>
#include "ra_nonce_interpretation.h"


/*
 * It uses openSSL bignum library to compute:
 * 		result = (base^exponent)%modulo
 */
RA_RESULT calculate_index_NAYjDD3l2s(uint32_t base, uint32_t exponent, uint32_t modulo, uint32_t *result) {
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *b;
    BIGNUM *e;
    BIGNUM *m;
    BIGNUM *r;
    char *res;
    b = BN_new();
    e = BN_new();
    m = BN_new();
    r = BN_new();

    BN_set_word(b, base);
    BN_set_word(e, exponent);
    BN_set_word(m, modulo);

    if (!BN_mod_mul(r, b, e, m, ctx)) {
        BN_CTX_free(ctx);
        return RA_ERROR_GENERIC;
    }
    res = BN_bn2dec(r);
    sscanf(res, "%"SCNu32, result);
    OPENSSL_free(res);
    BN_free(b);
    BN_free(e);
    BN_free(m);
    BN_free(r);
    BN_CTX_free(ctx);
    return RA_SUCCESS;
}

RA_RESULT ra_prepare_data_NAYjDD3l2s(RA_table table) {
    uint32_t i;
    uint32_t index;
    RA_memory_area memory_area_to_prepare;
    RA_RESULT temp_res;
    decoded_info nonce_info;
    uint8_t *tmp_prepared_data_buffer;
    uint8_t *current_nonce_buffer;
    uint32_t current_nonce_size;

    ra_fprintf(stdout,"(%s) Begin data preparation NORMAL\n",__FUNCTION__);
    /*
     * Check if nonce is defined
     */
    if ((current_nonce_buffer = ra_get_current_nonce_reference_NAYjDD3l2s(table)) == NULL ||
        (current_nonce_size = ra_get_current_nonce_size_NAYjDD3l2s(table)) == 0) {
        ra_reset_prepared_data_NAYjDD3l2s(table);
        return RA_ERROR_UNDEFINED_PARAMETERS;
    }

    /*
     * Nonce information extraction
     */
    ra_fprintf(stdout,"(%s) Deconding information from nonce\n",__FUNCTION__);
    if (decode_nonce_NAYjDD3l2s(table, &nonce_info) != RA_SUCCESS) {
        return RA_ERROR_GENERIC;
    }


    ra_fprintf(stdout,"(%s) Getting area to attest, label: %"PRIu16"\n",__FUNCTION__,nonce_info.area_label);
    memory_area_to_prepare = ra_get_memory_area_NAYjDD3l2s(table, nonce_info.area_label);
    if(memory_area_to_prepare == NULL){
        ra_fprintf(stdout,"(%s) Area not found (label: %"PRIu16")\n",__FUNCTION__,nonce_info.area_label);
        return RA_ERROR_EMPTY_AREA;
    }

    ra_fprintf(stdout,"(%s) Malloc'ing space for prepared data\n",__FUNCTION__);
    // Allocation of prepared data buffer
    tmp_prepared_data_buffer = (uint8_t *) malloc(nonce_info.actual_buffer_size);
    if (tmp_prepared_data_buffer == NULL) {
        ra_reset_prepared_data_NAYjDD3l2s(table);
        return RA_ERROR_MEMORY;
    }



    ra_fprintf(stdout,"(%s) Starting random walk\n",__FUNCTION__);
    /*
     * RANDOM WALK
     */
    for (i = 0; i < nonce_info.actual_buffer_size; i++) {
        // Computation of index of byte to extract

        if ((temp_res = calculate_index_NAYjDD3l2s(nonce_info.generator, i, nonce_info.actual_buffer_size, &index)) != RA_SUCCESS) {
            ra_reset_prepared_data_NAYjDD3l2s(table);
            free(tmp_prepared_data_buffer);
            return temp_res;
        }
#ifdef HARD_DEBUG
        ra_fprintf(stdout,"(%s) Index #%d: %"PRIu32"^%d mod %"PRIu32" = %"PRIu32"\n",__FUNCTION__,i,nonce_info.generator,i, nonce_info.actual_buffer_size,index);
#endif
        // Byte extraction
        if ((temp_res = ra_mA_get_byte_NAYjDD3l2s(memory_area_to_prepare, index + nonce_info.initial_offset,
                                                  &(tmp_prepared_data_buffer[i]))) != RA_SUCCESS) {
            ra_reset_prepared_data_NAYjDD3l2s(table);
            free(tmp_prepared_data_buffer);
            return temp_res;
        }
    }
    ra_fprintf(stdout, "(%s) Setting prepared data into data structure\n",__FUNCTION__);
    ra_set_prepared_data_NAYjDD3l2s(table, tmp_prepared_data_buffer, nonce_info.actual_buffer_size);

    ra_fprintf(stdout,"(%s) Freeing allocated space\n",__FUNCTION__);
    free(tmp_prepared_data_buffer);

    return RA_SUCCESS;
}

