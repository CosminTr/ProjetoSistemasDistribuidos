#include "network_client.h"
#include <errno.h>
#include "client_stub-private.h" 
#include "inet.h"
#include "message_private.h"


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

struct message_t *network_send_receive(struct rtree_t * rtree, struct message_t *msg){
    int msglen = message_t__get_packed_size(&msg->message);
    int resposta_len;
    uint8_t *buffer = malloc(msglen); //podera ser char em vez de uint8_t?
    //send
    message_t__pack(&msg->message, buffer);
    // int netlong = htonl(msglen);
    // write(rtree->socket_num, &netlong, sizeof(int));

    int resultado;

    while (msglen > 0) {
        resultado = write(rtree->socket_num, buffer, msglen);
        if (resultado < 0) {
            //deu erro
            return NULL;

        }
        msglen = msglen - resultado;
        buffer = buffer + resultado;
    }

    free(buffer);

    //receive
    read(rtree->socket_num, &resposta_len, sizeof(int));
    resposta_len = ntohl(resposta_len);
    uint8_t resp[resposta_len];

    int index = 0;
    int resultado;
    while (index < resposta_len) {
        resultado = read(rtree->socket_num, resp + index, resposta_len - index);
        if (resultado < 1) {
            //erro
            return NULL; //o que fazer em caso de resultado = 0?
        }
        index = index + resultado;

    }
    resp[resposta_len] = '\0';

    //CONFUSAO AQUI 
    MessageT *temp = message_t__unpack(NULL, resposta_len, resp);
    msg->message = *temp;
    return NULL;
}

int network_close(struct rtree_t * rtree){
    close(rtree->socket_num);
    return 0;
}