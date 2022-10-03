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

    //entry_destroy(entry); //destroy stuff before closing
    //data_destroy(duped_value);
    //free(duped_key);

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
            return data_dup(tree->data->value);
        }
        else 
            return NULL;
    }
}


struct tree_t* minValNode(struct tree_t* node){
    struct tree_t* current = node;
  
    /* loop down to find the leftmost leaf */
    while (current && current->left != NULL)
        current = current->left;
  
    return current;
};


int tree_del(struct tree_t *tree, char *key){
    //if the key to be deleted < root's key -> left subtree
    if (strcmp(key, tree->data->key) < 0)
        tree_del(tree->left, key);

    //if the key to be deleted > root's key -> right subtree
    else if(strcmp(key, tree->data->key) > 0)
        tree_del(tree->right, key);

    //if key to be deleted == root's key -> node to be deleted
    else{
        //No Children: node deleted
        if(tree->left == NULL && tree->right == NULL){
            entry_destroy(tree->data);
            return 0;
        }

        //One Child: replace root with minimum of sub-left tree
        else if(tree->left == NULL || tree->right==NULL){
            struct tree_t *temp;
            if(tree->left == NULL)
                temp = tree->right;
            else
                temp = tree->left;
            entry_replace(tree->data, temp->data->key, temp->data->value);
            entry_destroy(tree->data);
            return 0;
        }

        //Two Children
        else{
            struct tree_t *temp = minValNode(tree->right);
            tree->data = temp->data;
            tree_del(tree->right, temp->data->key);
            return 0;
        }
    }
    return -1;
    
}

int tree_size(struct tree_t *tree){
    if(tree == NULL || tree->data == NULL)
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
    
    get_keys_recursive(tree, positionCounter, key_list);
    key_list[treeSize + 1] = "/0"; 
    
    free(positionCounter);
    
    return key_list;
}

void **tree_get_values(struct tree_t *tree){
    int treeSize = tree_size(tree);
    void **value_list = malloc(treeSize + 1);
    int *positionCounter = malloc(sizeof(int));
    *positionCounter = 0;

    get_values_recursive(tree, positionCounter, value_list);
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
        tree = tree_create();
        tree->data = entry;
        return 0;
    }
    else if(tree->data == NULL){
        tree->data = entry;
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

void get_keys_recursive(struct tree_t *tree, int *positionCounter, char **key_list){
    if(tree == NULL){
        return;
    }
    else{
        get_keys_recursive(tree->left, positionCounter, key_list);
        key_list[*positionCounter] = malloc(strlen(tree->data->key)+1);
        memcpy(key_list[*positionCounter], tree->data->key, strlen(tree->data->key)+1);
        *positionCounter += 1;
        get_keys_recursive(tree->right, positionCounter, key_list);
        return;
    }
}

void get_values_recursive(struct tree_t *tree, int *positionCounter, void **value_list){
    if(tree == NULL){
        return;
    }
    else{
        get_values_recursive(tree->left, positionCounter, value_list);
        value_list[*positionCounter] = malloc(tree->data->value->datasize + 1);
        memcpy(value_list[*positionCounter], tree->data->value, tree->data->value->datasize+1);
        *positionCounter += 1;
        get_values_recursive(tree->right, positionCounter, value_list);
        return;
    }
}
