#include "network_server.h"
#include "inet.h"
#include <signal.h>

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

void close_free(int sig){
    network_server_close();
    printf("Server Closed due to Ctrl+C\n");
}

struct sockaddr_in server, client;
int sockfd, connsockfd;
socklen_t size_client;

int network_server_init(short port) {
    // struct sockaddr_in server;
    // int sockfd;

    signal(SIGINT, close_free);
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket\n");
        return -1;
    }
    int um = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &um, sizeof(um)) < 0) {
        perror("Erro em setsockopt, network_server_init\n");
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
        perror("Erro ao fazer bind\n");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 0) < 0){
        perror("Erro ao executar listen\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int network_main_loop(int listening_socket){
    // struct sockaddr_in client;
    // int connsockfd;
    // socklen_t size_client;
    signal(SIGINT, close_free);
    //aceita a conexão do client
    while((connsockfd = accept(listening_socket,(struct sockaddr *) &client, &size_client)) != -1){
        printf("Cliente Conetou-se\n");
        
        int client_running = 1;
        while (client_running == 1){
            struct message_t *mss = network_receive(connsockfd);
            

            //client da quit
            if(mss == NULL){
                printf("Cliente Desconetou-se\n");
                message_t__free_unpacked(&mss->message, NULL);
                client_running = 0;
                close(connsockfd);
                continue;
            }
            
            if(invoke(mss) == -1){
                printf("Ocorreu um erro ao executar o pedido\n");
                continue;
            }
            
            //envia a mensagem ao cliente
            if(network_send(connsockfd, mss) == -1){
                close(connsockfd);
                return -1;
            }
        }
    }
    return 0;
}


struct message_t *network_receive(int client_socket) {
    struct message_t *ret = message_create();
    int msglen;
    int res;
    if ((res = read(client_socket, &msglen, sizeof(int))) == 0) {
        return NULL;
    }
    msglen = ntohl(msglen);
    uint8_t mensagem [msglen];

    read_all(client_socket, mensagem, msglen);

    ret->message = *message_t__unpack(NULL, msglen, mensagem);

    return ret;
}

int network_send(int client_socket, struct message_t *msg) {
    int msglen = message_t__get_packed_size(&msg->message);
    int netlong = htonl(msglen);
    uint8_t *buffer = malloc(msglen);
    
    message_t__pack(&msg->message, buffer);
    write(client_socket, &netlong, sizeof(int));
    write_all(client_socket, buffer, msglen);
    free(buffer);
    message_t__free_unpacked(&msg->message, NULL);

    return 0;
}

int network_server_close(){
    tree_skel_destroy();
    close(sockfd);
    return 0;
}
