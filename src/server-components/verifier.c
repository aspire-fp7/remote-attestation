/*
 * VERIFIER process implementation
 */

#include "ra_print_wrapping.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "ra_do_hash.h"
#include <time.h>
#include "ra_database.h"



#define INPUT_STREAM_FD				0

int main(int argc, char **argv) {

	uint8_t attestation_header_buffer[VERIFIER_RESP_HEADER_SIZE];
	uint8_t received_app_AID[AID_SIZE];
	uint64_t received_app_attestator_number;
	uint64_t received_request_id;
	uint8_t *received_hash_buffer;
	time_t received_response_time;
	uint32_t received_hash_buffer_size;
	ssize_t actual_received_hash_buffer_size;

	uint8_t* computed_hash_buffer;
	uint32_t computed_hash_buffer_size;

	uint8_t *nonce_buffer_from_db;
	uint32_t nonce_buffer_size_from_db;
	uint8_t* data_buffer_from_db;
	uint32_t data_buffer_size_from_db;
	time_t send_time_from_db;
	request_status_t status_from_db;
	request_status_t status_to_db;
	uint16_t validity_time_sec_from_db;
    uint64_t  application_id_from_db;
    
    
	ssize_t general_tmp_var;
	RA_RESULT return_res;

	FILE *verifier_log_file = fopen(VERIFIER_LOG_PATH, "a");


	time_t  time1 = time(NULL);
	fprintf(verifier_log_file, "(Verifier) Started at %s\n", ctime(&time1));

#define TIME_EVALUATION
#ifdef TIME_EVALUATION
	float t0, t1;
	t0 = ((float) clock()) / CLOCKS_PER_SEC;
#endif
	/** Receive HEADER **/
	if ((general_tmp_var = read(INPUT_STREAM_FD, attestation_header_buffer, VERIFIER_RESP_HEADER_SIZE)) ==
        VERIFIER_RESP_HEADER_SIZE) {
		received_response_time = *(time_t*) attestation_header_buffer;
		for (int i = 0; i < AID_SIZE; ++i) {
			received_app_AID[i] = (attestation_header_buffer + RESP_TIME_SIZE)[i];
		}
		received_app_attestator_number = *(uint64_t*) (attestation_header_buffer + RESP_TIME_SIZE + AID_SIZE);
		received_request_id = *(uint64_t*) (attestation_header_buffer + RESP_TIME_SIZE + AID_SIZE +
											RESP_ATTESTATOR_NO_SIZE);
		received_hash_buffer_size = *(uint32_t*) (attestation_header_buffer + RESP_TIME_SIZE + AID_SIZE +
												  RESP_ATTESTATOR_NO_SIZE + RESP_REQUEST_ID_SIZE);

        char received_AID_string[AID_SIZE * 2 + 1];
        for (int i = 0; i < AID_SIZE; i++) {
            if (snprintf(received_AID_string + i * 2, 3, "%02"PRIX8, received_app_AID[i]) != 2)
                return RA_ERROR_MEMORY;
        }

        ra_db_get_application_id(received_AID_string, &application_id_from_db);



		fprintf(verifier_log_file, "(Verifier) Received %zuB header data...\n", general_tmp_var);
		for (int c = 0; time1 < general_tmp_var; time1++) {
			fprintf(verifier_log_file, "%02X", (unsigned char) (*((int8_t*) (attestation_header_buffer + time1))));
			if (time1 % 16 == 15) fprintf(verifier_log_file, "\n");
		}
		fprintf(verifier_log_file, "\n");
		fprintf(verifier_log_file, "(Verifier) --- Received response information ---\n");
		fprintf(verifier_log_file, "(Verifier) --- R time: %32"PRIu64"               \n", received_response_time);
		fprintf(verifier_log_file, "(Verifier) --- AID:    ");
		for (int i = 0; i < AID_SIZE; ++i) 	fprintf(verifier_log_file, "%02"PRIX8, received_app_AID[i]);
		fprintf(verifier_log_file, "\n");
		fprintf(verifier_log_file, "(Verifier) --- Attestator number: %20"PRIu64"    \n", received_app_attestator_number);
		fprintf(verifier_log_file, "(Verifier) ----Req id: %32"PRIu64"               \n", received_request_id);
		fprintf(verifier_log_file, "(Verifier) ----H size: %32"PRIu32"               \n", received_hash_buffer_size);
		fprintf(verifier_log_file, "(Verifier) --- ------------------------------ ---\n");
		/** Receive hash buffer of size declared in header **/
		if ((received_hash_buffer = (uint8_t*) malloc(received_hash_buffer_size)) != NULL) {
			if ((actual_received_hash_buffer_size = read(INPUT_STREAM_FD, received_hash_buffer, received_hash_buffer_size)) == received_hash_buffer_size) {
				/** Match identifiers command-line-defined | received **/
				/** Extract corresponding prepared data **/
				if ((return_res = ra_db_get_data_to_verify(received_request_id,
						&nonce_buffer_from_db,
						&nonce_buffer_size_from_db,
						&data_buffer_from_db,
						&data_buffer_size_from_db,
						&send_time_from_db,
						&status_from_db,
						&validity_time_sec_from_db)) == RA_SUCCESS) {

#ifdef DEBUG
					/*** Verbose output ***/
					fprintf(verifier_log_file, "(Verifier) _Nonce:\n");
					for (int p = 0; p < nonce_buffer_size_from_db; p++) {
						fprintf(verifier_log_file, "%02X", (unsigned char) (*((int8_t*) (nonce_buffer_from_db + p))));
					}
					fprintf(verifier_log_file, "\n");
					fprintf(verifier_log_file, "(Verifier) Data size %"PRIu32"\n", data_buffer_size_from_db);

					int p;
					for (p = 0; p < data_buffer_size_from_db; p++) {
						fprintf(verifier_log_file, "%02X", (unsigned char) (*((int8_t*) (data_buffer_from_db + p))));
						if (p % 16 == 15) fprintf(verifier_log_file, "\n");
					}
					fprintf(verifier_log_file, "\n");
					/**********************/
#endif

					/** Generate hash from stored data to be compared with received one **/
					if (ra_do_hash_NAYjDD3l2s(nonce_buffer_from_db,
											  nonce_buffer_size_from_db,
											  data_buffer_from_db,
											  data_buffer_size_from_db,
											  received_app_AID, received_app_attestator_number, NULL, 0,
											  &computed_hash_buffer,
											  &computed_hash_buffer_size) == RA_SUCCESS) {
						/*** Hashes comparison ***/
						if (memcmp(computed_hash_buffer, received_hash_buffer, received_hash_buffer_size) == 0) {
							/* Right response */

							if (difftime(received_response_time, send_time_from_db) < validity_time_sec_from_db) {
								/** in time **/
								status_to_db = SUCCESS;
							} else {
								/** out of time **/
								status_to_db = EXPIRED_SUCCESS;
							}
						} else {
							/* Wrong response */
							if (difftime(received_response_time, send_time_from_db) < validity_time_sec_from_db) {
								/** in time **/
								status_to_db = FAILED;
							} else {
								/** out of time **/
								status_to_db = EXPIRED_FAILED;
							}
                            ra_db_set_reaction_status(application_id_from_db, REACTION_STATUS_COMPROMISED);
						}
						/** Complete request on DB **/
						ra_db_finalize_request(received_request_id, status_to_db, (uint32_t) (received_response_time - send_time_from_db));
						free(computed_hash_buffer);
						free(data_buffer_from_db);
						fprintf(verifier_log_file, "(Verifier) Response verification result: %s\n", request_status_to_string(status_to_db));

#ifdef TIME_EVALUATION
						t1 = ((float) clock()) / CLOCKS_PER_SEC;
						fprintf(verifier_log_file, "(Verifier) Response verified in = %f s\n", t1 - t0);
#endif

					} else {
						fprintf(verifier_log_file, "(Verifier) Error doing verify hash...\n");
					}
				} else {
					fprintf(verifier_log_file, "(Verifier) Error (DB) data to verify extraction, %d...\n", return_res);
				}
			} else {
				fprintf(verifier_log_file, "(Verifier) Error receiving hash buffer, received %zu/%"PRIu32"...\n", actual_received_hash_buffer_size, received_hash_buffer_size);
			}
			if (received_hash_buffer != NULL) free(received_hash_buffer);
		} else {
			fprintf(verifier_log_file, "(Verifier) Error allocation incoming buffer...\n");
		}
	} else {
		fprintf(verifier_log_file, "(Verifier) Error receiving header, received %zu/%ld B\n", general_tmp_var,
                VERIFIER_RESP_HEADER_SIZE);
	}

    time1 = time(NULL);
	fprintf(verifier_log_file, "(Verifier) Execution finished at: %s", ctime(&time1));

	fclose(verifier_log_file);
}

