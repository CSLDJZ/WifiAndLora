#ifndef INT_KEY_H
#define INT_KEY_H

#include <stdint.h>

typedef enum {
    KEY_NO = 0, // 表无输入
    KEY_UP,     //
    KEY_DOWN,   //
    KEY_LEFT,   // 又兼返回
    KEY_RIGHT   // 又兼前进
} INT_KEYS_t;

// 2×2 矩阵按键：行线输出（扫描时逐行拉低），列线为带上拉的输入
#define INT_KEY_ROW_COUNT (2)
#define INT_KEY_COL_COUNT (2)

// 矩阵引脚待定，硬件确定后在此统一修改；也可由编译宏覆盖
#ifndef INT_KEY_PIN_ROW0
#define INT_KEY_PIN_ROW0 (-1)
#endif

#ifndef INT_KEY_PIN_ROW1
#define INT_KEY_PIN_ROW1 (-1)
#endif

#ifndef INT_KEY_PIN_COL0
#define INT_KEY_PIN_COL0 (-1)
#endif

#ifndef INT_KEY_PIN_COL1
#define INT_KEY_PIN_COL1 (-1)
#endif

// 矩阵中单个按键的采样与消抖状态
typedef struct {
    uint8_t raw_level;    // 上一次采样电平
    uint8_t stable_level; // 消抖后的稳定电平
} INT_KEY_CELL_t;

#ifdef __cplusplus
extern "C" {
#endif

void int_key_main(void);
INT_KEYS_t int_key_get_now_key(void);

#ifdef __cplusplus
}
#endif

#endif
