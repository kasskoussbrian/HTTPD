#ifndef HTTP_H
#define HTTP_H
#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

#include "../config/config.h"
#include "../daemon/daemon.h"
#include "../server/server.h"
#include "../utils/string/string.h"

struct argo
{
    int dry;
    int reload;
    int start;
    int stop;
    int restart;
    char *target;
};

struct parser
{
    int i;
    char *arr;
    int len;
};

struct request
{
    struct string *method;
    struct string *target;
    struct string *version;

    struct string *host;
    struct string *content_lenght;
    struct string *body;
};

int check_crlf(struct parser *pars);
void free_request(struct request *req);
int parse_command(int argc, char *argv[], struct argo *toret);
struct request *init_starting(void);
void add_element(struct string **str, int *cpi, char *tocop);
int asprint(char **res, const char *arg1, const char *arg2, const char *mid);
int get_file_size(char *filename);
int main_function(int argc, char *argv[]);
void read_send_response(int client_socket, char *request, struct config *conf);

#endif /* ! HTTP_H */
