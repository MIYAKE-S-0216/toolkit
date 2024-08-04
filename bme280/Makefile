CC = gcc
CFLAGS = -Iinclude -I../i2c/include -I../spi/include -I../common/include -Wall -O2 -DBME280_USE_I2C
SRCS = src/main.c src/bme280.c ../i2c/src/i2c.c ../spi/src/spi.c
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
