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
        case MESSAGE_T__OPCODE__OP_SIZE://falta so ver os erros
            msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            int size = tree_size(rtree);//merge with line below
            msg->result = size;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_HEIGHT: //falta ver os erros
            msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            int height = tree_height(rtree);//merge with line below
            msg->result = height;
            return 0;
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS: ;
            char** keys = tree_get_keys(rtree);
            if(keys == NULL){//potencial problema de memoria por nao dar free 
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->n_keys = tree_size(rtree);
            msg->keys = keys;
            return 0; 
            break;
        case MESSAGE_T__OPCODE__OP_GETVALUES: ;//Verificar quao bem funciona este keyArray_to_buf com values
            void** values = tree_get_values(rtree);
            if(values == NULL){//potencial problema de memoria por nao dar free 
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

            msg->n_values = tree_size(rtree);
            msg->values = (ProtobufCBinaryData*) malloc(msg->n_values * sizeof(ProtobufCBinaryData));
            struct data_t *temp;
            for (int i = 0; i < msg->n_values; i++){
                //funciona para void* mas na temos maneira de saber o tamanho
                // int size = sizeof(values[i]);
                // msg->values[i].len = size;
                // msg->values[i].data = malloc(size);
                // msg->values[i].data = values[i];

                temp = (struct data_t*) values[i];
                msg->values[i].len = temp->datasize;
                msg->values[i].data = temp->data;
                free(values[i]);
            }
            free(temp);
            free(values);
            
            return 0; 
            break;
        default:
            return -1;
    }
}