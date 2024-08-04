#ifndef BME280_H
#define BME280_H

#include "common.h"

typedef struct calib_data calib_data_t;

typedef struct {
	calib_data_t *calib_data;
	int		(*reset)(void);
	float	(*read_temperature)(void);
	float	(*read_pressure)(void);
	float	(*read_humidity)(void);
} bme280_t;

bme280_t *bme280_new(void);

#endif // BME280_H
