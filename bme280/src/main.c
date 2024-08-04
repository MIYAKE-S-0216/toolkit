#include <stdio.h>
#include "bme280.h"

int main() {
	bme280_t *bme280 = bme280_new();

	bme280->reset();

	// センサーデータの取得
	float temperature	= bme280->read_temperature();
	float pressure		= bme280->read_pressure();
	float humidity		= bme280->read_humidity();

	// センサーデータの表示
	printf("T: %.2f C\nP: %.2f hPa\nH: %.2f %%\n", temperature, pressure, humidity);

	return 0;
}

