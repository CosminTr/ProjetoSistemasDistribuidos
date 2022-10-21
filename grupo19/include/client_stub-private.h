#include<netinet/in.h>
#include "inet.h"
//#include "tree-rpivate.h"
struct rtree_t {
    struct sockaddr_in server_socket; //socket do server, right?
    int socket_num;
    //char* hostname;? usado no inet para fazer ligacao ao servidor
    //struct tree_t arvore
};