#include "message_private.h"
#include <signal.h>

/*Trabalho realizado por 
    Cosmin Trandafir fc57101
    Beatriz Silva fc52911
    João Serafim fc56376
*/

MessageT *message_create(){
    MessageT *msg = (MessageT *) malloc(sizeof(MessageT));
    message_t__init(msg);
    if(msg == NULL){
        return NULL;
    }

    return msg;
}

int write_all(int socket_num, uint8_t *buffer, int len) {
    signal(SIGPIPE, SIG_IGN);
    int ret = len;
    while (len > 0) {
        int resultado = write(socket_num, buffer, len);
        if (resultado < 0) {
            perror("Erro na escrita, write_all \n");
            return resultado;
        }
        len -= resultado;
        buffer += resultado;
    }
    return ret;
}

int read_all(int socket_num, uint8_t *buffer, int len) {
    int index = 0;
    int resultado;
    while (index < len) {
        resultado = read(socket_num, buffer + index, len - index);
        if(resultado == 0) { //EOF
            return index;
        }
        if (resultado < 1) {
            perror("Erro na leitura, read_all \n");
            return resultado;
        }
        index += resultado;

    }
    buffer[len] = '\0';
    return index;
}