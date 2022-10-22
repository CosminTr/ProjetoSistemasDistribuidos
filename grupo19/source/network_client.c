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
    //Ja esta no stub 
    // //UNSURE ABOUT THIS BIT
    // rtree->server_socket.sin_family = AF_INET;
    // rtree->server_socket.sin_port = htons(atoi(rtree->socket_num));
    // if(inet_pton(AF_INET, &rtree->server_socket, &rtree->server_socket.sin_addr) < 1) {
    //     printf("Erro ao converter IP\n");
    //     close(rtree->socket_num);
    //     return -1;
    // }
    //
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
    char *buffer = malloc(msglen); 
    
    //send
    message_t__pack(&msg->message, buffer);
    int netlong = htonl(msglen);
    write(rtree->socket_num, &netlong, sizeof(int));
    write_all(rtree->socket_num, buffer, msglen);

    free(buffer);

    //receive
    read(rtree->socket_num, &resposta_len, sizeof(int));
    resposta_len = ntohl(resposta_len);
    char *resp[resposta_len];
    read_all(rtree->socket_num, resp, resposta_len);
    
    //CONFUSAO AQUI 
    MessageT *temp = message_t__unpack(NULL, resposta_len, resp);
    msg->message = *temp;

    //NEEDS TESTING, CHANGE TO != in case this doesnt work
    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        return NULL;
    }
    
    return msg;
    
}

int network_close(struct rtree_t * rtree){
    close(rtree->socket_num);
    return 0;
}