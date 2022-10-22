#include "network_server.h"
#include "inet.h"
#include "message_private.h"

struct sockaddr_in server;
int sockfd;

int network_server_init(short port) {
//    int sockfd; ?
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }
    int um = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &um, sizeof(um)) < 0) {
        perror("Erro em setsockopt, network_server_init");
        close(sockfd);
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int tree = tree_skel_init();
    if (tree == -1) {
        perror("Erro ao inicializar tree: \n");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 0) < 0){
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    }
    return sockfd;
}
//FALTA ISTO
//
int network_main_loop(int listening_socket);
//

struct message_t *network_receive(int client_socket) {
    struct message_t *ret = create_message();
    int msglen;
    int res;
    if ((res = read(client_socket, &msglen, sizeof(int))) == 0) {
        return NULL;
    }
    msglen = ntohl(msglen);
    uint8_t mensagem [msglen];
    read_all(client_socket, mensagem, msglen);
    MessageT *temp = message_t__unpack(NULL, msglen, mensagem);
    ret->message = *temp;
    return ret;
}

int network_send(int client_socket, struct message_t *msg) {
    int msglen = message_t__get_packed_size(&msg->message);
    int netlong = htonl(msglen);
    uint8_t *buffer = malloc(msglen);
    
    message_t__pack(&msg->message, buffer);

    write(client_socket, &netlong, sizeof(int));
    write_all(client_socket, buffer, msglen);

    return 0;
}

int network_server_close(){
    tree_skel_destroy();
    close(sockfd);
    return 0;
}
int write_all(int socket_num, uint8_t *buffer, int len) {
    int ret = len;
    while (len > 0) {
        int resultado = write(socket_num, buffer, len);
        if (resultado < 0) {
            perror("Erro na escrita, write_all \n");
            return resultado;

        }
        len = len - resultado;
        buffer = buffer + resultado;
    }
    return ret;
}

int read_all(int socket_num, uint8_t *buffer, int len) {
    int index = 0;
    int resultado;
    while (index < len) {
        resultado = read(socket_num, buffer + index, len - index);
        //not sure about this
        if(resultado == 0) {
            return 0;
        }
        if (resultado < 1) {
            perror("Erro na leitura, read_all \n");
            return resultado;
        }
        index = index + resultado;

    }
    buffer[len] = '\0';
    return index;
}