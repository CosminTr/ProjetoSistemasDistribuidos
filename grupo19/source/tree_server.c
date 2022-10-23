#include "network_server.h"

int main(int argc, char * argv[]){

    if (argc != 2){
        printf("Input errado!\n Deverá introduzir ./server <server_port>");
        return -1;
    }

    int server_socket = network_server_init(atoi(argv[1]));
    network_main_loop(server_socket);
    network_server_close();

}