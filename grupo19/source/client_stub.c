#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "message_private.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

struct rtree_t *tree_remota;

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
    printf("Client Connected\n");
    return(tree_remota);
}
int rtree_disconnect(struct rtree_t *rtree) {
    if (network_close(rtree) != 0)
        return -1;
    
    free(tree_remota);
    return 0;
}

int rtree_put(struct rtree_t *rtree, struct entry_t *entry){
    struct message_t *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->message.entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->message.entry);

    if(msg->message.entry == NULL){
        //dar free ao message caso entry deu mau malloc
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }

    msg->message.entry->data = (DataT *) malloc(sizeof(DataT));
    data_t__init(msg->message.entry->data);

    if(msg->message.entry->data == NULL){
        entry_t__free_unpacked(msg->message.entry, NULL);//necessario?
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }

    msg->message.entry->key = entry->key;
    msg->message.entry->data->datasize = entry->value->datasize;
    msg->message.entry->data->data = entry->value->data;

    msg->message.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    msg = network_send_receive(rtree, msg);
    //Verificar se deu td correto
    if(msg == NULL)
        return -1;
    else{
        message_t__free_unpacked(&msg->message, NULL);
        return 0;
    }
        
}

struct data_t *rtree_get(struct rtree_t *rtree, char *key){
    struct message_t *msg = message_create();
    if(msg == NULL){
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }

    msg->message.entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->message.entry);
    msg->message.entry->data = (DataT *) malloc(sizeof(DataT));
    data_t__init(msg->message.entry->data);
    
    if(msg->message.entry == NULL){
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }

    msg->message.entry->key = key;
    msg->message.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    msg = network_send_receive(rtree, msg);

    if(msg == NULL)
        return NULL;

    int data_len = msg->message.entry->data->datasize;
    struct data_t *data = data_create(data_len);
    memcpy(data->data, msg->message.entry->data->data, data_len);
    
    message_t__free_unpacked(&msg->message, NULL);
    return data;

}

int rtree_del(struct rtree_t *rtree, char *key){
    struct message_t *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->message.entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->message.entry);
    
    if(msg->message.entry == NULL){
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }

    msg->message.entry->key = key;
    msg->message.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    msg = network_send_receive(rtree, msg);
    if(msg == NULL)
        return -1;
    else {
        message_t__free_unpacked(&msg->message, NULL);
        return 0;
    }
        
}

int rtree_size(struct rtree_t *rtree){
    struct message_t *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->message.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg);
    if(msg == NULL)
        return -1;
    else{
        int size = msg->message.result;
        message_t__free_unpacked(&msg->message, NULL);
        return size;
    } 
        
}

int rtree_height(struct rtree_t *rtree){
    struct message_t *msg = message_create();
    if(msg == NULL)
        return -1;

    msg->message.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg);
    if(msg == NULL)
        return -1;
    else{
        int height = msg->message.result;
        message_t__free_unpacked(&msg->message, NULL);
        return height;
    } 
}

char **rtree_get_keys(struct rtree_t *rtree){
    struct message_t *msg = message_create();
    if(msg == NULL)
        return NULL;

    msg->message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg);   
    if(msg == NULL)
        return NULL;
    else{
        char **keys = msg->message.keys;
        return keys;
    } 
}

void **rtree_get_values(struct rtree_t *rtree){
    struct message_t *msg = message_create();
    if(msg == NULL)
        return NULL;

    msg->message.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(rtree, msg); 
    if(msg == NULL)
        return NULL;
    else{
        void **values = (void **)msg->message.values;
        return values;
    } 
}