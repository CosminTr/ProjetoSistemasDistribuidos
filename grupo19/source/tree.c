#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "tree-private.h"
#include "entry.h"
#include "data.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

struct tree_t *tree_create() {
    struct tree_t *tree = malloc(sizeof(struct tree_t));
    tree->data = NULL;
    tree->left = NULL;
    tree->right = NULL;

    if(tree != NULL)
        return tree;
    else 
        return NULL;
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

    return tree_put_recursive(tree, entry);
}

int tree_put_recursive(struct tree_t *tree, struct entry_t *entry) {
    if (tree == NULL) { //empty, ok over!
        return -1;
    }
    else if(tree->data == NULL){
        tree->data = entry;
        return 0;
    }
    else { 
        int comp = entry_compare(entry, tree->data);
        if (comp < 0) { //go left
            if(tree->left == NULL){
                struct tree_t *new_node = tree_create();
                tree->left = new_node;
            }
            return tree_put_recursive(tree->left, entry);
        }
        else if (comp >0) { //go right
            if(tree->right == NULL){
                struct tree_t *new_node = tree_create();
                tree->right = new_node;
            }
            return tree_put_recursive(tree->right, entry);
        }
        else if (comp == 0) { //already existing key
            entry_replace(tree->data, entry->key, entry->value);
            free(entry);
            return 0;
        }
        else {
            return -1; //something must've gone wrong
        }
    }
}

struct data_t *tree_get(struct tree_t *tree, char *key){
    if(tree == NULL || key == NULL){
        return NULL;
    }
    else{
        int comp = strcmp(tree->data->key, key);
        if(comp < 0){
            return tree_get(tree->right, key);
        }
        else if(comp > 0){
            return tree_get(tree->left, key);
        }
        else if(comp == 0){
            return data_dup(tree->data->value);
        }
        else 
            return NULL;
    }
}

int tree_del(struct tree_t *tree, char *key){
    struct data_t *temp = tree_get(tree, key);
    if(tree == NULL || key == NULL || temp == NULL){
        data_destroy(temp);
        return -1;
    }
    data_destroy(temp);
    
    tree = tree_del_recursive(tree, key);
    
    return 0;
}

struct tree_t *tree_del_recursive(struct tree_t *root, char *key){
    if(root == NULL)
        return root;
    
    //Key is in right child
    if(strcmp(root->data->key, key) < 0)
        root->right = tree_del_recursive(root->right, key);
    
    //Key is in left child
    else if(strcmp(root->data->key, key) > 0)
        root->left = tree_del_recursive(root->left, key);
    
    //Key is here
    else{
        //This node has right child or no child
        if(root->left == NULL){
            struct tree_t *temp = root->right;
            entry_destroy(root->data);
            free(root);
            return temp;
        }
        //This node has Left child or no child
        else if(root->right == NULL){
            struct tree_t *temp = root->left;
            entry_destroy(root->data);
            free(root);
            return temp;
        }
        //This node has 2 children
        struct tree_t *temp = minValTree(root->right);
            //replace entry of this node with a copy of the entry of node that will replace it
        entry_replace(root->data, strdup(temp->data->key), data_dup(temp->data->value));

        root->right = tree_del_recursive(root->right, temp->data->key);
    }

    return root;
}

struct tree_t* minValTree(struct tree_t* node){
    struct tree_t* current = node;
  
    /* loop down to find the leftmost leaf */
    while (current != NULL && current->left != NULL)
        current = current->left;
  
    return current;
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

char **tree_get_keys(struct tree_t *tree){ 
    int treeSize = tree_size(tree);
    char **key_list = malloc((treeSize + 1)* sizeof(*key_list)); 
    int *positionCounter = malloc(sizeof(int));
    *positionCounter = 0;
    
    get_keys_recursive(tree, positionCounter, key_list);
    key_list[treeSize] = NULL; 
    
    free(positionCounter);
    
    return key_list;
}

void get_keys_recursive(struct tree_t *tree, int *positionCounter, char **key_list){
    if(tree == NULL){
        return;
    }
    else{
        get_keys_recursive(tree->left, positionCounter, key_list);
        key_list[*positionCounter] = malloc(strlen(tree->data->key)+1);
        strcpy(key_list[*positionCounter], tree->data->key);
        *positionCounter += 1;
        get_keys_recursive(tree->right, positionCounter, key_list);
        return;
    }
}

void **tree_get_values(struct tree_t *tree){
    int treeSize = tree_size(tree);
    void **value_list = malloc((treeSize + 1) * sizeof(struct data_t));
    int *positionCounter = malloc(sizeof(int));
    *positionCounter = 0;

    get_values_recursive(tree, positionCounter, value_list);
    value_list[treeSize] = NULL;

    free(positionCounter);

    return value_list;
}

void get_values_recursive(struct tree_t *tree, int *positionCounter, void **value_list){
    if(tree == NULL){
        return;
    }
    else{
        get_values_recursive(tree->left, positionCounter, value_list); 
        value_list[*positionCounter] = data_dup(tree->data->value);
        *positionCounter += 1;
        get_values_recursive(tree->right, positionCounter, value_list);
        return;
    }
}

void tree_free_keys(char **keys) {
    int index = 0;
    while(keys[index] != NULL) {
        free(keys[index]);
        index +=1;
    }
    free(keys);
}

void tree_free_values(void **values) {
    int index = 0;
    while(values[index] != NULL) {
        data_destroy(values[index]);
        index +=1;
    }
    free(values);
}






