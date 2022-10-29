#include "network_client.h"
#include <errno.h>
#include "client_stub-private.h" 
#include "inet.h"
#include "message_private.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

int network_connect(struct rtree_t *rtree){
    
    //Criar socket TCP
    if((rtree->socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Erro ao criar socket TCP - Cliente");
        return -1;
    }
    
    //Estabelece conexao com o servidor
    if(connect(rtree->socket_num,(struct sockaddr *)&rtree->server_socket, sizeof(rtree->server_socket)) < 0){
        perror("Erro ao conetar ao servidor - Client");
        close(rtree->socket_num);
        return -1;
    }

    return 0;
}

MessageT *network_send_receive(struct rtree_t * rtree, MessageT *msg){
    int msglen = message_t__get_packed_size(msg);
    int resposta_len ;
    uint8_t *buffer = malloc(msglen); //mudar? uint8_t *buffer[msglen]; nao pq e pointer
    
    //send
    message_t__pack(msg, buffer);
    int netlong = htonl(msglen);
    write(rtree->socket_num, &netlong, sizeof(int));
    write_all(rtree->socket_num, buffer, msglen);

    free(buffer);
    message_t__free_unpacked(msg, NULL);
    
    //receive
    read(rtree->socket_num, &resposta_len, sizeof(int));
    resposta_len = ntohl(resposta_len);
    uint8_t resp[resposta_len];
    read_all(rtree->socket_num, resp, resposta_len);
    
    MessageT *ret = message_t__unpack(NULL, resposta_len, resp);

    if (ret->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(ret, NULL); //????
        return NULL;
    }
    
    return ret;
    
}

int network_close(struct rtree_t * rtree){
    
    return close(rtree->socket_num);
}