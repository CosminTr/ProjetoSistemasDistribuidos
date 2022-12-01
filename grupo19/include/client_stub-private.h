#include<netinet/in.h>
#include "inet.h"
struct rtree_t {
    struct sockaddr_in server_socket; 
    int socket_num;

    //CONFIRM
    char *zk_identifier;
    char *zk_next_id;

};