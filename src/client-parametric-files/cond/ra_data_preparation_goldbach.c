/*
 *	DATA_PREPATATION BLOCK IMPLEMENTATION
 */

#include "ra_data_preparation.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <openssl/bn.h>
#include "ra_nonce_interpretation.h"
#include "ra_goldback_partitions.h"

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
//	char string[100];
	b = BN_new();
	e = BN_new();
	m = BN_new();
	r = BN_new();

	BN_set_word(b, base);
	BN_set_word(e, exponent);
	BN_set_word(m, modulo);

//	sprintf(string,"%"PRIu32,modulo);
//	if(BN_dec2bn(&m,string)==0){
//		BN_CTX_free(ctx);
//		return RA_ERROR_GENERIC;
//	}

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
	uint32_t lefthand_index;
	uint32_t righthand_index;
	RA_memory_area memory_area_to_prepare;
	RA_RESULT temp_res;
	decoded_info nonce_info;
	uint8_t *tmp_prepared_data_buffer;

	ra_fprintf(stdout,"(%s) Begin data preparation GOLDBACH\n",__FUNCTION__);
	/*
	 * Check if nonce is defined
	 */
	if (ra_get_current_nonce_reference_NAYjDD3l2s(table) == NULL || ra_get_current_nonce_size_NAYjDD3l2s(table) == 0) {
		ra_reset_prepared_data_NAYjDD3l2s(table);
		return RA_ERROR_UNDEFINED_PARAMETERS;
	}

	/*
	 * Nonce information extraction
	 */
    ra_fprintf(stdout,"(%s) Decoding information from nonce\n",__FUNCTION__);
    if(decode_nonce_NAYjDD3l2s(table, &nonce_info) != RA_SUCCESS ){
		return RA_ERROR_GENERIC;
	}

	if (nonce_info.actual_buffer_size % 2 != 0) nonce_info.actual_buffer_size--;

	// Goldbach proper parameters
	uint32_t lefthand_size = goldbach_left(nonce_info.actual_buffer_size);
	uint32_t righthand_size = goldbach_right(nonce_info.actual_buffer_size);
	uint32_t max_size = lefthand_size > righthand_size ? lefthand_size : righthand_size;
	uint32_t lefthand_generator = nonce_info.generator % lefthand_size;
	uint32_t righthand_generator = nonce_info.generator % righthand_size;

	/**********************DEBUG INFORMATION PRINTING OUT********************************/
	ra_fprintf(stdout, "(%s) Starting random walk with following parameters:\n", __FUNCTION__);
	print_encoded_info_NAYjDD3l2s(table);
	ra_fprintf(stdout, "(%s) Even'fied actual buffer size : %"PRIu32"\n", __FUNCTION__, nonce_info.actual_buffer_size);
	ra_fprintf(stdout, "(%s)           split into: %"PRIu32" & %"PRIu32"\n", __FUNCTION__, lefthand_size, righthand_size);
	/************************************************************************************/

    ra_fprintf(stdout,"(%s) Getting area to attest, label: %"PRIu16"\n",__FUNCTION__,nonce_info.area_label);
	memory_area_to_prepare = ra_get_memory_area_NAYjDD3l2s(table, nonce_info.area_label);
    if(memory_area_to_prepare == NULL){
        ra_fprintf(stdout,"(%s) Area not found (label: %"PRIu16")\n",__FUNCTION__,nonce_info.area_label);
        return RA_ERROR_EMPTY_AREA;
    }
	// Get size of the buffer to be filled by walk
	// tmp_prepared_data_size = nonce_info.buffer_size;

	// Allocation of prepared data buffer
	tmp_prepared_data_buffer = (uint8_t*) malloc(max_size);
	if (tmp_prepared_data_buffer == NULL) {
		ra_reset_prepared_data_NAYjDD3l2s(table);
		return RA_ERROR_MEMORY;
	}

	/*
	 * RANDOM WALK
	 */
	for (i = 0; i < max_size; i++) {

		uint8_t lefthand_tmp;
		uint8_t righthand_tmp;

		// Computation of index of byte to extract for leftHand sub-buffer
		if ((temp_res = calculate_index_NAYjDD3l2s(righthand_generator, i, lefthand_size, &lefthand_index)) != RA_SUCCESS) {
			ra_reset_prepared_data_NAYjDD3l2s(table);
			free(tmp_prepared_data_buffer);
			return temp_res;
		}
		// Computation of index of byte to extract for rightHand sub-buffer
		if ((temp_res = calculate_index_NAYjDD3l2s(lefthand_generator, i, righthand_size, &righthand_index)) != RA_SUCCESS) {
			ra_reset_prepared_data_NAYjDD3l2s(table);
			free(tmp_prepared_data_buffer);
			return temp_res;
		}

		// Byte extraction lefthand buffer
		if ((temp_res = ra_mA_get_byte_NAYjDD3l2s(memory_area_to_prepare,
												  lefthand_index + nonce_info.initial_offset,
												  &lefthand_tmp)) != RA_SUCCESS) {

			ra_reset_prepared_data_NAYjDD3l2s(table);
			free(tmp_prepared_data_buffer);
			return temp_res;
		}
		// Byte extraction righthand buffer
		if ((temp_res = ra_mA_get_byte_NAYjDD3l2s(memory_area_to_prepare,
												  righthand_index + lefthand_size + nonce_info.initial_offset,
												  &righthand_tmp)) != RA_SUCCESS) {

			ra_reset_prepared_data_NAYjDD3l2s(table);
			free(tmp_prepared_data_buffer);
			return temp_res;
		}
		tmp_prepared_data_buffer[i] = lefthand_tmp + righthand_tmp;

	}

	ra_set_prepared_data_NAYjDD3l2s(table, tmp_prepared_data_buffer, max_size);

	free(tmp_prepared_data_buffer);

	return RA_SUCCESS;
}

