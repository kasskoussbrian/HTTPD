#ifndef DAEMON_H
#define DAEMON_H
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../config/config.h"

int start_do(struct config *conf);
int quit_do(struct config *conf);
int restart_do(struct config *conf);

// int create_daemon(char *a_arg,struct config *conf);
#endif /* ! DAEMON_H */
