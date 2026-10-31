/**
 * @file font6x8.h
 * @brief 6x8 像素点阵字库：ASCII 可见字符 0x20 ~ 0x7E，共 95 个字符。
 *
 * 存储格式：每个字符 6 字节，第 i 字节对应第 i 列像素（列优先/竖排）；
 * 字节的 bit0 是该列最上面一行，bit7 是最下面一行，与 SSD1309 页式显存
 * 的竖向排列完全一致。字符笔画占用左侧 5 列，第 5 列(索引 5)为字间距。
 * 全部 95 个字符共 95 x 6 = 570 字节，放在 flash 的 .rodata 段。
 */
#ifndef FONT6X8_H
#define FONT6X8_H

#include <stdint.h>

#define FONT6X8_WIDTH 6
#define FONT6X8_HEIGHT 8
#define FONT6X8_FIRST_CHAR 0x20
#define FONT6X8_LAST_CHAR 0x7E
#define FONT6X8_CHAR_COUNT (FONT6X8_LAST_CHAR - FONT6X8_FIRST_CHAR + 1)

extern const uint8_t font6x8[FONT6X8_CHAR_COUNT][FONT6X8_WIDTH];

#endif