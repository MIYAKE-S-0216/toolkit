SRCS	+=	test.c
TARGET	=	test

# パラメータを読み込む
include param.mk

# Cコンパイラ
CC = gcc

# コンパイルオプション
CFLAGS += -Wall -O2

# define プション
CFLAGS += -DBME280_USE_I2C

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
