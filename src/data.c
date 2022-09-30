#include <data.h>
#include <stdio.h>

struct data_t *data_create(int size) {
    struct data_t *ret;
    ret = (struct data_t*) malloc(sizeof(struct data_t));
    ret->datasize = size;
    ret->data = malloc (size);
    return ret;
}

struct data_t *data_create2(int size, void *data) {
    struct data_t *ret;
    ret = (struct data_t*) malloc(sizeof(struct data_t));
    ret->datasize = size;
    ret->data = data;
    return ret;
}

void data_destroy(struct data_t *data) {
    free(data->data);
    free(data);
}

struct data_t *data_dup(struct data_t *data) {
    struct data_t *dupe;
    dupe = data_create2(data->datasize, data->data);
    return dupe;
}

void data_replace(struct data_t *data, int new_size, void *new_data) {
    free(data->data);
    data->datasize = new_size;
    data->data = new_data;
}
