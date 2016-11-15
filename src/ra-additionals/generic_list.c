/*
 * generic_list.c
 *
 */

#include "generic_list.h"
#include <string.h>
#include <stdlib.h>
#include "ra_print_wrapping.h"

typedef struct generic_list_item *generic_list_item_ptr;
typedef struct generic_list_item {
    void *data;
    generic_list_item_ptr next;
} generic_list_item_t;

struct generic_list {
    char name[MAX_NAME_SIZE + 1];
    generic_list_item_ptr head;
    uint32_t items_count;
    size_t item_size;
    gl_items_comparator_func compartor;
    gl_items_destructor_func destructor;
    gl_items_print_info_func printer;
};

generic_list_t gl_create(
        char *list_name,
        size_t item_size,
        gl_items_comparator_func comparator,
        gl_items_destructor_func destructor,
        gl_items_print_info_func printer) {

    if (destructor == NULL || comparator == NULL) return NULL;
    generic_list_t new = (generic_list_t) malloc(sizeof(struct generic_list));
    if (new == NULL) return NULL;

    new->head = NULL;
    strncpy(new->name, list_name, MAX_NAME_SIZE);
    new->items_count = 0;
    new->item_size = item_size;
    new->compartor = comparator;
    new->destructor = destructor;
    new->printer = printer;
    return new;
}

uint32_t gl_count(generic_list_t list) {
    return list->items_count;
}

gl_results_e gl_insert(generic_list_t list, void *data) {

    if (gl_is_containing(list, data)) return GL_ERROR_ITEM_ALREADY_PRESENT;
    generic_list_item_t *new = (generic_list_item_t *) malloc(sizeof(generic_list_item_t));
    if (new == NULL) return GL_ERROR_MEMORY;
    new->data = malloc(list->item_size);
    if (new->data == NULL) {
        free(new);
        return GL_ERROR_MEMORY;
    }
    memcpy(new->data, data, list->item_size);
    new->next = list->head;
    list->head = new;
    list->items_count++;
    return GL_SUCCESS;

}

bool gl_is_containing(generic_list_t list, void *data_to_comparator) {
    generic_list_item_t *iter;

    for (iter = list->head; iter != NULL; iter = iter->next) {
        if (list->compartor(iter->data, data_to_comparator) == 0) return true;
    }

    return false;
}

void *gl_get_reference(generic_list_t list, void *data_to_comparator) {
    generic_list_item_t *iter;

    for (iter = list->head; iter != NULL; iter = iter->next) {
        if (list->compartor(iter->data, data_to_comparator) == 0) return iter->data;
    }

    return NULL;
}

gl_results_e gl_delete(generic_list_t list, void *data_to_comparator) {

    generic_list_item_ptr iter, tmp;

    if (list->compartor(list->head->data, data_to_comparator) == 0) {
        tmp = list->head;
        list->head = list->head->next;
        list->destructor(tmp->data);
        free(tmp);
        return GL_SUCCESS;
    }

    for (iter = list->head->next; iter->next != NULL; iter = iter->next) {
        if (list->compartor(iter->next->data, data_to_comparator) == 0) {
            tmp = iter->next;
            iter->next = tmp->next;
            list->destructor(tmp->data);
            free(tmp);
            return GL_SUCCESS;
        }
    }

    return GL_ERROR_ITEM_NOT_FOUND;
}

void *gl_remove(generic_list_t list, void *data_to_comparator) {

    generic_list_item_ptr iter, tmp;
    void *tmpData = NULL;

    if (list->compartor(list->head->data, data_to_comparator) == 0) {
        tmp = list->head;
        list->head = list->head->next;
        tmpData = tmp->data;
        free(tmp);
        list->items_count--;
        return tmpData;
    }

    for (iter = list->head; iter->next != NULL; iter = iter->next) {
        if (list->compartor(iter->next->data, data_to_comparator) == 0) {
            tmp = iter->next;
            iter->next = tmp->next;
            tmpData = tmp->data;
            free(tmp);
            list->items_count--;
            return tmpData;
        }
    }

    return  NULL;
}

void gl_destroy(generic_list_t list) {
    generic_list_item_ptr iter, tmp;

    for (iter = list->head; iter != NULL; iter = iter->next) {
        tmp = iter;
        iter->next = tmp->next;
        list->destructor(tmp->data);
        free(tmp);
    }

    free(list);
    list = NULL;
    return;
}

void gl_print_info(generic_list_t list) {

    generic_list_item_t *iter;

    if(list == NULL) return;

    printf("-------------------------------------------------\n");
    printf("%s List Information:\nConstains %"PRIu32" items with size %zu\n", list->name, list->items_count, list->item_size);
    if (list->printer == NULL) {
        printf("No print function provided, cannot print internal items data, bye...\n");
    } else {
        for (iter = list->head; iter != NULL; iter = iter->next) {
            list->printer(iter->data);
            printf("\n");
        }
    }
    printf("-------------------------------------------------\n");

    return;
}

