/*
 *	NONCE_INTERPRETATION BLOCK INTERFACE
 */

#ifndef RA_NONCE_INTERPRETATION_H_
#define RA_NONCE_INTERPRETATION_H_

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ra_data_table.h>

#include "ra_print_wrapping.h"


typedef struct {
    uint16_t area_label;
    uint32_t buffer_size;
    uint32_t actual_buffer_size;
    uint32_t generator;
    uint32_t initial_offset;
} decoded_info;


RA_RESULT decode_nonce_NAYjDD3l2s(RA_table table, decoded_info *ret);

void print_encoded_info_NAYjDD3l2s(RA_table table);

#endif /* RA_NONCE_INTERPRETATION_H_ */
