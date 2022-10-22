#include "message_private.h"

struct message_t *message_create(){
    return NULL;
}

int write_all(int socket_num, char *buffer, int len) {
    int ret = len;
    while (len > 0) {
        int resultado = write(socket_num, buffer, len);
        if (resultado < 0) {
            perror("Erro na escrita, write_all \n");
            return resultado;

        }
        len = len - resultado;
        buffer = buffer + resultado;
    }
    return ret;
}

int read_all(int socket_num, char *buffer, int len) {
    int index = 0;
    int resultado;
    while (index < len) {
        resultado = read(socket_num, buffer + index, len - index);
        if(resultado == 0) { //EOF
            return 0;
        }
        if (resultado < 1) {
            perror("Erro na leitura, read_all \n");
            return resultado;
        }
        index = index + resultado;

    }
    buffer[len] = '\0';
    return index;
}