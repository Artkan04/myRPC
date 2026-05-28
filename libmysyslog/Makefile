CC = gcc
CFLAGS = -Wall -Wextra -fPIC
TARGET = libmysyslog.a
SO_TARGET = libmysyslog.so
OBJS = mysyslog.o

.PHONY: all clean

all: $(TARGET) $(SO_TARGET)

$(TARGET): $(OBJS)
	ar rcs $@ $^

$(SO_TARGET): $(OBJS)
	$(CC) -shared -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.a *.so
