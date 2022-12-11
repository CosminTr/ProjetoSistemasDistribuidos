#include "network_server.h"
#include "inet.h"
#include "tree_skel.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

int main(int argc, char * argv[]){

    if (argc != 3){
        printf("Input errado!\n Deve introduzir: ./tree-server <server_port> <ZookeeperIP>:<ZookeeperPort>\n");
        return -1;
    }
    printf("Server Iniciado\n");
    int server_socket = network_server_init(atoi(argv[1]));
    int tree = tree_skel_init();
    if (tree == -1) {
        perror("Erro ao inicializar tree: \n");
        return -1;
    }
    if (start_ts_zk(argv[2], atoi(argv[1])) == -1) {
        perror("Erro ao inicializar tree: \n");
        return -1;
    }

    network_main_loop(server_socket);
    network_server_close();

}