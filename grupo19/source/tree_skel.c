#include "tree_skel.h"
#include "tree.h"
#include <errno.h>
#include "message_private.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

struct tree_t *rtree;
int last_assigned;

struct op_proc{        //maybe move this somewhere else
    int max_proc;
    int in_progress[];           // UNSURE WHAT SIZE TO GIVE
    //
} o_p_current;
struct request_t *queue_head;

int tree_skel_init() {
    rtree = tree_create();
    if(rtree == NULL) {
        perror("Erro ao iniciar a tree - t_s_i");
        return -1;
    }
    return 0;
}

void tree_skel_destroy() {
    tree_destroy(rtree);
}

int invoke(MessageT *msg) {
    MessageT__Opcode opcode = msg->opcode;

    switch (opcode) {
        case MESSAGE_T__OPCODE__OP_PUT: ;
            struct data_t *data_value = data_create(msg->entry->data->datasize);
            memcpy(data_value->data, msg->entry->data->data, msg->entry->data->datasize);
            if (tree_put(rtree, msg->entry->key, data_value) == -1){//não existe
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                data_destroy(data_value);
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            data_destroy(data_value);
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_GET: ;
            struct data_t *data = tree_get(rtree, msg->entry->key);
            if (data == NULL) { //não existe
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            msg->entry->data->data = data->data;
            msg->entry->data->datasize = data->datasize;

            free(data);
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_DEL:
            if (tree_del(rtree, msg->entry->key) == -1) { //não existe
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_SIZE:
            msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = tree_size(rtree);
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_HEIGHT: 
            msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = tree_height(rtree);
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS: ;
            char** keys = tree_get_keys(rtree);
            if(keys == NULL){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                tree_free_keys(keys);
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->n_keys = tree_size(rtree);
            msg->keys = keys;
            return 0; 
            break;
        case MESSAGE_T__OPCODE__OP_GETVALUES: ;
            void** values = tree_get_values(rtree);
            if(values == NULL){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                tree_free_values(values);
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

            msg->n_values = tree_size(rtree);
            msg->values = (ProtobufCBinaryData*) malloc(msg->n_values * sizeof(ProtobufCBinaryData));
            struct data_t *temp;
            for (int i = 0; i < msg->n_values; i++){
                temp = (struct data_t*) values[i];
                msg->values[i].len = temp->datasize;
                msg->values[i].data = temp->data;
                free(values[i]);
            }
            free(values);
            
            return 0; 
            break;
        default:
            return -1;
    }
}