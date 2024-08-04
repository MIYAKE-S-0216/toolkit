#include "bme280.h"
#include "i2c.h"
#include "spi.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// スレーブアドレス
#define BME280_ADDRESS  0x77

// レジスタアドレス
#define ADDR_RESET      0xE0
#define ADDR_CTRL_MEAS  0xF4
#define ADDR_CTRL_HUM   0xF2
#define ADDR_CONFIG     0xF5
#define ADDR_CALIB      0x88
#define ADDR_CALIB_H    0xE1
#define ADDR_TEMP_MSB   0xFA
#define ADDR_PRESS_MSB  0xF7
#define ADDR_HUM_MSB    0xFD

typedef struct calib_data {
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	uint8_t dig_H1;
	int16_t dig_H2;
	uint8_t dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t dig_H6;
} calib_data_t;

static int		writeb(uint8_t reg_addr, uint8_t *data, size_t length);
static int		readb(uint8_t reg_addr, uint8_t *data, size_t length);
static int		reset(void);
static float	read_temperature(void);
static float	read_pressure(void);
static float	read_humidity(void);

// グローバル変数として t_fine を宣言
static int32_t t_fine;

static bme280_t *this = NULL;

static int writeb(uint8_t reg_addr, uint8_t *data, size_t length) {
#ifdef BME280_USE_I2C
	i2c_t* i2c = i2c_new();
	return i2c->write( &(i2c_param_t){
			.addr     = BME280_ADDRESS,
			.sub_addr = reg_addr,
			.data     = data,
			.length   = length,
			} );
#else
	spi_t* spi = spi_new();
	return spi->write( &(spi_param_t){
			.addr		= reg_addr,
			.data		= data,
			.length		= length,
			.mode		= 3,
			} );	
#endif
}

static int readb(uint8_t reg_addr, uint8_t *data, size_t length) {
#ifdef BME280_USE_I2C
	i2c_t* i2c = i2c_new();
	return i2c->read( &(i2c_param_t){
			.addr     = BME280_ADDRESS,
			.sub_addr = reg_addr,
			.data     = data,
			.length   = length,
			} );
#else
	spi_t* spi = spi_new();
	return spi->read( &(spi_param_t){
			.addr		= reg_addr,
			.data		= data,
			.length		= length,
			.mode		= 3,
			} );
#endif
}

// BME280センサーの初期化関数
static int reset(void) {
	int ret = 0;

	struct reset_command {
		uint8_t		addr;
		uint8_t		data;
		uint16_t	delayUsec;
	} reset_command[] = {
		{ADDR_RESET,		0xB6,	2000},	// リセットコマンド
		{ADDR_CTRL_MEAS,	0x27,	   0},	// 温度オーバーサンプリングx1、気圧オーバーサンプリングx1、通常モード
		{ADDR_CTRL_HUM,		0x01,	   0},	// 湿度オーバーサンプリングx1
		{ADDR_CONFIG,		0xA0,	   0},	// スタンバイ時間1000ms、フィルター係数16
	};

	// リセットコマンドの送信
	for(int i = 0; i < ARRAY_SIZE(reset_command); i++) {
		if(0 > writeb(reset_command[i].addr, &(uint8_t){reset_command[i].data}, 1)) {
			ret = -1;
			goto end;
		}
		if(reset_command[i].delayUsec > 0) {
			usleep(reset_command[i].delayUsec);
		}
	}

	// キャリブレーションデータの読み取り
	uint8_t calib_data_l[26];
	if(0 > readb(ADDR_CALIB, calib_data_l, 26)) {
		ret = -1;
		goto end;
	}

	// キャリブレーションデータを構造体に保存
	this->calib_data->dig_T1 = (calib_data_l[ 1] << 8) | calib_data_l[0];
	this->calib_data->dig_T2 = (calib_data_l[ 3] << 8) | calib_data_l[2];
	this->calib_data->dig_T3 = (calib_data_l[ 5] << 8) | calib_data_l[4];
	this->calib_data->dig_P1 = (calib_data_l[ 7] << 8) | calib_data_l[6];
	this->calib_data->dig_P2 = (calib_data_l[ 9] << 8) | calib_data_l[8];
	this->calib_data->dig_P3 = (calib_data_l[11] << 8) | calib_data_l[10];
	this->calib_data->dig_P4 = (calib_data_l[13] << 8) | calib_data_l[12];
	this->calib_data->dig_P5 = (calib_data_l[15] << 8) | calib_data_l[14];
	this->calib_data->dig_P6 = (calib_data_l[17] << 8) | calib_data_l[16];
	this->calib_data->dig_P7 = (calib_data_l[19] << 8) | calib_data_l[18];
	this->calib_data->dig_P8 = (calib_data_l[21] << 8) | calib_data_l[20];
	this->calib_data->dig_P9 = (calib_data_l[23] << 8) | calib_data_l[22];
	this->calib_data->dig_H1 =  calib_data_l[25];

	// 湿度キャリブレーションデータの読み取り
	uint8_t calib_data_h[7];
	if(0 > readb(ADDR_CALIB_H, calib_data_h, 7)) {
		ret = -1;
		goto end;
	}

	this->calib_data->dig_H2 = (calib_data_h[1] << 8) | calib_data_h[0];
	this->calib_data->dig_H3 = calib_data_h[2];
	this->calib_data->dig_H4 = (calib_data_h[3] << 4) | (calib_data_h[4] & 0x0F);
	this->calib_data->dig_H5 = (calib_data_h[5] << 4) | (calib_data_h[4] >> 4);
	this->calib_data->dig_H6 = calib_data_h[6];

end:
	return ret;
}

// 温度データの読み取り関数
static float read_temperature(void) {
	uint8_t data[3];
	readb(ADDR_TEMP_MSB, data, 3);

	int32_t adc_T = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

	int32_t var1 = ((((adc_T >> 3) - ((int32_t)this->calib_data->dig_T1 << 1))) * ((int32_t)this->calib_data->dig_T2)) >> 11;
	int32_t var2 = (((((adc_T >> 4) - ((int32_t)this->calib_data->dig_T1)) * ((adc_T >> 4) - ((int32_t)this->calib_data->dig_T1))) >> 12) * ((int32_t)this->calib_data->dig_T3)) >> 14;

	t_fine = var1 + var2;
	float T = (t_fine * 5 + 128) >> 8;
	return T / 100.0;
}

// 気圧データの読み取り関数
static float read_pressure(void) {
	uint8_t data[3];
	readb(ADDR_PRESS_MSB, data, 3);

	int32_t adc_P = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

	int32_t var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
	int32_t var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)this->calib_data->dig_P6);
	var2 = var2 + ((var1 * ((int32_t)this->calib_data->dig_P5)) << 1);
	var2 = (var2 >> 2) + (((int32_t)this->calib_data->dig_P4) << 16);
	var1 = (((this->calib_data->dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)this->calib_data->dig_P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t)this->calib_data->dig_P1)) >> 15);
	if (var1 == 0) {
		return 0; // 0除算を避けるために0を返す
	}
	int32_t P = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
	if (P < 0x80000000) {
		P = (P << 1) / ((uint32_t)var1);
	} else {
		P = (P / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)this->calib_data->dig_P9) * ((int32_t)(((P >> 3) * (P >> 3)) >> 13))) >> 12;
	var2 = (((int32_t)(P >> 2)) * ((int32_t)this->calib_data->dig_P8)) >> 13;
	P = (int32_t)((int32_t)P + ((var1 + var2 + this->calib_data->dig_P7) >> 4));
	return P / 100.0;
}

// 湿度データの読み取り関数
static float read_humidity(void) {
	uint8_t data[2];
	readb(ADDR_HUM_MSB, data, 2);

	int32_t adc_H = (data[0] << 8) | data[1];

	int32_t v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)this->calib_data->dig_H4) << 20) - (((int32_t)this->calib_data->dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)this->calib_data->dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)this->calib_data->dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)this->calib_data->dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)this->calib_data->dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	float H = (v_x1_u32r >> 12);
	return H / 1024.0;
}

// BME280センサー構造体の生成関数
bme280_t *bme280_new(void) {
	// すでにインスタンスが存在している場合はそのまま返す
	if(this != NULL) {
		return this;
	}

	this = MALLOC(bme280_t, 1);
	if (this == NULL) {
		fprintf(stderr, "BME280センサー構造体のメモリ確保に失敗しました\n");
		return NULL;
	}

	this->calib_data = MALLOC(calib_data_t, 1);
	if (this->calib_data == NULL) {
		fprintf(stderr, "キャリブレーションデータのメモリ確保に失敗しました\n");
		goto error;
	}

	this->reset				= reset;
	this->read_temperature		= read_temperature;
	this->read_pressure		= read_pressure;
	this->read_humidity		= read_humidity;

	return this;

error:
	FREE(this->calib_data);
	FREE(this);
	return NULL;
}

