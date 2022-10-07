#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

struct data_t *data_create(int size) {
    if(size < 1 )
        return NULL;
    else{
        struct data_t *ret = malloc(sizeof(struct data_t));
        ret->datasize = size;
        ret->data = malloc (size);
        return ret;
    }
}

struct data_t *data_create2(int size, void *data) {
    if(size < 1 || data == NULL)
        return NULL;
    else{
        struct data_t *ret;
        ret = (struct data_t*) malloc(sizeof(struct data_t));
        ret->datasize = size;
        ret->data = data;
        return ret;
    }
}

void data_destroy(struct data_t *data) {
    if (data != NULL){
        free(data->data);
        free(data);    
    }
}

struct data_t *data_dup(struct data_t *data) {
    struct data_t *dupe;
    if (data == NULL || data->data == NULL || data->datasize < 1)
        return NULL;
    else{
        dupe = data_create(data->datasize);
        memcpy(dupe->data, data->data, data->datasize);
        return dupe;
    }
}

void data_replace(struct data_t *data, int new_size, void *new_data) {
    free(data->data);
    data->datasize = new_size;
    data->data = new_data; 
}
