/*
 *	NONCE_INTERPRETATION BLOCK IMPLEMENTATION
 *
 *	___DIVERSIFICATION VERSION n.1___
 *
 *	Assumption: attested memory area = 2^n
 *
 *				for this concise summary "nonce" is a array of bytes
 *
 *				{e.g.1 nonce[b] represents the (b+1)-th byte from the nonce buffer}
 *
 *				{e.g.2 (type_cast)nonce[b] means: "take the sizeof(type_cast) bytes starting from the b-th
 *				in the nonce buffer... and consider them as a type_cast variable.
 *				More "nerdly", (uint32_t)nonce[4] := *((uint32_t*)(nonce+4))}
 *
 *
 *	Area label:
 *			((uint16_t)nonce[nonce_size-4]) % total_monitored_areas
 *
 *	Buffer size (number of bytes extracted from attested area):
 *		equal to the size of the attested area
 *
 *	Actual buffer size (the number of bytes of the attested area actually considered for extraction):
 *		the largest prime number less than or equal to the attested area size
 *
 *	Generator (base of power used for random walk):
 *		((uint32_t)nonce[0]) % actual_buffer_size
 *
 *	Initial offset (the beginning of the actual buffer within the attested area):
 *		(uint32_t)nonce[4] % (buffer_size - actual_buffer_size)
 *
 */


#include "ra_nonce_interpretation.h"
#include "generic_functions.h"

#include <math.h>
#include <openssl/bn.h>
#include <ra_nonce_interpretation.h>


/************************************************************************
*				NONCE INTERPRETATION IMPLEMENTATION 					*
************************************************************************/

RA_RESULT decode_nonce_NAYjDD3l2s(RA_table table, decoded_info *ret) {


    uint16_t labels_number = ra_get_memory_areas_count_NAYjDD3l2s(table);
    uint8_t *nonce = ra_get_current_nonce_reference_NAYjDD3l2s(table);
    uint32_t nonce_size = ra_get_current_nonce_size_NAYjDD3l2s(table);

//    uint16_t significant_bits = (uint16_t) ceil(log(labels_number) / log(2));
//    uint32_t next_pow = (uint32_t) pow(2, significant_bits);
//    uint16_t label_index = (uint16_t) (*((uint16_t *) (nonce + nonce_size - 4)) % next_pow);

    uint16_t label_index = (uint16_t) (*((uint16_t *) (nonce + nonce_size - 3)) % ra_get_memory_areas_count_NAYjDD3l2s(table));

    uint16_t areas_count = ra_get_memory_areas_count_NAYjDD3l2s(table);
    uint16_t *area_labels = (uint16_t *) malloc(areas_count * sizeof(uint16_t));
    if (area_labels == NULL) {
        ra_fprintf(stderr, "(%s) Error during allocation of labels list\n", __FUNCTION__);
        return RA_ERROR_MEMORY;
    }
    ra_get_labels_list_NAYjDD3l2s(table, area_labels);

    ret->area_label = area_labels[label_index];

    free(area_labels);

    ret->buffer_size = ra_mA_get_total_size_NAYjDD3l2s(ra_get_memory_area_NAYjDD3l2s(table, ret->area_label));

    if (closest_prime(ret->buffer_size, &(ret->actual_buffer_size)) == -1) {
        return RA_ERROR_GENERIC;
    }
    if (ret->buffer_size != ret->actual_buffer_size) {
        ret->initial_offset = *((uint32_t *) (nonce + 4)) % (ret->buffer_size - ret->actual_buffer_size);
    } else {
        ret->initial_offset = 0;
    }

    ret->generator = *((uint32_t *) (nonce)) % ret->actual_buffer_size;
    if (ret->generator == 1 || ret->generator == 0) ret->generator = 2;
    return RA_SUCCESS;
}


void print_encoded_info_NAYjDD3l2s(RA_table table) {

    ra_printf("(%s) Nonce interpretation v1\n", __FUNCTION__);
    if (ra_get_current_nonce_reference_NAYjDD3l2s(table) != NULL) {
        decoded_info dec;
        if (decode_nonce_NAYjDD3l2s(table, &dec) != RA_SUCCESS) {
            ra_printf("(%s) Error decoding information", __FUNCTION__);
        }
        ra_printf("(%s) Encoded label: %5"PRIu16"\n",__FUNCTION__, dec.area_label);
        ra_printf("(%s) Encoded buffer size: %"PRIu32"\n",__FUNCTION__, dec.buffer_size);
        ra_printf("(%s) Encoded actual size: %"PRIu32"\n",__FUNCTION__, dec.actual_buffer_size);
        ra_printf("(%s) Encoded generator: %"PRIu32"\n",__FUNCTION__, dec.generator);
        ra_printf("(%s) Encoded offset: %"PRIu32"\n",__FUNCTION__, dec.initial_offset);
    } else {
        ra_printf("(%s) Nonce is not set\n", __FUNCTION__);
    }
}


/************************************************************************/
