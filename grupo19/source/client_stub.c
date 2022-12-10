#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "message_private.h"
#include <signal.h>
#include "zookeeper/zookeeper.h"
#include <errno.h>

/*Trabalho realizado por
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

struct rtree_t *zkConn;

/* Zookeeper stuff */
// PRECISAMOS DO HOST_PORT GLOBAL?
#define ZDATALEN 1024 
struct rtree_t *head;
struct rtree_t *tail;
static char *root_path = "/chain";
typedef struct String_vector zoo_string;
zoo_string *children_list;
static char *watcher_ctx = "ZooKeeper Data Watcher";

/* --------------- */

//Preciso uma vez que ZNode é efemero?
void close_free(int sig){
    rtree_disconnect(zkConn);//nao interessa o parametro
    printf("Cliente fechou devido a Ctrl+C\n");
    exit(1);
}

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void *context){
    if (type == ZOO_SESSION_EVENT){
        if (state == ZOO_CONNECTED_STATE){
            zkConn->is_connected = 1;
        }
        else{
            zkConn->is_connected = 0;
        }
    }
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx){
    //filhos tiveram update entao precisamos ver novos head/tail e ligar a esses
    int data_len = 50;
    children_list = (zoo_string *)malloc(sizeof(zoo_string));
    char headPath[120];
    char tailPath[120];
    char *headInfo = malloc(data_len);
    char *tailInfo = malloc(data_len);
    if (state == ZOO_CONNECTED_STATE){
        if (type == ZOO_CHILD_EVENT){
            /* Get the updated children and reset the watch */
            if (ZOK != zoo_wget_children(zkConn->zh, root_path, child_watcher, watcher_ctx, children_list)){
                fprintf(stderr, "Error setting watch at %s!\n", root_path);
            }
            //conseguir nome do menor node e maior node(ex: node0000001)
            fprintf(stderr, "\n=== znode listing === [ %s ]", root_path);
            for (int i = 0; i < children_list->count; i++){
                head->zk_identifier = children_list->data[0];
                tail->zk_identifier = children_list->data[0];
                for (int i = 0; i < children_list->count; i++){
                    if (strcmp(children_list->data[i], head->zk_identifier) < 0)
                        head->zk_identifier = children_list->data[i];
                    if (strcmp(children_list->data[i], tail->zk_identifier) > 0)
                        tail->zk_identifier = children_list->data[i];
                }
            }
            fprintf(stderr, "\n=== done ===\n");
            //conseguir path do maior e menor node (ex: /chain/node000001)
            strcpy(headPath, root_path);
            strcat(headPath, head->zk_identifier);
            strcpy(tailPath, root_path);
            strcat(tailPath, tail->zk_identifier);
            //conseguir data do maior e menor node("IP:Port") e guardar em headInfo e tailInfo
            zoo_get(zkConn->zh, headPath, 0, headInfo, &data_len, NULL);
            zoo_get(zkConn->zh, tailPath, 0, tailInfo, &data_len, NULL);
            //conetar a esses servidores para enviar/receber pedidos
            //dados conexao mantidos em rtree head/tail
            if(connectToZKServer(head, headInfo) == -1){
                printf("Erro ao conetar a Head");
                exit(1);
            }
            if(connectToZKServer(tail, tailInfo) == -1){
                printf("Erro ao conetar a Tail");
                exit(1);
            }
        }
    }
    free(children_list);
    free(headInfo);
    free(tailInfo);
}

//return 0(OK) or -1 in case couldnt connect to server
int connectToZKServer(struct rtree_t *server, char *serverInfo){
    //VERIFICAR SE FUNCIONA
    char *host = strtok((char *)serverInfo, ":"); // hostname    removed:
    int port = atoi(strtok(NULL, ":"));     // port      '<' ':' '>'

    server->server_socket.sin_family = AF_INET;
    server->server_socket.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server->server_socket.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        return -1;
    }
    if (network_connect(server) == -1) { //ERRO
        return -1;
    }

    return 0;
}
struct rtree_t *rtree_connect(const char *address_port){
    zkConn = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    head = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    //head->zk_identifier = malloc(ZDATALEN);
    tail = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    //tail->zk_identifier = malloc(ZDATALEN);
    children_list = (zoo_string *)malloc(sizeof(zoo_string));
 
    // Ligar a ZooKeeper
    zkConn->zh = zookeeper_init(address_port, connection_watcher, 20000, 0, NULL, 0);
    if (zkConn->zh == NULL){
        fprintf(stderr, "Error connecting to ZooKeeper server[%d]!\n", errno);
        exit(EXIT_FAILURE);
    }
    sleep(3); //dorme para conectar

    // Colocar um Watcher para os servers + Conseguir head e tails
    // No exemplo PQ o while(1) e como saimos dele?
    int data_len = 50;
    char headPath[120];
    char tailPath[120];
    char *headInfo = malloc(data_len);
    char *tailInfo = malloc(data_len);

    if (zkConn->is_connected){
        // Possibilidade de usar um watch do /chain se nao existir (no valor 0)
        if (ZNONODE == zoo_exists(zkConn->zh, root_path, 0, NULL)){
            printf("Error: %s doesnt exist!!", root_path);
            //exit(1);
        }
        
        if (ZOK != zoo_wget_children(zkConn->zh, root_path, &child_watcher, watcher_ctx, children_list)){
            fprintf(stderr, "Error setting watch at %s!\n", root_path);
        }

        //conseguir nome do menor node e maior node(ex: node0000001)
        fprintf(stderr, "\n=== znode Head and Tail set === [ %s ]", root_path);
        head->zk_identifier = children_list->data[0];
        tail->zk_identifier = children_list->data[0];
        for (int i = 0; i < children_list->count; i++){
            if (strcmp(children_list->data[i], head->zk_identifier) < 0)
                head->zk_identifier = children_list->data[i];
            if (strcmp(children_list->data[i], tail->zk_identifier) > 0)
                tail->zk_identifier = children_list->data[i];
        }
        fprintf(stderr, "\n=== done ===\n");
        //conseguir path do maior e menor node (ex: /chain/node000001)
        strcpy(headPath, root_path);
        strcat(headPath, "/");
        strcat(headPath, head->zk_identifier);

        strcpy(tailPath, root_path);
        strcat(tailPath, "/");
        strcat(tailPath, tail->zk_identifier);
        //conseguir data do maior e menor node("IP:Port") e guardar em headInfo e tailInfo
        zoo_get(zkConn->zh, headPath, 0, headInfo, &data_len, NULL);
        zoo_get(zkConn->zh, tailPath, 0, tailInfo, &data_len, NULL);
    }
    printf("HEAD INFO: %s\n", headInfo);
    printf("TAIL INFO: %s\n", tailInfo);
    //conetar a esses servidores para enviar/receber pedidos
    //dados conexao mantidos em rtree head/tail
    if(connectToZKServer(head, headInfo) == -1){
        printf("Erro ao conetar a Head");
        return NULL;
    }
    if(connectToZKServer(tail, tailInfo) == -1){
        printf("Erro ao conetar a Tail");
        return NULL;
    }

    free(children_list);
    free(headInfo);
    free(tailInfo);

    signal(SIGINT, close_free);

    //nao interessa o que retorna
    return tail;
}
int rtree_disconnect(struct rtree_t *rtree) {
    if (network_close(head) != 0)
        return -1;
    if (network_close(tail) != 0)
        return -1;
    free(head->zk_identifier);
    free(head);
    free(tail->zk_identifier);
    free(tail);
    zookeeper_close(zkConn->zh);
    free(zkConn);
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
    
    msg = network_send_receive(head, msg);
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

    msg = network_send_receive(tail, msg);

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

    msg = network_send_receive(head, msg);
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

    msg = network_send_receive(tail, msg);
    if(msg == NULL)
        return -1;
    
    int size = msg->result;
    message_t__free_unpacked(msg, NULL);
    return size;
}

int rtree_height(struct rtree_t *rtree){
    MessageT *msg = message_create();
    if (msg == NULL)
        return -1;

    msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(tail, msg);
    if (msg == NULL)
        return -1;

    int height = msg->result;
    message_t__free_unpacked(msg, NULL);
    return height;
}

char **rtree_get_keys(struct rtree_t *rtree){
    MessageT *msg = message_create();
    if (msg == NULL)
        return NULL;

    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(tail, msg);
    if (msg == NULL)
        return NULL;

    char **keys = (char **)malloc((msg->n_keys + 1) * sizeof(char *));
    for (int i = 0; i < msg->n_keys; i++)
    {
        keys[i] = malloc(strlen(msg->keys[i]) + 1);
        strcpy(keys[i], msg->keys[i]);
    }
    keys[msg->n_keys] = '\0';

    message_t__free_unpacked(msg, NULL);
    return keys;
}

void **rtree_get_values(struct rtree_t *rtree){
    MessageT *msg = message_create();
    if (msg == NULL)
        return NULL;

    msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg = network_send_receive(tail, msg);
    if (msg == NULL)
        return NULL;

    void **values = malloc((msg->n_values + 1) * sizeof(*msg->values));
    for (int i = 0; i < msg->n_values; i++)
    {
        size_t size = msg->values[i].len;
        values[i] = malloc(size);
        memcpy(values[i], msg->values[i].data, size);
    }
    values[msg->n_values] = '\0';

    message_t__free_unpacked(msg, NULL);
    return values;
}

int rtree_verify(struct rtree_t *rtree, int op_n) {

    MessageT *msg = message_create();
    if (msg == NULL) {
        printf("Erro ao criar mensagem, c_s_verify\n");
        return -1;
    }
    msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT; 
    msg->op_n = op_n;

    msg = network_send_receive(tail, msg);
    if(msg == NULL) 
        return -1;
    
    int result = msg->result;

    message_t__free_unpacked(msg, NULL);
    return result;
}
