#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bme280.h"
#include "ssd1306.h"

int main()
{
	// センサー・ディスプレイの初期化
	bme280_t *bme280	= bme280_new();
	ssd1306_t *ssd1306	= ssd1306_new();

	bme280->reset();
	ssd1306->reset();

	// メインループ
	while(1)
	{
		// センサーデータの取得
		float temperature	= bme280->read_temperature();
		float pressure		= bme280->read_pressure();

		// バッファのクリア
		ssd1306->clear_buffer();

		// テキストの描画
		char text[128];
		sprintf(text, "T: %.2f C\nP: %.2f hPa\n", temperature, pressure);
		ssd1306->draw_text(text);

		// 画像の表示
		ssd1306->display();

		// 100ms待機
		usleep(100000);
	}

	return 0;
}

