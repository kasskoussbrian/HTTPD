#define _POSIX_C_SOURCE 200809L
#include "daemon.h"

static void sig(int sig)
{
    if (sig == SIGINT)
    {
        exit(0);
    }
    return;
}

int start_do(struct config *conf)
{
    FILE *file;
    if ((file = fopen(conf->pid_file, "r")))
    {
        char *pid_stored = malloc(100 * sizeof(char));
        if (fgets(pid_stored, 100, file) != NULL)
        {
            if (kill(atoi(pid_stored), 0) == 0 && strlen(pid_stored) != 0)
            {
                free(pid_stored);
                fclose(file);
                return 1;
            }
            fclose(file);
        }
        free(pid_stored);
    }
    file = fopen(conf->pid_file, "w+");
    if (file == NULL)
    {
        fprintf(stderr,
                "create_daemon: start_do: can not create or open file\n");
        return 1;
    }
    pid_t val = fork();
    struct sigaction sigs;
    sigs.sa_flags = 0;

    if (val == -1)
    {
        fprintf(stderr, "create_deamon: fork error\n");
        return 1;
    }
    if (val != 0)
    {
        exit(0);
    }
    sigs.sa_handler = sig;
    sigemptyset(&sigs.sa_mask);

    sigaction(SIGINT, &sigs, NULL);

    fprintf(file, "%d", getpid());
    fclose(file);
    return 0;
}

int quit_do(struct config *conf)
{
    FILE *file;
    if (!(file = fopen(conf->pid_file, "r")))
    {
        return 0;
    }
    char *pid_stored = malloc(100 * sizeof(char));
    if (fgets(pid_stored, 100, file) != NULL)
    {
        if (kill(atoi(pid_stored), 0) == 0 && strlen(pid_stored) != 0)
        {
            if (kill(atoi(pid_stored), SIGINT) == -1)
            {
                fprintf(stderr, "daemon.c: quit_do: %s\n", strerror(errno));
                return 1;
            }
        }
        fclose(file);
    }
    free(pid_stored);
    if (remove(conf->pid_file) == -1)
    {
        fprintf(stderr, "daemon.c: quit_do: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}

int restart_do(struct config *conf)
{
    int q = quit_do(conf);
    int s = start_do(conf);
    return q | s;
}

/*
int create_daemon(char *a_arg,struct config *conf)
{
    if (!strcmp("start",a_arg))
    {
        if(start_do(conf))
        {
            return 1;
        }
    }
    else if (!strcmp("stop",a_arg))
    {
        if (quit_do(conf))
        {
            exit(1);
        }
        exit(0);
    }
    else
    {
        fprintf(stderr,"daemon.c: create_daemon: Not an implemented command\n");
        exit(1);
    }
    return 0;
}
*/
/*
int main(int argc, char *argv[])
{
    struct config *conf = malloc(sizeof(struct config));
    argc = argc;
    conf->pid_file = "hey";
    if (create_daemon(argv[1],conf))
    {
        return 1;
    }
    if (create_server("localhost","8000") == 1)
    {
        fprintf(stderr,"deamon.c: main : error returning 1\n");
        return 1;
    }

    return 0;
}
*/
