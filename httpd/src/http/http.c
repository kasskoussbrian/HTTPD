#include "http.h"

#include "../logger/logger.h"
static int my_err = 0;
// my_err 0 -> NO_ERROR
// my_err 1 -> 403 - NO_PERMISSION_RESSOURCE
// my_err 2 -> 404 - NO_RESSOURCE
// my_err 3 -> 405 - METHOD_NOT_ALLOWED
// my_err 5 -> 505 - VERSION NOT SUPPORTED
// my_err 6 -> 400 - BAD REQUEST
static void send_start_line(int client_socket)
{
    char *res;
    if (my_err == 0)
    {
        res = "HTTP/1.1 200 OK\r\n";
    }
    else if (my_err == 1)
    {
        res = "HTTP/1.1 403 Forbidden\r\n";
    }
    else if (my_err == 2)
    {
        res = "HTTP/1.1 404 Not Found\r\n";
    }
    else if (my_err == 3)
    {
        res = "HTTP/1.1 405 Method Not Allowed\r\n";
    }
    else if (my_err == 5)
    {
        res = "HTTP/1.1 505 HTTP Version Not Supported\r\n";
    }
    else if (my_err == 6)
    {
        res = "HTTP/1.1 400 Bad Request\r\n";
    }
    if (send(client_socket, res, strlen(res), 0) == -1)
    {
        fprintf(stderr, "http.c: send_start_line: error in send\n");
        exit(1);
    }
}

static int check_starting_args(struct request *req)
{
    if (req->method == NULL || req->target == NULL || req->version == NULL)
    {
        my_err = 6;
        return 1;
    }
    if (req->target->data[0] != '/')
    {
        my_err = 6;
        return 1;
    }
    if (!(string_compare_n_str(req->method, "GET", 3) == 0
          || string_compare_n_str(req->method, "HEAD", 4) == 0))
    {
        my_err = 3;
        return 1;
    }
    if ((string_compare_n_str(req->version, "HTTP/1.1", 8) != 0))
    {
        my_err = 5;
        return 1;
    }
    return 0;
}
static int check_host(struct request *req, struct config *conf)
{
    if (req == NULL)
    {
        if (my_err == 0)
        {
            my_err = 6;
        }
        return 1;
    }
    if (req->host == NULL)
    {
        if (my_err == 0)
        {
            my_err = 6;
        }
        return 1;
    }
    for (size_t i = 0; i < conf->nb_servers; i++)
    {
        if (req->host->size == conf->servers[i].server_name->size)
        {
            if (!string_compare_n_str(req->host,
                                      conf->servers[i].server_name->data,
                                      req->host->size))
            {
                return 0;
            }
        }
        if (req->host->size == strlen(conf->servers[i].ip))
        {
            if (!string_compare_n_str(req->host, conf->servers[i].ip,
                                      req->host->size))
            {
                return 0;
            }
        }
        struct string *tmp =
            string_create(conf->servers[i].ip, strlen(conf->servers[i].ip));
        string_concat_str(tmp, ":", 1);
        string_concat_str(tmp, conf->servers[i].port,
                          strlen(conf->servers[i].port));
        if (tmp->size == req->host->size)
        {
            if (!string_compare_n_str(tmp, req->host->data, req->host->size))
            {
                string_destroy(tmp);
                return 0;
            }
        }
        string_destroy(tmp);
    }
    if (my_err == 0)
    {
        my_err = 6;
    }
    return 1;
}

static int parse_start_line(struct request *req, struct parser *pars)
{
    int cpi = 0;
    int position = 0;
    while (1)
    {
        if (pars->i == pars->len - 1 || pars->len == 0)
        {
            // we didnt find CLRF
            my_err = 6; // BAD REQUEST
            return 1;
        }
        if (pars->arr[pars->i] == '\r' && pars->arr[pars->i + 1] == '\n')
        {
            pars->i = pars->i + 2;
            // everything seems allright till now
            return check_starting_args(req);
        }
        if (pars->arr[pars->i] == ' ')
        {
            if (position >= 2)
            {
                my_err = 6;
                return 1;
            }
            pars->i = pars->i + 1;
            position = position + 1;
            cpi = 0;
            continue;
        }
        if (position == 0)
        {
            add_element(&req->method, &cpi, pars->arr + pars->i);
            pars->i = pars->i + 1;
        }
        else if (position == 1)
        {
            add_element(&req->target, &cpi, pars->arr + pars->i);
            pars->i = pars->i + 1;
        }
        else if (position == 2)
        {
            add_element(&req->version, &cpi, pars->arr + pars->i);
            pars->i = pars->i + 1;
        }
        else
        {
            fprintf(stderr, "I have an error\n");
        }
    }
}
static int do_the_rest(struct request *req, struct parser *pars, int size)
{
    int new = 0;
    pars->i += size;
    while (pars->i < pars->len - 3
           && (pars->arr[pars->i] == '\t' || pars->arr[pars->i] == ' '))
    {
        pars->i = pars->i + 1;
    }
    if (pars->i == pars->len - 3)
    {
        // bad request
        my_err = 6;
        return 1;
    }
    while (pars->i < pars->len - 3 && pars->arr[pars->i] != '\t'
           && pars->arr[pars->i] != ' ' && pars->arr[pars->i + 1] != '\n'
           && pars->arr[pars->i] != '\r')
    {
        if (check_crlf(pars))
        {
            if (size == 5 && req->host == NULL)
            {
                // Bad request
                my_err = 6;
                return 1;
            }
            if (size == 15 && req->content_lenght == NULL)
            {
                // Bad request
                my_err = 6;
                return 1;
            }
            pars->i += 4;
            return 0;
        }
        if (size == 5)
        {
            add_element(&req->host, &new, pars->arr + pars->i);
        }
        else
        {
            add_element(&req->content_lenght, &new, pars->arr + pars->i);
        }
        pars->i = pars->i + 1;
    }
    if (pars->i >= pars->len - 3)
    {
        // bad request
        my_err = 6;
        return 1;
    }
    while (pars->i < pars->len - 3
           && (pars->arr[pars->i] == '\t' || pars->arr[pars->i] == ' '))
    {
        pars->i = pars->i + 1;
    }
    if (check_crlf(pars))
    {
        pars->i += 4;
        return 0;
    }
    else if (pars->arr[pars->i] == '\r' && pars->arr[pars->i + 1] == '\n')
    {
        return 5;
    }
    else
    {
        // bad request
        my_err = 6;
        return 1;
    }
}
static int parse_field(struct request *req, struct parser *pars)
{
    if (pars->arr[pars->i] == '\r' && pars->arr[pars->i + 1] == '\n')
    {
        pars->i = pars->i + 2;
        return 0;
    }
    int new = 1;
    while (1)
    {
        if (pars->i >= pars->len - 3)
        {
            // NO CRLF
            my_err = 6;
            return 1;
        }
        if (check_crlf(pars))
        {
            pars->i = pars->i + 4;
            return 0;
        }
        if ((pars->arr[pars->i] == '\r' && pars->arr[pars->i + 1] == '\n')
            || new)
        {
            if (new == 0)
            {
                pars->i = pars->i + 2;
            }
            new = 0;
            if (strncmp("Host:", pars->arr + pars->i, 5) == 0)
            {
                int res = do_the_rest(req, pars, 5);
                if (res == 1)
                {
                    return 1;
                }
                else if (res == 5)
                {
                    continue;
                }
                else
                {
                    return 0;
                }
            }
            else if (strncmp("Content_Length:", pars->arr + pars->i, 15) == 0)
            {
                int res = do_the_rest(req, pars, 15);
                if (res == 1)
                {
                    return 1;
                }
                else if (res == 5)
                {
                    continue;
                }
                else
                {
                    return 0;
                }
            }
        }
        pars->i = pars->i + 1;
    }
}

static int read_body(struct request *req, struct parser *pars)
{
    if (req->content_lenght == NULL)
    {
        return 0;
    }
    int new = 0;
    char *len = malloc(req->content_lenght->size + 1);
    len[req->content_lenght->size] = '\0';
    memcpy(len, req->content_lenght->data, req->content_lenght->size);
    int size = atoi(len);
    free(len);
    int real_len = 0;
    while (pars->i < pars->len && real_len < size)
    {
        add_element(&req->body, &new, pars->arr + pars->i);
        pars->i = pars->i + 1;
        real_len += 1;
    }
    if (real_len < size)
    {
        /*
        char buf[32];
        while (real_len < size)
        {
            int res = recv(client_socket, &buf, 32, 0);
            if (res == -1)
            {
                fprintf(stderr, "http.c: read_body: error reading\n");
                return 1;
            }
            real_len = real_len + res;
        }
        string_concat_str(req->body, buf, size - real_len);
    */
        my_err = 6;
        return 1;
    }
    return 0;
}

static void send_time(int client_socket)
{
    time_t base;
    struct tm *transformed_base;
    time(&base);
    transformed_base = gmtime(&base);
    char *result = malloc(
        sizeof(char) * (strlen("Date: Sat, 01 Nov 2022 23:42:00 GMT\r\n") + 1));
    result[(strlen("Date: Sat, 01 Nov 2022 23:42:00 GMT\r\n"))] = '\0';
    strftime(result, 50, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n",
             transformed_base);
    if (send(client_socket, result, strlen(result), 0) == -1)
    {
        fprintf(stderr, "http.c: get_time: error in send\n");
        exit(1);
    }
    free(result);
}

struct request *read_request(char *request)
{
    struct parser *pars = malloc(sizeof(struct parser));
    pars->i = 0;
    pars->arr = request;
    pars->len = strlen(pars->arr);
    struct request *req = init_starting();
    // parse_start_line
    if (parse_start_line(req, pars) == 1)
    {
        free(pars);
        free_request(req);
        return NULL;
    }
    // parse_field
    if (parse_field(req, pars) == 1)
    {
        free(pars);
        free_request(req);
        return NULL;
    }
    // check_host here
    // end check_host
    // parse_body
    if (read_body(req, pars) == 1)
    {
        free(pars);
        free_request(req);
        return NULL;
    }
    free(pars);
    return req;
}

char *r_s_r_prep(struct config *conf, struct request *req)
{
    char *to_open;
    if (req == NULL
        || ((req->target->size == strlen("/")
             && !string_compare_n_str(req->target, "/", req->target->size))))
    {
        if (conf->servers[0].default_file == NULL)
        {
            conf->servers[0].default_file = malloc(strlen("/index.html") + 1);
            conf->servers[0].default_file =
                memcpy(conf->servers[0].default_file, "/index.html",
                       strlen("/index.html"));
            conf->servers[0].default_file[strlen("/index.html")] = '\0';
        }
        asprint(&to_open, conf->servers[0].root_dir,
                conf->servers[0].default_file, "");
    }
    else
    {
        char *tmp = malloc(req->target->size + 1);
        memcpy(tmp, req->target->data, req->target->size);
        tmp[req->target->size] = '\0';
        asprint(&to_open, conf->servers[0].root_dir, tmp, "");
        free(tmp);
    }
    return to_open;
}

void read_send_response(int client_socket, char *request, struct config *conf)
{
    struct request *req = read_request(request);
    check_host(req, conf);
    write_request_logs(conf, req, my_err);
    char *to_open = r_s_r_prep(conf, req);
    /*char *to_open;
    if (req == NULL
        || ((req->target->size == strlen("/")
             && !string_compare_n_str(req->target, "/", req->target->size))))
    {
        if (conf->servers[0].default_file == NULL)
        {
            conf->servers[0].default_file = malloc(strlen("/index.html") + 1);
            conf->servers[0].default_file =
                memcpy(conf->servers[0].default_file, "/index.html",
                       strlen("/index.html"));
            conf->servers[0].default_file[strlen("/index.html")] = '\0';
        }
        asprint(&to_open, conf->servers[0].root_dir,
                conf->servers[0].default_file, "");
    }
    else
    {
        char *tmp = malloc(req->target->size + 1);
        memcpy(tmp, req->target->data, req->target->size);
        tmp[req->target->size] = '\0';
        asprint(&to_open, conf->servers[0].root_dir, tmp, "");
        free(tmp);
    }*/
    FILE *check = fopen(to_open, "r");
    if (check == NULL)
    {
        if (my_err == 0)
        {
            my_err = 2;
        }
    }
    else
    {
        fclose(check);
    }
    int get = 0;
    if (my_err == 0
        && (strlen("GET") == req->method->size
            && !string_compare_n_str(req->method, "GET", 3)))
    {
        get = 1;
    }
    if (req)
    {
        free_request(req);
    }
    send_start_line(client_socket);
    send_time(client_socket);
    if (my_err != 0)
    {
        char *tosend = "Content-Length: 0\r\n";
        send(client_socket, tosend, strlen(tosend), 0);
    }
    else
    {
        char len[50];
        int file_size = get_file_size(to_open);
        // doing this
        if (snprintf(len, 50, "Content-Length: %d\r\n", file_size) < 0)
        {
            fprintf(
                stderr,
                "theres a mistake snprint probably need more space to store\n");
        }
        send(client_socket, len, strlen(len), 0);
    }
    char *final = "Connection: close\r\n\r\n";
    send(client_socket, final, strlen(final), 0);
    if (my_err == 0 && get == 1)
    {
        int cl_fd = open(to_open, O_RDONLY);
        int file_size = get_file_size(to_open);
        if (sendfile(client_socket, cl_fd, NULL, file_size) == -1)
        {
            fprintf(stderr, "Mistkake sendfile probably\n");
        }
    }
    my_err = 0;
    free(to_open);
}
