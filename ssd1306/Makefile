CC = gcc
CFLAGS = -Iinclude -I../i2c/include -I../common/include -Wall -O2
SRCS = src/main.c src/ssd1306.c ../i2c/src/i2c.c
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
