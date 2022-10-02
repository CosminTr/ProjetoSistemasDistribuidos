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

struct data_t *tree_get(struct tree_t *tree, char *key);

int tree_del(struct tree_t *tree, char *key);

int tree_size(struct tree_t *tree);

int tree_height(struct tree_t *tree);

char **tree_get_keys(struct tree_t *tree);

void **tree_get_values(struct tree_t *tree);

void tree_free_keys(char **keys);

void tree_free_values(void **values);

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