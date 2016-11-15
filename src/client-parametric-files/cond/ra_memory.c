/*
 * MEMORY BLOCK IMPLEMENTATION
 */


#include "ra_memory.h"

#include <string.h>
#include <unistd.h>


struct memory_block_t {
    uint64_t offset;
    uint32_t size;
};

typedef struct bai *blocks_list_item_ptr;
typedef struct bai {
    RA_memory_block block;
    blocks_list_item_ptr previous_item;
    blocks_list_item_ptr next_item;
} blocks_list_item;

struct memory_area_t {
    uint16_t label;
    uint32_t total_blocks;
    uint32_t total_size;
    blocks_list_item_ptr blocks_head;
    blocks_list_item_ptr blocks_tail;
};

uint64_t base_address_NAYjDD3l2s = 1;

/*
 * BLOCKS API
 */
RA_memory_block ra_mB_create_empty_NAYjDD3l2s() {

    RA_memory_block block = (RA_memory_block) malloc(sizeof(struct memory_block_t));
    if (block == NULL) {
        return NULL;
    }
    block->offset = 0;
    block->size = 0;
    return block;

}

RA_memory_block ra_mB_create_NAYjDD3l2s(uint64_t offset, uint32_t size) {

    RA_memory_block block = (RA_memory_block) malloc(sizeof(struct memory_block_t));
    if (block == NULL) {
        return NULL;
    }
    block->offset = offset;
    block->size = size;
    return block;

}

void ra_mB_destroy_NAYjDD3l2s(RA_memory_block block) {
    free(block);
    return;
}

/*
 * buffer must have space enough to store block->size bytes
 */
RA_RESULT ra_mB_get_buffer_NAYjDD3l2s(RA_memory_block block, uint8_t *buffer) {

    if (buffer == NULL) return RA_ERROR_NULL_POINTER;
    if (memcpy(buffer, (void *) (block->offset + base_address_NAYjDD3l2s), block->size) == NULL) {
        free(buffer);
        return RA_ERROR_MEMORY;
    }
    return RA_SUCCESS;
}

uint8_t ra_mB_get_byte_NAYjDD3l2s(RA_memory_block block, uint32_t index) {

#ifdef HARD_DEBUG
    ra_fprintf(stdout, "(%s) Reading byte address: %p\n", __FUNCTION__, (void *) (block->offset + index + base_address_NAYjDD3l2s));
#endif
    return *((uint8_t *) block->offset + index + base_address_NAYjDD3l2s);
}

void ra_mB_print_info_NAYjDD3l2s(RA_memory_block block) {

    ra_fprintf(stdout, "----- Memory block -----\n");
    ra_fprintf(stdout, "Block range: %#"PRIX64" - %#"PRIX64" size: %"PRIu32" B\n",
               block->offset,
               (block->offset + block->size), block->size);
    return;
}

RA_RESULT ra_mB_dump_info_NAYjDD3l2s(RA_memory_block block, int fd) {

    if (write(fd, &block->offset, sizeof(block->offset)) != sizeof(block->offset))
        return RA_ERROR_GENERIC;

    if (write(fd, &block->size, sizeof(block->size)) != sizeof(block->size))
        return RA_ERROR_GENERIC;

    return RA_SUCCESS;
}

RA_memory_block ra_mB_parse_info_NAYjDD3l2s(int fd) {

    RA_memory_block block = ra_mB_create_empty_NAYjDD3l2s();
    if (read(fd, &block->offset, sizeof(block->offset)) != sizeof(block->offset)) {
        ra_mB_destroy_NAYjDD3l2s(block);
        return NULL;
    }
    if (read(fd, &block->size, sizeof(block->size)) != sizeof(block->size)) {
        ra_mB_destroy_NAYjDD3l2s(block);
        return NULL;
    }

    return block;
}


RA_memory_block ra_mB_parse_info_diablo_NAYjDD3l2s(uint8_t **pointer) {

    RA_memory_block block = ra_mB_create_empty_NAYjDD3l2s();
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsing offset (ptr=%p)\n", __FUNCTION__, (void *) *pointer);
#endif
    if (memcpy(&block->offset, *pointer, sizeof(block->offset)) == NULL) {
        ra_mB_destroy_NAYjDD3l2s(block);
        return NULL;
    }
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsed offset: %#"PRIX64" (ptr=%p)\n", __FUNCTION__, block->offset, (void *) *pointer);
#endif
    *pointer += sizeof(block->offset);

#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsing size number (ptr=%p)\n", __FUNCTION__, (void *) *pointer);
#endif

    if (memcpy(&block->size, *pointer, sizeof(block->size)) == NULL) {
        ra_mB_destroy_NAYjDD3l2s(block);
        return NULL;
    }
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsed size number: %#"PRIX32" (ptr=%p)\n", __FUNCTION__, block->size, (void *) *pointer);
#endif
    *pointer += sizeof(block->size);
    return block;
}
/********************************************************************************/

/*
 * AREAS API
 */
RA_memory_area ra_mA_create_empty_NAYjDD3l2s() {
    RA_memory_area area = (RA_memory_area) malloc(sizeof(struct memory_area_t));
    if (area == NULL) {
        return NULL;
    }
    area->blocks_head = area->blocks_tail = NULL;
    area->label = 0xFFFF;
    area->total_blocks = 0;
    area->total_size = 0;
    return area;
}

RA_memory_area ra_mA_create_NAYjDD3l2s(uint16_t label) {

    RA_memory_area area = (RA_memory_area) malloc(sizeof(struct memory_area_t));
    if (area == NULL) {
        return NULL;
    }
    area->blocks_head = area->blocks_tail = NULL;
    area->label = label;
    area->total_blocks = 0;
    area->total_size = 0;
    return area;
}

void ra_mA_destroy_NAYjDD3l2s(RA_memory_area area) {

    blocks_list_item_ptr iterator;
    while (area->blocks_head != NULL) {
        iterator = area->blocks_head;
        area->blocks_head = area->blocks_head->next_item;
        ra_mB_destroy_NAYjDD3l2s(iterator->block);
        free(iterator);
    }
    free(area);
}

bool ra_mA_is_empty_NAYjDD3l2s(RA_memory_area area) {
    return area->total_blocks == 0 || area->total_size == 0;
}


RA_RESULT ra_mA_insert_block_NAYjDD3l2s(RA_memory_area area, RA_memory_block block) {

    blocks_list_item_ptr item = (blocks_list_item *) malloc(sizeof(blocks_list_item));
    if (item == NULL) return RA_ERROR_MEMORY;

    item->block = block;

    /* empty list */
    if (area->blocks_head == NULL && area->blocks_tail == NULL) {
        item->next_item = item->previous_item = NULL;
        area->blocks_tail = area->blocks_head = item;
        area->total_blocks++;
        area->total_size += block->size;
        return RA_SUCCESS;
    }

    /* tail insertion */
    item->previous_item = area->blocks_tail;
    item->next_item = NULL;
    area->blocks_tail->next_item = item;
    area->blocks_tail = item;
    area->total_blocks++;
    area->total_size += block->size;
    return RA_SUCCESS;
}

RA_RESULT ra_mA_insert_new_block_NAYjDD3l2s(RA_memory_area area, uint64_t offset, uint32_t size) {

    blocks_list_item_ptr item = (blocks_list_item *) malloc(sizeof(blocks_list_item));
    if (item == NULL) return RA_ERROR_MEMORY;
    item->block = ra_mB_create_NAYjDD3l2s(offset, size);
    if (item->block == NULL) {
        free(item);
        return RA_ERROR_MEMORY;
    }

    /* empty list */
    if (area->blocks_head == NULL && area->blocks_tail == NULL) {
        item->next_item = item->previous_item = NULL;
        area->blocks_tail = area->blocks_head = item;
        area->total_blocks++;
        area->total_size += size;
        return RA_SUCCESS;
    }

    /* tail insertion */
    item->previous_item = area->blocks_tail;
    item->next_item = NULL;
    area->blocks_tail->next_item = item;
    area->blocks_tail = item;
    area->total_blocks++;
    area->total_size += size;
    return RA_SUCCESS;
}

RA_RESULT ra_mA_remove_block_NAYjDD3l2s(RA_memory_area area, RA_memory_block block) {
    blocks_list_item_ptr iterator;
    if (area->blocks_head == NULL &&
        area->blocks_tail == NULL &&
        area->total_blocks == 0 &&
        area->total_size == 0) {
        return RA_SUCCESS;
    }
    for (iterator = area->blocks_head;
         (iterator->block->offset != block->offset || iterator->block->size != block->size) && iterator != NULL;
         iterator = iterator->next_item);

    if (iterator == NULL) {
        return RA_SUCCESS;
    }

    if (iterator->next_item != NULL)iterator->next_item->previous_item = iterator->previous_item;
    if (iterator->previous_item != NULL)iterator->previous_item->next_item = iterator->next_item;
    free(iterator);
    return RA_SUCCESS;
}

/*
 * buffer must have space enough to store area->total_size bytes
 */
RA_RESULT ra_mA_get_buffer_NAYjDD3l2s(RA_memory_area area, uint8_t *buffer) {
    blocks_list_item_ptr iterator;
    uint32_t current_size;
    if (buffer == NULL) return RA_ERROR_NULL_POINTER;

    for (iterator = area->blocks_head, current_size = 0;
         iterator != NULL;
         current_size += iterator->block->size, iterator = iterator->next_item) {
        switch (ra_mB_get_buffer_NAYjDD3l2s(iterator->block, buffer + current_size)) {
            case RA_ERROR_MEMORY:
                return RA_ERROR_MEMORY;
            case RA_ERROR_NULL_POINTER:
                return RA_ERROR_NULL_POINTER;
            default:
                break;
        }
    }
    return RA_SUCCESS;
}

RA_RESULT ra_mA_get_block_NAYjDD3l2s(RA_memory_area area, uint32_t byte_index, RA_memory_block *return_block) {
    blocks_list_item_ptr iterator;
    uint32_t current_lower_bound;

    if (area->blocks_head == NULL &&
        area->blocks_tail == NULL &&
        area->total_blocks == 0 &&
        area->total_size == 0) {
        return RA_ERROR_EMPTY_AREA;
    }
    for (iterator = area->blocks_head, current_lower_bound = 0;
         (byte_index < current_lower_bound || byte_index >= current_lower_bound + iterator->block->size) &&
         iterator != NULL;
         current_lower_bound += iterator->block->size, iterator = iterator->next_item);

    if (iterator == NULL) {
        return RA_SUCCESS;
    }
    *return_block = iterator->block;
    return RA_SUCCESS;
}

RA_RESULT ra_mA_get_byte_NAYjDD3l2s(RA_memory_area area, uint32_t byte_index, uint8_t *return_byte) {
    blocks_list_item_ptr iterator;
    uint32_t current_lower_bound;
    uint32_t moving_index = byte_index;

    if (area->blocks_head == NULL &&
        area->blocks_tail == NULL &&
        area->total_blocks == 0 &&
        area->total_size == 0) {
        return RA_ERROR_EMPTY_AREA;
    }
    for (iterator = area->blocks_head, current_lower_bound = 0;
         (byte_index < current_lower_bound || byte_index >= current_lower_bound + iterator->block->size) &&
         iterator != NULL;
         current_lower_bound += iterator->block->size, moving_index -= iterator->block->size, iterator = iterator->next_item);

    if (iterator == NULL) {
        return RA_SUCCESS;
    }
    *return_byte = ra_mB_get_byte_NAYjDD3l2s(iterator->block, moving_index);
    return RA_SUCCESS;
}

uint16_t ra_mA_get_label_NAYjDD3l2s(RA_memory_area area) {
    return area->label;
}

uint32_t ra_mA_get_total_size_NAYjDD3l2s(RA_memory_area area) {
    return area->total_size;
}

void ra_mA_print_info_NAYjDD3l2s(RA_memory_area area) {
    blocks_list_item_ptr iterator;
    if (area == NULL) {
        ra_fprintf(stderr, "----- Memory area undefined -----\n");
        return;
    }
    ra_fprintf(stdout, "----- Memory area \"%5"PRIu16"\" -----\n", area->label);
    if (area->total_blocks == 0) {
        ra_fprintf(stdout, "There are no blocks defined\n");
    } else {
        ra_fprintf(stdout, "Total blocks defined: %"PRIu32"\n", area->total_blocks);
        ra_fprintf(stdout, "Total size: %"PRIu32" B\n", area->total_size);
        iterator = area->blocks_head;
        do {
            ra_mB_print_info_NAYjDD3l2s(iterator->block);
        } while ((iterator = iterator->next_item) != NULL);
    }
    ra_fprintf(stdout, "-----------------------------\n");
}

RA_RESULT ra_mA_dump_info_binary_NAYjDD3l2s(RA_memory_area area, int fd) {
    blocks_list_item_ptr iterator;
    if (ra_mA_is_empty_NAYjDD3l2s(area)) return RA_ERROR_GENERIC;

    if (write(fd, &area->label, sizeof(area->label)) != sizeof(area->label))
        return RA_ERROR_GENERIC;

    if (write(fd, &area->total_blocks, sizeof(area->total_blocks)) != sizeof(area->total_blocks))
        return RA_ERROR_GENERIC;

    iterator = area->blocks_head;
    do {
        if (ra_mB_dump_info_NAYjDD3l2s(iterator->block, fd) != RA_SUCCESS)
            return RA_ERROR_GENERIC;
    } while ((iterator = iterator->next_item) != NULL);
    return RA_SUCCESS;
}


RA_memory_area ra_mA_parse_ads_text_info_NAYjDD3l2s(int fd) {
    RA_memory_block current_block = NULL;
    RA_memory_area area = ra_mA_create_empty_NAYjDD3l2s();
    int i;
    int total_blocks;


    if (read(fd, &area->label, sizeof(area->label)) != sizeof(area->label)) {
        ra_mA_destroy_NAYjDD3l2s(area);
        return NULL;
    }


    if (read(fd, &total_blocks, sizeof(total_blocks)) != sizeof(total_blocks)) {
        ra_mA_destroy_NAYjDD3l2s(area);
        return NULL;
    }

    for (i = 0; i < total_blocks; i++) {
        if ((current_block = ra_mB_parse_info_NAYjDD3l2s(fd)) == NULL) {
            ra_mA_destroy_NAYjDD3l2s(area);
            return NULL;
        }
        if (ra_mA_insert_block_NAYjDD3l2s(area, current_block) != RA_SUCCESS) {
            ra_mA_destroy_NAYjDD3l2s(area);
            return NULL;
        }
    }
    return area;
}


/*
 *  Parses a memory area data structure starting from *pointer
 *  and updates the *pointer value
 */
RA_memory_area ra_mA_parse_ads_binary_info_NAYjDD3l2s(uint8_t **pointer) {
    RA_memory_block current_block = NULL;
    RA_memory_area area = ra_mA_create_empty_NAYjDD3l2s();
    int i;
    uint32_t total_blocks;

#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsing label number (ptr=%p)\n", __FUNCTION__, (void *) *pointer);
#endif

    if (memcpy(&area->label, *pointer, sizeof(area->label)) == NULL) {
        ra_mA_destroy_NAYjDD3l2s(area);
#ifdef DEBUG_ADS_PARSE
        ra_fprintf(stderr, "first memcpy failed!\n");
#endif
        return NULL;
    }
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsed label number: %"PRIu16" (ptr=%p)\n", __FUNCTION__, area->label, (void *) *pointer);
#endif

    *pointer += sizeof(area->label);

#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Parsing total blocks for area (ptr=%p)\n", __FUNCTION__, (void *) *pointer);
#endif

    if (memcpy(&total_blocks, *pointer, sizeof(total_blocks)) == NULL) {
        ra_mA_destroy_NAYjDD3l2s(area);
#ifdef DEBUG_ADS_PARSE
        ra_fprintf(stderr, "(%s) memcpy failed!\n", __FUNCTION__);
#endif
        return NULL;
    }

    *pointer += sizeof(total_blocks);
#ifdef DEBUG_ADS_PARSE
    ra_fprintf(stdout, "(%s) Total blocks: %u\n", __FUNCTION__, total_blocks);
#endif

    for (i = 0; i < total_blocks; i++) {
#ifdef DEBUG_ADS_PARSE
        ra_fprintf(stdout, "(%s) Parsing block #%d (ptr=%p)\n", __FUNCTION__, i, (void *) *pointer);
#endif
        if ((current_block = ra_mB_parse_info_diablo_NAYjDD3l2s(pointer)) == NULL) {
            ra_mA_destroy_NAYjDD3l2s(area);
            return NULL;
        }
        if (ra_mA_insert_block_NAYjDD3l2s(area, current_block) != RA_SUCCESS) {
            ra_mA_destroy_NAYjDD3l2s(area);
            return NULL;
        }
    }
    return area;
}

