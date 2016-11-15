#include "verifiers_list.h"
#include <stdlib.h>
#include "ra_print_wrapping.h"


typedef struct verifier* verifier_ptr;
typedef struct verifier {
	uint64_t id;
	uint32_t client_count;
	pid_t pid;
	verifier_ptr next;
} verifier_t;

struct verifiers_t {
	uint32_t verifiers_count;
	verifier_ptr head;
};

/***************************** PRIVATE FUNCTIONS *************************************/
/*
 * Same as RA_VER_insert but, for internal (parse) purpose, it takes also the value of the count to
 * be set for the new verifier to be inserted
 */
RA_RESULT insert_with_count(RA_verifiers_list verifiers, uint64_t id, pid_t pid, uint32_t count) {

	verifier_ptr iterator;

	for (iterator = verifiers->head; iterator != NULL && iterator->id != id; iterator = iterator->next)
		;

	if (iterator != NULL) return RA_ERROR_VERIFIER_ALREADY_PRESENT;

	verifier_ptr verifier = (verifier_ptr) malloc(sizeof(verifier_t));
	if (verifier == NULL) {
		return RA_ERROR_MEMORY;
	}

	verifier->client_count = count;
	verifier->id = id;
	verifier->next = verifiers->head;
	verifier->pid = pid;
	verifiers->head = verifier;
	verifiers->verifiers_count++;
	return RA_SUCCESS;
}

/*************************************************************************************/

RA_verifiers_list RA_VER_create() {
	RA_verifiers_list ret;

	if ((ret = (RA_verifiers_list) malloc(sizeof(struct verifiers_t))) == NULL) {
		return NULL;
	}

	ret->head = NULL;
	ret->verifiers_count = 0;
	return ret;

}

RA_verifiers_list RA_VER_parse(const char* filename) {

	RA_verifiers_list list = RA_VER_create();
	FILE* fp = fopen(filename, "r");
	uint64_t tmp_app_id;
	uint32_t tmp_count;
	pid_t tmp_pid;

	while (fscanf(fp, "%"SCNu64" : { %d , %"SCNu32"}", &tmp_app_id, &tmp_pid, &tmp_count) != EOF) {
		insert_with_count(list, tmp_app_id, tmp_pid, tmp_count);
	}
	fclose(fp);
	return list;
}

void RA_VER_serialize(RA_verifiers_list verifiers, const char* filename) {
	FILE* fp = fopen(filename, "w");
	verifier_ptr iterator;

	for (iterator = verifiers->head; iterator != NULL; iterator = iterator->next) {
		fprintf(fp, "%"PRIu64" : { %d , %"PRIu32" }", iterator->id, iterator->pid, iterator->client_count);
	}

	fclose(fp);
}

uint32_t RA_VER_count(RA_verifiers_list verifiers) {
	return verifiers->verifiers_count;
}

/* Assumes that the added verifier had just been instantiated and
 * it is already serving 1 client
 */
RA_RESULT RA_VER_insert(RA_verifiers_list verifiers, uint64_t id, pid_t pid) {

	verifier_ptr iterator;

	for (iterator = verifiers->head; iterator != NULL && iterator->id != id; iterator = iterator->next)
		;

	if (iterator != NULL) return RA_ERROR_VERIFIER_ALREADY_PRESENT;

	verifier_ptr verifier = (verifier_ptr) malloc(sizeof(verifier_t));
	if (verifier == NULL) {
		return RA_ERROR_MEMORY;
	}

	verifier->client_count = 1;
	verifier->id = id;
	verifier->next = verifiers->head;
	verifier->pid = pid;
	verifiers->head = verifier;
	verifiers->verifiers_count++;
	return RA_SUCCESS;
}

RA_RESULT RA_VER_update(RA_verifiers_list verifiers, uint64_t id) {

	verifier_ptr iterator;

	for (iterator = verifiers->head; iterator != NULL && iterator->id != id; iterator = iterator->next)
		;

	if (iterator != NULL) {
		iterator->client_count++;
		return RA_SUCCESS;
	} else return RA_ERROR_VERIFIER_NOT_FOUND;
}

int RA_VER_is_running(RA_verifiers_list verifiers, uint64_t id) {
	verifier_ptr iterator;

	for (iterator = verifiers->head; iterator != NULL && iterator->id != id; iterator = iterator->next)
		;

	return iterator != NULL ? iterator->client_count : 0;
}

pid_t RA_VER_get_pid(RA_verifiers_list verifiers, uint64_t id) {
	verifier_ptr iterator;

	for (iterator = verifiers->head; iterator != NULL && iterator->id != id; iterator = iterator->next)
		;

	if (iterator != NULL) return iterator->pid;
	else return -1;
}

pid_t RA_VER_remove(RA_verifiers_list verifiers, uint64_t id) {
	verifier_ptr iterator, tmp;

	pid_t ret;

	if (verifiers->head == NULL || verifiers->verifiers_count == 0) return -1;

	if (verifiers->head != NULL && verifiers->head->id == id) {
		tmp = verifiers->head;
		verifiers->head = tmp->next;
		ret = tmp->pid;
		verifiers->verifiers_count--;
		free(tmp);
		return ret;
	}
	for (iterator = verifiers->head; iterator->next != NULL && iterator->next->id != id; iterator = iterator->next)
		;

	if (iterator->next != NULL) {
		tmp = iterator->next;
		iterator->next = tmp->next;
		ret = tmp->pid;
		verifiers->verifiers_count--;
		free(tmp);
		return ret;
	} else return -1;
}

RA_RESULT RA_VER_dec_clients(RA_verifiers_list verifiers, uint64_t id, pid_t *process_to_be_killed) {
	verifier_ptr iterator, tmp;

	*process_to_be_killed = -1;

	if (verifiers->head == NULL || verifiers->verifiers_count == 0) return RA_ERROR_VERIFIER_NOT_FOUND;

	if (verifiers->head != NULL && verifiers->head->id == id) {
		if ((--(verifiers->head->client_count)) == 0) {
			tmp = verifiers->head;
			verifiers->head = tmp->next;
			*process_to_be_killed = tmp->pid;
			verifiers->verifiers_count--;
			free(tmp);
		}
		return RA_SUCCESS;
	}
	for (iterator = verifiers->head; iterator->next != NULL && iterator->next->id != id; iterator = iterator->next)
		;

	if (iterator->next != NULL) {
		if ((--(iterator->next->client_count)) == 0) {
			tmp = iterator->next;
			iterator->next = tmp->next;
			*process_to_be_killed = tmp->pid;
			verifiers->verifiers_count--;
			free(tmp);
		}
		return RA_SUCCESS;
	} else return RA_ERROR_VERIFIER_NOT_FOUND;
}

void RA_VER_print_info(RA_verifiers_list list) {

	fprintf(stdout, "------Verifiers info-----------\n");
	fprintf(stdout, "%"PRIu32" verifiers defined\n", list->verifiers_count);
	for (verifier_ptr iterator = list->head; iterator != NULL; iterator = iterator->next) {
		fprintf(stdout, "%"PRIu64") pid: %d serving:%"PRIu32" clients\n", iterator->id, iterator->pid, iterator->client_count);
	}
	fprintf(stdout, "-------------------------------\n");
}
