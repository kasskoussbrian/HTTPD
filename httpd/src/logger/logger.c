#include "logger.h"

static char *give_time(void)
{
    time_t base;
    struct tm *transformed_base;
    time(&base);
    transformed_base = gmtime(&base);
    char *result = malloc(
        sizeof(char) * (strlen("Date: Sat, 01 Nov 2022 23:42:00 GMT") + 1));
    result[(strlen("Date: Sat, 01 Nov 2022 23:42:00 GMT"))] = '\0';
    strftime(result, 50, "Date: %a, %d %b %Y %H:%M:%S GMT", transformed_base);
    return result;
}

void write_request_logs(struct config *conf, struct request *req, int my_err)
{
    if (conf->log == false)
    {
        return;
    }
    char done[200];
    char *time = give_time();
    char *name = malloc(conf->servers[0].server_name->size + 1);
    memcpy(name, conf->servers[0].server_name->data,
           conf->servers[0].server_name->size);
    name[conf->servers[0].server_name->size] = '\0';
    if (my_err == 0)
    {
        char *method = malloc(req->method->size + 1);
        memcpy(method, req->method->data, req->method->size);
        method[req->method->size] = '\0';
        char *from = "218.20.4.2";
        sprintf(done, "%s [%s] recieved %s from %s\n", time, name, method,
                from);
    }
    else
    {
        char *from = "218.20.4.2";
        char *method = "Bad Request";
        sprintf(done, "%s [%s] recieved %s from %s\n", time, name, method,
                from);
    }
    if (conf->log_file != NULL || strlen(conf->log_file) == 0)
    {
        FILE *to_op = fopen(conf->log_file, "a+");
        if (to_op == NULL)
        {
            fprintf(stderr, "LOGGER: cant open file\n");
            exit(1);
        }
        if (fwrite(done, sizeof(char), strlen(done), to_op) == 0)
        {
            fprintf(stderr, "LOGGER: cant write\n");
            exit(1);
        }
        fclose(to_op);
    }
    else
    {
        printf("%s", done);
    }
    free(time);
    free(name);
}
