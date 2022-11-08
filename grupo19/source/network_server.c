#include "network_server.h"
#include "inet.h"
#include <signal.h>
#include <poll.h>

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

void close_free(int sig){
    network_server_close();
    printf("Server fechado devido a Ctrl+C\n");
    exit(1);
}

struct sockaddr_in server, client;
int sockfd, connsockfd;
socklen_t size_client;

int nfdesc; //número de sockets para clientes + 1 para o Listen....possivelmente fazer const

int network_server_init(short port) {

    signal(SIGINT, close_free);

    nfdesc = 5; //maybe? possivelmente fazer uma constante
    
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
    signal(SIGINT, close_free);
    
    //vars for multiplexing
    struct pollfd conns[nfdesc];
    int nfds, kfds, i;

    //inicializar os FileDescriptors das conns a -1
    for(i = 0; i < nfdesc; i++)
        conns[i].fd = -1;// poll ignora estruturas com fd < 0
    
    //Listening socket será a conns[0]
    conns[0].fd = sockfd;
    conns[0].events = POLLIN;//esperar ligações

    nfds = 1; //número de FileDescriptors

    //While que fica a espera de eventos... Não há TIMEOUT?
    while((kfds = poll(conns, nfds, -1)) >= 0){ //NOTA poll devolve 0 se timeout ou >0(número de descritores com eventos) 
        if(kfds > 0){ //numero de descritores com evento ou erro
            
            if((conns[0].revents & POLLIN) && (nfds < nfdesc))//recebeu ligacao
                if((connsockfd = accept(listening_socket,(struct sockaddr *) &client, &size_client)) != -1){
                   conns[nfds].fd = connsockfd; //TALVEZ DAR MERGE COM A LINHA DE CIMA
                   conns[nfds].events = POLLIN; 
                   nfds++;
                }
            
            for(i = 1; i < nfdesc; i++){
                if(conns[i].revents & POLLIN){
                    MessageT *mss = network_receive(conns[i].fd);
                    //client dá quit
                    if(mss == NULL){
                        printf("Cliente Desconetou-se\n");
                        message_t__free_unpacked(mss, NULL);
                        close(conns[i].fd);
                        conns[i].fd = -1;//remove client from conns
                    }else{
                        if(invoke(mss) == -1)
                            printf("Ocorreu um erro ao executar o pedido\n");
                        
                        //envia a mensagem ao cliente
                        if(network_send(conns[i].fd, mss) == -1){
                            close(conns[i].fd);//POTENCIAL DE DAR ERRADO
                            conns[i].fd = -1;//remove client from conns
                            return -1;//PENSO NAO SER NECESSARIO(pq operacao continua sem esse client)
                        }
                    }
                }
                if((conns[i].revents & POLL_ERR) || (conns[i].revents & POLL_HUP)){
                    close(conns[i].fd);
                    conns[i].fd = -1; //remove client from conns
                }
            }
        } 
    }
    return 0; //INCERTO QUANTO A ONDE POR ESTA PARTE

    /*loop antigo(proj2)
    while((connsockfd = accept(listening_socket,(struct sockaddr *) &client, &size_client)) != -1){
        printf("Cliente Conetou-se\n");
        
        int client_running = 1;
        while (client_running == 1){
            MessageT *mss = network_receive(connsockfd);
            

            //client dá quit
            if(mss == NULL){
                printf("Cliente Desconetou-se\n");
                message_t__free_unpacked(mss, NULL);
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
    return 0;*/
}


MessageT *network_receive(int client_socket) {
    int msglen;
    int res;
    if ((res = read(client_socket, &msglen, sizeof(int))) == 0) {
        return NULL;
    }
    msglen = ntohl(msglen);
    uint8_t mensagem [msglen];

    read_all(client_socket, mensagem, msglen);

    return message_t__unpack(NULL, msglen, mensagem);
}

int network_send(int client_socket, MessageT *msg) {
    int msglen = message_t__get_packed_size(msg);
    int netlong = htonl(msglen);
    uint8_t *buffer = malloc(msglen);
    
    message_t__pack(msg, buffer);
    write(client_socket, &netlong, sizeof(int));
    write_all(client_socket, buffer, msglen);
    free(buffer);
    message_t__free_unpacked(msg, NULL);
    return 0;
}

int network_server_close(){
    tree_skel_destroy();
    close(sockfd);
    return 0;
}
