/**
 * @file icon6x8.h
 * @brief 6x8 点阵图形符号库：三角形、高度递增的填充块、分割线/分隔符。
 *
 * 存储格式与 font6x8 完全一致：每个图形 6 字节，第 i 字节对应第 i 列像素
 * （列优先/竖排）；字节 bit0 是该列最上面一行，bit7 是最下面一行，与 SSD1309
 * 页式显存的竖向排列一致，可直接整字节写入帧缓冲区。
 *
 * 分割类图形共用同一条横向基准线和纵向基准线（ICON6X8_LINE_ROW /
 * ICON6X8_LINE_COL），因此相邻图块拼在一起时线条能自然接上：
 *   - 横线/竖线/十字分别占满整格；
 *   - T 型与边角只保留朝向内的那一段，正好嵌在网格的边中点上。
 *
 * 三角形按等边比例绘制：4x4 居中，四周各留 1 像素空边，每 2 行收窄 2 列。
 */
#ifndef ICON6X8_H
#define ICON6X8_H

#include <stdint.h>

#define ICON6X8_WIDTH 6
#define ICON6X8_HEIGHT 8

/* 横向分割线所在行、纵向分割线所在列（0 起，与图块左上角对齐） */
#define ICON6X8_LINE_ROW 3
#define ICON6X8_LINE_COL 2

/**
 * @brief 图形编号，同时作为 icon6x8 数组的下标。
 */
typedef enum {
    ICON6X8_TRI_UP = 0,  /* ▲ 填充三角形，尖端朝上 */
    ICON6X8_TRI_DOWN,    /* ▼ 填充三角形，尖端朝下 */
    ICON6X8_TRI_LEFT,    /* ◀ 填充三角形，尖端朝左 */
    ICON6X8_TRI_RIGHT,   /* ▶ 填充三角形，尖端朝右 */

    /* 填充色块，底边对齐最下一行，高度依次为 1 ~ 8 像素 */
    ICON6X8_BAR_H1,
    ICON6X8_BAR_H2,
    ICON6X8_BAR_H3,
    ICON6X8_BAR_H4,
    ICON6X8_BAR_H5,
    ICON6X8_BAR_H6,
    ICON6X8_BAR_H7,
    ICON6X8_BAR_H8,

    ICON6X8_LINE_H,     /* ─ 横向分割线 */
    ICON6X8_LINE_V,     /* │ 纵向分割线 */
    ICON6X8_CROSS,      /* ┼ 十字 */

    ICON6X8_TEE_DOWN,   /* ┳ 横线 + 竖线向下伸 */
    ICON6X8_TEE_UP,     /* ┻ 横线 + 竖线向上伸 */
    ICON6X8_TEE_RIGHT,  /* ┣ 竖线 + 横线向右伸 */
    ICON6X8_TEE_LEFT,   /* ┫ 竖线 + 横线向左伸 */

    ICON6X8_CORNER_TL,  /* ┌ 左上边角 */
    ICON6X8_CORNER_TR,  /* ┐ 右上边角 */
    ICON6X8_CORNER_BL,  /* └ 左下边角 */
    ICON6X8_CORNER_BR,  /* ┘ 右下边角 */

    ICON6X8_COUNT
} icon6x8_id_t;

extern const uint8_t icon6x8[ICON6X8_COUNT][ICON6X8_WIDTH];

#endif // ICON6X8_H
