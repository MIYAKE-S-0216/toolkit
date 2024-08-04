#include "spi.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>

#define SPI_DEVICE_PATH "/dev/spidev0.0"

typedef struct spi_status {
	uint8_t mode;
} spi_status_t;

static int	spi_get_mode(uint8_t *mode);
static int	writeb(spi_param_t *p);
static int	readb(spi_param_t *p);

static spi_t *this = NULL;

// SPI の mode を取得する
static int spi_get_mode(uint8_t *mode) {
	int spi_fd = open(SPI_DEVICE_PATH, O_RDWR);
	if (spi_fd < 0) {
		perror("Failed to open SPI bus");
		return -1;
	}

	int ret = ioctl(spi_fd, SPI_IOC_RD_MODE, mode);
	if (ret < 0) {
		perror("Failed to get SPI mode");
	}

	close(spi_fd);
	return ret;
}

static int writeb(spi_param_t *p) {
	int spi_fd = open(SPI_DEVICE_PATH, O_RDWR);
	if (spi_fd < 0) {
		perror("Failed to open SPI bus");
		return -1;
	}

	int ret = 0;
	if( (this->status->mode != p->mode) &&
		(ioctl(spi_fd, SPI_IOC_WR_MODE, &p->mode) < 0) ) {
		perror("Failed to set SPI mode");
		ret = -1;
	}

	struct spi_ioc_transfer tr[2] = {
		{
			.tx_buf = (unsigned long)&p->addr,
			.len = 1,
			.speed_hz = 500000,
			.delay_usecs = 0,
			.bits_per_word = 8,
		},
		{
			.tx_buf = (unsigned long)p->data,
			.len = p->length,
			.speed_hz = 500000,
			.delay_usecs = 0,
			.bits_per_word = 8,
		},
	};

	if (ioctl(spi_fd, SPI_IOC_MESSAGE(2), tr) < 2) {
		perror("Failed to write to SPI bus");
		ret = -1;
	}

	close(spi_fd);
	return ret;
}

static int readb(spi_param_t *p) {
	int spi_fd = open(SPI_DEVICE_PATH, O_RDWR);
	if (spi_fd < 0) {
		perror("Failed to open SPI bus");
		return -1;
	}

	int ret = 0;
	if( (this->status->mode != p->mode) &&
		(ioctl(spi_fd, SPI_IOC_WR_MODE, &p->mode) < 0) ) {
		perror("Failed to set SPI mode");
		ret = -1;
	}

	struct spi_ioc_transfer tr[2] = {
		{
			.tx_buf = (unsigned long)&p->addr,
			.len = 1,
			.speed_hz = 500000,
			.delay_usecs = 0,
			.bits_per_word = 8,
		},
		{
			.rx_buf = (unsigned long)p->data,
			.len = p->length,
			.speed_hz = 500000,
			.delay_usecs = 0,
			.bits_per_word = 8,
		},
	};

	if (ioctl(spi_fd, SPI_IOC_MESSAGE(2), tr) < 2) {
		perror("Failed to read from SPI bus");
		ret = -1;
	}

	close(spi_fd);
	return ret;
}

spi_t *spi_new(void) {
	// すでにインスタンスが存在している場合はそのまま返す
	if(this != NULL) {
		return this;
	}

	this = MALLOC(spi_t, 1);
	if (this == NULL) {
		perror("Failed to allocate memory for spi_t");
		return NULL;
	}

	this->status = MALLOC(spi_status_t, 1);
	if (this->status == NULL) {
		perror("Failed to allocate memory for spi_status_t");
		goto error;
	}

	this->write = writeb;
	this->read = readb;
	spi_get_mode(&this->status->mode);

	return this;

error:
	FREE(this->status);
	FREE(this);
	return NULL;
}
