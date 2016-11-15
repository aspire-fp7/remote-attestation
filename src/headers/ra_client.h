/*
 * ra_client.h
 *
 *  Created on: Jul 4, 2015
 *      Author: alessio
 */

#ifndef RA_CLIENT_H_
#define RA_CLIENT_H_

#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

#include <ra_results.h>
#include <ra_defines.h>

#include "ra_print_wrapping.h"



typedef struct ra_client_t* RA_client_t;

typedef int SOCKET;

size_t 		RA_CLI_element_size();

RA_client_t RA_CLI_create(uint8_t *AID,
						  uint64_t attestator_number,
						  uint64_t attestator_id,
						  uint32_t sleep_avg_seconds,
						  uint32_t sleep_var_seconds,
						  uint64_t session_id,
						  SOCKET connection);

/*
 * returns:
 * 		 1 if first > second
 * 		 0 if first = second
 * 		-1 if first < second
 */
int		 	RA_CLI_compare(void* first, void* second);


/* getters */
uint8_t *	RA_CLI_get_AID_reference(RA_client_t client);
RA_RESULT	RA_CLI_get_AID_as_string(RA_client_t client, char *AID_string);
uint64_t 	RA_CLI_get_attestator_number(RA_client_t client);
uint64_t 	RA_CLI_get_attestator_id(RA_client_t client);
uint32_t 	RA_CLI_get_sleep_avg_seconds(RA_client_t client);
uint32_t 	RA_CLI_get_sleep_var_seconds(RA_client_t client);
uint64_t 	RA_CLI_get_session_id(RA_client_t client);

/* setters */
void 		RA_CLI_set_AID(RA_client_t client, uint8_t *AID);
void 		RA_CLI_set_attestator_number(RA_client_t client, uint64_t data);
void 		RA_CLI_set_attestator_id(RA_client_t client, uint64_t product_id);
void		RA_CLI_set_sleep_avg_seconds(RA_client_t client, uint32_t data);
void		RA_CLI_set_sleep_var_seconds(RA_client_t client, uint32_t data);
void 		RA_CLI_set_session_id(RA_client_t client, uint64_t session);

void 		RA_CLI_destroy(void* client);

void 		RA_CLI_print_info(void* client);

#endif /* RA_CLIENT_H_ */
