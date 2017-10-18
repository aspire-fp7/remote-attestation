/*
 * 	Attestator IMPLEMENTATION
 */

#include <attestator.h>

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <accl.h>

#include <ra_do_hash.h>
#include <ra_data_preparation.h>
#include <ra_nonce_interpretation.h>
#include <ra_defines.h>
#include <ra_print_wrapping.h>

#define MAX_CONNECTION_RETRY 10


pthread_t ws_polling_thread_NAYjDD3l2s;
pthread_mutex_t websocket_mutex_NAYjDD3l2s;
pthread_mutex_t deinit_mutex_NAYjDD3l2s;

volatile int force_exit_NAYjDD3l2s;

pthread_cond_t init_finished_cond_NAYjDD3l2s;
pthread_mutex_t init_finished_mutex_NAYjDD3l2s;
volatile int init_finished_status_NAYjDD3l2s;

void *attestator_routine_NAYjDD3l2s(void *in_buffer, size_t len) {
    pthread_mutex_lock(&deinit_mutex_NAYjDD3l2s);
    if (force_exit_NAYjDD3l2s == 1 || in_buffer == NULL) {
        ra_fprintf(stdout, "(Attestator XXX) Exiting because application is shutting down, or in_buffer is NULL\n");
        pthread_mutex_unlock(&deinit_mutex_NAYjDD3l2s);
        return NULL;
    }

    ra_fprintf(stdout, "(Attestator XXX) Starting attestation\n");
    ra_fflush(stdout);

    uint8_t *defined_AID;
    ra_get_AID_reference_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s, &defined_AID);

    uint64_t defined_attestator_number;
    ra_get_attestator_number_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s, &defined_attestator_number);

    ra_fprintf(stdout, "(Attestator %"PRIu64") %zuB buffer received\n", defined_attestator_number, len);
    ra_fflush(stdout);


    /*** TIME_EVALUATION ***********************/
    float t0, t1;
    t0 = ((float) clock()) / CLOCKS_PER_SEC;
    /*******************************************/

    /* ------------------- ATTESTATION DATA PREPARATION ----------------------------- */
    uint64_t req_id = *((uint64_t *) (in_buffer + 0));
    ra_fprintf(stdout, "(Attestator %"PRIu64") Received req_id=%"PRIu64"\n", defined_attestator_number, req_id);
    ra_fflush(stdout);


    uint32_t nonce_size = *((uint32_t *) (in_buffer + REQUEST_ID_SIZE));
    ra_fprintf(stdout, "(Attestator %"PRIu64") Received nonce size: %"PRIu32"B\n", defined_attestator_number, nonce_size);
    ra_fflush(stdout);


    uint8_t *nonce_buffer = (in_buffer + REQUEST_HEADER_SIZE);
    ra_fprintf(stdout, "(Attestator %"PRIu64") Received request id %"PRIu64" containing %"PRIu32"B nonce\n", defined_attestator_number, req_id,
               nonce_size);

    ra_set_current_nonce_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s, nonce_buffer, nonce_size);
    ra_fprintf(stdout, "(Attestator %"PRIu64") Nonce set in data structure\n", defined_attestator_number);
    ra_fflush(stdout);
    print_encoded_info_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);

    ra_fprintf(stdout, "(Attestator %"PRIu64") Launching data preparation\n", defined_attestator_number);
    if (ra_prepare_data_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s) != RA_SUCCESS) {
        ra_fprintf(stderr, "(Attestator %"PRIu64") Error while preparing data\n", defined_attestator_number);
        ra_fflush(stderr);
        pthread_mutex_unlock(&deinit_mutex_NAYjDD3l2s);
        return NULL;
    }

#ifdef HARD_DEBUG
    uint32_t dp_size = ra_get_prepared_data_size_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);
    uint8_t *dp = ra_get_prepared_data_reference_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);
    ra_fprintf(stdout, "(Attestator %"PRIu64") Prepared data size %"PRIu32"\n", defined_attestator_number,dp_size);
    for (int p = 0; p < dp_size; p++) {
        ra_fprintf(stdout, "%02X", (unsigned char) (*((int8_t*) (dp + p))));
    }
    ra_fprintf(stdout, "\n");
#endif

    uint8_t *packed_buffer;
    uint32_t packed_buffer_size;

    if (ra_do_hash_NAYjDD3l2s(
            ra_get_current_nonce_reference_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s),
            ra_get_current_nonce_size_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s),
            ra_get_prepared_data_reference_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s),
            ra_get_prepared_data_size_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s),
            defined_AID,
            defined_attestator_number,
            NULL,
            0,
            &packed_buffer,
            &packed_buffer_size
    ) != RA_SUCCESS) {
        ra_fprintf(stdout, "(Attestator %"PRIu64") Error hashing prepared data\n", defined_attestator_number);
        ra_fflush(stdout);
        pthread_mutex_unlock(&deinit_mutex_NAYjDD3l2s);
        return NULL;
    }

    ra_fprintf(stdout, "(Attestator %"PRIu64") Attestation data hashed, %"PRIu32"B buffer\n", defined_attestator_number, packed_buffer_size);

#ifdef HARD_DEBUG
    ra_fprintf(stdout, "(Attestator %"PRIu64") Hashed data size %"PRIu32"\n", defined_attestator_number,packed_buffer_size);
    for (int p = 0; p < packed_buffer_size; p++) {
        ra_fprintf(stdout, "%02X", (unsigned char) (*((int8_t*) (packed_buffer + p))));
    }
    ra_fprintf(stdout, "\n");
#endif
    /* ------------------- ATTESTATION DATA PREPARED ----------------------------- */




    /*
     * Destroy prepared data in order to keep it in memory
     * no longer than necessary
     */
    ra_reset_prepared_data_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);


    /* ------------------- ATTESTATION RESPONSE PREPARATION ----------------------------- */
    uint8_t *response_buffer = (uint8_t *) malloc((ATTESTATOR_RESP_HEADER_SIZE + packed_buffer_size) * sizeof(uint8_t));
    if (response_buffer == NULL) {
        ra_fprintf(stderr, "(Attestator %"PRIu64") Error during allocation\n", defined_attestator_number);
        ra_fflush(stderr);
        pthread_mutex_unlock(&deinit_mutex_NAYjDD3l2s);
        return NULL;
    }

    memcpy(response_buffer, defined_AID, AID_SIZE);
    ra_fprintf(stdout, "(Attestator %"PRIu64") Writing AID to response_buffer\n", defined_attestator_number);


    ra_fprintf(stdout, "(Attestator %"PRIu64") Writing attestator number to response_buffer\n", defined_attestator_number);
    memcpy(response_buffer + AID_SIZE, &defined_attestator_number, RESP_ATTESTATOR_NO_SIZE);


    ra_fprintf(stdout, "(Attestator %"PRIu64") Writing req_id to response_buffer\n", defined_attestator_number);
    memcpy(response_buffer + AID_SIZE + RESP_ATTESTATOR_NO_SIZE, &req_id, RESP_REQUEST_ID_SIZE);


    ra_fprintf(stdout, "(Attestator %"PRIu64") Writing packed_buffer_size to nonce_buffer\n", defined_attestator_number);
    memcpy(response_buffer + AID_SIZE + RESP_ATTESTATOR_NO_SIZE + RESP_REQUEST_ID_SIZE, &packed_buffer_size, RESP_HASH_LENGTH_SIZE);


    ra_fprintf(stdout, "(Attestator %"PRIu64") Writing packed_buffer to nonce_buffer\n", defined_attestator_number);
    memcpy(response_buffer + AID_SIZE + RESP_ATTESTATOR_NO_SIZE + RESP_REQUEST_ID_SIZE + RESP_HASH_LENGTH_SIZE, packed_buffer, packed_buffer_size);
    /* ------------------- ATTESTATION RESPONSE PREPARED ----------------------------- */


    /* ------------------- ATTESTATION RESPONSE SENDING ----------------------------- */
    ra_fprintf(stdout, "(Attestator %"PRIu64") ACCL_SEND %dB of data\n", defined_attestator_number,
               (int) (ATTESTATOR_RESP_HEADER_SIZE + packed_buffer_size));
    ra_fflush(stdout);
    if (acclSend(ACCL_TID_RA_VERIFIER,
                 (const int) (ATTESTATOR_RESP_HEADER_SIZE + packed_buffer_size), (char *) response_buffer) == ACCL_SUCCESS) {
        ra_fprintf(stdout, "(Attestator %"PRIu64") ACCL SEND SUCCESS\n", defined_attestator_number);
        ra_fflush(stdout);
    } else {
        ra_fprintf(stdout, "(Attestator %"PRIu64") ACCL SEND ERROR\n", defined_attestator_number);
        ra_fflush(stdout);
    }
    /* ------------------- ATTESTATION RESPONSE SENT ON NOT ------------------------- */

    free(packed_buffer);
    free(response_buffer);

    /*** TIME_EVALUATION ***********************/
    t1 = ((float) clock()) / CLOCKS_PER_SEC;
    ra_fprintf(stdout, "(Attestator %"PRIu64") Request processed in = %f s\n", defined_attestator_number, t1 - t0);
    ra_fflush(stdout);
    /*******************************************/
    pthread_mutex_unlock(&deinit_mutex_NAYjDD3l2s);
    return NULL;

}


struct libwebsocket_context *atomic_acclWebSocketInit_NAYjDD3l2s(const int T_ID, void *(*callback)(void *, size_t)) {
    pthread_mutex_lock(&websocket_mutex_NAYjDD3l2s);
    struct libwebsocket_context *rv = acclWebSocketInit(T_ID, callback);
    pthread_mutex_unlock(&websocket_mutex_NAYjDD3l2s);
    return rv;
}

int atomic_acclWebSocketShutdown_NAYjDD3l2s(struct libwebsocket_context *context) {
    pthread_mutex_lock(&websocket_mutex_NAYjDD3l2s);
    int rv = acclWebSocketShutdown(context);
    pthread_mutex_unlock(&websocket_mutex_NAYjDD3l2s);
    return rv;
}

int atomic_libwebsocket_service_NAYjDD3l2s(struct libwebsocket_context *context, int timeout_ms) {
    pthread_mutex_lock(&websocket_mutex_NAYjDD3l2s);
    int rv = libwebsocket_service(context, timeout_ms);
    pthread_mutex_unlock(&websocket_mutex_NAYjDD3l2s);
    return rv;
}

void *ws_polling_routine_NAYjDD3l2s(void *useless) {

    struct libwebsocket_context *context_NAYjDD3l2s;
    uint64_t attestator_number;

    if (ra_get_attestator_number_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s, &attestator_number) != RA_SUCCESS) {
        ra_fprintf(stderr, "(Attestator init %"PRIu64") Error: not defined attestator number, attestator cannot start\n", attestator_number);
        ra_fflush(stderr);
        ra_destroy_table_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);
        return NULL;
    }


    ra_fprintf(stdout, "(Attestator polling %"PRIu64") Initialization of ACCL\n", attestator_number);
    ra_fflush(stdout);

    ra_fprintf(stdout, "(Attestator polling %"PRIu64")  technique id for initialization: %"PRIu64"\n", attestator_number,
               ACCL_RA_ATTESTATOR_0 + attestator_number);

    int c;
    for (c = 0;
         (context_NAYjDD3l2s = atomic_acclWebSocketInit_NAYjDD3l2s((int) (ACCL_RA_ATTESTATOR_0 + (int) attestator_number), attestator_routine_NAYjDD3l2s)) == NULL &&
         c < MAX_CONNECTION_RETRY; c++) {
        ra_fprintf(stdout, "(Attestator polling %"PRIu64") acclWebSocketInit failed, retry...\n", attestator_number);
        if(force_exit_NAYjDD3l2s == 1){
            ra_fprintf(stdout, "(Attestator polling %"PRIu64") Attestator terminated, exiting thread\n", attestator_number);
            if(context_NAYjDD3l2s != NULL) {
                ra_fprintf(stdout, "(Attestator polling %"PRIu64") Destroying ACCL\n", attestator_number);
                atomic_acclWebSocketShutdown_NAYjDD3l2s(context_NAYjDD3l2s);
                ra_fprintf(stdout, "(Attestator polling %"PRIu64") Destroying ACCL: succeeded\n", attestator_number);
            }
            pthread_mutex_lock(&init_finished_mutex_NAYjDD3l2s);
            init_finished_status_NAYjDD3l2s = -1;
            pthread_cond_signal(&init_finished_cond_NAYjDD3l2s);
            pthread_mutex_unlock(&init_finished_mutex_NAYjDD3l2s);
            return NULL;
        }
    }
    if (context_NAYjDD3l2s == NULL || c == MAX_CONNECTION_RETRY) {
        ra_fprintf(stderr, "(Attestator polling %"PRIu64") acclWebSocketInit failed too many times, attestator cannot start\n", attestator_number);
        pthread_mutex_lock(&init_finished_mutex_NAYjDD3l2s);
        init_finished_status_NAYjDD3l2s = -1;
        pthread_cond_signal(&init_finished_cond_NAYjDD3l2s);
        pthread_mutex_unlock(&init_finished_mutex_NAYjDD3l2s);
        return NULL;
    }


    int flag = 0;
    ra_fprintf(stdout, "(Attestator polling %"PRIu64") ACCL successfully initialized\n", attestator_number);
    while (force_exit_NAYjDD3l2s == 0) {
        //ra_fprintf(stdout, "(Attestator polling %"PRIu64") Poll\n", attestator_number);
        atomic_libwebsocket_service_NAYjDD3l2s(context_NAYjDD3l2s, 50);
        if(flag == 0) {
            flag = 1;
            pthread_mutex_lock(&init_finished_mutex_NAYjDD3l2s);
            init_finished_status_NAYjDD3l2s = 1;
            pthread_cond_signal(&init_finished_cond_NAYjDD3l2s);
            pthread_mutex_unlock(&init_finished_mutex_NAYjDD3l2s);
        }
    }

    ra_fprintf(stdout, "(Attestator polling %"PRIu64") Attestator terminated, exiting thread\n", attestator_number);
    ra_fprintf(stdout, "(Attestator polling %"PRIu64") Destroying ACCL\n", attestator_number);
    atomic_acclWebSocketShutdown_NAYjDD3l2s(context_NAYjDD3l2s);
    ra_fprintf(stdout, "(Attestator polling %"PRIu64") Destroying ACCL: succeeded\n", attestator_number);
    return NULL;
}


int attestator_init_NAYjDD3l2s() {

    uint64_t attestator_number;

    ra_fprintf(stdout, "(Attestator) An attestator is being started\n");
    ra_fflush(stdout);

    if ((remote_attestation_data_table_NAYjDD3l2s = ra_parse_binary_ads_NAYjDD3l2s()) == NULL) {
        ra_fprintf(stderr, "(Attestator init) Error while parsing ADS, attestator cannot start\n");
        ra_fflush(stderr);
        return 1;
    }

    ra_print_table_info_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);

    if (ra_get_attestator_number_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s, &attestator_number) != RA_SUCCESS) {
        ra_fprintf(stderr, "(Attestator init %"PRIu64") Error: not defined attestator number, attestator cannot start\n", attestator_number);
        ra_fflush(stderr);
        ra_destroy_table_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);
        return 1;
    }


    ra_fprintf(stderr, "(Attestator init %"PRIu64") Register attestator deinit function\n", attestator_number);
    ra_fflush(stderr);
    if (atexit(attestator_deinit_NAYjDD3l2s) != 0) {
        ra_fprintf(stderr, "(Attestator init %"PRIu64") Cannot register exit function, attestator cannot start\n", attestator_number);
        ra_fflush(stderr);
        return 1;
    }


    force_exit_NAYjDD3l2s = 0;
    init_finished_status_NAYjDD3l2s = 0;
    pthread_mutex_init(&websocket_mutex_NAYjDD3l2s, NULL);
    pthread_mutex_init(&init_finished_mutex_NAYjDD3l2s, NULL);
    pthread_mutex_init(&deinit_mutex_NAYjDD3l2s, NULL);
    pthread_cond_init(&init_finished_cond_NAYjDD3l2s, NULL);

    ra_fprintf(stdout, "(Attestator init %"PRIu64") Creating communication thread\n", attestator_number);
    ra_fflush(stdout);
    if (pthread_create(&ws_polling_thread_NAYjDD3l2s, NULL, ws_polling_routine_NAYjDD3l2s, NULL)) {
        ra_fprintf(stderr, "(Attestator init %"PRIu64") Creating WebSockets polling thread, attestator cannot start\n", attestator_number);
        ra_fflush(stderr);
        ra_destroy_table_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);
        return 1;
    }

    if(init_finished_status_NAYjDD3l2s == 0) {
        ra_fprintf(stdout, "(Attestator init %"PRIu64") Polling routine initialization not yet finished\n", attestator_number);
        ra_fflush(stdout);
        pthread_mutex_lock(&init_finished_mutex_NAYjDD3l2s);
        pthread_cond_wait(&init_finished_cond_NAYjDD3l2s, &init_finished_mutex_NAYjDD3l2s);
        pthread_mutex_unlock(&init_finished_mutex_NAYjDD3l2s);
    }

    if(init_finished_status_NAYjDD3l2s == 1) {
        ra_fprintf(stdout, "(Attestator init %"PRIu64") Successfully started\n", attestator_number);
        ra_fflush(stdout);
    }else{
        ra_fprintf(stdout, "(Attestator init %"PRIu64") Error diring initialization\n", attestator_number);
        ra_fflush(stdout);
    }
    return 0;
}

void attestator_deinit_NAYjDD3l2s() {


    ra_fprintf(stdout,
               "(Attestator deinit XXX) Application closed, destroying attestation data\n");

    uint64_t attesator_number;
    ra_get_attestator_number_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s, &attesator_number);
    ra_fprintf(stdout,
               "(Attestator deinit %"PRIu64") Application closed, destroying attestation data\n",
               attesator_number);

    pthread_mutex_lock(&deinit_mutex_NAYjDD3l2s);

    force_exit_NAYjDD3l2s = 1;

    ra_fprintf(stdout, "(Attestator deinit %"PRIu64") RA thread termination induced\n", attesator_number);

    pthread_mutex_unlock(&deinit_mutex_NAYjDD3l2s);

    if (pthread_join(ws_polling_thread_NAYjDD3l2s, NULL) != 0) {
        ra_fprintf(stderr, "(Attestator deinit %"PRIu64")  Error while cancelling thread\n");
        exit(1);
    }


    ra_fprintf(stdout, "(Attestator deinit %"PRIu64") Destroying attestation data\n", attesator_number);
    ra_destroy_table_NAYjDD3l2s(remote_attestation_data_table_NAYjDD3l2s);
    
    pthread_mutex_destroy(&init_finished_mutex_NAYjDD3l2s);
    pthread_mutex_destroy(&deinit_mutex_NAYjDD3l2s);
    pthread_mutex_destroy(&websocket_mutex_NAYjDD3l2s);

    ra_fprintf(stdout, "(Attestator deinit %"PRIu64") Successfully destroyed attestation data\n", attesator_number);
    return;
}
