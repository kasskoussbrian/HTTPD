#include "http.h"
int check_null(struct argo *toret)
{
    if (toret->target == NULL)
    {
        fprintf(stderr, "parse_command: No config file\n");
        return 1;
    }
    return 0;
}

int parse_command(int argc, char *argv[], struct argo *toret)
{
    int a = 0;
    for (int i = 1; i < argc; i++)
    {
        if (!fnmatch("--dry-run", argv[i], 0))
        {
            if (toret->dry != 0)
            {
                fprintf(stderr, "parse_command: more than 1 --dry-run\n");
                return 1;
            }
            toret->dry = 1;
        }
        else if (!fnmatch("-a", argv[i], 0))
        {
            if (a == 1)
            {
                fprintf(stderr, "parse_command: more than 1 -a\n");
                return 1;
            }
            a = a + 1;
            if (i + 1 >= argc)
            {
                fprintf(stderr, "parse_command: -a require an argument\n");
                return 1;
            }
            i = i + 1;
            if (!fnmatch("start", argv[i], 0))
            {
                toret->start = 1;
            }
            else if (!fnmatch("stop", argv[i], 0))
            {
                toret->stop = 1;
            }
            else if (!fnmatch("reload", argv[i], 0))
            {
                toret->reload = 1;
            }
            else if (!fnmatch("restart", argv[i], 0))
            {
                toret->restart = 1;
            }
            else
            {
                fprintf(stderr,
                        "parse_command: arguments for -a are start stop reload "
                        "restart\n");
                return 1;
            }
        }
        else
        {
            FILE *file;
            if (!(file = fopen(argv[i], "r")))
            {
                fprintf(stderr,
                        "parse_command: wrong arguments or no valid file\n");
                return 1;
            }
            toret->target = argv[i];
            fclose(file);
        }
    }
    /*
    if (toret->target == NULL)
    {
        fprintf(stderr, "parse_command: No config file\n");
        return 1;
    }
    */
    return check_null(toret);
}

struct request *init_starting(void)
{
    struct request *req = malloc(sizeof(struct request));
    req->method = NULL;
    req->target = NULL;
    req->version = NULL;
    req->host = NULL;
    req->content_lenght = NULL;
    req->body = NULL;
    return req;
}
void free_request(struct request *req)
{
    if (req->method != NULL)
    {
        string_destroy(req->method);
    }
    if (req->target != NULL)
    {
        string_destroy(req->target);
    }
    if (req->version != NULL)
    {
        string_destroy(req->version);
    }
    if (req->host != NULL)
    {
        string_destroy(req->host);
    }
    if (req->content_lenght != NULL)
    {
        string_destroy(req->content_lenght);
    }
    if (req->body != NULL)
    {
        string_destroy(req->body);
    }
    free(req);
}
void add_element(struct string **str, int *cpi, char *tocop)
{
    if (*cpi == 0)
    {
        *cpi = 1;
        *str = string_create(tocop, 1);
    }
    else
    {
        string_concat_str(*str, tocop, 1);
    }
}

int asprint(char **res, const char *arg1, const char *arg2, const char *mid)
{
    *res = calloc(strlen(arg1) + strlen(arg2) + strlen(mid) + 2, sizeof(char));
    if (*res == NULL)
    {
        return -1;
    }
    if (sprintf(*res, "%s%s%s", arg1, mid, arg2) < -1)
    {
        fprintf(stderr, "myfind:asprintf : problem with sprintf\n");
    }
    return 0;
}

int check_crlf(struct parser *pars)
{
    if (pars->arr[pars->i] == '\r' && pars->arr[pars->i + 1] == '\n'
        && pars->arr[pars->i + 2] == '\r' && pars->arr[pars->i + 3] == '\n')
    {
        // is crlf
        return 1;
    }
    // not
    return 0;
}
int get_file_size(char *filename)
{
    struct stat file_status;
    if (stat(filename, &file_status) < 0)
    {
        return -1;
    }

    return file_status.st_size;
}
