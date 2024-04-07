include flags.mk

OBJ = main.o

LIBS = config/libconfig.a daemon/libdaemon.a http/libhttp.a server/libserver.a logger/liblogger.a
LIBS += utils/libutils.a 

all: httpd

httpd: $(OBJ) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) -Wl,--start-group $(LIBS) -Wl,--end-group

clean:
	rm *.o
	$(MAKE) -C config clean
	$(MAKE) -C daemon clean
	$(MAKE) -C http clean
	$(MAKE) -C server clean
	$(MAKE) -C utils clean
	$(MAKE) -C logger clean


include libs.mk

.PHONY: all
