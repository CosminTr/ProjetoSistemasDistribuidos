#include "network_client.h"
#include <errno.h>

int network_connect(struct rtree_t *rtree){
    
    //Criar socket TCP
    if((rtree->socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Erro ao criar socket TCP - Cliente");
        return -1;
    }

    //Preenche estrutura para o servidor na rtree para estabelecer conexão
    /* Como ficaria a funcao inet de client_stub
    if (inet_pton(AF_INET, rtree->hostname, rtree->server_socket.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(rtree->socket_num); // ou network_close(rtree->socket_num);?
        return -1;
    }
    */

    //Estabelece conexao com o servidor
    if(connect(rtree->socket_num,(struct sockaddr *)rtree->server_socket, sizeof(rtree->server_socket)) < 0){
        perror("Erro ao conetar ao servidor - Client");
        close(rtree->socket_num);
        return -1;
    }

    return 0;
}

struct message_t *network_send_receive(struct rtree_t * rtree, struct message_t *msg){
    return NULL;
}

int network_close(struct rtree_t * rtree){
    close(rtree->socket_num);
    return 0;
}