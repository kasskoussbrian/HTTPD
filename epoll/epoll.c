#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("./epoll: Bad usage ./epoll <pipe_name>\n");
        return 1;
    }
    struct epoll_event event;
    struct epoll_event events[501];

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        fprintf(stderr, "Failed to  create epoll fd\n");
    }

    event.events = EPOLLIN | EPOLLET;
    int cl_fd = open(argv[1], O_RDONLY);
    // je me passure tt est bon
    if (cl_fd == -1)
    {}
    event.data.fd = cl_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cl_fd, &event))
    {
        fprintf(stderr, "Can't add fd to interest\n");
        close(cl_fd);
        close(epoll_fd);
        return 1;
    }
    while (1)
    {
        int event_count = epoll_wait(epoll_fd, events, 501, -1);
        char r_buf[65];
        for (int i = 0; i < event_count; i++)
        {
            size_t bytes_read = read(events[i].data.fd, r_buf, 64);
            r_buf[bytes_read] = '\0';
            if (strncmp(r_buf, "ping", strlen(r_buf)) == 0)
            {
                printf("pong!\n");
            }
            else if (strncmp(r_buf, "pong", strlen(r_buf)) == 0)
            {
                printf("ping!\n");
            }
            else if (strncmp(r_buf, "quit", strlen(r_buf)) == 0)
            {
                printf("quit\n");
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cl_fd, &event))
                {
                    fprintf(stderr, "Can't add fd to interest\n");
                    close(cl_fd);
                    close(epoll_fd);
                    return 1;
                }
                close(cl_fd);
                close(epoll_fd);
                return 0;
            }
            else
            {
                printf("Unknown: %s\n", r_buf);
            }
        }
    }
}
