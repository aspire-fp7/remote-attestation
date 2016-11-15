/*
 * MEMORY BLOCK INTERFACE
 */

#ifndef RA_MEMORY_H_
#define RA_MEMORY_H_

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ra_results.h>

#include "ra_print_wrapping.h"


typedef struct memory_area_t* RA_memory_area;
typedef struct memory_block_t* RA_memory_block;

extern uint64_t base_address_NAYjDD3l2s;

/*
 *  BLOCKS API, nearly private
 */
RA_memory_block ra_mB_create_empty_NAYjDD3l2s();
RA_memory_block ra_mB_create_NAYjDD3l2s(uint64_t offset, uint32_t size);
void 			ra_mB_destroy_NAYjDD3l2s(RA_memory_block block);
RA_RESULT 		ra_mB_get_buffer_NAYjDD3l2s(RA_memory_block block, uint8_t *buffer);
uint8_t 		ra_mB_get_byte_NAYjDD3l2s(RA_memory_block block, uint32_t index);
void 			ra_mB_print_info_NAYjDD3l2s(RA_memory_block block);
/********************************************************************************/

/*
 * AREAS API
 */
RA_memory_area 	ra_mA_create_empty_NAYjDD3l2s();
RA_memory_area 	ra_mA_create_NAYjDD3l2s(uint16_t label);
void 			ra_mA_destroy_NAYjDD3l2s(RA_memory_area area);
bool 			ra_mA_is_empty_NAYjDD3l2s(RA_memory_area area);

/* addictional functions, implementation dipendent*/
RA_RESULT 		ra_mA_insert_block_NAYjDD3l2s(RA_memory_area area, RA_memory_block block);
RA_RESULT 		ra_mA_insert_new_block_NAYjDD3l2s(RA_memory_area area, uint64_t offset, uint32_t size);
RA_RESULT 		ra_mA_remove_block_NAYjDD3l2s(RA_memory_area area, RA_memory_block block);

RA_RESULT 		ra_mA_get_buffer_NAYjDD3l2s(RA_memory_area area, uint8_t *buffer);

/* addictional function, implementation dipendent*/
RA_RESULT 		ra_mA_get_block_NAYjDD3l2s(RA_memory_area area, uint32_t byte_index, RA_memory_block *return_block);

RA_RESULT 		ra_mA_get_byte_NAYjDD3l2s(RA_memory_area area, uint32_t byte_index, uint8_t *return_byte);
uint16_t		ra_mA_get_label_NAYjDD3l2s(RA_memory_area area);
uint32_t 		ra_mA_get_total_size_NAYjDD3l2s(RA_memory_area area);

void			ra_mA_print_info_NAYjDD3l2s(RA_memory_area area);

/* Dumps area info on file fd at the current position, fd must be already opened  */
RA_RESULT 		ra_mA_dump_info_binary_NAYjDD3l2s(RA_memory_area area, int fd);
/* Parse area info from the current position of file fd, fd must be already opened  */
RA_memory_area 	ra_mA_parse_ads_text_info_NAYjDD3l2s(int fd);
RA_memory_area 	ra_mA_parse_ads_binary_info_NAYjDD3l2s(uint8_t **pointer);

#endif /* RA_MEMORY_H_ */
