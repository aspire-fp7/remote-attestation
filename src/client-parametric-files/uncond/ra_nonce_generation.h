/*
 * NONCE_GENERATION BLOCK INTERFACE
 */

#ifndef RA_NONCE_GENERATION_H_
#define RA_NONCE_GENERATION_H_

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ra_results.h>

#include "ra_print_wrapping.h"


RA_RESULT 	random_generate_nonce_NAYjDD3l2s(uint8_t *generated_nonce, uint32_t required_size);
#endif /* RA_NONCE_GENERATION_H_ */
