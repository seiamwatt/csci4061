#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void list_init(list_t *list) {
    list->head = NULL;
    list->size = 0;
}

void list_add(list_t *list, const char *data) {
    node_t *node = malloc(sizeof(node_t));

    if(node == NULL){
        return;
    }

    strcpy(node->data,data);
    node->data[MAX_LEN-1]= '\0';
    node->next = NULL;

    if(list -> head == NULL){
        list->head = node;
    }else{
        node_t *curr_node = list->head;

        while(curr_node->next != NULL){
            curr_node = curr_node ->next;
        }

        curr_node -> next = node;
    }

    list->size++;
    // TODO
}

int list_size(const list_t *list) {
    // TODO
    return list->size;
}

char *list_get(const list_t *list, int index) {
    // TODO
    int count = 0;
    node_t *current = list->head;
    while(current != NULL){
        if(count == index){
            return current->data;
        }

        current = current->next;
        count ++;
    }

    return NULL;
}

void list_clear(list_t *list) {
    node_t *current = list->head;

    while(current != NULL){
        node_t *next_node = current -> next;
        free(current);
        current= next_node;
    }

    list->size = 0;
    list->head = NULL;
    // TODO
}

int list_contains(const list_t *list, const char *query) {
    // TODO
    node_t *current = list->head;

    while(current != NULL){
        if(strcmp(current->data,query) == 0 ){
            return 1;
        }
        current = current ->next;
    }
    return 0;
}

void list_print(const list_t *list) {
    int i = 0;
    node_t *current = list->head;
    while (current != NULL) {
        printf("%d: %s\n", i, current->data);
        current = current->next;
        i++;
    }
}
