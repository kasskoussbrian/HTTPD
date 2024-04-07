SRC_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

config/libconfig.a:
	$(MAKE) -C $(SRC_DIR)config

logger/liblogger.a:
	$(MAKE) -C $(SRC_DIR)logger

daemon/libdaemon.a:
	$(MAKE) -C $(SRC_DIR)daemon

server/libserver.a:
	$(MAKE) -C $(SRC_DIR)server

utils/libutils.a:
	$(MAKE) -C $(SRC_DIR)utils

http/libhttp.a:
	$(MAKE) -C $(SRC_DIR)http


