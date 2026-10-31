#ifndef INT_OLED_H
#define INT_OLED_H

#include <stdint.h>

#define INT_OLED_WIDTH 128
#define INT_OLED_HEIGHT 64
#define INT_OLED_PAGE_COUNT (INT_OLED_HEIGHT / 8)
#define INT_OLED_BUFFER_SIZE (INT_OLED_WIDTH * INT_OLED_PAGE_COUNT)

#ifndef INT_OLED_PIN_MOSI
#define INT_OLED_PIN_MOSI (-1)
#endif

#ifndef INT_OLED_PIN_SCLK
#define INT_OLED_PIN_SCLK (-1)
#endif

#ifndef INT_OLED_PIN_CS
#define INT_OLED_PIN_CS (-1)
#endif

#ifndef INT_OLED_PIN_DC
#define INT_OLED_PIN_DC (-1)
#endif

#ifndef INT_OLED_PIN_RESET
#define INT_OLED_PIN_RESET (-1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 SSD1309 OLED 和 SPI 总线。
 *
 * 引脚宏必须先配置；CS 引脚可设为 -1，表示模块的 CS# 已固定拉低。
 */
void int_oled_init(void);

/**
 * @brief 将完整帧缓冲区刷新到 OLED。
 *
 * 帧缓冲区为 128×64 单色页式布局，共 1024 字节；每页 128 字节，
 * 每个字节对应竖直连续的 8 个像素。
 *
 * @param framebuffer 指向至少 INT_OLED_BUFFER_SIZE 字节的帧缓冲区。
 */
void int_oled_display(const uint8_t *framebuffer);

/**
 * @brief 清空 OLED 显示内容。
 */
void int_oled_clear(void);

#ifdef __cplusplus
}
#endif

#endif
