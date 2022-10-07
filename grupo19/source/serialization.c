#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serialization.h"

int keyArray_to_buffer(char **keys, char **keys_buf){
    if(keys  == NULL){
        return -1;
    }

    int num_words = 0;
    int num_letters = 0;

    while (keys[num_words] != NULL)
    {
        num_letters = num_letters + strlen(keys[num_words])+1;
        num_words += 1;
    }

    keys_buf = malloc(sizeof(int)*num_words + num_letters);

    int size = 0;
    int offset = sizeof(int);
    memcpy(keys_buf, &num_words, sizeof(int));
    
    for (int i = 0; i < num_words; i++){
        size = strlen(keys[i])+1;

        memcpy(offset + keys_buf, &size, sizeof(int));
        offset = offset + sizeof(int);

        memcpy(offset + keys_buf, keys[i], size);
        offset = offset + size;
    }

    return offset;
}


char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){
    int offset = sizeof(int);
    int num_words;
    memcpy(&num_words, keys_buf, sizeof(int));

    char **key_list = malloc((num_words+1) * sizeof(*key_list));

    int sizee = 0;
    int j=0;
    while (offset < keys_buf_size){
        memcpy(&sizee, offset + keys_buf, sizeof(int));
        offset = offset + sizeof(int);
        key_list[j] = malloc(sizee);
        memcpy(key_list[j], offset + keys_buf, sizee);
        offset = offset + sizee;
        j += 1;
    }
    key_list[j] = NULL;

    return key_list;
} 

