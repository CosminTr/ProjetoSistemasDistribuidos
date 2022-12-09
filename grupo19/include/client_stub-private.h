#include<netinet/in.h>
#include "inet.h"
#include "zookeeper/zookeeper.h"
struct rtree_t {
    struct sockaddr_in server_socket; 
    int socket_num;

    //CONFIRM
    char *zk_identifier; //nome do node

    //para a conecao ao Zookeeper
    zhandle_t *zh;
    int is_connected;

};