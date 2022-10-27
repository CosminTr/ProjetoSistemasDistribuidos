#include "tree_skel.h"
#include "tree.h"
#include <errno.h>

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

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
        case MESSAGE_T__OPCODE__OP_PUT: ;
            struct data_t *data_value = data_create2(msg->message.entry->data->datasize, msg->message.entry->data->data);
            if (tree_put(rtree, msg->message.entry->key, data_value) == -1){//não existe
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                //data_destroy(data_value);
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            //data_destroy(data_value);
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_GET: ;
            struct data_t *data = tree_get(rtree, msg->message.entry->key);
            if (data == NULL) { //não existe
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            msg->message.entry->data->data = data->data;
            msg->message.entry->data->datasize = data->datasize;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_DEL:
            if (tree_del(rtree, msg->message.entry->key) == -1) { //não existe
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_SIZE://falta so ver os erros
            msg->message.opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            int size = tree_size(rtree);//merge with line below
            msg->message.result = size;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_HEIGHT: //falta ver os erros
            msg->message.opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            int height = tree_height(rtree);//merge with line below
            msg->message.result = height;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS: ;
            char** keys = tree_get_keys(rtree);
            if(keys == NULL){//potencial problema de memoria por nao dar free 
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->message.n_keys = tree_size(rtree);
            msg->message.keys = keys;
            return 0; 
            break;
        case MESSAGE_T__OPCODE__OP_GETVALUES: ;//Verificar quao bem funciona este keyArray_to_buf com values
            void** values = tree_get_values(rtree);
            if(values == NULL){//potencial problema de memoria por nao dar free 
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEYS;

            // for (int i = 0; i < treeSize; i++){
            //     msg->message.values[i] = malloc(sizeof(ProtobufCBinaryData));
            //     msg->message.values[i].len = sizeof(values[i]);
            //     msg->message.values[i].data = (uint8_t*)values[i];
            // }
            // msg->message.values = (ProtobufCBinaryData *)values;
            msg->message.n_values = tree_size(rtree);
            msg->message.values = (char**)values;
            return 0; 
            break;
        default:
            return -1;
    }
}