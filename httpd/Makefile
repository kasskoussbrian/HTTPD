export CC = gcc
export CPPFLAGS = -Isrc 
export CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla 

BINARY=httpd

all: $(BINARY)

$(BINARY):
	$(MAKE) -C src
	mv src/$(BINARY) .
clean:
	$(MAKE) -C src clean


.PHONY: all
