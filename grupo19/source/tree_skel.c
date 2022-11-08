#include "tree_skel.h"
#include "tree.h"
#include <errno.h>
#include "message_private.h"
#include <pthread.h>

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

struct tree_t *rtree;

//Multiplexagem-------------
int last_assigned = 1;
struct op_proc{        //maybe move this somewhere else
    int max_proc;
    int *in_progress;// identificadores das operacoes de escrita em execucao
} op_current;

struct request_t *queue_head = NULL; //INICIALIZAR A NULL?

pthread_mutex_t op_lock;
pthread_mutex_t tree_lock;
pthread_mutex_t queue_lock;
pthread_cond_t queue_not_empty;
//----------------------------


int tree_skel_init(int N) {
    rtree = tree_create();
    if(rtree == NULL) {
        perror("Erro ao iniciar a tree - t_s_i");
        return -1;
    }

    op_current.in_progress = (int*) malloc(N * sizeof(int));

    if(pthread_mutex_init(&op_lock, NULL) != 0){//POTENCIAL MEM LOSS
        perror(strerror(errno));
        return -1;
    }
    
    if(pthread_mutex_init(&tree_lock, NULL) != 0){//POTENCIAL MEM LOSS
        perror(strerror(errno));
        return -1;
    }

    if(pthread_mutex_init(&queue_lock, NULL) != 0){//POTENCIAL MEM LOSS
        perror(strerror(errno));
        return -1;
    }

    if(pthread_cond_init(&queue_not_empty, NULL) != 0){//POTENCIAL MEM LOSS
        perror(strerror(errno));
        return -1;
    }

    pthread_t thread[N];
    int *r;

    //Create
    for (int i = 0; i < N; i++){
        if (pthread_create(thread[i], NULL, &process_request, NULL) != 0){
			printf("\nThread %d não criada.\n", i);
			exit(EXIT_FAILURE);
		}
    }

    //Join--------POTENCIAIS ERROS COM A COLOCACAO DO JOIN
    for (int i = 0; i < N; i++){
        if (pthread_join(thread[i], (void**) &r) != 0){
			printf("\nErro no join\n");
			exit(EXIT_FAILURE);
		}
    }
    
    return 0;
}

void tree_skel_destroy() {
    tree_destroy(rtree);
    free(op_current.in_progress);
    pthread_cond_destroy(&queue_not_empty);
    pthread_mutex_destroy(&queue_lock);
    pthread_mutex_destroy(&tree_lock);
    pthread_mutex_destroy(&op_lock);
}
//NOTA QUAIS OS CASOS DE ERRO PARA SIZE, HEIGHT, VERIFY, PUT, DEL?
//INCOMPLETA
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
        case MESSAGE_T__OPCODE__OP_VERIFY:
            msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = verify(msg->op_n);// 1:True, 0:False
        default:
            return -1;
    }

}

//verifica se op_n esta na lista de in_progress para responmder aos clientes
int verify(int op_n) {
    //potencial ponto critico para aceder a "in_progress" (talvez fazer no invoke antes de chamar esta funcao)
    for (int i = 0; i < (sizeof(op_current.in_progress)/sizeof(int)); i++){
        if(op_current.in_progress[i] == op_n)
        return 1;//True
    }
    return 0;//False
}

void *process_request(void *params){ //WHAT ARE THE *PARAMS??
    pthread_mutex_lock(&queue_lock);
    while(queue_head == NULL)// fila vazia->esperar
        pthread_cond_wait(&queue_not_empty, &queue_lock);
    struct request_t *current = queue_head;
    queue_head = current->next;
    
    //colocar o id na fila "in_progress"------------
    pthread_mutex_lock(&op_lock);//talvez um trylock??
    for(int i = 0; i < (sizeof(op_current.in_progress)/sizeof(int)); i++)
        if(op_current.in_progress[i] != 0)
            op_current.in_progress[i] = current->op_n;
    pthread_mutex_unlock(&op_lock);
    //----------------------------------------------

    //processar o current request-------------------
    if(current->op == 1){//put
        struct data_t *data_value = data_create(strlen(current->data));
        memcpy(data_value->data, current->data, strlen(current->data)+1);
        pthread_mutex_lock(&tree_lock);
        tree_put(rtree, current->key, data_value);
        data_destroy(data_value);
        pthread_mutex_unlock(&tree_lock);
    }else if(current->op == 0){//del
        pthread_mutex_lock(&tree_lock);
        tree_del(rtree, current->key);
        pthread_mutex_unlock(&tree_lock);
    }
    //----------------------------------------------
    //DAR FREE AO CURRENT??
    pthread_mutex_unlock(&queue_lock);
}