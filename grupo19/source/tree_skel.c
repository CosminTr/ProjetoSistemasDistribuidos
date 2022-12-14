#include "tree_skel.h"
#include "tree.h"
#include <errno.h>
#include "message_private.h"
#include <pthread.h>
#include <netdb.h>
#include <string.h>

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

//zookeeper stuff ------------------------
struct rtree_t *zk_tree;
#define ZDATALEN 1024 
typedef struct String_vector zoo_string; 
zoo_string *children_list;
const char *zoo_path = "/chain";
static char *watcher_ctx = "ZooKeeper Data Watcher";
char* new_path;

//remote tree---------------
struct tree_t *rtree;

//Multiplexagem-------------
pthread_t thread;

int last_assigned = 1;
struct op_proc{     
    int max_proc;
    int *in_progress;// identificadores das operacoes de escrita em execucao
} op_current;

struct request_t *queue_head = NULL; 

pthread_mutex_t op_lock;
pthread_mutex_t tree_lock;
pthread_mutex_t queue_lock;
pthread_cond_t queue_not_empty;

int close_threads = 0;
//----------------------------
int tree_skel_init() {
    rtree = tree_create();
    if(rtree == NULL) {
        perror("Erro ao iniciar a tree - t_s_i");
        return -1;
    }

    op_current.in_progress = (int*) calloc(2, sizeof(int));
    op_current.in_progress[1] = -1;

    if(pthread_mutex_init(&op_lock, NULL) != 0){
        perror(strerror(errno));
        return -1;
    }
    
    if(pthread_mutex_init(&tree_lock, NULL) != 0){
        perror(strerror(errno));
        return -1;
    }

    if(pthread_mutex_init(&queue_lock, NULL) != 0){
        perror(strerror(errno));
        return -1;
    }

    if(pthread_cond_init(&queue_not_empty, NULL) != 0){
        perror(strerror(errno));
        return -1;
    }
    
    //Create
    if (pthread_create(&thread, NULL, &process_request, NULL) != 0){
		printf("\nThread não criada.\n");
		exit(EXIT_FAILURE);
	}
    
    return 0;
}

void tree_skel_destroy() {
    //free zk related-----
    free(new_path);
    free(zk_tree->zk_identifier);
    if(strcmp(zk_tree->next_server, "none")!=0)
        free(zk_tree->next_server);
    //--------------------
    //multithreaded related
    pthread_mutex_lock(&queue_lock);
    close_threads = 1;
    pthread_cond_broadcast(&queue_not_empty);
    pthread_mutex_unlock(&queue_lock);
    pthread_join(thread, NULL);
    free(op_current.in_progress);
    pthread_cond_destroy(&queue_not_empty);
    pthread_mutex_destroy(&queue_lock);
    pthread_mutex_destroy(&tree_lock);
    pthread_mutex_destroy(&op_lock);
    //-----------------------
    //strctural related(zk + tree)
    zookeeper_close(zk_tree->zh);
    free(zk_tree);
    tree_destroy(rtree);
    //--------------------------
}
//NOTA QUAIS OS CASOS DE ERRO PARA SIZE, HEIGHT, VERIFY, PUT, DEL?
int invoke(MessageT *msg) {
    MessageT__Opcode opcode = msg->opcode;
    //printf("\n-----------CODIGO: %d------------:\n", opcode);

    switch (opcode) {
        case MESSAGE_T__OPCODE__OP_PUT: ; //adicionar a fila(request_t queue_head)
            struct data_t *data_value = data_create(msg->entry->data->datasize);
            memcpy(data_value->data, msg->entry->data->data, msg->entry->data->datasize);
            pthread_mutex_lock(&queue_lock);
            //coloca elemento no fim da fila
            struct request_t *temporary1 = (struct request_t *) malloc(sizeof(struct request_t));
            temporary1->op_n = last_assigned;
            temporary1->op = 1;
            temporary1->key = malloc(strlen(msg->entry->key)+1);
            strcpy(temporary1->key, msg->entry->key);
            temporary1->data = data_value;
            temporary1->next = NULL;
            if(queue_head == NULL)
                queue_head = temporary1;
            else{ 
                struct request_t *current1 = queue_head;
                while(current1->next != NULL)
                    current1 = current1->next;
                current1->next = temporary1;
            }
            last_assigned++;
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->op_n = temporary1->op_n;
            pthread_cond_broadcast(&queue_not_empty);
            pthread_mutex_unlock(&queue_lock);
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
        case MESSAGE_T__OPCODE__OP_DEL: //adicionar a fila(request_t queue_head)
            pthread_mutex_lock(&queue_lock);
            //coloca elemento no fim da fila
            struct request_t *temporary2 = (struct request_t *) malloc(sizeof(struct request_t));
            temporary2->op_n = last_assigned;
            temporary2->op = 0;
            temporary2->key = malloc(strlen(msg->entry->key)+1);
            strcpy(temporary2->key, msg->entry->key);
            temporary2->data = NULL;
            temporary2->next = NULL;
            if(queue_head == NULL)
                queue_head = temporary2;
            else{ 
                struct request_t *current2 = queue_head;
                while(current2->next != NULL)
                    current2 = current2->next;
                current2->next = temporary2;
            }
            last_assigned++;
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->op_n = temporary2->op_n;
            pthread_cond_broadcast(&queue_not_empty);
            pthread_mutex_unlock(&queue_lock);
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
        case MESSAGE_T__OPCODE__OP_VERIFY:
            msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = verify(msg->op_n);// 1:True, 0:False
            return 0;
            break;
        default:
            return -1;
    }

}

//verifica se op_n esta na lista de in_progress para responmder aos clientes
int verify(int op_n) {
    //input invalido: String ou 0, os processos começam a ser identificados por 1.
    if (op_n == 0) {
        return -3;
    }
    //maior que o identifador do ultimo processado.
    if (op_n > op_current.max_proc)
        return -2;
    //potencial ponto critico para aceder a "in_progress" (talvez fazer no invoke antes de chamar esta funcao)
    if(*op_current.in_progress == op_n)
        return 1;//True
    
    return 0;//False
}

MessageT *createPutMessage(char *key, struct data_t *data){
    MessageT *msg = message_create();
    if(msg == NULL)
        return NULL;

    msg->entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->entry);

    if(msg->entry == NULL){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    msg->entry->data = (EntryT__DataT *) malloc(sizeof(EntryT__DataT));
    entry_t__data_t__init(msg->entry->data);

    if(msg->entry->data == NULL){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    //struct data_t *temp = data_dup(data);
    msg->entry->key = strdup(key);
    msg->entry->data->datasize = data->datasize;
    msg->entry->data->data = data->data;
    //memcpy(msg->entry->data->data, data->data, data->datasize);

    msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    return msg;
}
MessageT *createDelMessage(char *key){
   MessageT *msg = message_create();
    if(msg == NULL)
        return NULL;

    msg->entry = (EntryT *) malloc(sizeof(EntryT));
    entry_t__init(msg->entry);
    
    if(msg->entry == NULL){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    msg->entry->key = strdup(key);
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    return msg;
}   

//preciso mudar locks?
void *process_request(void *params){ 
    while(close_threads==0){
        pthread_mutex_lock(&queue_lock);
        while (queue_head == NULL){ // fila vazia->esperar
            pthread_cond_wait(&queue_not_empty, &queue_lock);
            if (close_threads == 1){
                pthread_mutex_unlock(&queue_lock);
                return NULL;
            }
        }
        struct request_t *current = queue_head; // pega no elemento do inicio da fila
        queue_head = current->next;

        // colocar o id na fila "in_progress"------------
        pthread_mutex_lock(&op_lock); 
        for (int i = 0; op_current.in_progress[i] != -1; i++)
            if (op_current.in_progress[i] != 0)
                op_current.in_progress[i] = current->op_n;
        pthread_mutex_unlock(&op_lock);
        //----------------------------------------------

        // processar o current request-------------------
        if (current->op == 1){ // put
            pthread_mutex_lock(&tree_lock);
            if (tree_put(rtree, current->key, current->data) == -1)
                printf("Error inserting entry\n");
            pthread_mutex_unlock(&tree_lock);
            if(strcmp(zk_tree->next_server, "none") != 0){
                network_send(zk_tree->socket_num, createPutMessage(current->key, current->data));
            }
        }
        else if (current->op == 0){ // del
            pthread_mutex_lock(&tree_lock);
            if (tree_del(rtree, current->key) == -1)
                printf("Key not found\n");
            pthread_mutex_unlock(&tree_lock);
            if(strcmp(zk_tree->next_server, "none") != 0){
                network_send(zk_tree->socket_num, createDelMessage(current->key));
            }
        }
        //----------------------------------------------
        pthread_mutex_lock(&op_lock);
        if (current->op_n > op_current.max_proc) // substituir max_proc se necessario
            op_current.max_proc = current->op_n;

        for (int i = 0; op_current.in_progress[i] != -1; i++) // remover id de in_progress
            if (op_current.in_progress[i] == current->op_n)
                op_current.in_progress[i] = 0;
        pthread_mutex_unlock(&op_lock);

        //dar free ao current
        free(current->key);
        if(strcmp(zk_tree->next_server, "none")==0)
            data_destroy(current->data);//for tail server
        else
            free(current->data);//for all other servers
        free(current);

        pthread_mutex_unlock(&queue_lock);
    }
    return NULL;
}

//NAO ESQUECER ARRANAJR FORMA DE DESLIGAR CONEXAO
int connectToZKServer(struct rtree_t *server, char *serverInfo){
    //printf("SERVER INFO: %s\n\n", serverInfo);
    //VERIFICAR SE FUNCIONA
    char *host = strtok((char *)serverInfo, ":"); // hostname    removed:
    int port = atoi(strtok(NULL, ":"));     // port      '<' ':' '>'

    server->server_socket.sin_family = AF_INET;
    server->server_socket.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server->server_socket.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        return -1;
    }
    //Criar socket TCP
    if((server->socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Erro ao criar socket TCP - Cliente");
        exit(1);
    }
    //printf("SOCKET NUMBER: %d\n\n\n", server->socket_num);
    
    //Estabelece conexao com o servidor
    if(connect(server->socket_num,(struct sockaddr *)&server->server_socket, sizeof(server->server_socket)) < 0){
        perror("Erro ao conetar ao servidor - Client");
        close(server->socket_num);
        exit(1);
    }

    // signal(SIGPIPE, conn_lost);

    printf("Conetado ao próximo servidor\n");

    return 0;
}
//from zchildwatcher.c
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			zk_tree->is_connected = 1; 
		} else {
			zk_tree->is_connected = 0; 
		}
	} 
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	//quando filhos fazem update encontrar o novo next_server(se houver) e conetar a ele
    children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
	 	   /* Get the updated children and reset the watch */ 
 			if (ZOK != zoo_wget_children(zk_tree->zh, zoo_path, child_watcher, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", zoo_path);
 			}
			//encontrar o proximo node
            //"next" tem o nome do node(ex:node000001)
            //estado inicial "0" serve como um NULL ou vazio
            char *next = malloc(ZDATALEN);
            strcpy(next, "0");
            printf("A reconetar ao próximo servidor...\n");
            //fprintf(stderr, "\n=== znode listing === [ %s ]", zoo_path); 
		    for (int i = 0; i < children_list->count; i++)  {
                //printf("\n%s", children_list->data[i]);
			    if(strcmp(next, "0")==0 && strcmp(children_list->data[i], zk_tree->zk_identifier)>0){
                    strcpy(next, children_list->data[i]);
                }
                if(strcmp(children_list->data[i], zk_tree->zk_identifier)>0 && strcmp(children_list->data[i], next)<0)
                    strcpy(next, children_list->data[i]);
            }
            sleep(2);
            //fprintf(stderr, "\n=== done ===\n");
            //printf("\nNEXTCHILD %s\n", next);

            //pode dar um problema de memoria
            //no caso de haver um node a seguir a nós(next != "0") conetar a ele
            if(strcmp(next, "0")!=0){
                //copiar o identificador do proximo server(ex: node000002)
                zk_tree->next_server= strdup(next);
                //conseguir data do next_server("IP:Port")
                int data_size = 50;
                char *data = malloc(data_size);
                char dataPath[120];
                strcpy(dataPath, zoo_path);
                strcat(dataPath, "/");
                strcat(dataPath, next);
                zoo_get(zk_tree->zh, dataPath, 0, data, &data_size, NULL);
                //conetar a esse server e guardar conexao em zk_tree
                connectToZKServer(zk_tree, data);
                free(data);
            }
            free(next);
		} 
	}
	free(children_list);
}

int start_ts_zk(const char *zk_addr, int serverPort) {
    //serverInfo(IP:Port) a partir do serverPort------------
    char hostbuf[256];
    struct hostent *host_entry;

    int hostname = gethostname(hostbuf, sizeof(hostbuf));
    host_entry = gethostbyname(hostbuf);
    //hostbuf: nome / IPbuf: XX.XX.XX.XXX
    char *IPbuf = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    
    char *port = malloc(sizeof(int)+1);
    char *serverInfo = malloc(40 * sizeof(char));
    sprintf(port, "%d", serverPort);
    strcpy(serverInfo, IPbuf);
    strcat(serverInfo, ":");
    strcat(serverInfo, port);
    free(port);
    //printf("IP SERVER: %s\n\n", IPbuf);
    //-------------------------
    zk_tree = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    
    zk_tree->next_server = "none";
    zk_tree->zh = zookeeper_init(zk_addr, connection_watcher, 20000, 0, NULL, 0);
    
    if (zk_tree->zh == NULL) {
        printf("Erro ao connectar com o servidor Zookeeper, t_s, (zookeeper_init) \n");
        return -1;
    }
    sleep(3); //dorme para conectar

    if (zk_tree->is_connected) {
        //se nao existe chain, criar
        if(ZNONODE == zoo_exists(zk_tree->zh, zoo_path, 0, NULL)) {
            //criar chain
            if (ZOK == zoo_create(zk_tree->zh, zoo_path, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
                printf("criado o nó chain. \n");
            } else {
                printf("Erro ao criar o node chain \n");
                return -1;
            }
        }
        //criar node de server atual
        char node_path[40] = "";
		strcat(node_path,zoo_path); 
		strcat(node_path,"/node"); 

        //path do nosso Znode depois de criado(ex: /chain/node000001)
		new_path = malloc (ZDATALEN);
        char nodeName[120];
        //criar node relativo ao server atual
        if (ZOK != zoo_create(zk_tree->zh, node_path, serverInfo, 40, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, ZDATALEN)) {
				fprintf(stderr, "Error creating znode from path %s!\n", node_path);
			    exit(EXIT_FAILURE);
		}
        free(serverInfo);
        strcpy(nodeName, new_path);
        strtok(nodeName, "/");
        zk_tree->zk_identifier = strdup(strtok(NULL, ""));
        //printf("\nNODENAME: %s\n\n", zk_tree->zk_identifier);
        printf("Server Iniciado\n");
        // colocar watcher nos filhos para se houver um update saber a quem me ligar
        if (ZOK != zoo_wget_children(zk_tree->zh, zoo_path, &child_watcher, watcher_ctx, children_list)) {
           printf("Erro ao criar um watcher para /chain");
        }

        return 0;
    }
    return -1;
}