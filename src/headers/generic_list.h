/*
 * generic_list.h
 *
 *  Created on: Jul 10, 2015
 *      Generic single linked list,
 *      always insert in head position,
 *      no duplicates allowed
 */

#ifndef GENERIC_LIST_H_
#define GENERIC_LIST_H_

#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

#include "ra_print_wrapping.h"


#define MAX_NAME_SIZE 30

typedef struct generic_list *generic_list_t;

typedef int (*gl_items_comparator_func)(void *, void *);

typedef void (*gl_items_destructor_func)(void *);

typedef void (*gl_items_print_info_func)(void *);

typedef enum {
    GL_SUCCESS,
    GL_ERROR_MEMORY,
    GL_ERROR_ITEM_ALREADY_PRESENT,
    GL_ERROR_ITEM_NOT_FOUND
} gl_results_e;


generic_list_t gl_create(char *list_name,
                         size_t item_size,
                         gl_items_comparator_func comparator,
                         gl_items_destructor_func destructor,
                         gl_items_print_info_func printer);

uint32_t gl_count(generic_list_t clients);

gl_results_e gl_insert(generic_list_t clients, void *data);

bool gl_is_containing(generic_list_t clients, void *data_to_comparator);

void *gl_get_reference(generic_list_t clients, void *data_to_comparator);

gl_results_e gl_delete(generic_list_t clients, void *data_to_comparator);

void *gl_remove(generic_list_t list, void *data_to_comparator);

void gl_destroy(generic_list_t list);

void gl_print_info(generic_list_t list);

#endif /* GENERIC_LIST_H_ */
