#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void list_init(list_t *list) {
    list->head = NULL;
    list->size = 0;
}

void list_add(list_t *list, const char *data) {
    // TODO
}

int list_size(const list_t *list) {
    // TODO
    return 0;
}

char *list_get(const list_t *list, int index) {
    // TODO
    return NULL;
}

void list_clear(list_t *list) {
    // TODO
}

int list_contains(const list_t *list, const char *query) {
    // TODO
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
