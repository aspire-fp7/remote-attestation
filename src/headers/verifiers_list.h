/*
 * manager_support.h
 *
 *  Created on: Jul 4, 2015
 *      Author: alessio
 */

#ifndef VERIFIERS_LIST_H_
#define VERIFIERS_LIST_H_

#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include "ra_results.h"


typedef struct verifiers_t* RA_verifiers_list;

RA_verifiers_list 	RA_VER_create();

RA_verifiers_list 	RA_VER_parse(const char* filename);
void 				RA_VER_serialize(RA_verifiers_list verifiers, const char* filename);


uint32_t 			RA_VER_count(RA_verifiers_list verifiers);
/* Assumes that the added verifier had just been instantiated and
 * it is already serving 1 client
 */
RA_RESULT 			RA_VER_insert(RA_verifiers_list verifiers, uint64_t id, pid_t pid);
RA_RESULT 			RA_VER_update(RA_verifiers_list verifiers, uint64_t id);
int 				RA_VER_is_running(RA_verifiers_list verifiers, uint64_t id);
pid_t 				RA_VER_get_pid(RA_verifiers_list verifiers, uint64_t id);
pid_t 				RA_VER_remove(RA_verifiers_list verifiers, uint64_t id);
RA_RESULT 			RA_VER_dec_clients(RA_verifiers_list verifiers, uint64_t id, pid_t *process_to_be_killed);



void 				RA_VER_print_info(RA_verifiers_list list);
#endif /* VERIFIERS_LIST_H_ */
