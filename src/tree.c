#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tree.h>
#include <tree-private.h>
#include <entry.h>
#include <data.h>

struct tree_t *tree_create() {
    struct tree_t *tree;
    tree = malloc(sizeof(struct tree_t));
    tree->data = NULL;
    tree->left = NULL;
    tree->right = NULL;
}

void tree_destroy(struct tree_t *tree) {
    if (tree == NULL) {
        return;
    }
    else {
        tree_destroy(tree->left);
        tree_destroy(tree->right);
        entry_destroy(tree->data);
        free(tree);
    }
}

int tree_put(struct tree_t *tree, char *key, struct data_t *value) {
    char *duped_key = malloc(strlen(key)+1);
    strcpy(duped_key, key);
    struct data_t *duped_value = data_dup(value);
    struct entry_t *entry = entry_create(duped_key, duped_value);

    int get_status = tree_put_recursive(tree, entry); //recursion starts here

    entry_destroy(entry); //destroy stuff before closing
    data_destroy(duped_value);
    free(duped_key);

    return get_status;
}

struct data_t *tree_get(struct tree_t *tree, char *key){
    if(tree == NULL || key == NULL){
        return NULL;
    }
    else{
        int comp = strcmp(tree->data->key, key);
        if(comp < 0){
            return tree_get(tree->left, key);
        }
        else if(comp > 0){
            return tree_get(tree->right, key);
        }
        else if(comp == 0){
            return data_dup(tree->data);
        }
        else 
            return NULL;
    }
}

int tree_del(struct tree_t *tree, char *key);

int tree_size(struct tree_t *tree){
    if(tree == NULL)
        return 0;
    else
        return tree_size(tree->left) + 1 + tree_size(tree->right);
}

int tree_height(struct tree_t *tree){
    if(tree == NULL){
        return 0;
    }
    else{
        int left_size = tree_height(tree->left);
        int right_size = tree_height(tree->right);
        if(left_size < right_size)
            return right_size + 1;
        else 
            return left_size + 1;
    }
}

char **tree_get_keys(struct tree_t *tree){ //get keys meio scuffed com funcao auxiliar...possivel melhorar(tentar retornar recursivamente uma lista de keys atualizada)
    int treeSize = tree_size(tree);
    char **key_list = malloc(treeSize + 1); // o +1 representa a ultima posicao a /0
    int *positionCounter = malloc(sizeof(int));
    *positionCounter = 0;
    
    get_list_recursive(tree, positionCounter, key_list);
    key_list[treeSize + 1] = "/0"; 
    
    free(positionCounter);
    
    return key_list;
}

void **tree_get_values(struct tree_t *tree){
    int treeSize = tree_size(tree);
    void **value_list = malloc(treeSize + 1);
    int *positionCounter = malloc(sizeof(int));
    *positionCounter = 0;

    get_list_recursive(tree, positionCounter, value_list);
    value_list[treeSize + 1] = "/0";

    free(positionCounter);

    return value_list;
}

void tree_free_keys(char **keys) {
    int index = 0;
    while(*(keys+index) != NULL) {
        free(*(keys+index));
        index +=1;
    }
    free(keys);
}

void tree_free_values(void **values) {
    int index = 0;
    while(*(values+index) != NULL) {
        free(*(values+index));
        index +=1;
    }
    free(values);
}

int tree_put_recursive(struct tree_t *tree, struct entry_t *entry) {
    if (tree == NULL) { //vazio, ok over!
        entry_replace(tree->data, entry->key, entry->value);
        return 0;
    }
    else { 
        int comp = entry_compare(entry, tree->data);
        if (comp < 0) { //go left
            return tree_put_recursive(tree->left, entry);
        }
        else if (comp >0) { //go right
            return tree_put_recursive(tree->right, entry);
        }
        else if (comp == 0) { //already existing key
            entry_replace(tree->data, entry->key, entry->value);
            return 0;
        }
        else {
            return -1; //something must've gone worng
        }
    }
}

void get_list_recursive(struct tree_t *tree, int *positionCounter, void **list){
    if(tree == NULL){
        return;
    }
    else{
        get_list_recursive(tree->left, positionCounter, list);
        *positionCounter += 1;
        memcpy(list[*positionCounter], tree->data->key, strlen(tree->data->key)+1);
        get_list_recursive(tree->right, positionCounter, list);
        return;
    }
}