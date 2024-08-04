#include "i2c.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>

#define I2C_DEVICE_PATH "/dev/i2c-1"

static int	writeb(i2c_param_t *p);
static int	readb(i2c_param_t *p);

static i2c_t *this = NULL;

static int writeb(i2c_param_t *p) {
	int i2c_fd = open(I2C_DEVICE_PATH, O_RDWR);
	if (i2c_fd < 0) {
		perror("Failed to open I2C bus");
		return -1;
	}

	int ret = 0;
	uint8_t buffer[p->length + 1];

	if (ioctl(i2c_fd, I2C_SLAVE, p->addr) < 0) {
		perror("Failed to acquire bus access and/or communicate with slave");
		ret = -1;
		goto end;
	}

	buffer[0] = p->sub_addr;
	for (size_t i = 0; i < p->length; i++) {
		buffer[i + 1] = p->data[i];
	}

	if (write(i2c_fd, buffer, p->length + 1) != p->length + 1) {
		perror("Failed to write to I2C bus");
		ret = -1;
		goto end;
	}

end:
	close(i2c_fd);
	return ret;
}

static int readb(i2c_param_t *p) {
	int i2c_fd = open(I2C_DEVICE_PATH, O_RDWR);
	if (i2c_fd < 0) {
		perror("Failed to open I2C bus");
		return -1;
	}

	int ret = 0;

	if (ioctl(i2c_fd, I2C_SLAVE, p->addr) < 0) {
		perror("Failed to acquire bus access and/or communicate with slave");
		ret = -1;
		goto end;
	}

	if (write(i2c_fd, &(p->sub_addr), 1) != 1) {
		perror("Failed to write sub-address");
		ret = -1;
		goto end;
	}

	if (read(i2c_fd, p->data, p->length) != p->length) {
		perror("Failed to read from I2C bus");
		ret = -1;
		goto end;
	}

end:
	close(i2c_fd);
	return ret;
}

i2c_t *i2c_new(void) {
	// すでにインスタンスが存在している場合はそのまま返す
	if(this != NULL) {
		return this;
	}

	this = MALLOC(i2c_t, 1);
	if (this == NULL) {
		perror("Failed to allocate memory for i2c_t");
		return NULL;
	}

	this->write = writeb;
	this->read = readb;

	return this;
}
