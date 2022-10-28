#include "network_server.h"
#include "inet.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

int main(int argc, char * argv[]){

    if (argc != 2){
        printf("Input errado!\n Deve introduzir: ./tree-server <server_port>\n");
        return -1;
    }
    printf("Server Iniciado\n");
    int server_socket = network_server_init(atoi(argv[1]));
    network_main_loop(server_socket);
    network_server_close();

}