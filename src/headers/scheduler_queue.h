#ifndef SCHEDULER_QUEUE_H_
#define SCHEDULER_QUEUE_H_


#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <ra_client.h>

#include "ra_print_wrapping.h"


#define MAX_NAME_SIZE       30
#define EMPTY_SQ_LIST_TTS   3600.0

typedef struct scheduler_queue *scheduler_queue_t;
typedef enum {
    SQ_SUCCESS,
    SQ_ERROR_MEMORY,
    SQ_ERROR_ITEM_ALREADY_PRESENT,
    SQ_ERROR_ITEM_NOT_FOUND,
    SQ_ERROR_INVALID_TIME
} sq_results_e;


scheduler_queue_t sq_create();

uint32_t sq_count(scheduler_queue_t clients);

sq_results_e sq_insert(scheduler_queue_t clients, RA_client_t data, float time_to_sleep_sec);

sq_results_e sq_update_time_unslept(scheduler_queue_t clients, float time_left_sec);

float sq_get_time_to_sleep(scheduler_queue_t clients);

RA_client_t sq_remove(scheduler_queue_t clients);

RA_client_t sq_remove_all_scheduled(scheduler_queue_t clients, RA_client_t client);

void sq_destroy(scheduler_queue_t list);

void sq_print_info(scheduler_queue_t list);


#endif /* SCHEDULER_QUEUE_H_ */
