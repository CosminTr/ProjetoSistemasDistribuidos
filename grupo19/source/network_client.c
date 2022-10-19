#include "network_client.h"
#include "client_stub.h"
#include "client_stub-private.h"

int network_connect(struct rtree_t *rtree) {
    //checks erro
    if (rtree->socket_num = socket(AF_INET, SOCK_STREAM, 0) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }
    if(connect(rtree->socket_num, &rtree->client_socket, sizeof(rtree->client_socket))) {
        perror("Erro ao conectar-se ao servidor");
        close(rtree->socket_num);
        return -1;
    }
    return 0;
}

struct message_t *network_send_receive(struct rtree_t * rtree,
                                       struct message_t *msg);

int network_close(struct rtree_t * rtree) {
    //
    return close(rtree->socket_num);
}