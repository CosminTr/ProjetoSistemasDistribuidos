#ifndef _TREE_SKEL_H
#define _TREE_SKEL_H

#include "sdmessage.pb-c.h"
#include "tree.h"
#include "message_private.h"
#include "zookeeper/zookeeper.h"
#include "network_client.h"

struct request_t {
    int op_n;       //o número da operação
    int op;         //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key;      //a chave a remover ou adicionar
    struct data_t* data;     // os dados a adicionar em caso de put, ou NULL em caso de delete
    //adicionar campo(s) necessário(s) para implementar fila do tipo produtor/consumidor
    struct request_t *next;
};

struct rtree_t {
    struct sockaddr_in server_socket; 
    int socket_num;

    //CONFIRM
    char *zk_identifier;
    char *zk_next_id;

    zhandle_t *zh;
    int is_connected;

};

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * A função deve lançar N threads secundárias responsáveis por atender 
 * pedidos de escrita na árvore.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init();

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy();

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(MessageT *msg);

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n);

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void *process_request(void *params);

int connectToZKServer(struct rtree_t *server, char *serverInfo);
int start_ts_zk(char *zk_addr, int serverPort);

#endif
