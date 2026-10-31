#include "app_view.h"

#include <stddef.h>
#include <string.h>

#include "..\int\int_oled.h"

#include "..\font6x8.h"
#include "..\icon6x8.h"

// 与 SSD1309 GDDRAM 同布局：[页][列]，字节内 bit0 为最上面一行
static uint8_t s_app_view_buffer[INT_OLED_PAGE_COUNT][INT_OLED_WIDTH];

// 帧缓冲是否被改动过，为 0 时跳过整帧传输
static uint8_t s_app_view_dirty;

/**
 * @brief 清空内部帧缓冲区，需调用 app_view_commit 后才会刷新到屏幕。
 *
 * @note 只改缓冲区，不直接操作屏幕。
 */
void app_view_clear(void) {
    memset(s_app_view_buffer, 0, sizeof(s_app_view_buffer));
    s_app_view_dirty = 1;
}

/**
 * @brief 绘制单个像素，坐标越界时直接忽略。
 *
 * 像素落到第 y/8 页的 bit(y%8) 上，按 on 置位或清零。
 *
 * @param x 横坐标，有效范围 0 ~ 127。
 * @param y 纵坐标，有效范围 0 ~ 63。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_pixel(uint8_t x, uint8_t y, uint8_t on) {
    if (x >= INT_OLED_WIDTH || y >= INT_OLED_HEIGHT) {
        return;
    }

    s_app_view_dirty = 1;
    uint8_t *byte = &s_app_view_buffer[y / 8][x];
    uint8_t mask = (uint8_t)(1u << (y % 8));
    if (on) {
        *byte |= mask;
    } else {
        *byte &= (uint8_t)~mask;
    }
}

/**
 * @brief 绘制直线，屏幕外的像素由 app_view_draw_pixel 自动裁掉。
 *
 * @param x0 起点横坐标。
 * @param y0 起点纵坐标。
 * @param x1 终点横坐标。
 * @param y1 终点纵坐标。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t on) {
    // Bresenham 直线算法，dy 取负值以便统一比较
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);
    int step_x = (x0 < x1) ? 1 : -1;
    int step_y = (y0 < y1) ? 1 : -1;
    int x = x0;
    int y = y0;
    int error = dx + dy;

    for (;;) {
        app_view_draw_pixel((uint8_t)x, (uint8_t)y, on);
        if (x == x1 && y == y1) {
            break;
        }
        int error2 = error * 2;
        if (error2 >= dy) {
            error += dy;
            x += step_x;
        }
        if (error2 <= dx) {
            error += dx;
            y += step_y;
        }
    }
}

/**
 * @brief 绘制矩形边框，宽或高为 0 时不绘制。
 *
 * @param x 左上角横坐标。
 * @param y 左上角纵坐标。
 * @param width 宽度（像素）。
 * @param height 高度（像素）。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t on) {
    if (width == 0 || height == 0) {
        return;
    }

    uint8_t x1 = (uint8_t)(x + width - 1);
    uint8_t y1 = (uint8_t)(y + height - 1);
    app_view_draw_line(x, y, x1, y, on);
    app_view_draw_line(x, y1, x1, y1, on);
    app_view_draw_line(x, y, x, y1, on);
    app_view_draw_line(x1, y, x1, y1, on);
}

/**
 * @brief 绘制单个 6x8 字符，左上角对齐 (x, y)，非可见 ASCII 字符不绘制。
 *
 * @param x 字符左上角横坐标。
 * @param y 字符左上角纵坐标。
 * @param ch 待绘制字符，有效范围 0x20 ~ 0x7E。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_char(uint8_t x, uint8_t y, char ch, uint8_t on) {
    if (ch < FONT6X8_FIRST_CHAR || ch > FONT6X8_LAST_CHAR) {
        return;
    }

    // 字模列优先，bit0 在最上面
    const uint8_t *glyph = font6x8[ch - FONT6X8_FIRST_CHAR];
    for (int col = 0; col < FONT6X8_WIDTH; col++) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < FONT6X8_HEIGHT; row++) {
            if ((bits >> row) & 1u) {
                app_view_draw_pixel((uint8_t)(x + col), (uint8_t)(y + row), on);
            }
        }
    }
}

/**
 * @brief 绘制字符串，每字符占 6 列，写到屏幕右侧外即停止。
 *
 * @param x 首字符左上角横坐标。
 * @param y 字符左上角纵坐标。
 * @param text 以 '\0' 结尾的字符串，为 NULL 时不绘制。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_string(uint8_t x, uint8_t y, const char *text, uint8_t on) {
    if (text == NULL) {
        return;
    }

    int cursor = x;
    for (; *text != '\0' && cursor < INT_OLED_WIDTH; text++, cursor += FONT6X8_WIDTH) {
        app_view_draw_char((uint8_t)cursor, y, *text, on);
    }
}

/**
 * @brief 绘制一个 6x8 图形符号，左上角对齐 (x, y)。
 *
 * @param x 图形左上角横坐标。
 * @param y 图形左上角纵坐标。
 * @param icon 图形编号，不小于 ICON6X8_COUNT 时忽略。
 * @param on 非 0 置亮，0 熄灭。
 */
void app_view_draw_icon(uint8_t x, uint8_t y, icon6x8_id_t icon, uint8_t on) {
    if (icon >= ICON6X8_COUNT) {
        return;
    }

    // 与字模同样的列优先布局，bit0 在最上面
    const uint8_t *glyph = icon6x8[icon];
    for (int col = 0; col < ICON6X8_WIDTH; col++) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < ICON6X8_HEIGHT; row++) {
            if ((bits >> row) & 1u) {
                app_view_draw_pixel((uint8_t)(x + col), (uint8_t)(y + row), on);
            }
        }
    }
}

/**
 * @brief 将内部帧缓冲区整屏提交到 OLED。
 *
 * @note 提交内容即当前缓冲区的全部 1024 字节，会覆盖屏幕原有显示；
 *       缓冲区自上次提交后无改动时直接返回。
 */
void app_view_commit(void) {
    if (!s_app_view_dirty) {
        return;
    }

    int_oled_display(&s_app_view_buffer[0][0]);
    s_app_view_dirty = 0;
}

/**
 * @brief 显示主循环，由调度器按 20 ms 周期（约 50 fps）调用。
 *
 * 只负责把改动过的帧缓冲刷到屏幕，页面本身的绘制由页面代码完成。
 */
void app_view_main(void) {
    app_view_commit();
}

