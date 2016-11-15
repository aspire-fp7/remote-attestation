/*
 *	DATA_TABLE INTERFACE
 */

#ifndef RA_DATA_TABLE_H_
#define RA_DATA_TABLE_H_

#include <inttypes.h>
#include <stdint.h>

#include "ra_memory.h"
#include "ra_results.h"

#include "ra_print_wrapping.h"


typedef struct ra_table_t* RA_table;

extern RA_table remote_attestation_data_table_NAYjDD3l2s;

extern uint64_t data_structure_blob_NAYjDD3l2s;

RA_table 		ra_create_table_NAYjDD3l2s();
void 			ra_destroy_table_NAYjDD3l2s(RA_table table);

RA_RESULT 		ra_set_AID_NAYjDD3l2s(RA_table table, uint8_t *AID);
RA_RESULT 		ra_get_AID_reference_NAYjDD3l2s(RA_table table, uint8_t **AID);
RA_RESULT      ra_get_AID_as_string_NAYjDD3l2s(RA_table table, char *AID);
RA_RESULT 		ra_set_attestator_number_NAYjDD3l2s(RA_table table, uint64_t attestator_number);
RA_RESULT 		ra_get_attestator_number_NAYjDD3l2s(RA_table table, uint64_t *attestator_number);

RA_RESULT 		ra_set_current_nonce_NAYjDD3l2s(RA_table table, uint8_t *nonce, uint32_t size);
RA_RESULT 		ra_get_current_nonce_NAYjDD3l2s(RA_table table, uint8_t *nonce);
uint8_t*        ra_get_current_nonce_reference_NAYjDD3l2s(RA_table table);
uint32_t 		ra_get_current_nonce_size_NAYjDD3l2s(RA_table table);

RA_RESULT 		ra_set_current_key_NAYjDD3l2s(RA_table table, uint8_t *key, uint8_t size);
RA_RESULT 		ra_get_current_key_NAYjDD3l2s(RA_table table, uint8_t *key);
uint8_t*        ra_get_current_key_reference_NAYjDD3l2s(RA_table table);
uint8_t 		ra_get_current_key_size_NAYjDD3l2s(RA_table table);


uint16_t 		ra_get_memory_areas_count_NAYjDD3l2s(RA_table table);

RA_RESULT 		ra_insert_memory_area_NAYjDD3l2s(RA_table table, RA_memory_area area);
RA_RESULT 		ra_remove_memory_area_NAYjDD3l2s(RA_table table, uint16_t label);
RA_memory_area 	ra_get_memory_area_NAYjDD3l2s(RA_table table, uint16_t label);
RA_memory_area 	ra_get_whole_code_memory_area_NAYjDD3l2s(RA_table table);


void 			ra_print_table_info_NAYjDD3l2s(RA_table table) ;
RA_RESULT 		ra_dump_table_info_NAYjDD3l2s(RA_table table, const char *out_file_name);
RA_table 		ra_parse_binary_ads_NAYjDD3l2s();
RA_RESULT 		ra_get_labels_list_NAYjDD3l2s(RA_table table, uint16_t *list);


RA_RESULT 		ra_set_prepared_data_NAYjDD3l2s(RA_table table, uint8_t *buffer, uint32_t size);
uint32_t		ra_get_prepared_data_size_NAYjDD3l2s(RA_table table);
void			ra_reset_prepared_data_NAYjDD3l2s(RA_table table);
uint8_t*        ra_get_prepared_data_reference_NAYjDD3l2s(RA_table table);
RA_RESULT		ra_get_prepared_data_NAYjDD3l2s(RA_table table, uint8_t *buffer);

#endif /* RA_DATA_TABLE_H_ */
