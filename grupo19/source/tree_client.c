#include <stdio.h>
#include <string.h>
#include "client_stub.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

int main(int argc, char *argv[]) {
    struct rtree_t *tree;

    if (argc != 2){
        printf("Input errado!\n Deve introduzir: ./client <server>:<port>");
        return -1;
    }

    tree = rtree_connect((char *)argv[1]);

    char* pedido;
    int maxSize = 128; //ALTERAR caso haja standard ou nao ig?
    char* input[maxSize];
    if(tree != NULL) {
        int running = 1;
        while (running) {
            fgets(input, maxSize, stdin);
            pedido = strtok(input, " \n"); //White space e new line -----pq new line?

            if(strcmp(pedido, "put") == 0) {
                char* temp = strtok(NULL, " "); //key input
                char* key = malloc(sizeof(temp)+1);  //fazer malloc +1 para \n?
                strcpy(key, temp);
                
                temp = strtok(NULL, "\n");//data input
                void* data_temp = malloc(sizeof(temp)+1);
                memcpy(data_temp, temp, sizeof(temp));
                struct data_t *data = data_create2(sizeof(temp)+1, data_temp);
                struct entry_t *entry = entry_create(key, data);
                if(rtree_put(tree, entry) == -1)
                    printf("Ocorreu um erro!");
                else
                    printf("Dados inseridos com sucesso");
                
                data_destroy(data);
                entry_destroy(entry);
                free(key);
                free(data_temp);
            }
            else if (strcmp(pedido, "get") == 0) {
                char *temp = strtok(NULL, " ");
                char *key = malloc(strlen(temp)+1);
                strcpy(key, temp);

                struct data_t *data = rtree_get(tree, key);
                if (data == NULL)  //ERRO
                    printf("Não foi possivel get com a chave %s \n", key);
                else 
                    printf("Data com tamanho: %d, obtida \n", data->datasize);

                free(key);

            }
            else if (strcmp(pedido, "del") == 0){
                char *temp = strtok(NULL, " ");
                char *key = malloc(strlen(temp));
                strcpy(key, temp);
                if (rtree_del(tree, key) == -1) //deu erro
                    printf("Não foi possivel apagar a entrada com chave: %s \n", key);
                else 
                    printf("Apagado com sucesso a entrada com chave: %s \n", key);

                free(key);
            }
            else if (strcmp(pedido, "size") == 0){
                int tam = rtree_size(tree);
                if(tam == -1) {
                    printf("Erro na obtenção do tamanho da arvore \n");
                }
                else 
                    printf("O tamanho da arvore é: %d \n", tam);
            }
            else if (strcmp(pedido, "height") == 0){
                int height = rtree_height(tree);
                if(height == -1)
                    printf("Erro na obtenção da altura da arvore \n");
                else 
                    printf("O altura da arvore é: %d \n", height);          
            }
            else if (strcmp(pedido, "getkeys") == 0) {
                char **keys = rtree_get_keys(tree);
                if (keys == NULL) {
                    printf("Erro ao obter as chaves \n");
                }
                else {
                    for(int i = 0; keys[i] != NULL; i++) {
                       // if (keys[i] != NULL) {
                            printf("Chave: %s \n", keys[i]);
                       // }
                    }
                }
            }
            else if (strcmp(pedido, "getvalues") == 0) {
                char **values = rtree_get_values(tree);
                if (values == NULL) {
                    printf("Erro ao obter os valores \n");
                }
                else {
                    for(int i = 0; values[i] != NULL; i++) {
                       // if (keys[i] != NULL) {
                            printf("Valor: %s \n", values[i]);
                       // }
                    }
                }
            }
            else if (strcmp(pedido, "quit") == 0){
                rtree_disconnect(tree);
                running = 0;
                printf("Conexão terminada");         
            }
        }
    }
    return 0;
}
