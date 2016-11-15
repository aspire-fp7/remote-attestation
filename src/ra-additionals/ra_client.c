#include "ra_client.h"
#include <stdlib.h>
#include <string.h>
#include "ra_print_wrapping.h"


struct ra_client_t {
    uint8_t AID[AID_SIZE];
    uint64_t attestator_number;
    uint64_t attestator_id;
    uint32_t sleep_avg_seconds;
    uint32_t sleep_var_seconds;
    uint64_t session_id;
};

size_t RA_CLI_element_size() {
    return sizeof(struct ra_client_t);
}

RA_client_t RA_CLI_create(
        uint8_t *AID,
        uint64_t attestator_number,
        uint64_t attestator_id,
        uint32_t sleep_avg_seconds,
        uint32_t sleep_var_seconds,
        uint64_t session_id,
        SOCKET connection) {

    RA_client_t new = (RA_client_t) malloc(sizeof(struct ra_client_t));
    if (new == NULL) return NULL;

    for (int i = 0; i < AID_SIZE; ++i) {
        new->AID[i] = AID[i];
    }

    new->attestator_number = attestator_number;
    new->attestator_id = attestator_id;
    new->sleep_avg_seconds = sleep_avg_seconds;
    new->sleep_var_seconds = sleep_var_seconds;
    new->session_id = session_id;

    return new;
}

int RA_CLI_compare(void *first_blob, void *second_blob) {

    RA_client_t first = (RA_client_t) first_blob;
    RA_client_t second = (RA_client_t) second_blob;

    if (first->attestator_number == second->attestator_number &&
        memcmp(first->AID, second->AID, AID_SIZE) == 0)
        return 0;

    int res;
    if ((res = memcmp(first->AID, second->AID, AID_SIZE)) != 0) {
        return res;
    }
    return (int) (first->attestator_number - second->attestator_number);

}

void RA_CLI_destroy(void *client) {
    free(client);
    return;
}

void RA_CLI_print_info(void *client_blob) {

    RA_client_t client = (RA_client_t) client_blob;

    printf("Client:\n");
    fprintf(stdout, "AID: ");
    for (int i = 0; i < AID_SIZE; i++) {
        fprintf(stdout, "%02X", client->AID[i]);
    }
    fprintf(stdout, "\n");
    printf("Attestator number: %"PRIu64"\n", client->attestator_number);
    printf("Attestation average sleep time: %"PRIu32"\n", client->sleep_avg_seconds);
    printf("Attestation sleep time variation: %"PRIu32"\n", client->sleep_var_seconds);

    return;
}


/* getters */
uint8_t *RA_CLI_get_AID_reference(RA_client_t client) {
    return client->AID;
}
/*
 * AID_string must have enough space, AID_SIZE*2+1
 */
RA_RESULT RA_CLI_get_AID_as_string(RA_client_t client, char *AID_string) {

    for (int i = 0; i < AID_SIZE; i++) {
        if (snprintf(AID_string + i * 2, 3, "%02"PRIX8, client->AID[i]) != 2)
            return RA_ERROR_MEMORY;
    }

    return RA_SUCCESS;
}


uint64_t RA_CLI_get_attestator_number(RA_client_t client) {
    return client->attestator_number;
}

uint64_t RA_CLI_get_attestator_id(RA_client_t client) {
    return client->attestator_id;;
}

uint32_t RA_CLI_get_sleep_avg_seconds(RA_client_t client) {
    return client->sleep_avg_seconds;
}

uint32_t RA_CLI_get_sleep_var_seconds(RA_client_t client) {
    return client->sleep_var_seconds;
}

uint64_t RA_CLI_get_session_id(RA_client_t client) {
    return client->session_id;
}

/* setters */
void RA_CLI_set_AID(RA_client_t client, uint8_t *AID) {
    for (int i = 0; i < AID_SIZE; ++i) {
        client->AID[i] = AID[i];
    }
}

void RA_CLI_set_attestator_number(RA_client_t client, uint64_t data) {
    client->attestator_number = data;
}

void RA_CLI_set_attestator_id(RA_client_t client, uint64_t product_id) {
    client->attestator_id = product_id;
}

void RA_CLI_set_sleep_avg_seconds(RA_client_t client, uint32_t data) {
    client->sleep_avg_seconds = data;
}

void RA_CLI_set_sleep_var_seconds(RA_client_t client, uint32_t data) {
    client->sleep_var_seconds = data;
}


void RA_CLI_set_session_id(RA_client_t client, uint64_t session_id) {
    client->session_id = session_id;
}