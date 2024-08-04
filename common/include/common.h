#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>

// malloc と free のラッパー関数
#define MALLOC(type, size) ((type *)malloc(sizeof(type) * (size)))
#define FREE(ptr) do { free(ptr); ptr = NULL; } while (0)

// 配列の要素数を取得する define
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#endif // COMMON_H
