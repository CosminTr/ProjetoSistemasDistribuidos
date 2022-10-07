#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <entry.h>

struct entry_t *entry_create(char *key, struct data_t *data) {
    struct entry_t *ret = malloc(sizeof(struct entry_t));
    ret->key = key;
    ret->value = data;
    return ret;
}
void entry_destroy(struct entry_t *entry) {
    if (entry != NULL){
        data_destroy(entry->value);
        free(entry->key);
        free(entry);   
    }
}
struct entry_t *entry_dup(struct entry_t *entry) { 
    struct entry_t *dupe;
    dupe = malloc(sizeof(struct entry_t));
    //duplicate key
    int key_size = strlen(entry->key)+1;
    dupe->key = malloc(key_size);
    memcpy(dupe->key, entry->key, key_size);
    //duplicate value
    dupe->value = data_dup(entry->value);

    return dupe;
}
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    free(entry->key);
    data_destroy(entry->value);
    
    entry->key = new_key;
    entry->value = new_value;
}
int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
    int result = strcmp(entry1->key, entry2->key);
    if (result == 0)
        return result;
    else if(result < 0)
        return -1;
    else
        return 1;
}