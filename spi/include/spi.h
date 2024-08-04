#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stddef.h>
#include "common.h"

typedef struct spi_status spi_status_t;

typedef struct {
	uint8_t	addr;
	uint8_t	*data;
	size_t	length;
	uint8_t	mode;
} spi_param_t;

typedef struct {
	spi_status_t	*status;
	int				(*write)(spi_param_t *param);
	int				(*read)(spi_param_t *param);
} spi_t;

spi_t *spi_new(void);

#endif // SPI_H
