#define _POSIX_C_SOURCE 200809L
#include "server.h"

#include <fcntl.h>
#include <sys/epoll.h>
#define MAX_EVENTS 501

static int host_socket;

int found_crlf(struct msg *msg)
{
    for (size_t i = 0; i < msg->req_str->size; i++)
    {
        if (msg->req_str->data[i] == '\r' && msg->req_str->data[i + 1] == '\n'
            && msg->req_str->data[i + 2] == '\r'
            && msg->req_str->data[i + 3] == '\n')
        {
            return 1;
        }
    }
    return 0;
}
/*
static struct msg *find_sock(struct msg_container *msg_cont,int sock)
{
    for (int i = 0; i < msg_cont->size; i++)
    {
        if (msg_cont->container[i] != NULL && msg_cont->container[i]->fd ==
sock)
        {
            return msg_cont->container[i];
        }
    }
    fprintf(stderr,"find_sock: Theres an error socket is not inside list\n");
    return NULL;
}
static void add_sock(int fd,char *buf,struct msg_container *msg_container)
{
    msg_container->size +=1;
    msg_container->container = realloc(msg_container->container,sizeof(struct
msg*)*msg_container->size); msg_container->container[msg_container->size -1] =
NULL;
}
*/
static int echo_part(int client_socket, struct config *conf)
{
    char buf[64000];
    int written = -3;
    written = recv(client_socket, &buf, 64000, 0);
    buf[written] = '\0';
    if (written == -1)
    {
        fprintf(stderr, "echo_part: %s\n", strerror(written));
        return -1;
    }
    read_send_response(client_socket, buf, conf);
    // written = send(client_socket,&buf,written,0);
    if (written == -1)
    {
        fprintf(stderr, "echo_part: %s\n", strerror(written));
        return -1;
    }
    return 0;
}
static int give_iterator(struct addrinfo **toret_inf, char *node, char *service)
{
    // init the hint
    struct addrinfo first_hint = { 0 };
    first_hint.ai_family = AF_INET;
    first_hint.ai_socktype = SOCK_STREAM;
    first_hint.ai_protocol = IPPROTO_TCP;
    // init the res
    struct addrinfo *it_inf = NULL;

    // addrinfo and check error
    int gai = getaddrinfo(node, service, &first_hint, &it_inf);
    if (gai != 0)
    {
        fprintf(stderr, "give_iterator: getaddrinfo: %s\n", gai_strerror(gai));
        return 1;
    }
    *toret_inf = it_inf;
    return 0;
}

static int start_server(struct addrinfo *it_inf, int *toret_host)
{
    // create a socket
    host_socket = -1;
    for (; it_inf; it_inf = it_inf->ai_next)
    {
        host_socket =
            socket(it_inf->ai_family, it_inf->ai_socktype, it_inf->ai_protocol);
        if (host_socket > 0)
        {
            // printf("SUCESS! CREATED A SOCKET\n");
        }
        else
        {
            fprintf(stderr, "start_server: Socket not working yet\n");
            continue;
        }
        // check if we created socket

        // bind the socket to an adress
        // check for bind errorvoid string_concat_str(struct string *str, const
        // char *to_concat, size_t size);

        int oxx = 1;
        setsockopt(host_socket, SOL_SOCKET, SO_REUSEADDR, &oxx, sizeof(int));
        if (bind(host_socket, it_inf->ai_addr, it_inf->ai_addrlen) == -1)
        {
            fprintf(stderr, "start_server: bind_check: %s\n", strerror(errno));
            freeaddrinfo(it_inf);
            close(host_socket);
            return 1;
        }
        else
        {
            break;
        }
    }
    freeaddrinfo(it_inf);
    if (host_socket == -1)
    {
        fprintf(stderr, "start_server: could not bind\n");
        return 1;
    }
    *toret_host = host_socket;
    return 0;
}

static int while_server(int epoll_fd, struct epoll_event *events,
                        struct config *conf, struct epoll_event event)
{
    while (1)
    {
        int nb_ev = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nb_ev == -1)
        {
            fprintf(stderr, "Can't get fd\n");
            close(host_socket);
            close(epoll_fd);
            return 1;
        }
        for (int i = 0; i < nb_ev; i++)
        {
            if (events[i].data.fd == host_socket)
            {
                int tocom = accept(host_socket, NULL, NULL);
                if (tocom == -1)
                {
                    close(host_socket);
                    close(epoll_fd);
                    fprintf(stderr, "listen_communicate:  tocom accept\n");
                    return 1;
                }
                fcntl(tocom, O_NONBLOCK);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = tocom;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tocom, &event) == -1)
                {
                    close(host_socket);
                    close(epoll_fd);
                    close(tocom);
                    fprintf(stderr, "listen_communicate: error_adding event\n");
                    return 1;
                }
            }
            else
            {
                if (echo_part(events[i].data.fd, conf) == -1)
                {
                    close(events[i].data.fd);
                    close(host_socket);
                    return 1;
                }
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd,
                              &event)
                    == -1)
                {
                    fprintf(stderr, "epoll_ctl: listen_sock");
                    return 1;
                }
                close(events[i].data.fd);
            }
        }
    }
}
static int listen_communicate(int host_socket, struct config *conf)
{
    // start listenning
    if (listen(host_socket, SOMAXCONN) == -1)
    {
        fprintf(stderr, "listen_communicate: %s\n", strerror(errno));
        return 1;
    }
    // waiting for clients
    fcntl(host_socket, O_NONBLOCK);
    struct epoll_event events[MAX_EVENTS];
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        fprintf(stderr, "listen_communicate: epoll create  _ not working\n");
        return 1;
    }
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = host_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, host_socket, &event))
    {
        fprintf(stderr, "Can't add fd to interest\n");
        close(host_socket);
        close(epoll_fd);
        return 1;
    }
    return while_server(epoll_fd, events, conf, event);
}

int create_server(char *node, char *service, struct config *conf)
{
    struct addrinfo *it_inf;
    if (give_iterator(&it_inf, node, service) == 1)
    {
        return 1;
    }
    int host_socket;
    if (start_server(it_inf, &host_socket) == 1)
    {
        fprintf(stderr, "testnet.c: create_server: error returning 1\n");
        return 1;
    }
    if (listen_communicate(host_socket, conf) == 1)
    {
        return 1;
    }
    return 0;
}
