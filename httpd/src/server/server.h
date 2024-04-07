#ifndef SERVER_H
#define SERVER_H
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../config/config.h"
#include "../http/http.h"
#include "../utils/string/string.h"

struct msg_container
{
    struct msg **container;
    int size;
};

struct msg
{
    int fd;
    struct string *req_str;
};

int create_server(char *node, char *service, struct config *conf);

#endif /* ! SERVER_H */
