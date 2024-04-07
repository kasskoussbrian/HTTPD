#include "http.h"

static struct argo *init_argo_struct(void)
{
    struct argo *binary_args = calloc(1, sizeof(struct argo));
    if (binary_args == NULL)
    {
        return NULL;
    }
    binary_args->dry = 0;
    binary_args->reload = 0;
    binary_args->start = 0;
    binary_args->stop = 0;
    binary_args->restart = 0;
    binary_args->target = NULL;
    return binary_args;
}
static void treat_request_from_args(struct argo *the_args, struct config *conf)
{
    if (the_args->start != 0)
    {
        if (start_do(conf) == 1)
        {
            fprintf(stderr,
                    "tout.c: treat_request_from_args: Problem in START\n");
            exit(1);
        }
    }
    else if (the_args->stop != 0)
    {
        if (quit_do(conf) == 1)
        {
            fprintf(stderr,
                    "tout.c: treat_request_from_args: Problem in STOP\n");
            exit(1);
        }
        exit(0);
    }
    else if (the_args->restart != 0)
    {
        if (restart_do(conf) == 1)
        {
            fprintf(stderr,
                    "tout.c: treat_request_from_args: Problem in RESTART\n");
            exit(1);
        }
    }
    else if (the_args->reload != 0)
    {
        fprintf(stderr,
                "tout.c: treat_request_from_args: Not implemented yet\n");
        exit(1);
    }
}

int main_function(int argc, char *argv[])
{
    struct argo *bin_args = init_argo_struct();
    if (parse_command(argc, argv, bin_args) == 1)
    {
        free(bin_args);
        return 1;
    }
    struct config *server_config = parse_configuration(bin_args->target);
    if (server_config == NULL)
    {
        fprintf(stderr, "tout.c: main_function: Wrong config file\n");
        free(bin_args);
        config_destroy(server_config);
        return 2;
    }
    if (bin_args->dry != 0)
    {
        free(bin_args);
        config_destroy(server_config);
        return 0;
    }
    if (bin_args->dry == 1)
    {
        free(bin_args);
        config_destroy(server_config);
        return 1;
    }
    // start - stop - ..  etc
    treat_request_from_args(bin_args, server_config);
    if (create_server(server_config->servers[0].ip,
                      server_config->servers[0].port, server_config)
        == 1)
    {
        fprintf(stderr, "deamon.c: main : error returning 1\n");
        free(bin_args);
        config_destroy(server_config);
        return 1;
    }

    // free things
    free(bin_args);
    config_destroy(server_config);
    return 0;
}
