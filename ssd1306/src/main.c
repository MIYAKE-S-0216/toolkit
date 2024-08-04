#include <stdio.h>
#include <stdlib.h>
#include "ssd1306.h"

int main() {
	ssd1306_t *ssd1306 = ssd1306_new();

	// ディスプレイの初期化
	ssd1306->reset();

	// バッファのクリア
	ssd1306->clear_buffer();

	// テキストの描画
	ssd1306->draw_text("Hello, World!\nThis is a test.");

	// 画像の表示
	ssd1306->display();

	return 0;
}
