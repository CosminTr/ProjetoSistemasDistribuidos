#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "message_private.h"
#include <signal.h>

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

struct rtree_t *tree_remota;

void close_free(int sig){
    rtree_disconnect(tree_remota);
    printf("Cliente fechou devido a Ctrl+C\n");
    exit(1);
}

struct rtree_t *rtree_connect(const char *address_port) {
    tree_remota = malloc (sizeof(struct rtree_t));

    char *host = strtok((char *)address_port, ":"); // hostname    removed:
    int port = atoi(strtok(NULL, ":"));     // port      '<' ':' '>'
    
    tree_remota->server_socket.sin_family = AF_INET;
    tree_remota->server_socket.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &tree_remota->server_socket.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        return NULL;
    }
    if (network_connect(tree_remota) == -1) { //ERRO
        return NULL;
    }

    signal(SIGINT, close_free);
    
    return(tree_remota);
}
int rtree_disconnect(struct rtree_t *rtree) {
    if (network_close(rtree) != 0)
        return -1;
    
    free(tree_remota);
    return 0;
}

int rtree_put(struct rtree_t *rtree, struct entry_t *entry){
    MessageT *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->entry);

    if(msg->entry == NULL){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    msg->entry->data = (EntryT__DataT *) malloc(sizeof(EntryT__DataT));
    entry_t__data_t__init(msg->entry->data);

    if(msg->entry->data == NULL){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    msg->entry->key = entry->key;
    msg->entry->data->datasize = entry->value->datasize;
    msg->entry->data->data = entry->value->data;

    msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    
    msg = network_send_receive(rtree, msg);
    //Verificar se deu td correto
    if(msg == NULL)
        return -1;
    
    int op_n = msg->op_n; 
    
    message_t__free_unpacked(msg, NULL);
    return op_n;
        
}

struct data_t *rtree_get(struct rtree_t *rtree, char *key){
    MessageT *msg = message_create();
    if(msg == NULL){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    msg->entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->entry);
    msg->entry->data = (EntryT__DataT *) malloc(sizeof(EntryT__DataT));
    entry_t__data_t__init(msg->entry->data);
    
    if(msg->entry == NULL){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    msg->entry->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    msg = network_send_receive(rtree, msg);

    if(msg == NULL)
        return NULL;

    int data_len = msg->entry->data->datasize;
    struct data_t *data = data_create(data_len);
    memcpy(data->data, msg->entry->data->data, data_len);
    
    message_t__free_unpacked(msg, NULL);
    return data;

}

int rtree_del(struct rtree_t *rtree, char *key){
    MessageT *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->entry);
    
    if(msg->entry == NULL){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    msg->entry->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    msg = network_send_receive(rtree, msg);
    if(msg == NULL)
        return -1;

    int op_n = msg->op_n;

    message_t__free_unpacked(msg, NULL);
    return op_n;        
}

int rtree_size(struct rtree_t *rtree){
    MessageT *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg);
    if(msg == NULL)
        return -1;
    
    int size = msg->result;
    message_t__free_unpacked(msg, NULL);
    return size;
        
}

int rtree_height(struct rtree_t *rtree){
    MessageT *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg);
    if(msg == NULL)
        return -1;
    
    int height = msg->result;
    message_t__free_unpacked(msg, NULL);
    return height;
}

char **rtree_get_keys(struct rtree_t *rtree){
    MessageT *msg = message_create();
    if(msg == NULL)
        return NULL;

    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg);   
    if(msg == NULL)
        return NULL;
        
    char** keys = (char**) malloc((msg->n_keys + 1) * sizeof(char*));
    for (int i = 0; i < msg->n_keys; i++){
        keys[i] = malloc(strlen(msg->keys[i]) + 1);
        strcpy(keys[i], msg->keys[i]);
    }
    keys[msg->n_keys] = '\0';
    
    message_t__free_unpacked(msg, NULL);
    return keys;

}

void **rtree_get_values(struct rtree_t *rtree){
    MessageT *msg = message_create();
    if(msg == NULL)
        return NULL;

    msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg); 
    if(msg == NULL)
        return NULL;

    void **values = malloc((msg->n_values + 1) * sizeof(*msg->values));
    for (int i = 0; i < msg->n_values; i++){
        size_t size = msg->values[i].len;
        values[i] = malloc(size);
        memcpy(values[i], msg->values[i].data, size);
    }
    values[msg->n_values] = '\0';
    

    message_t__free_unpacked(msg, NULL);
    return values;
}

int rtree_verify(struct rtree_t *rtree, int op_n) {

    //Usar mais op_n?

    MessageT *msg = message_create();
    if (msg == NULL) {
        printf("Erro ao criar mensagem, c_s_verify\n");
        return -1;
    }
    msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY; //Unsure, needs new compiling
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT; 

    msg = network_send_receive(rtree, msg);
    if(msg == NULL) 
        return -1;
    
    int res = msg->op_n;

    message_t__free_unpacked(msg, NULL);
    return res;
}