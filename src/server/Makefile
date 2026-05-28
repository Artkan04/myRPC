CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I../../common
SOURCES = myRPC-server.c ../../common/protocol.c
TARGET = myRPC-server

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

clean:
	rm -f $(TARGET) *.o

install:
	mkdir -p /usr/local/bin
	cp $(TARGET) /usr/local/bin/
	mkdir -p /etc/myRPC
	cp ../../config/myRPC.conf /etc/myRPC/
	cp ../../config/users.conf /etc/myRPC/
