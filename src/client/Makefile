CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I../../common
SOURCES = myRPC-client.c ../../common/protocol.c
TARGET = myRPC-client

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

clean:
	rm -f $(TARGET) *.o
