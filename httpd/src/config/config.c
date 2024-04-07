#include "config.h"
/*
static void print_conf(struct config *conf)
{
    printf("Display of Config : \n");
    printf("pid_file = %s\n",conf->pid_file);
    printf("log_file = %s\n",conf->log_file);
    if (conf->log == true)
    {
        printf("log = true");
    }
    else
    {
        printf("log = false");

    }
}
static void print_conf_serv(struct config *conf)
{
    for (size_t i = 0; i < conf->nb_servers;i++)
    {
        struct server_config t = conf->servers[i];
        printf("Displau of Config : \n");
        printf("sever_name = xxxxxx\n");
        printf("port = %s\n",t.port);
        printf("ip = %s\n",t.ip);
        printf("root_dir = %s\n",t.root_dir);
        if (t.default_file != NULL)
        {
        printf("defautl_file = %s\n",t.default_file);
        }
        else
        {
        printf("defautl_file = (nill)\n");
        }
    }
}
*/
static void my_cpy(char **dst, char *src)
{
    *dst = malloc(300 * sizeof(char));
    size_t i = 0;
    while (src[i] != '\n')
    {
        (*dst)[i] = src[i];
        i = i + 1;
    }
    (*dst)[i] = '\0';
}

static void add_serv_array(struct config *conf)
{
    if (conf->nb_servers == 0)
    {
        conf->servers = malloc(sizeof(struct server_config));
    }
    else
    {
        conf->servers =
            realloc(conf->servers,
                    (conf->nb_servers + 1) * sizeof(struct server_config));
    }
    struct server_config *to = conf->servers;
    int i = conf->nb_servers;
    to[i].server_name = NULL;
    to[i].port = NULL;
    to[i].ip = NULL;
    to[i].root_dir = NULL;
    to[i].default_file = NULL;
    conf->nb_servers++;
}

int cpy_store(char *store, struct config *conf)
{
    int tr = 1;
    if (!fnmatch("server_name =*", store, 0))
    {
        char *f = strchr(store, '=');
        f = f + 2;
        f[strlen(f) - 1] = '\0';
        conf->servers[conf->nb_servers - 1].server_name =
            string_create(f, strlen(f));
    }
    else if (!fnmatch("port =*", store, 0))
    {
        char *f = strchr(store, '=');
        f = f + 2;
        my_cpy(&conf->servers[conf->nb_servers - 1].port, f);
    }
    else if (!fnmatch("ip =*", store, 0))
    {
        char *f = strchr(store, '=');
        f = f + 2;
        my_cpy(&conf->servers[conf->nb_servers - 1].ip, f);
    }
    else if (!fnmatch("root_dir =*", store, 0))
    {
        char *f = strchr(store, '=');
        f = f + 2;
        my_cpy(&conf->servers[conf->nb_servers - 1].root_dir, f);
    }
    else if (!fnmatch("default_file =*", store, 0))
    {
        char *f = strchr(store, '=');
        f = f + 2;
        my_cpy(&conf->servers[conf->nb_servers - 1].default_file, f);
    }
    else
    {
        tr = 0;
    }
    return tr;
}

static void init_serv_conf(struct config *conf, FILE *fd, int *err)
{
    char *store = calloc(300, sizeof(char));
    if (store == NULL)
    {
        *err = 1;
        free(store);
        return;
    }
    while (store != NULL)
    {
        if (fgets(store, 300, fd) == NULL)
        {
            if (conf->nb_servers == 0
                || conf->servers[conf->nb_servers - 1].server_name == NULL
                || conf->servers[conf->nb_servers - 1].server_name->data == NULL
                || conf->servers[conf->nb_servers - 1].port == NULL
                || conf->servers[conf->nb_servers - 1].ip == NULL
                || conf->servers[conf->nb_servers - 1].root_dir == NULL)
            {
                *err = 1;
            }
            free(store);
            return;
        }
        if (!fnmatch("\n", store, 0))
        {
            continue;
        }
        else if (strstr("[[vhosts]]\n", store)
                 && strcmp(strstr("[[vhosts]]\n", store), "[[vhosts]]\n") == 0)
        {
            add_serv_array(conf);
            continue;
        }
        else if (cpy_store(store, conf))
        {
            continue;
        }
        else
        {
            *err = 1;
            free(store);
            return;
        }
    }
    free(store);
}
static void err_free(int *err, char *store)
{
    *err = 1;
    free(store);
}
static int do_log(char *store, struct config *conf)
{
    int c = 1;
    char *f = strchr(store, '=');
    f = f + 2;
    if (!fnmatch("true*", f, 0))
    {
        conf->log = true;
    }
    else if (!fnmatch("false*", f, 0))
    {
        conf->log = false;
    }
    else
    {
        c = 0;
    }
    return c;
}
static void init_config(struct config *conf, FILE *fd, int *err)
{
    char *store = malloc(100 * sizeof(char));
    if (store == NULL)
    {
        *err = 1;
        return;
    }
    while (store != NULL && *store != EOF)
    {
        if (fgets(store, 100, fd) == NULL)
        {
            err_free(err, store);
            return;
        }
        if (!fnmatch("\n", store, 0))
        {
            break;
        }
        else if (strstr("[global]\n", store)
                 && strcmp(strstr("[global]\n", store), "[global]\n") == 0)
        {
            continue;
        }
        else if (!fnmatch("log_file =*", store, 0))
        {
            char *f = strchr(store, '=');
            f = f + 2;
            my_cpy(&conf->log_file, f);
            if (conf->log_file == NULL)
            {
                err_free(err, store);
                return;
            }
        }
        else if (!fnmatch("log =*", store, 0))
        {
            if (!do_log(store, conf))
            {
                err_free(err, store);
                return;
            }
        }
        else if (!fnmatch("pid_file =*", store, 0))
        {
            char *f = strchr(store, '=');
            f = f + 2;
            my_cpy(&conf->pid_file, f);
            if (conf->pid_file == NULL)
            {
                err_free(err, store);
                return;
            }
        }
        else
        {
            err_free(err, store);
            return;
        }
    }
    free(store);
}
static struct config *init_toret(void)
{
    struct config *toret = malloc(sizeof(struct config));
    toret->servers = NULL;
    toret->pid_file = NULL;
    toret->log_file = NULL;
    toret->log = true;
    return toret;
}

struct config *parse_configuration(const char *path)
{
    // Open file and checks if NULL
    FILE *fd_conf = fopen(path, "r");
    if (fd_conf == NULL)
    {
        return NULL;
    }
    // create config struct
    struct config *toret = init_toret();
    toret->nb_servers = 0;
    int err = 0;
    init_config(toret, fd_conf, &err);
    if (err == 1)
    {
        fclose(fd_conf);
        config_destroy(toret);
        return NULL;
    }
    if (toret->pid_file == NULL)
    {
        fclose(fd_conf);
        config_destroy(toret);
        return NULL;
    }
    init_serv_conf(toret, fd_conf, &err);
    if (err == 1)
    {
        fclose(fd_conf);
        config_destroy(toret);
        return NULL;
    }
    // print_conf(toret);
    // print_conf_serv(toret);
    fclose(fd_conf);
    return toret;
}
void config_destroy(struct config *config)
{
    if (config == NULL)
    {
        return;
    }
    free(config->pid_file);
    free(config->log_file);
    for (size_t i = 0; i < config->nb_servers; i++)
    {
        struct server_config t = config->servers[i];
        if (t.server_name != NULL)
        {
            string_destroy(t.server_name);
        }
        if (t.port != NULL)
        {
            free(t.port);
        }
        if (t.ip != NULL)
        {
            free(t.ip);
        }
        if (t.root_dir != NULL)
        {
            free(t.root_dir);
        }
        if (t.default_file != NULL)
        {
            free(t.default_file);
        }
    }
    free(config->servers);
    free(config);
}
