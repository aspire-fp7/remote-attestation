/*
 *	DATA_TABLE BLOCK IMPLEMENTATION
 */

#include "ra_data_table.h"
#include "ra_defines.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/*  Data Structures Declarations */

typedef struct ti *ra_table_item_ptr;
typedef struct ti {
    RA_memory_area area;
    ra_table_item_ptr next;
    ra_table_item_ptr previous;
} ra_table_item;

struct ra_table_t {
    uint8_t *prepared_data;
    uint32_t prepared_data_size;
    uint8_t *current_nonce;
    uint32_t current_nonce_size;
    uint8_t *current_key;
    uint8_t current_key_size;
    ra_table_item_ptr head;
    ra_table_item_ptr tail;
    uint16_t memory_areas_count;
    uint8_t app_AID[AID_SIZE];
    uint64_t app_attestator_number;
    bool is_AID_set;
    bool is_attestator_number_set;
};

/*******************************************************/

uint64_t data_structure_blob_NAYjDD3l2s = 1;
RA_table remote_attestation_data_table_NAYjDD3l2s;

/*******************************************************
 * Private Functions
 */
bool insert_table_item_NAYjDD3l2s(RA_table table, ra_table_item_ptr item) {

    /* empty list */
    if (table->head == NULL && table->tail == NULL) {
        item->next = item->previous = NULL;
        table->tail = table->head = item;
        table->memory_areas_count++;
        return true;
    }

    /* insertion, always at the end */
    item->previous = table->tail;
    item->next = NULL;
    table->tail->next = item;
    table->tail = item;
    table->memory_areas_count++;
    return true;
}

bool ra_is_label_defined_NAYjDD3l2s(RA_table table, uint16_t label) {
    int i;
    ra_table_item_ptr iterator;

    for (i = 0, iterator = table->head; i < table->memory_areas_count; i++, iterator = iterator->next)
        if (label == ra_mA_get_label_NAYjDD3l2s(iterator->area))
            return true;

    return false;
}

/*******************************************************/

/*******************************************************
 *  Public API Implementation
 */
RA_table ra_create_table_NAYjDD3l2s() {
    RA_table t;
    t = (RA_table) malloc(sizeof(struct ra_table_t));
    if (t == NULL) {
        return NULL;
    }
    t->head = t->tail = NULL;
    t->memory_areas_count = 0;
    t->is_AID_set = false;
    t->current_nonce = NULL;
    t->current_nonce_size = 0;
    t->prepared_data = NULL;
    t->prepared_data_size = 0;
    t->current_key = NULL;
    t->current_key_size = 0;

    /**********************DEBUG INFORMATION PRINTING OUT********************************/
    ra_fprintf(stdout, "(ra_create_table) Table created, base address: %"PRIu64"\n", base_address_NAYjDD3l2s);
    /************************************************************************************/
    return t;
}

void ra_destroy_table_NAYjDD3l2s(RA_table table) {
    ra_table_item_ptr iterator;

    if (table == NULL) return;

    while (table->head != NULL) {
        iterator = table->head;
        table->head = table->head->next;
        ra_mA_destroy_NAYjDD3l2s(iterator->area);
        free(iterator);
    }
    if (table->current_nonce != NULL)
        free(table->current_nonce);
    if (table->prepared_data != NULL)
        free(table->prepared_data);
    if (table->current_key != NULL)
        free(table->current_key);
    free(table);
    table = NULL;
}

/********** Serial Number Interface **********/
RA_RESULT ra_set_AID_NAYjDD3l2s(RA_table table, uint8_t *n) {

    if (memcpy(table->app_AID, n, AID_SIZE) == NULL)
        return RA_ERROR_MEMORY;
    table->is_AID_set = true;
    return RA_SUCCESS;

}

RA_RESULT ra_get_AID_reference_NAYjDD3l2s(RA_table table, uint8_t **AID) {

    if (table->is_AID_set) {
        *AID = table->app_AID;
        return RA_SUCCESS;
    }

    return RA_ERROR_UNDEFINED_SERIAL_NUMBER;

}

RA_RESULT ra_get_AID_as_string_NAYjDD3l2s(RA_table table, char *AID) {

    for (int i = 0; i < AID_SIZE; i++) {
        if (snprintf(AID + i * 2, 3, "%02"PRIX8, table->app_AID[i]) != 2)
            return RA_ERROR_MEMORY;
    }

    return RA_SUCCESS;
}

/********** Attestator Number Interface **********/
RA_RESULT ra_set_attestator_number_NAYjDD3l2s(RA_table table, uint64_t attestator_number) {
    table->app_attestator_number = attestator_number;
    table->is_attestator_number_set = true;
    return RA_SUCCESS;
}

RA_RESULT ra_get_attestator_number_NAYjDD3l2s(RA_table table, uint64_t *attestator_number) {
    if (table->is_attestator_number_set) {
        *attestator_number = table->app_attestator_number;
        return RA_SUCCESS;
    }
    return RA_ERROR_UNDEFINED_ATTESTATOR_NUMBER;
}

RA_RESULT ra_set_current_nonce_NAYjDD3l2s(RA_table table, uint8_t *nonce, uint32_t size) {
    if (table->current_nonce != NULL) {
        free(table->current_nonce);
        table->current_nonce_size = 0;
        table->current_nonce = NULL;
    }
    ra_fprintf(stdout, "(%s) Allocating nonce space\n", __FUNCTION__);
    table->current_nonce = (uint8_t *) malloc(size);
    if (table->current_nonce == NULL)
        return RA_ERROR_MEMORY;
    ra_fprintf(stdout, "(%s) Setting nonce\n", __FUNCTION__);
    if (memcpy(table->current_nonce, nonce, size) == NULL)
        return RA_ERROR_MEMORY;
    ra_fprintf(stdout, "(%s) Nonce set\n", __FUNCTION__);
    table->current_nonce_size = size;
    return RA_SUCCESS;
}

uint32_t ra_get_current_nonce_size_NAYjDD3l2s(RA_table table) {
    return table->current_nonce_size;
}

uint8_t *ra_get_current_nonce_reference_NAYjDD3l2s(RA_table table) {
    return table->current_nonce;
}

/*
 * nonce must be allocated elsewhere with size byte space, use ra_get_current_nonce_size_NAYjDD3l2s
 */
RA_RESULT ra_get_current_nonce_NAYjDD3l2s(RA_table table, uint8_t *nonce) {
    if (nonce == NULL)
        return RA_ERROR_NULL_POINTER;
    if (table->current_nonce == NULL || !table->current_nonce_size)
        return RA_ERROR_UNDEFINED_NONCE;
    if (memcpy(nonce, table->current_nonce, table->current_nonce_size) == NULL)
        return RA_ERROR_MEMORY;
    return RA_SUCCESS;
}

RA_RESULT ra_set_current_key_NAYjDD3l2s(RA_table table, uint8_t *key, uint8_t size) {
    if (table->current_key != NULL) {
        free(table->current_key);
        table->current_key_size = 0;
        table->current_key = NULL;
    }
    table->current_key = (uint8_t *) malloc(size);
    if (table->current_key == NULL)
        return RA_ERROR_MEMORY;
    if (memcpy(table->current_key, key, size) == NULL)
        return RA_ERROR_MEMORY;
    table->current_key_size = size;
    return RA_SUCCESS;
}

uint8_t *ra_get_current_key_reference_NAYjDD3l2s(RA_table table) {
    return table->current_key;
}

/*
 * key must be allocated elsewhere with size byte space
 */
RA_RESULT ra_get_current_key_NAYjDD3l2s(RA_table table, uint8_t *key) {
    if (table->current_key == NULL || table->current_key_size)
        return RA_ERROR_UNDEFINED_KEY;
    if (key == NULL)
        return RA_ERROR_NULL_POINTER;
    if (memcpy(key, table->current_key, table->current_key_size) == NULL)
        return RA_ERROR_MEMORY;
    return RA_SUCCESS;
}

uint8_t ra_get_current_key_size_NAYjDD3l2s(RA_table table) {
    return table->current_key_size;
}

uint16_t ra_get_memory_areas_count_NAYjDD3l2s(RA_table table) {
    return table->memory_areas_count;
}

RA_RESULT ra_insert_memory_area_NAYjDD3l2s(RA_table table, RA_memory_area area) {
    if (area == NULL) {
        return RA_ERROR_MALFORMED_MEMORY_AREA;
    }
    if (ra_is_label_defined_NAYjDD3l2s(table, ra_mA_get_label_NAYjDD3l2s(area))) {
        return RA_ERROR_MEMORY_LABEL_ALREADY_DEFINED;
    }
    ra_table_item *new_item = (ra_table_item *) malloc(sizeof(ra_table_item));
    if (new_item == NULL)
        return RA_ERROR_MEMORY;
    new_item->area = area;
    insert_table_item_NAYjDD3l2s(table, new_item);
    return RA_SUCCESS;
}

RA_RESULT ra_remove_memory_area_NAYjDD3l2s(RA_table table, uint16_t label) {
    ra_table_item_ptr iterator;

    for (iterator = table->head;
         ra_mA_get_label_NAYjDD3l2s(iterator->area) != label && iterator != NULL; iterator = iterator->next);
    if (iterator == NULL) {
        return RA_ERROR_UNDEFINED_MEMORY_LABEL;
    }

    if (iterator->next != NULL)
        iterator->next->previous = iterator->previous;
    if (iterator->previous != NULL)
        iterator->previous->next = iterator->next;
    free(iterator);
    return RA_SUCCESS;
}

RA_memory_area ra_get_memory_area_NAYjDD3l2s(RA_table table, uint16_t label) {
    ra_table_item_ptr iterator;

    for (iterator = table->head; iterator != NULL; iterator = iterator->next) {
        if (ra_mA_get_label_NAYjDD3l2s(iterator->area) == label)
            break;
    }
    if (iterator == NULL) {
        return NULL;
    }
    return iterator->area;
}

void ra_print_table_info_NAYjDD3l2s(RA_table table) {

    int counter;
    int i;
    ra_table_item_ptr iterator;
    ra_fprintf(stdout, "\n----------------------------------\n");
    if (table == NULL) {
        ra_fprintf(stderr, "RA table is not initialized\n");
        ra_fprintf(stderr, "\n----------------------------------\n");
        return;
    }

    if (table->is_AID_set == false)
        ra_fprintf(stdout, "AID is NOT set\n");
    else {
        ra_fprintf(stdout, "AID is set:\n ");
        for (i = 0; i < AID_SIZE; i++) {
            ra_fprintf(stdout, "%02X", table->app_AID[i]);
        }
        ra_fprintf(stdout, "\n");
    }
    if (table->is_attestator_number_set == false)
        ra_fprintf(stdout, "Attestator Number is NOT set\n");
    else {
        ra_fprintf(stdout, "Attestator Number is set:\n ");
        ra_fprintf(stdout, "%"PRIu64, table->app_attestator_number);
        ra_fprintf(stdout, "\n");
    }
    if (table->current_nonce == NULL || table->current_nonce_size == 0)
        ra_fprintf(stdout, "Current nonce is not set\n");
    else {
        ra_fprintf(stdout, "Current nonce is set:\n ");
        for (i = 0; i < table->current_nonce_size; i++)
            ra_fprintf(stdout, "%02x", table->current_nonce[i]);
        ra_fprintf(stdout, "\n");
    }
    if (table->current_key == NULL || table->current_key_size == 0)
        ra_fprintf(stdout, "Current key is not set\n");
    else {
        ra_fprintf(stdout, "Current key is set:\n ");
        for (i = 0; i < table->current_key_size; i++)
            ra_fprintf(stdout, "%02x", table->current_key[i]);
        ra_fprintf(stdout, "\n");
    }

    if (table->memory_areas_count == 0 || table->head == NULL || table->tail == NULL) {
        ra_fprintf(stdout, "RA table is empty\n");
    } else {
        ra_fprintf(stdout, "RA table contains %"PRIu32" memory areas\n", table->memory_areas_count);
        iterator = table->head;
        counter = 0;
        while (iterator != NULL && counter < table->memory_areas_count) {
            counter++;
            ra_mA_print_info_NAYjDD3l2s(iterator->area);
            iterator = iterator->next;
        }
    }
    if (table->prepared_data == NULL || table->prepared_data_size == 0) {
        ra_fprintf(stdout, "No prepared data defined");
    } else {
        ra_fprintf(stdout, "Prepared data is defined:\n");
        for (i = 0; i < table->prepared_data_size; i++) {
            ra_fprintf(stdout, "%02X", table->prepared_data[i]);
            if (i + 1 % 16 == 0)ra_fprintf(stdout, "\n");
        }
        ra_fprintf(stdout, "\n");
    }
    ra_fprintf(stdout, "\n----------------------------------\n");
}

RA_RESULT ra_dump_table_info_NAYjDD3l2s(RA_table table, const char *out_file_name) {
    int counter;
    ra_table_item_ptr iterator;

    int areas_binary_fd = open(out_file_name, O_WRONLY | O_CREAT, 0666);
    if (areas_binary_fd == -1) {
        return RA_ERROR_GENERIC;
    }

    if (!table->is_AID_set) {
        close(areas_binary_fd);
        remove(out_file_name);
        return RA_ERROR_GENERIC;
    }
    if (write(areas_binary_fd, table->app_AID, AID_SIZE) != sizeof(table->app_AID)) {
        close(areas_binary_fd);
        remove(out_file_name);
        return RA_ERROR_GENERIC;
    }

    if (!table->is_attestator_number_set) {
        close(areas_binary_fd);
        remove(out_file_name);
        return RA_ERROR_GENERIC;
    }
    if (write(areas_binary_fd, &table->app_attestator_number, sizeof(table->app_attestator_number)) !=
        sizeof(table->app_attestator_number)) {
        close(areas_binary_fd);
        remove(out_file_name);
        return RA_ERROR_GENERIC;
    }

    if (write(areas_binary_fd, &table->memory_areas_count, sizeof(table->memory_areas_count)) !=
        sizeof(table->memory_areas_count)) {
        close(areas_binary_fd);
        remove(out_file_name);
        return RA_ERROR_GENERIC;
    }

    counter = 0;
    iterator = table->head;
    while (iterator != NULL && counter < table->memory_areas_count) {
        counter++;

        if (ra_mA_dump_info_binary_NAYjDD3l2s(iterator->area, areas_binary_fd) != RA_SUCCESS) {
            close(areas_binary_fd);
            remove(out_file_name);

            return RA_ERROR_GENERIC;
        }
        iterator = iterator->next;
    }
    close(areas_binary_fd);
    return RA_SUCCESS;
}

RA_table ra_parse_binary_ads_NAYjDD3l2s() {
    int counter;
    RA_memory_area current_area = NULL;
    uint32_t memory_areas_count;
    uint8_t *data_ptr = (uint8_t *) data_structure_blob_NAYjDD3l2s;

    RA_table table = ra_create_table_NAYjDD3l2s();

    if (data_ptr == NULL || data_ptr == (uint8_t *) 0x1) {
#ifdef DEBUG_ADS_PARSE
        ra_fprintf(stderr, "(%s) ADS not present in the binary: ptr=%p\n", __FUNCTION__, (void *) data_ptr);
#endif
        return NULL;
    }

#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsing AID (ptr=%p)\n", __FUNCTION__, (void *) data_ptr);
#endif


    if (memcpy(table->app_AID, data_ptr, AID_SIZE) == NULL) {
        ra_destroy_table_NAYjDD3l2s(table);
        return NULL;
    }

    for (int i = 0; i < AID_SIZE / 2; ++i) {
        uint8_t swap;
        swap = table->app_AID[i];
        table->app_AID[i] = table->app_AID[AID_SIZE - i - 1];
        table->app_AID[AID_SIZE - i - 1] = swap;
    }


    table->is_AID_set = true;
    data_ptr += sizeof(table->app_AID);


    char parsedAID[AID_SIZE * 2 + 1];
    ra_get_AID_as_string_NAYjDD3l2s(table, parsedAID);
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsed AID: %s\n", __FUNCTION__, parsedAID);

    ra_fprintf(stdout, "(%s) Parsing attestator number ptr=%p\n", __FUNCTION__, (void *) data_ptr);
#endif

    if (memcpy(&table->app_attestator_number, data_ptr, sizeof(table->app_attestator_number)) == NULL) {
        ra_destroy_table_NAYjDD3l2s(table);
        return NULL;
    }
    table->is_attestator_number_set = true;
    data_ptr += sizeof(table->app_attestator_number);


    /**********************DEBUG INFORMATION PRINTING OUT********************************/
    uint64_t parsed_attestator_no;
    ra_get_attestator_number_NAYjDD3l2s(table, &parsed_attestator_no);
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsed attestator number: %"PRIu64"\n", __FUNCTION__, parsed_attestator_no);

    ra_fprintf(stdout, "(%s) Parsing total number of memory area (ptr=%p)\n", __FUNCTION__, (void *) data_ptr);
#endif

    /************************************************************************************/

    if (memcpy(&memory_areas_count, data_ptr, sizeof(memory_areas_count)) == NULL) {
        ra_destroy_table_NAYjDD3l2s(table);
        return NULL;
    }

    data_ptr += sizeof(memory_areas_count);

    /**********************DEBUG INFORMATION PRINTING OUT********************************/
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsed total number of memory area: %"PRIu32"\n", __FUNCTION__, memory_areas_count);
#endif
    /************************************************************************************/


    counter = 0;
    while (counter < memory_areas_count) {
        counter++;

        /**********************DEBUG INFORMATION PRINTING OUT********************************/
#ifdef DEBUG_ADS_PARSE
        ra_fprintf(stdout, "(%s) Parsing memory area #%d (ptr=%p)\n", __FUNCTION__, counter, (void *) data_ptr);
#endif
        /************************************************************************************/


        if ((current_area = ra_mA_parse_ads_binary_info_NAYjDD3l2s(&data_ptr)) == NULL) {
            ra_destroy_table_NAYjDD3l2s(table);
            return NULL;
        }
        if (ra_insert_memory_area_NAYjDD3l2s(table, current_area) != RA_SUCCESS) {
            ra_destroy_table_NAYjDD3l2s(table);
            return NULL;
        }
    }
    return table;
}

/*
 *  List must be allocated as a uint16_t array with enough items (use ra_get_memory_areas_count_NAYjDD3l2s)
 */
RA_RESULT ra_get_labels_list_NAYjDD3l2s(RA_table table, uint16_t *list) {
    int i;
    ra_table_item_ptr iterator;

    if (list == NULL)
        return RA_ERROR_MEMORY;

    for (i = 0, iterator = table->head; i < table->memory_areas_count; i++, iterator = iterator->next)
        list[i] = ra_mA_get_label_NAYjDD3l2s(iterator->area);
    return RA_SUCCESS;
}

RA_RESULT ra_set_prepared_data_NAYjDD3l2s(RA_table table, uint8_t *buffer, uint32_t size) {
    if (buffer == NULL && size == 0) {
        ra_reset_prepared_data_NAYjDD3l2s(table);
        return RA_SUCCESS;
    }
    table->prepared_data = (uint8_t *) malloc(size);
    if (table->prepared_data == NULL)
        return RA_ERROR_MEMORY;
    if (memcpy(table->prepared_data, buffer, size) == NULL)
        return RA_ERROR_MEMORY;
    table->prepared_data_size = size;
    return RA_SUCCESS;
}

void ra_reset_prepared_data_NAYjDD3l2s(RA_table table) {
    table->prepared_data_size = 0;
    if (table->prepared_data != NULL)
        free(table->prepared_data);
    table->prepared_data = NULL;
    return;
}

uint8_t *ra_get_prepared_data_reference_NAYjDD3l2s(RA_table table) {
    return table->prepared_data;
}

/*
 * buffer must be allocated elsewhere with enough space, use ra_get_prepared_data_size_NAYjDD3l2s
 */
RA_RESULT ra_get_prepared_data_NAYjDD3l2s(RA_table table, uint8_t *buffer) {
    if (buffer == NULL)
        return RA_ERROR_NULL_POINTER;
    if (table->prepared_data == NULL || table->prepared_data_size == 0)
        return RA_ERROR_EMPTY_PREPARED_DATA;
    if (memcpy(buffer, table->prepared_data, table->prepared_data_size * sizeof(uint8_t)) == NULL) {
        return RA_ERROR_MEMORY;
    }
    return RA_SUCCESS;
}

uint32_t ra_get_prepared_data_size_NAYjDD3l2s(RA_table table) {
    return table->prepared_data_size;

}
/*******************************************************/
