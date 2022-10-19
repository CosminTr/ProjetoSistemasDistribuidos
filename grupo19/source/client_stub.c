#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"

struct rtree_t *tree_remota;

struct rtree_t *rtree_connect(const char *address_port) {
    tree_remota = malloc (sizeof(struct rtree_t));

    char del[] = "<:>";
    char *host = strtok(address_port, del); // hostname    removed:
    int port = atoi(strtok(NULL, del));     // port      '<' ':' '>'
    
    tree_remota->server_socket.sin_family = AF_INET;
    tree_remota->server_socket.sin_port = htons(port);

    //Nota: talvez fazer no network_client para dar close socket em caso de erro?
    if (inet_pton(AF_INET, host, &tree_remota->server_socket.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        return NULL;
    }
    if (network_connect(tree_remota) == -1) { //ERRO
        return NULL;
    }

    return(tree_remota);
}
int rtree_disconnect(struct rtree_t *rtree) {
    int retorna = 0;
    if (network_close(rtree) != 0)
        retorna = -1;
    return retorna;
}

int rtree_put(struct rtree_t *rtree, struct entry_t *entry);

struct data_t *rtree_get(struct rtree_t *rtree, char *key);

int rtree_del(struct rtree_t *rtree, char *key);

int rtree_size(struct rtree_t *rtree);

int rtree_height(struct rtree_t *rtree);

char **rtree_get_keys(struct rtree_t *rtree);

void **rtree_get_values(struct rtree_t *rtree);