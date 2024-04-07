#ifndef LOGGER_H
#define LOGGER_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include "../http/http.h"

void write_request_logs(struct config *conf, struct request *req, int my_err);

#endif /* ! LOGGER_H */
