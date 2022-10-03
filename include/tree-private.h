#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"

struct tree_t {

	struct entry_t *data;
	
	struct tree_t *left;
	struct tree_t *right;
};

struct tree_t *minValNode(strct tree_t *node){
	struct tree_t* current = node;
  
    /* loop down to find the leftmost leaf */
    while (current && current->left != NULL)
        current = current->left;
  
    return current;
};

int tree_put_recursive(struct tree_t *tree, struct entry_t *entry);

void get_keys_recursive(struct tree_t *tree, int *positionCounter, char **key_list);

void get_values_recursive(struct tree_t *tree, int *positionCounter, void **value_list);

#endif
