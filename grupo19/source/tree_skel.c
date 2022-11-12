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

int close_threads = 0;
//----------------------------


int tree_skel_init(int N) {
    rtree = tree_create();
    if(rtree == NULL) {
        perror("Erro ao iniciar a tree - t_s_i");
        return -1;
    }

    op_current.in_progress = (int*) calloc(N+1, sizeof(int));
    op_current.in_progress[N] = -1; //terminator

    if(pthread_mutex_init(&op_lock, NULL) != 0){//POTENCIAL MEM LOSS (se der return antes de libertar o resto)
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

    //Create
    for (int i = 0; i < N; i++){
        if (pthread_create(&thread[i], NULL, &process_request, NULL) != 0){
			printf("\nThread %d não criada.\n", i);
			exit(EXIT_FAILURE);
		}
    }

    //Join--------POTENCIAIS ERROS COM A COLOCACAO DO JOIN
    for (int i = 0; i < N; i++){
        if (pthread_detach(thread[i]) != 0){
			printf("\nErro no detach\n");
			exit(EXIT_FAILURE);
		}
    }
    
    return 0;
}

void tree_skel_destroy() {
    pthread_mutex_lock(&queue_lock);
    close_threads = 1;
    pthread_cond_broadcast(&queue_not_empty);
    pthread_mutex_unlock(&queue_lock);
    free(op_current.in_progress);
    pthread_cond_destroy(&queue_not_empty);
    pthread_mutex_destroy(&queue_lock);
    pthread_mutex_destroy(&tree_lock);
    pthread_mutex_destroy(&op_lock);
    tree_destroy(rtree);
}
//NOTA QUAIS OS CASOS DE ERRO PARA SIZE, HEIGHT, VERIFY, PUT, DEL?
int invoke(MessageT *msg) {
    MessageT__Opcode opcode = msg->opcode;

    switch (opcode) {
        case MESSAGE_T__OPCODE__OP_PUT: ; //adicionar a fila(request_t queue_head)
            struct data_t *data_value = data_create(msg->entry->data->datasize);
            memcpy(data_value->data, msg->entry->data->data, msg->entry->data->datasize);
            //NOTA por o lock ca em cima OU antes de while mas colocar 
            //o temporary = last_assigned la dentro pra garantir
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
            //NOTA por o lock ca em cima OU antes de while mas colocar 
            //o temporary = last_assigned la dentro pra garantir
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
    for (int i = 0; op_current.in_progress[i] != -1; i++){
        if(op_current.in_progress[i] == op_n)
        return 1;//True
    }
    return 0;//False
}

void *process_request(void *params){ // WHAT ARE THE *PARAMS??
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
    pthread_mutex_lock(&op_lock); // talvez um trylock??
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
    }
    else if (current->op == 0){ // del
        pthread_mutex_lock(&tree_lock);
        if (tree_del(rtree, current->key) == -1)
            printf("Key not found\n");
        pthread_mutex_unlock(&tree_lock);
    }
    //----------------------------------------------
    pthread_mutex_lock(&op_lock);
    if (current->op_n > op_current.max_proc) // substituir max_proc se necessario
        op_current.max_proc = current->op_n;

    for (int i = 0; op_current.in_progress[i] != -1; i++) // remover id de in_progress
        if (op_current.in_progress[i] == current->op_n)
            op_current.in_progress[i] = 0;
    pthread_mutex_unlock(&op_lock);

    // DAR FREE AO CURRENT??
    free(current->key);
    data_destroy(current->data);
    free(current);

    pthread_mutex_unlock(&queue_lock);
    return NULL;
}