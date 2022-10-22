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

int network_main_loop(int listening_socket){
    /aceita a conexão do client
    int connsockfd = accept(listening_socket,(struct sockaddr *) &client, &size_client);
    
    //entrega a mensagem que recebe do receive ao skel
    if (tree_skel_init() == -1){
        perror("Erro a iniciar a skeleton");
    }else if (tree_skel_init() == 0){
        struct message_t *mss = network_receive(connsockfd);
        invoke(mss);
        //envia a mensagem ao cliente
        network_send(connsockfd, mss);
    }
}
//

struct message_t *network_receive(int client_socket) {
    struct message_t *ret = create_message();
    int msglen;
    int res;
    if ((res = read(client_socket, &msglen, sizeof(int))) == 0) {
        return NULL;
    }
    msglen = ntohl(msglen);
    char *mensagem [msglen];
    read_all(client_socket, mensagem, msglen);
    MessageT *temp = message_t__unpack(NULL, msglen, mensagem);
    ret->message = *temp;
    return ret;
}

int network_send(int client_socket, struct message_t *msg) {
    int msglen = message_t__get_packed_size(&msg->message);
    int netlong = htonl(msglen);
    char *buffer = malloc(msglen);
    
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
