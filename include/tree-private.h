#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"

struct tree_t {

	struct entry_t *data;
	
	struct tree_t *left;
	struct tree_t *right;
};

struct tree_t *minValTree(struct tree_t *node);

struct tree_t *tree_del_recursive(struct tree_t *root, char* key);

int tree_put_recursive(struct tree_t *tree, struct entry_t *entry);

void get_keys_recursive(struct tree_t *tree, int *positionCounter, char **key_list);

void get_values_recursive(struct tree_t *tree, int *positionCounter, void **value_list);

#endif
