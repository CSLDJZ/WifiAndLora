#ifndef APP_VIEW_H
#define APP_VIEW_H

#include <stdint.h>

#include "..\data_types.h"
#include "..\icon6x8.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 清空 app_view 内部帧缓冲区（不影响屏幕，需再调用 app_view_commit 生效）。
 */
void app_view_clear(void);

/**
 * @brief 绘制单个像素。
 *
 * @param x 横坐标，不小于 128 时忽略。
 * @param y 纵坐标，不小于 64 时忽略。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_pixel(uint8_t x, uint8_t y, uint8_t on);

/**
 * @brief 绘制直线，超出屏幕的像素自动裁掉。
 *
 * @param x0 起点横坐标。
 * @param y0 起点纵坐标。
 * @param x1 终点横坐标。
 * @param y1 终点纵坐标。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t on);

/**
 * @brief 绘制矩形边框。
 *
 * @param x 左上角横坐标。
 * @param y 左上角纵坐标。
 * @param width  宽度（像素），为 0 时不绘制。
 * @param height 高度（像素），为 0 时不绘制。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t on);

/**
 * @brief 绘制一个 6x8 字符，左上角对齐 (x, y)。
 *
 * 非 ASCII 可见字符（0x20 ~ 0x7E）不绘制。
 *
 * @param x 字符左上角横坐标。
 * @param y 字符左上角纵坐标。
 * @param ch 待绘制字符，有效范围 0x20 ~ 0x7E。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_char(uint8_t x, uint8_t y, char ch, uint8_t on);

/**
 * @brief 绘制字符串，逐字符向右排列，字宽 6 像素，超出右边界不再绘制。
 *
 * @param x 首字符左上角横坐标。
 * @param y 字符左上角纵坐标。
 * @param text 以 '\0' 结尾的字符串，为 NULL 时不绘制。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_string(uint8_t x, uint8_t y, const char *text, uint8_t on);

/**
 * @brief 绘制一个 6x8 图形符号，左上角对齐 (x, y)。
 *
 * @param x 图形左上角横坐标。
 * @param y 图形左上角纵坐标。
 * @param icon 图形编号，不小于 ICON6X8_COUNT 时忽略。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_icon(uint8_t x, uint8_t y, icon6x8_id_t icon, uint8_t on);

/**
 * @brief 将内部帧缓冲区整屏刷新到 OLED。
 *
 * @note 缓冲区自上次提交后无改动时不传输。
 */
void app_view_commit(void);

/**
 * @brief 显示主循环，由调度器按 20 ms 周期（约 50 fps）调用。
 */
void app_view_main(void);

#ifdef __cplusplus
}
#endif

#endif // APP_VIEW_H
