#include "scheduler_queue.h"
#include <stdlib.h>

typedef struct scheduler_queue_item *scheduler_queue_item_ptr;
typedef struct scheduler_queue_item {
    RA_client_t client;
    float delta_time_to_sleep;
    scheduler_queue_item_ptr next;
} scheduler_queue_item_t;

struct scheduler_queue {
    scheduler_queue_item_ptr head;
    uint32_t items_count;
};

scheduler_queue_t sq_create() {

    scheduler_queue_t new = (scheduler_queue_t) malloc(sizeof(struct scheduler_queue));
    if (new == NULL) return NULL;

    new->head = NULL;
    new->items_count = 0;
    return new;
}

uint32_t sq_count(scheduler_queue_t clients) {
    return clients->items_count;
}

sq_results_e sq_insert(scheduler_queue_t clients, RA_client_t client, float time_to_sleep_sec) {

    float time_sum;
    scheduler_queue_item_ptr iter;

    scheduler_queue_item_ptr new = (scheduler_queue_item_ptr) malloc(sizeof(scheduler_queue_item_t));
    if (new == NULL) return SQ_ERROR_MEMORY;
    new->client = client;


    /* If list is empty or time to sleep is less than the first scheduled element one */
    if (clients->head == NULL || (clients->head != NULL && clients->head->delta_time_to_sleep > time_to_sleep_sec)) {
        /* insert element as first element */
        new->delta_time_to_sleep = time_to_sleep_sec;
        new->next = clients->head;
        clients->head = new;
    } else {
        /* Else: search where to insert the new element */
        for (iter = clients->head, time_sum = clients->head->delta_time_to_sleep;
             iter->next != NULL && time_sum + iter->next->delta_time_to_sleep < time_to_sleep_sec;
             time_sum = time_sum + iter->next->delta_time_to_sleep, iter = iter->next);

        new->next = iter->next;
        iter->next = new;
        new->delta_time_to_sleep = time_to_sleep_sec - time_sum;
    }


    clients->items_count++;
    if (new->next != NULL) new->next->delta_time_to_sleep -= new->delta_time_to_sleep;
    return SQ_SUCCESS;
}

sq_results_e sq_update_time_unslept(scheduler_queue_t clients, float time_left_sec) {
    if (clients->head != NULL) {
        if (clients->head->delta_time_to_sleep < time_left_sec)
            return SQ_ERROR_INVALID_TIME;
        clients->head->delta_time_to_sleep = time_left_sec;
    }
    return SQ_SUCCESS;
}

float sq_get_time_to_sleep(scheduler_queue_t clients) {
    if (clients->head != NULL) return clients->head->delta_time_to_sleep;
    else return EMPTY_SQ_LIST_TTS;
}

RA_client_t sq_remove(scheduler_queue_t clients) {
    if (clients->head == NULL)
        return NULL;
    scheduler_queue_item_ptr tmp = clients->head;
    RA_client_t client = tmp->client;
    clients->head = tmp->next;
    free(tmp);
    clients->items_count--;
    return client;
}

RA_client_t sq_remove_all_scheduled(scheduler_queue_t clients, RA_client_t client) {
    float time_sum;
    scheduler_queue_item_ptr iter, tmp;
    RA_client_t clientTmp;

    if (clients->head == NULL) {
        return NULL;
    } else {

        if (RA_CLI_compare(clients->head->client, client) == 0) {

            if (clients->head->next != NULL)
                clients->head->next->delta_time_to_sleep += clients->head->delta_time_to_sleep;

            tmp = clients->head;
            clients->head = tmp->next;
            clientTmp = tmp->client;
            free(tmp);
            clients->items_count--;
            return clientTmp;
        }

        for (iter = clients->head; iter->next != NULL && RA_CLI_compare(iter->next->client, client) != 0; iter = iter->next);

        if (iter->next == NULL) return NULL;

        if (iter->next->next != NULL) {
            iter->next->next->delta_time_to_sleep += iter->next->delta_time_to_sleep;
        }
        tmp = iter->next;
        iter->next = iter->next->next;
        clientTmp = tmp->client;
        free(tmp);
        clients->items_count--;
        return clientTmp;
    }
}

void sq_destroy(scheduler_queue_t clients) {
    scheduler_queue_item_ptr tmp;

    for (; clients->items_count > 0; clients->items_count--) {
        tmp = clients->head;
        clients->head = tmp->next;
        free(tmp);
    }

    free(clients);
    clients = NULL;
}

void sq_print_info(scheduler_queue_t list) {

    scheduler_queue_item_ptr iter;
    if(list == NULL) return;

    printf("-------Scheduled queue info--------\n");
    printf("%"PRIu32" clients scheduled\n", list->items_count);

    for (iter = list->head; iter != NULL; iter = iter->next) {
        RA_CLI_print_info(iter->client);
        printf("delta time to sleep: %.3f\n", iter->delta_time_to_sleep);
    }
    printf("-----------------------------------\n");

}
