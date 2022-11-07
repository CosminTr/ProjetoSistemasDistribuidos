#include "inet.h"
#include "sdmessage.pb-c.h"

struct request_t {
int op_n; //o número da operação
int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
char* key; //a chave a remover ou adicionar
char* data; // os dados a adicionar em caso de put, ou NULL em caso de delete
//adicionar campo(s) necessário(s) para implementar fila do tipo produtor/consumidor
};

MessageT *message_create();

int write_all(int socket_num, uint8_t *buffer, int len);

int read_all(int socket_num, uint8_t *buffer, int len);
