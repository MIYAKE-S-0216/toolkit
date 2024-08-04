#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include "common.h"

typedef struct buffer buffer_t;

/**
 * @brief SSD1306ディスプレイ構造体
 */
typedef struct {
	buffer_t		*buffer;
	int				(*reset)(void);
	int				(*clear_buffer)(void);
	int				(*draw_text)(const char* text);
	int				(*display)(void);
} ssd1306_t;

ssd1306_t *ssd1306_new(void);

#endif // SSD1306_H
