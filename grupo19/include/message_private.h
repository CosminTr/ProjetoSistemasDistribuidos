#include "inet.h"
#include "sdmessage.pb-c.h"

struct message_t{
    MessageT message;
};

struct message_t *message_create();

int write_all(int socket_num, char *buffer, int len);

int read_all(int socket_num, char *buffer, int len);