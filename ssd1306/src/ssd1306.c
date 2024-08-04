#include "ssd1306.h"
#include "i2c.h"
#include "font.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ディスプレイの幅と高さ
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// スレーブアドレス
#define SSD1306_ADDRESS 0x3C

// レジスタアドレス
#define ADDR_CONTROL    0x00
#define ADDR_DATA       0x40

// 1バイトあたりのピクセル数
#define PIXEL_PER_BYTE 8

// ページあたりの行数
#define ROWS_PER_PAGE 8

//typedef uint8_t* buffer_t;
typedef struct buffer {
	uint8_t*	buf;
	int			width;
	int			height;
} buffer_t;

static int	writeb(uint8_t reg_addr, uint8_t *data, size_t length);
// static int readb(uint8_t reg_addr, uint8_t *data, size_t length);
static void	drawChar(buffer_t* buffer, char c, int x, int y);
static int	reset(void);
static int	clear_buffer(void);
static int	draw_text(const char* text);
static int	display(void);

static ssd1306_t *this = NULL;

static int writeb(uint8_t reg_addr, uint8_t *data, size_t length) {
	i2c_t* i2c = i2c_new();
	return i2c->write( &(i2c_param_t){
			.addr     = SSD1306_ADDRESS,
			.sub_addr = reg_addr,
			.data     = data,
			.length   = length,
			} );
}

#if 0
static int readb(uint8_t reg_addr, uint8_t *data, size_t length) {
	i2c_t* i2c = i2c_new();
	return i2c->read( &(i2c_param_t){
			.addr     = SSD1306_ADDRESS,
			.sub_addr = reg_addr,
			.data     = data,
			.length   = length,
			} );
}
#endif

// 文字を描画する関数
static void	drawChar(buffer_t* buffer, char c, int x, int y) {
	if (c < 32 || c > 127) {
		return; // サポートされていない文字は無視する
	}

	const uint8_t* glyph = font[c - 32];
	for (int i = 0; i < FONT_WIDTH; i++) {
		uint8_t line = glyph[i];
		for (int j = 0; j < FONT_HEIGHT; j++) {
			if (line & 1) {
				int pixelIndex = (y + j) * buffer->width + (x + i);
				if (pixelIndex < buffer->width * buffer->height) {
					buffer->buf[pixelIndex / PIXEL_PER_BYTE] |= (1 << (pixelIndex % PIXEL_PER_BYTE));
				}
			}
			line >>= 1;
		}
	}
}

// SSD1306ディスプレイを初期化する関数
static int reset(void) {
	uint8_t init_commands[] = {
		0xAE, // ディスプレイをオフにする
		0xD5, 0x80, // ディスプレイクロックの分周比/発振周波数を設定する
		0xA8, 0x3F, // マルチプレックス比を設定する（1から64まで）
		0xD3, 0x00, // ディスプレイオフセットを設定する
		0x40, // 開始行アドレスを設定する
		0x8D, 0x14, // チャージポンプレギュレータを有効にする
		0x20, 0x00, // メモリアドレッシングモードを設定する
		0xA1, // セグメントリマップを設定する（0から127まで）
		0xC8, // COM出力スキャン方向を設定する
		0xDA, 0x12, // COMピンのハードウェア構成を設定する
		0x81, 0xCF, // コントラスト制御レジスタを設定する
		0xD9, 0xF1, // プリチャージ期間を設定する
		0xDB, 0x40, // VCOMH非選択レベルを設定する
		0xA4, // 全体のディスプレイをONにする
		0xA6, // 通常のディスプレイ
		0xAF // ディスプレイをONにする
	};

	if (writeb(ADDR_CONTROL, init_commands, sizeof(init_commands)) != 0) {
		fprintf(stderr, "SSD1306の初期化に失敗しました\n");
		return -1;
	}
	return 0;
}

// バッファをクリアする関数
static int clear_buffer(void) {
	memset(this->buffer->buf, 0, this->buffer->width * this->buffer->height / PIXEL_PER_BYTE);
	return 0;
}

// テキストを描画する関数
static int draw_text(const char* text) {
	int x = 0;
	int y = 0;
	const char* p = text;

	while (*p) {
		if (*p == '\n') {
			x = 0;
			y += FONT_HEIGHT;
		} else {
			drawChar(this->buffer, *p, x, y);
			x += FONT_WIDTH;
			if (x + FONT_WIDTH > this->buffer->width) {
				x = 0;
				y += FONT_HEIGHT;
			}
		}
		p++;
	}

	return 0;
}

// バッファをディスプレイに表示する関数
static int display(void) {
	for (uint8_t page = 0; page < (this->buffer->height / ROWS_PER_PAGE); page++) {
		// ページアドレスを設定する
		uint8_t command[] = {
			0xB0 + page, // ページアドレスを設定する
			0x00, // 下位カラムアドレスを設定する
			0x10 // 上位カラムアドレスを設定する
		};
		writeb(ADDR_CONTROL, command, sizeof(command));

		// バッファを回転させる
		uint8_t bufferRotated[this->buffer->width / PIXEL_PER_BYTE * ROWS_PER_PAGE];
		memset(bufferRotated, 0, this->buffer->width / PIXEL_PER_BYTE * ROWS_PER_PAGE);

		uint8_t* bufHead = &(this->buffer->buf[this->buffer->width / PIXEL_PER_BYTE * ROWS_PER_PAGE * page]);
		uint8_t* bufRotatedHead = bufferRotated;
		const uint8_t widthBytes = this->buffer->width / PIXEL_PER_BYTE;

		for (int l = 0; l < widthBytes; l++) {
			for (int m = 0; m < PIXEL_PER_BYTE; m++) {
				uint8_t b = 0x00;

				for (int n = 0; n < ROWS_PER_PAGE; n++) {
					uint8_t dummy = (bufHead[l + (n * widthBytes)] >> m) & 0x01;
					b |= (dummy << n);
				}

				*bufRotatedHead++ = b;
			}
		}

		// 回転したバッファをディスプレイに書き込む
		writeb(ADDR_DATA, bufferRotated, this->buffer->width);
	}

	return 0;
}

// SSD1306センサー構造体を生成する関数
ssd1306_t* ssd1306_new(void) {
	// すでにインスタンスが存在している場合はそのまま返す
	if(this != NULL) {
		return this;
	}

	this = MALLOC(ssd1306_t, 1);
	if (this == NULL) {
		fprintf(stderr, "SSD1306センサー構造体のメモリ確保に失敗しました\n");
		return NULL;
	}

	this->buffer = MALLOC(buffer_t, 1);
	if (this->buffer == NULL) {
		fprintf(stderr, "ディスプレイバッファのメモリ確保に失敗しました\n");
		goto error;
	}
	this->buffer->width		= DISPLAY_WIDTH;
	this->buffer->height	= DISPLAY_HEIGHT;

	this->buffer->buf = MALLOC(uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT / PIXEL_PER_BYTE);
	if (this->buffer->buf == NULL) {
		fprintf(stderr, "ディスプレイバッファのメモリ確保に失敗しました\n");
		goto error;
	}

	this->reset			= reset;
	this->clear_buffer	= clear_buffer;
	this->draw_text		= draw_text;
	this->display		= display;

	return this;

error:
	FREE(this->buffer->buf);
	FREE(this->buffer);
	FREE(this);
	return NULL;
}

