RELATIVE_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

CFLAGS += \
	-I$(RELATIVE_PATH)/common/include \
	-I$(RELATIVE_PATH)/bme280/include \
	-I$(RELATIVE_PATH)/i2c/include \
	-I$(RELATIVE_PATH)/spi/include \
	-I$(RELATIVE_PATH)/ssd1306/include

SRCS += \
	$(RELATIVE_PATH)/bme280/src/bme280.c \
	$(RELATIVE_PATH)/i2c/src/i2c.c \
	$(RELATIVE_PATH)/spi/src/spi.c \
	$(RELATIVE_PATH)/ssd1306/src/ssd1306.c


LDFLAGS += \
	-lcurl \
	-ljson-c

