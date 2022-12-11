#include <stdio.h>
#include <string.h>
#include "inet.h"
#include "client_stub.h"

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

void commands(){
    printf("\nTem os seguintes comandos disponiveis: \n");
    printf("put <key> <data> \n");
    printf("get <key> \n");
    printf("del <key> \n");
    printf("size \n");
    printf("height \n");
    printf("getkeys \n");
    printf("getvalues \n");
    printf("verify <op_n> \n");
    printf("quit \n\n");
}

int main(int argc, char *argv[]) {
    struct rtree_t *tree;

    if (argc != 2){
        printf("Input errado!\n Deve introduzir: ./client <server>:<port>\n");
        return -1;
    }
    //tree contem tail como representação da árvore
    //mas tree não tem peso nas funcoes de client_stub porque o client_stub encaminha sozinho conforme o pedido
    tree = rtree_connect(argv[1]);

    char* pedido;
    int maxSize = 128; 
    char input[maxSize];

    commands();

    int running = 1;
    while (running)
    {
        fgets(input, maxSize, stdin);
        pedido = strtok(input, " \n");
        if (strcmp(pedido, "put") == 0)
        {
            char *temp = strtok(NULL, " "); // key input
            if (temp == NULL)
            {
                printf("Input incorreto.\n");
                commands();
                continue;
            }
            char *key = malloc(strlen(temp) + 1);
            strcpy(key, temp);

            temp = strtok(NULL, "\n"); // data input
            if (temp == NULL)
            {
                printf("Input incorreto.\n");
                free(key);
                commands();
                continue;
            }
            struct data_t *data = data_create(strlen(temp) + 1);
            memcpy(data->data, temp, strlen(temp) + 1);
            struct entry_t *entry = entry_create(key, data);

            printf("Dados a inserir:\tChave:%s\tValor:%s\n", entry->key, (char *)entry->value->data);
            int result = rtree_put(tree, entry);
            if (result == -1)
                printf("Ocorreu um erro!\n");
            else
                printf("O put foi enviado e o número de processo é %d\n", result);

            free(entry->value);
            free(entry);
        }
        else if (strcmp(pedido, "get") == 0)
        {
            char *temp = strtok(NULL, " \n");
            if (temp == NULL)
            {
                printf("Input incorreto.\n");
                commands();
                continue;
            }
            char *key = malloc(strlen(temp) + 1);
            strcpy(key, temp);

            struct data_t *data = rtree_get(tree, key);
            if (data == NULL) // ERRO
                printf("Não foi possivel encontrar dados com a chave %s \n", temp);
            else
                printf("Obtida entrada com tamanho: %d e mensagem %s \n", data->datasize - 1, (char *)data->data);

            data_destroy(data);
        }
        else if (strcmp(pedido, "del") == 0)
        {
            char *temp = strtok(NULL, " \n");
            if (temp == NULL)
            {
                printf("Input incorreto.\n");
                commands();
                continue;
            }
            char *key = malloc(strlen(temp) + 1);
            strcpy(key, temp);

            int result = rtree_del(tree, key);
            if (result == -1)
                printf("Não foi possivel apagar a entrada com chave: %s \n", temp);
            else
                printf("A operação foi enviada com sucesso e o número do processo é %d\n", result);
        }
        else if (strcmp(pedido, "size") == 0)
        {
            int tam = rtree_size(tree);
            if (tam == -1)
            {
                printf("Erro na obtenção do tamanho da arvore \n");
            }
            else
                printf("O tamanho da arvore é: %d \n", tam);
        }
        else if (strcmp(pedido, "height") == 0)
        {
            int height = rtree_height(tree);
            if (height == -1)
                printf("Erro na obtenção da altura da arvore \n");
            else
                printf("O altura da arvore é: %d \n", height);
        }
        else if (strcmp(pedido, "getkeys") == 0)
        {
            char **keys = rtree_get_keys(tree);
            if (keys == NULL)
            {
                printf("Erro ao obter as chaves \n");
            }
            else
            {
                printf("Chaves:\n");
                for (int i = 0; keys[i] != NULL; i++)
                {
                    printf("<%s> \n", keys[i]);
                    free(keys[i]);
                }
            }
            free(keys);
        }
        else if (strcmp(pedido, "getvalues") == 0)
        {
            void **values = rtree_get_values(tree);
            if (values == NULL)
            {
                printf("Erro ao obter os valores \n");
            }
            else
            {
                printf("Valores:\n");
                for (int i = 0; values[i] != NULL; i++)
                {
                    printf("<%s> \n", (char *)values[i]);
                    free(values[i]);
                }
            }
            free(values);
        }
        else if (strcmp(pedido, "verify") == 0)
        {
            char *temp = strtok(NULL, " \n");
            if (temp == NULL)
            {
                printf("Input incorreto.\n");
                commands();
                continue;
            }
            int op_n = atoi(temp);
            if (op_n <= 0)
                printf("Input para verificação invalido, certifique-se que inseriu um valor superior ou igual a 1.\n");

            int result = rtree_verify(tree, op_n);
            if (result == -1)
                printf("Erro na verificação, t_c_verify\n");
            else
            {
                if (result == -2)
                    printf("A operacao identificada por %d não está na lista de espera nem foi executada.\n", op_n);
                if (result == 0)
                    printf("A operacao identificada por %d não está na lista de espera logo foi executada.\n", op_n);
                else if (result == 1)
                    printf("A operacao identificada por %d está na lista de espera logo ainda não foi executada.\n", op_n);
            }
        }
        else if (strcmp(pedido, "quit") == 0)
        {
            rtree_disconnect(tree);
            running = 0;
            printf("Conexão terminada\n");
        }
        else
        {
            printf("Pedido não existe\n");
            commands();
        }
    }
    return 0;
}
