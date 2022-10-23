#include "tree_skel.h"
#include "tree.h"
#include <errno.h>
#include "message_private.h"
#include "serialization.h"

struct tree_t *rtree;

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

int invoke(struct message_t *msg) {
    MessageT__Opcode opcode = msg->message.opcode;

    switch (opcode) {
        case MESSAGE_T__OPCODE__OP_SIZE://falta so ver os erros
            msg->message.opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            // vvvvvvvvvvvvvvvvvvvvvvvv
            //int size = tree_size(rtree);
            //msg->message.result = size;
            // IS THIS RIGHT? ^ yes
            return 0;
            break;

        case MESSAGE_T__OPCODE__OP_HEIGHT: //falta ver os erros
            msg->message.opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            //int height = tree_height(rtree);
            //msg->message.result = height;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_DEL:
            int res = tree_del(rtree, msg->message.entry->key);
            if (res == -1) { //não existe
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_GET:
            struct data_t *data = tree_get(rtree, msg->message.entry->key);
            if (data == NULL) { //não existe
                msg->message.opcode = MESSAGE_T__OPCODE__OP_GET + 1;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->message.entry->data->data = NULL;
                msg->message.entry->data->datasize = 0;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            msg->message.entry->data->data = data->data;
            msg->message.entry->data->datasize = data->datasize;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_PUT:
            if (tree_put(rtree, msg->message.entry->key, msg->message.entry->data) == -1){//não existe
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS:
            char** keys = tree_get_keys(rtree);
            char** keys_buf;
            if(keyArray_to_buffer(keys, keys_buf) == -1){//potencial problema de memoria por nao dar free 
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->message.data = keys_buf;
            return 0; 
            break;
        case MESSAGE_T__OPCODE__OP_GETVALUES://Verificar quao bem funciona este keyArray_to_buf com values
            char** values = tree_get_values(rtree);
            char** values_buf;
            if(keyArray_to_buffer(values, values_buf) == -1){//potencial problema de memoria por nao dar free 
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->message.data = values_buf;
            return 0; 
            break;
        default:
            return -1;
    }
}