#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <entry.h>

struct entry_t *entry_create(char *key, struct data_t *data) {
    struct entry_t *ret;
    ret = (struct entry_t*) malloc(sizeof(struct entry_t));
    ret->key = key;
    ret->value = data;
    return ret;
}
void entry_destroy(struct entry_t *entry) {
    data_destroy(entry->value);
    free(entry->key);
    free(entry);
}
struct entry_t *entry_dup(struct entry_t *entry) {
    struct entry_t *dupe;
    dupe = entry_create(entry->key, entry->value);
    return dupe;
}
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    free(entry->key);
    data_destroy(entry->value);
    entry->key = new_key;
    entry->value = new_value;
}
int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
    //idk?
}