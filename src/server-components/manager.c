/*
 * MANAGER process implementation
 */

#define _GNU_SOURCE

#include "ra_print_wrapping.h"

#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <linux/limits.h>

#include <ra_client.h>
#include <generic_list.h>
#include <ra_database.h>
#include <scheduler_queue.h>
#include <ascl.h>
#include <signal.h>


typedef enum {

    client_up,
    client_down,
    die

} actual_manager_action_t;

typedef struct {
    pthread_t thread;
    uint32_t id;
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    uint32_t clients_count;

    actual_manager_action_t action;
    RA_client_t passingClient;
} actual_manager_t;

float time_left(struct timespec timeToWait) {
    struct timeval now_now;
    gettimeofday(&now_now, NULL);
    float diff = (float) (timeToWait.tv_sec - now_now.tv_sec);
    diff = diff + ((float) (timeToWait.tv_nsec - now_now.tv_usec * 1000) / (float) 1000000000);
    return diff;
}


/****** GLOBAL DATA ******/
volatile int force_exit;
int total_actual_managers;
actual_manager_t *actual_managers;

struct lws_context *attestator_contexts[MAX_ATTESTATORS_ALLOWED];

pthread_mutex_t lws_contexts_mutex;
/*************************/

/**************** DEFINITIONS OF FUNCTIONS ********************************************************/
void launch_extractor(int caller_actual_manager_id, char string_AID[AID_SIZE * 2 + 1], uint64_t attestator_number);

struct timespec transform_time_to_sleep(float secs_to_sleep);

float compute_random_sleep(RA_client_t client);

bool serve_client(RA_client_t client_to_serve, int caller_actual_manager_id);

bool send_ra_request(int caller_actual_manager_id, uint64_t prepared_data_id, const uint8_t *nonce, uint32_t nonce_size, char *string_AID,
                     uint64_t attestator_number, bool is_startup, uint64_t session_id);

bool attest_client_at_startup(RA_client_t client_to_attest, int caller_actual_manager_id);

void *actual_manager_routine(void *param);

void sighandler(int useless);

int atomic_asclWebSocketSend(uint64_t attestator_number, char string_AID[AID_SIZE * 2 + 1], void *buffer, size_t buffer_length);
/**************************************************************************************************/



/******************** MAIN FUINCTION **************************************************************/
int main(int argc, char **argv) {

    /****** Determine how many actual manager to be allocated and allocate them *******/
    if (argc != 2) {
        fprintf(stdout, "(Manager) No total threads number specified, default is 4\n");
        total_actual_managers = 4;
    } else {
        total_actual_managers = atoi(argv[1]);
        fprintf(stdout, "(Manager) Total threads number specified: %d\n", total_actual_managers);
    }

    if ((actual_managers = (actual_manager_t *) malloc(total_actual_managers * sizeof(actual_manager_t))) == NULL) {
        fprintf(stderr, "(Manager) Error, cannot allocate memory for actual managers, exiting...\n");
        exit(1);
    }
    /**********************************************************************************/

    /******* Initialize ASCL channels ******/
    fprintf(stdout, "(Manager) Initializing lws contexts\n");
    for (int i = 0; i < MAX_ATTESTATORS_ALLOWED; i++) {
        fprintf(stdout, "(Manager) lws context # %d\n", i);
        attestator_contexts[i] = asclWebSocketInit(RA_ATTESTATOR_0 + i);
//        pthread_mutex_init(&(lws_contexts_mutexes[i]), NULL);
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);


    }
    pthread_mutexattr_t attr1;
    pthread_mutexattr_init(&attr1);
    pthread_mutexattr_settype(&attr1, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lws_contexts_mutex, &attr1);
    /***************************************/


    fprintf(stdout, "(Manager) Let's go...\n");

    /******* Launch actual managers *******/
    for (uint32_t i = 0; i < total_actual_managers; i++) {
        actual_managers[i].id = i;
        actual_managers[i].clients_count = 0;
        //pthread_mutex_init(&(actual_managers[i].mutex), NULL);
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(actual_managers[i].mutex), &attr);

        pthread_cond_init(&(actual_managers[i].condition), NULL);
        if (pthread_create(&(actual_managers[i].thread), NULL, actual_manager_routine, &actual_managers[i])) {
            fprintf(stderr, "(Manager) Error creating actual manager #%d thread\n", i);

            /**** Rolling back initializations done so far ****/
            for (uint32_t j = i; j >= 0; j--) {
                pthread_mutex_destroy(&(actual_managers[i].mutex));
                pthread_cond_destroy(&(actual_managers[i].condition));
                if (pthread_cancel(actual_managers[i].thread)) {
                    fprintf(stderr, "(Manager) Error cancelling actual manager #%d thread\n", i);
                }
                for (int y = 0; y < MAX_ATTESTATORS_ALLOWED; y++) {
                    asclWebSocketShutdown(attestator_contexts[y]);
                }
            }
            pthread_mutex_destroy(&lws_contexts_mutex);
            free(actual_managers);
            exit(1);
        }
    }

    force_exit = 0;
    if (signal(SIGINT, sighandler) == SIG_ERR) {
        fprintf(stdout, "(Manager) An error occurred while setting a signal handler.\n");
        return 1;
    }

    while (force_exit == 0) {

        usleep(100000);
        for (int i = 0; i < MAX_ATTESTATORS_ALLOWED; ++i) {
            lws_service(attestator_contexts[i], 50);
        }

    }

    for (uint32_t i = 0; i < total_actual_managers; i++) {

        pthread_mutex_lock(&actual_managers[i].mutex);
        actual_managers[i].action = die;
        pthread_cond_signal(&actual_managers[i].condition);
        pthread_mutex_unlock(&actual_managers[i].mutex);

        if (pthread_join(actual_managers[i].thread, NULL)) {
            fprintf(stderr, "(Manager) Error cancelling actual manager #%d thread\n", i);
        }

        pthread_cond_destroy(&(actual_managers[i].condition));
        pthread_mutex_destroy(&(actual_managers[i].mutex));

    }

    for (int i = 0; i < MAX_ATTESTATORS_ALLOWED; i++) {

        asclWebSocketShutdown(attestator_contexts[i]);

    }
    pthread_mutex_destroy(&lws_contexts_mutex);

    free(actual_managers);


    return 0;
}

/******************** ************** **************************************************************/


struct timespec transform_time_to_sleep(float secs_to_sleep) {
    struct timespec tts;
    struct timeval now;
    long seconds = (long) floor(secs_to_sleep);
    long nano_seconds = (long) round((secs_to_sleep - floor(secs_to_sleep)) * 1000000000);

    gettimeofday(&now, NULL);

    tts.tv_sec = now.tv_sec + seconds + nano_seconds / 1000000000;

    tts.tv_nsec = (now.tv_usec * 1000 + nano_seconds) % 1000000000;

    return tts;
}

float compute_random_sleep(RA_client_t client) {
    uint32_t sleep_avg_seconds = RA_CLI_get_sleep_avg_seconds(client);
    uint32_t sleep_var_seconds = RA_CLI_get_sleep_var_seconds(client);

    srand((unsigned int) time(NULL));
    int rd = rand();
    return ((float) (1000 * (sleep_avg_seconds + sleep_var_seconds) - (rd % (2 * sleep_var_seconds * 1000)))) / 1000;

}

bool serve_client(RA_client_t client_to_serve, int caller_actual_manager_id) {

    uint64_t request_prepared_data_id;
    uint8_t *nonce_buffer = NULL;
    uint32_t nonce_length;
    uint32_t valid_nonce_left;
    RA_RESULT ra_result;
    uint16_t memory_area_to_attest;
    /** Extract valid nonce and verify nonce availability **/

    char app_AID_string[AID_SIZE * 2 + 1];
    uint64_t attestator_number = RA_CLI_get_attestator_number(client_to_serve);
    RA_CLI_get_AID_as_string(client_to_serve, app_AID_string);

    ra_result = ra_db_get_valid_nonce(RA_CLI_get_attestator_id(client_to_serve), &request_prepared_data_id, &nonce_buffer, &nonce_length,
                                      &valid_nonce_left, &memory_area_to_attest);


    /** Error while getting nonce and data, skip **/
    if (ra_result != RA_SUCCESS && ra_result != RA_ERROR_DB_NO_DATA) {
        fprintf(stderr, "(Actual manager %d) Error retrieving nonce data\n", caller_actual_manager_id);
        return false;
    }
    /** **************************************** **/

    /** Available nonces number is under critical threshold, launch Extractor **/
    if (valid_nonce_left < NONCE_CRITICAL_THRESHOLD) {

        fprintf(stdout, "(Actual manager %d) NONCES ARE RUNNING OUT \nLet's create %s fresh nonces...\n",
                caller_actual_manager_id, DEFAULT_NONCE_TOTAL);

        launch_extractor(caller_actual_manager_id, app_AID_string, attestator_number);
        return false;
    }
    /*******************************************************/


#ifdef DEBUG
    printf("(Actual manager %d) Nonce to be sent\n", caller_actual_manager_id);
    for (int i = 0; i < nonce_length; i++) {
        printf("%02X", (unsigned char) (*(nonce_buffer + i)));
    }
    printf("\n");
    printf("(Actual manager %d) Memory area to be attested: %"PRIu16"\n", caller_actual_manager_id, memory_area_to_attest);
#endif

    bool return_value = send_ra_request(caller_actual_manager_id, request_prepared_data_id, nonce_buffer, nonce_length, app_AID_string,
                                        attestator_number, false, RA_CLI_get_session_id(client_to_serve));

    free(nonce_buffer);
    return return_value;
}

bool send_ra_request(int caller_actual_manager_id, uint64_t prepared_data_id, const uint8_t *nonce, uint32_t nonce_size, char *string_AID,
                     uint64_t attestator_number, bool is_startup, uint64_t session_id) {

    /****** Connenct to DB **********/
    MYSQL *connection = mysql_init(NULL);

    if (connection == NULL) {
        fprintf(stderr, "(Actual manager %d) mysql_init() failed\n", caller_actual_manager_id);
    }
    if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) == NULL) {
        fprintf(stderr, "(Actual manager %d) mysql_real_connect() failed\n", caller_actual_manager_id);
        fprintf(stderr, "(Actual manager %d) %s\n", caller_actual_manager_id, mysql_error(connection));
    }
    /********************************/

    mysql_autocommit(connection, 0);
    char query[1024];
    sprintf(query, DBMS_QUERY_INSERT_REQUEST, prepared_data_id, session_id, 1800, is_startup);

    /***** Create new request *****/
    if (mysql_query(connection, query)) {
        fprintf(stderr, "(Actual manager %d) Error: %s\n", caller_actual_manager_id, mysql_error(connection));
        mysql_rollback(connection);
        return false;
    }
    uint64_t request_id = mysql_insert_id(connection);
    /***** ****************** *****/

    uint8_t *send_data_buffer;
    bool return_value = false;
    if ((send_data_buffer = (uint8_t *) malloc(REQUEST_HEADER_SIZE + nonce_size)) != NULL) {
        if (memcpy(send_data_buffer, &request_id, REQUEST_ID_SIZE) != NULL) {
            if (memcpy(send_data_buffer + REQUEST_ID_SIZE, &nonce_size, REQUEST_NONCE_LENGTH_SIZE) != NULL) {
                if (memcpy(send_data_buffer + REQUEST_ID_SIZE + REQUEST_NONCE_LENGTH_SIZE, nonce,
                           nonce_size) != NULL) {

                    printf("(Actual manager %d) Sending packet: \n", caller_actual_manager_id);
                    printf("   |      request id      |  nonce size  | nonce ...\n");
                    printf("DEC| %020"PRIu64" |  %010"PRIu32"  | ...\n", request_id, nonce_size);
                    printf("HEX|   %016"PRIX64"   |   %08"PRIX32"   | ", request_id, nonce_size);

                    for (int i = 0; i < nonce_size; i++) {
                        printf("%02X", (unsigned char) (*(nonce + i)));
                    }
                    printf("\n");

                    fprintf(stdout, "(Actual manager %d) Sending request\n", caller_actual_manager_id);


                    if (atomic_asclWebSocketSend(attestator_number, string_AID,
                                                 send_data_buffer,
                                                 REQUEST_HEADER_SIZE + nonce_size)) {

                        fprintf(stdout, "(Actual manager %d) sent\n", caller_actual_manager_id);
                        mysql_commit(connection);
                        return_value = true;

                    } else {
                        printf("(Actual manager %d) not sent...\n", caller_actual_manager_id);
                        mysql_rollback(connection);
                        return_value = false;
                    }
                } else {
                    fprintf(stderr, "(Actual manager %d) Error copying memory...\n", caller_actual_manager_id);
                }
            } else {
                fprintf(stderr, "(Actual manager %d) Error copying memory...\n", caller_actual_manager_id);
            }
        } else {
            fprintf(stderr, "(Actual manager %d) Error copying memory...\n", caller_actual_manager_id);
        }
    } else {
        fprintf(stderr, "(Actual manager %d) Error memory allocation...\n", caller_actual_manager_id);
    }
    mysql_autocommit(connection, 1);
    mysql_close(connection);
    free(send_data_buffer);
    return return_value;
}

void launch_extractor(int caller_actual_manager_id, char *string_AID, uint64_t attestator_number) {

    char extractor_cmd_path[PATH_MAX];
    char attestator_number_string[21];

    sprintf(attestator_number_string, "%020"PRId64, attestator_number);

    sprintf(extractor_cmd_path, EXTRACTOR_COMMAND_PATH_STRING_FORMAT, string_AID, attestator_number);

    fprintf(stdout, "(Actual manager %d) Launching extractor %s\n", caller_actual_manager_id, extractor_cmd_path);

    if (fork() == 0) {
        fprintf(stdout, "(CHILD Actual manager %d) Launching extractor: \n", caller_actual_manager_id);
        execl(extractor_cmd_path, EXTRACTOR_COMMAND, string_AID, attestator_number_string, DEFAULT_NONCE_LENGTH,
              DEFAULT_NONCE_TOTAL,
              NULL);
        fprintf(stderr, "(CHILD Actual manager %d) Error executing extractor\n", caller_actual_manager_id);
        perror("");
        exit(0);
    }

    fprintf(stdout, "(Actual manager %d) Extraction child process running\n", caller_actual_manager_id);
}

bool attest_client_at_startup(RA_client_t client_to_attest, int caller_actual_manager_id) {


    uint16_t *labels;
    uint16_t labels_count;
    uint64_t attestator_id = RA_CLI_get_attestator_id(client_to_attest);
    uint64_t attestator_number = RA_CLI_get_attestator_number(client_to_attest);
    bool return_value = true;
    char string_AID[AID_SIZE * 2 + 1];
    RA_CLI_get_AID_as_string(client_to_attest, string_AID);


    fprintf(stdout, "(Actual Manager %d) Startup attestation started for  client %s-%"PRIu64"\n",
            caller_actual_manager_id, string_AID, attestator_id);
    fprintf(stdout, "(Actual Manager %d) Retrieving areas to attest at start up \n", caller_actual_manager_id);

    if (ra_db_get_area_labels_to_attest_at_startup(attestator_id, &labels, &labels_count) != RA_SUCCESS) {
        fprintf(stderr, "(Actual Manager %d) Error retrieving areas to attest at start up\n", caller_actual_manager_id);
        return 1;
    } else {
        if (labels_count == 0) {
            fprintf(stdout, "(Actual Manager %d) No label to attest at startup for this client\n", caller_actual_manager_id);
//            uint64_t application_id_from_DB ;
//            ra_db_get_application_id(string_AID, &application_id_from_DB);
//            ra_db_set_reaction_status(application_id_from_DB, REACTION_STATUS_NONE);
        } else {
            printf("(Actual Manager %d) Found %"PRIu16" startup areas to attest\n", caller_actual_manager_id, labels_count);
            for (int i = 0; i < labels_count; ++i) {
                printf("(Actual Manager %d) Startup attest label: %"PRIu16"\n", caller_actual_manager_id, labels[i]);
                uint64_t prepared_data_id;
                uint8_t *nonce_buffer;
                uint32_t nonce_size;
                RA_RESULT result;
                int count_errors = 0;
                while ((result = ra_db_get_valid_nonce_by_area_label(attestator_id, labels[i], &prepared_data_id, &nonce_buffer, &nonce_size)) !=
                       RA_SUCCESS) {
                    if (result == RA_ERROR_DB_NO_DATA) {
                        fprintf(stdout, "(Actual Manager %d) No prepared data for this area, launching extractor\n", caller_actual_manager_id);
                        launch_extractor(caller_actual_manager_id, string_AID, attestator_number);
                    } else {
                        if (++count_errors >= ATTEST_AT_STARTUP_MAX_ERRORS) {
                            fprintf(stderr, "(Actual Manager %d) %d errors while retrieving prepared data to attest area %"PRIu16" at startup\n",
                                    caller_actual_manager_id, count_errors);
                            return_value = false;
                            break;
                        }
                    }
                }

                if (!send_ra_request(caller_actual_manager_id, prepared_data_id, nonce_buffer, nonce_size, string_AID, attestator_number, true,
                                     RA_CLI_get_session_id(client_to_attest))) {
                    fprintf(stderr, "(Actual Manager %d) Error sending startup attestation request for area %"PRIu16"\n", caller_actual_manager_id,
                            labels[i]);
                }
                free(nonce_buffer);
            }
        }

    }

    return return_value;

}

void *actual_manager_routine(void *param) {


    /******* Generate management data structure ********/
    actual_manager_t *this = (actual_manager_t *) param;

    /*** served clients list ***/
    generic_list_t clients_list = gl_create("Serving clients", RA_CLI_element_size(), RA_CLI_compare, RA_CLI_destroy,
                                            RA_CLI_print_info);

    /*** schedule clients queue ***/
    scheduler_queue_t scheduled_clients = sq_create();
    /***************************************************/

    fprintf(stdout, "(Actual Manager %d) Started\n", this->id);

    struct timespec current_time_to_sleep;

    int local_force_exit = 0;
    while (local_force_exit == 0) {

        /** Lock to wait an event **/
        pthread_mutex_lock(&this->mutex);
        float tmp_secs = sq_get_time_to_sleep(scheduled_clients);
        current_time_to_sleep = transform_time_to_sleep(tmp_secs);
        printf("(Actual Manager %d) Going to sleep for %.3fs\n", this->id, tmp_secs);

        /** Wait an event **/
        if (pthread_cond_timedwait(&this->condition, &this->mutex, &current_time_to_sleep) == ETIMEDOUT) {

            /**
             * Event=timeout elapsed ---> serve the scheduled client
             */

            RA_client_t client_to_serve;
            if ((client_to_serve = sq_remove(scheduled_clients)) != NULL) {
                /******** Serve client *********/
                char AID_to_serve_string[AID_SIZE * 2 + 1];
                RA_CLI_get_AID_as_string(client_to_serve, AID_to_serve_string);
                printf("(Actual Manager %d) Timed out, now serve client %s-%"PRIu64"\n", this->id, AID_to_serve_string,
                       RA_CLI_get_attestator_number(client_to_serve));

                if (serve_client(client_to_serve, this->id)) {
                    /*******   Client correctly served   ********/
                    /******* Reschedule client as needed ********/
                    sq_insert(scheduled_clients, client_to_serve, compute_random_sleep(client_to_serve));
                } else {
                    /*******     Client not corrrectly served    ********/
                    /******* Reschedule client with default time ********/
                    sq_insert(scheduled_clients, client_to_serve, NOT_SERVED_RE_SCHEDULING_STANDARD_TIME);
                }
            }
#ifdef  DEBUG
            sq_print_info(scheduled_clients);
#endif
        } else {

            /**
             * Event=ConnectionListener signaled me ---> client UP or DOWN
             */

            /******* Update previously scheduled first client *********/
            float time_left_to_sleep = time_left(current_time_to_sleep);
            printf("(Actual Manager %d) I've been sleeping for %.3fs, but signaled, process action to perform\n",
                   this->id, tmp_secs - time_left_to_sleep);
            sq_update_time_unslept(scheduled_clients, time_left_to_sleep);
            /**********************************************************/


            if (this->action == client_up) {

                /********* New client UP event **********/

                fprintf(stdout, "(Actual Manager %d) A client has been launched\n", this->id);
                if (this->passingClient == NULL) {
                    /**** ERROR passed client ****/
                    fprintf(stderr, "(Actual Manager %d) New client not defined, continue without inserting anything\n", this->id);
                } else {
                    /****** Attest areas that must be attested at startup ******/
//                    fprintf(stdout,
//                            "(Actual Manager %d) Wait 5000 ms\n", this->id);
//                    usleep(5000000);

                    attest_client_at_startup(this->passingClient, this->id);

                    /****** Insert and schedule new client ******/
                    if (gl_insert(clients_list, this->passingClient) == GL_ERROR_ITEM_ALREADY_PRESENT) {
                        printf("(Actual Manager %d) New client already present\n", this->id);
                    } else {
                        this->clients_count = gl_count(clients_list);
                        sq_insert(scheduled_clients, this->passingClient, compute_random_sleep(this->passingClient));
                        this->passingClient = NULL;
                        printf("(Actual Manager %d) New client inserted, now serving %d clients\n", this->id, this->clients_count);
                    }
                }

#ifdef DEBUG
                gl_print_info(clients_list);
                sq_print_info(scheduled_clients);
#endif

            } else if (this->action == client_down) {

                /********* Client DOWN event **********/

                fprintf(stdout, "(Actual Manager %d) A client has been shut down\n", this->id);
                if (gl_is_containing(clients_list, this->passingClient)) {

                    fprintf(stdout, "(Actual Manager %d) Removing client from scheduler\n", this->id);
                    if (sq_remove_all_scheduled(scheduled_clients, this->passingClient) == NULL) {
                        fprintf(stderr, "(Actual Manager %d) Client not scheduled\n", this->id);
                    }

                    fprintf(stdout, "(Actual Manager %d) Removing client from served clients\n", this->id);
                    RA_client_t clientToDestroy = gl_remove(clients_list, this->passingClient);
                    if (clientToDestroy == NULL) {
                        fprintf(stderr, "(Actual Manager %d) Client not present in served clients list\n", this->id);
                    }
                    RA_CLI_destroy(clientToDestroy);

                    RA_CLI_destroy(this->passingClient);
                    this->clients_count = gl_count(clients_list);
                    printf("(Actual Manager %d) Client removed, now serving %d clients\n", this->id, this->clients_count);
                } else {
                    printf("(Actual Manager %d) Client not served by me, nothing to remove\n", this->id);
                }

                this->passingClient = NULL;


#ifdef DEBUG
                gl_print_info(clients_list);
                sq_print_info(scheduled_clients);
#endif
            } else if (this->action == die) {
                fprintf(stdout, "(Actual Manager %d) Shutting down all activities\n", this->id);
                local_force_exit = 1;
                sq_destroy(scheduled_clients);
                fprintf(stdout, "(Actual Manager %d) Scheduler queue destroyed \n", this->id);
                gl_destroy(clients_list);
                fprintf(stdout, "(Actual Manager %d) Client list destroyed\n", this->id);
            }


        }
        pthread_mutex_unlock(&this->mutex);
    }


    return NULL;
}

void sighandler(int useless) {

    fprintf(stdout, "\nCTRL+c received, shutting down...\n");
    force_exit = 1;

}

int atomic_asclWebSocketSend(uint64_t attestator_number, char string_AID[AID_SIZE * 2 + 1], void *buffer, size_t buffer_length) {

    pthread_mutex_lock(&lws_contexts_mutex);
    int rt = asclWebSocketSend(attestator_contexts[attestator_number], string_AID, RA_ATTESTATOR_0 + attestator_number, buffer, buffer_length);
    pthread_mutex_unlock(&lws_contexts_mutex);
    return rt;

}

/*
 * Implements extern ASCL function
 */
int asclWebSocketDispatcherMessage(TECHNIQUE_ID technique_id, char application_id[32], int message_id, size_t length, const char *payload,
                                   size_t *response_length, char *response) {
    uint64_t received_app_attestator_number = (uint64_t) (technique_id - RA_ATTESTATOR_0);
    uint64_t attestator_id_from_db;
    uint32_t app_sleep_avg;
    uint32_t app_sleep_var;
    uint64_t application_id_from_db;


    uint8_t received_app_AID_bin[AID_SIZE];

    fprintf(stdout, "(Connection Listener) Received T_ID: %d\n", technique_id);
    fprintf(stdout, "(Connection Listener) Received A_ID: ");

    for (int j = 0; j < AID_SIZE; ++j) {
        fprintf(stdout, "%c%c", application_id[j * 2], application_id[j * 2 + 1]);
        sscanf(&(application_id[j * 2]), "%02"SCNx8, &received_app_AID_bin[j]);
    }
    fprintf(stdout, "\n");

    /******** Read DB and get client information *********/
    if (ra_db_get_attestation_parameters(application_id, received_app_attestator_number,
                                         &application_id_from_db, &attestator_id_from_db, &app_sleep_avg, &app_sleep_var) != RA_SUCCESS) {

        fprintf(stderr,
                "(Connection Listener) Error: application %s-%"PRIu64" doesn't exist. Refusing connection...\n",
                application_id, received_app_attestator_number);
        return ASCL_ERROR;
    }
    /*****************************************************/


    uint64_t session_id;

    /***** Create client object *****/
    RA_client_t client = RA_CLI_create(received_app_AID_bin, received_app_attestator_number,
                                       attestator_id_from_db, app_sleep_avg, app_sleep_var,
                                       0xFFFFFFFFFFFFFFFF, 0);



    switch (message_id) {
        case ASCL_WS_MESSAGE_OPEN:

            /*********** NEW CLIENT UP ************/

            fprintf(stdout,
                    "(Connection Listener) Starting session for %s-%"PRIu64"\n",
                    application_id, received_app_attestator_number);
            if (ra_db_start_session(attestator_id_from_db, &session_id) != RA_SUCCESS) {
                fprintf(stderr,
                        "(Connection Listener) Error: cannot start session for %s-%"PRIu64". Refusing connection...\n",
                        application_id, received_app_attestator_number);
                return ASCL_ERROR;
            }

            fprintf(stdout,
                    "(Connection Listener) Got session id %"PRIu64"\n",
                    session_id);
            RA_CLI_set_session_id(client, session_id);

            fprintf(stdout,
                    "(Connection Listener) Setting reaction status as UNKNOWN\n");
            if (ra_db_set_reaction_status(application_id_from_db, REACTION_STATUS_UNKNOWN) != RA_SUCCESS) {
                fprintf(stderr,
                        "(Connection Listener) Error: cannot set reaction status for application %s-%"PRIu64". Refusing connection...\n",
                        application_id, received_app_attestator_number);
                ra_db_revoke_session(session_id);
                return ASCL_ERROR;
            }


            printf("(Connection Listener) Connection opened (TID: %d, AID: %s)\n", technique_id, application_id);

            /******* Select the least loaded actual manager to which the client will be assigned *******/
            uint32_t i_min = 0;
            if (total_actual_managers != 1) {
                for (uint32_t i = 1; i < total_actual_managers; i++) {
                    if (actual_managers[i].clients_count < actual_managers[i_min].clients_count) {
                        i_min = i;
                    }
                }
            }
            /*******************************************************************************************/


            printf("(Connection Listener) Assigning client to actual manager #%d\n attestator %s-%"PRIu64"\n",
                   i_min, application_id, received_app_attestator_number);

            /****** Pass the client to the actual manager ******/
            pthread_mutex_lock(&actual_managers[i_min].mutex);

            actual_managers[i_min].passingClient = client;
            actual_managers[i_min].action = client_up;

            pthread_cond_signal(&actual_managers[i_min].condition);
            pthread_mutex_unlock(&actual_managers[i_min].mutex);
            /****************************************************/

            break;

        case ASCL_WS_MESSAGE_SEND:
            printf("BACKEND: Send received (TID: %d, AID: %s, buf: %zu bytes, %s)\n", technique_id, application_id, length, payload);
            break;
        case ASCL_WS_MESSAGE_EXCHANGE:
            printf("BACKEND: Exchange received (TID: %d, AID: %s, buf: %zu bytes, %s)\n", technique_id, application_id, length, payload);
            break;

        case ASCL_WS_MESSAGE_CLOSE:

            /*********** CLIENT DOWN ************/

            fprintf(stdout,
                    "(Connection Listener) Connection closed (TID: %d, AID: %s)\n", technique_id, application_id);

            fprintf(stdout,
                    "(Connection Listener) Getting session for %s-%"PRIu64"\n",
                    application_id, received_app_attestator_number);
            if (ra_db_get_active_session(attestator_id_from_db, &session_id) != RA_SUCCESS) {
                fprintf(stderr,
                        "(Connection Listener) Error: cannot get session for %s-%"PRIu64". Refusing connection...\n",
                        application_id, received_app_attestator_number);
                return ASCL_ERROR;
            }

            fprintf(stdout,
                    "(Connection Listener) Got session id %"PRIu64"\n",
                    session_id);

            fprintf(stdout,
                    "(Connection Listener) Closing session...\n");
            if (ra_db_close_session(session_id) != RA_SUCCESS) {
                fprintf(stderr,
                        "(Connection Listener) Cannot close session for client %s-%"PRIu64"\n",
                        application_id, received_app_attestator_number);
                return ASCL_ERROR;
            }

            fprintf(stdout,
                    "(Connection Listener) Setting reaction status as UNKNOWN\n");
            if (ra_db_set_reaction_status(application_id_from_db, REACTION_STATUS_UNKNOWN) != RA_SUCCESS) {
                fprintf(stderr,
                        "(Connection Listener) Error: cannot set reaction status for application %s-%"PRIu64". Refusing connection...\n",
                        application_id, received_app_attestator_number);
                ra_db_revoke_session(session_id);
                return ASCL_ERROR;
            }

            fprintf(stdout,
                    "(Connection Listener) Notifying to actual managers shutdown of client %s-%"PRIu64"\n",
                    application_id, received_app_attestator_number);

            /*********** "Broadcast" the actual managers the event **********/
            for (int i = 0; i < total_actual_managers; ++i) {
                printf("(Connection Listener) Notifying to actual managers %d\n", i);

                pthread_mutex_lock(&actual_managers[i].mutex);

                actual_managers[i].passingClient = client;
                actual_managers[i].action = client_down;

                pthread_cond_signal(&actual_managers[i].condition);
                pthread_mutex_unlock(&actual_managers[i].mutex);
            }
            /****************************************************************/

            break;
        default:
            break;
    }

    return ASCL_SUCCESS;
}
