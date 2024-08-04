#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stddef.h>
#include "common.h"

typedef struct {
	uint8_t addr;
	uint8_t sub_addr;
	uint8_t *data;
	size_t  length;
} i2c_param_t;

typedef struct {
	int		(*write)(i2c_param_t *param);
	int		(*read)(i2c_param_t *param);
} i2c_t;

i2c_t *i2c_new(void);

#endif // I2C_H
