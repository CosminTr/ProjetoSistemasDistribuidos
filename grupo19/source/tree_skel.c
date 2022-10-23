#include "tree_skel.h"
#include "tree.h"
#include <errno.h>
#include "message_private.h"

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
        case MESSAGE_T__OPCODE__OP_SIZE:
            msg->message.opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            // vvvvvvvvvvvvvvvvvvvvvvvv
            //int size = tree_size(rtree);
            //msg->message.result = size;
            // IS THIS RIGHT? ^
            return 0;
            break;

        case MESSAGE_T__OPCODE__OP_HEIGHT:
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
            
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS:
            
            break;
        case MESSAGE_T__OPCODE__OP_GETVALUES:
            
            break;
        default:
            return -1;
    }
}