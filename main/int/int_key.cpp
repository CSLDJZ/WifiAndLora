#include "int_key.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"

/*
 * 列线内部上拉，按键闭合时该行与列短接，列线读到低电平。
 * 行线为开漏输出：选中行拉低，其余行释放为高阻。同一列两个按键同时闭合时，
 * 高阻行不会与拉低行互相对顶，避免两个 GPIO 驱动直接短路。
 * 代价是未选中行的列线只能靠列线上拉回升，所以切行后要等上拉把列线拉高再采样。
 */
#define INT_KEY_LEVEL_PRESSED (0)
#define INT_KEY_LEVEL_RELEASED (1)

// 切行后等待列线上拉稳定，避免读到上一行的残留低电平，50us 足够覆盖 45k 上拉加线容
#define INT_KEY_SETTLE_US (50)

static const int s_int_key_row_pins[INT_KEY_ROW_COUNT] = {INT_KEY_PIN_ROW0, INT_KEY_PIN_ROW1};
static const int s_int_key_col_pins[INT_KEY_COL_COUNT] = {INT_KEY_PIN_COL0, INT_KEY_PIN_COL1};

// 按键映射，行列下标即行优先扫描顺序；靠后的按键后写入，覆盖先前事件
static const INT_KEYS_t s_int_key_map[INT_KEY_ROW_COUNT][INT_KEY_COL_COUNT] = {
    {KEY_UP, KEY_DOWN},
    {KEY_LEFT, KEY_RIGHT},
};

// 每个按键的采样与消抖状态，初值按释放（高电平）处理
static INT_KEY_CELL_t s_int_key_cells[INT_KEY_ROW_COUNT][INT_KEY_COL_COUNT] = {
    {{INT_KEY_LEVEL_RELEASED, INT_KEY_LEVEL_RELEASED}, {INT_KEY_LEVEL_RELEASED, INT_KEY_LEVEL_RELEASED}},
    {{INT_KEY_LEVEL_RELEASED, INT_KEY_LEVEL_RELEASED}, {INT_KEY_LEVEL_RELEASED, INT_KEY_LEVEL_RELEASED}},
};

// 单元素事件缓存，读取后清零
static INT_KEYS_t s_int_key_cache = KEY_NO;
static bool s_int_key_is_inited = false;
static bool s_int_key_is_ready = false;

/**
 * @brief 检查矩阵引脚是否为有效且互不冲突的 GPIO
 *
 * 行线需可作输出，列线只需有效引脚。
 *
 * @return 引脚配置有效时返回 true，否则返回 false
 */
static bool int_key_pins_are_valid(void){
    uint64_t pin_mask = 0;

    for (uint8_t row = 0; row < INT_KEY_ROW_COUNT; row++){
        int pin = s_int_key_row_pins[row];
        if (!GPIO_IS_VALID_GPIO(pin) || !GPIO_IS_VALID_OUTPUT_GPIO(pin)){
            return false;
        }

        uint64_t bit = (uint64_t)1 << (unsigned)pin;
        if (pin_mask & bit){
            return false;
        }
        pin_mask |= bit;
    }

    for (uint8_t col = 0; col < INT_KEY_COL_COUNT; col++){
        int pin = s_int_key_col_pins[col];
        if (!GPIO_IS_VALID_GPIO(pin)){
            return false;
        }

        uint64_t bit = (uint64_t)1 << (unsigned)pin;
        if (pin_mask & bit){
            return false;
        }
        pin_mask |= bit;
    }
    return true;
}

/**
 * @brief 选中一行扫描：该行拉低，其余行释放为高阻
 *
 * @param row 行号
 */
static void int_key_select_row(uint8_t row){
    for (uint8_t i = 0; i < INT_KEY_ROW_COUNT; i++){
        gpio_set_level((gpio_num_t)s_int_key_row_pins[i], i == row ? 0 : 1);
    }
}

/**
 * @brief 扫描结束后行线全部释放为高阻，按住期间不产生灌电流
 */
static void int_key_release_rows(void){
    for (uint8_t i = 0; i < INT_KEY_ROW_COUNT; i++){
        gpio_set_level((gpio_num_t)s_int_key_row_pins[i], 1);
    }
}

/**
 * @brief 行线配置为输出、列线配置为带上拉输入，不开启中断
 *
 * @return 配置成功返回 true，否则返回 false
 */
static bool int_key_init(void){
    if (!int_key_pins_are_valid()){
        return false;
    }

    gpio_config_t row_config = {};
    row_config.mode = GPIO_MODE_OUTPUT_OD;
    row_config.pull_up_en = GPIO_PULLUP_DISABLE;
    row_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    row_config.intr_type = GPIO_INTR_DISABLE;
    for (uint8_t i = 0; i < INT_KEY_ROW_COUNT; i++){
        row_config.pin_bit_mask |= (uint64_t)1 << (unsigned)s_int_key_row_pins[i];
    }
    if (gpio_config(&row_config) != ESP_OK){
        return false;
    }

    gpio_config_t col_config = {};
    col_config.mode = GPIO_MODE_INPUT;
    col_config.pull_up_en = GPIO_PULLUP_ENABLE;
    col_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    col_config.intr_type = GPIO_INTR_DISABLE;
    for (uint8_t i = 0; i < INT_KEY_COL_COUNT; i++){
        col_config.pin_bit_mask |= (uint64_t)1 << (unsigned)s_int_key_col_pins[i];
    }
    if (gpio_config(&col_config) != ESP_OK){
        return false;
    }

    int_key_release_rows();
    return true;
}

/**
 * @brief 单键消抖：连续两次采样电平相同才更新稳定电平，释放沿写入事件缓存
 *
 * @param row       行号
 * @param col       列号
 * @param raw_level 本次采样电平
 */
static void int_key_update_cell(uint8_t row, uint8_t col, uint8_t raw_level){
    INT_KEY_CELL_t *cell = &s_int_key_cells[row][col];

    // 电平刚变化时只记录采样值，下一次采样相同才确认
    if (raw_level != cell->raw_level){
        cell->raw_level = raw_level;
        return;
    }
    if (raw_level == cell->stable_level){
        return;
    }

    // 稳定电平由按下变为释放才上报，按住不放不重复上报
    if (cell->stable_level == INT_KEY_LEVEL_PRESSED){
        s_int_key_cache = s_int_key_map[row][col];
    }
    cell->stable_level = raw_level;
}

void int_key_main(void){
    // 引脚占位未配置时保持静默，不产生任何按键事件
    if (!s_int_key_is_inited){
        s_int_key_is_inited = true;
        s_int_key_is_ready = int_key_init();
    }
    if (!s_int_key_is_ready){
        return;
    }

    // 一次调度周期内扫完整个矩阵，每个按键采样间隔即 10ms，两次采样共 20ms 消抖
    for (uint8_t row = 0; row < INT_KEY_ROW_COUNT; row++){
        int_key_select_row(row);
        esp_rom_delay_us(INT_KEY_SETTLE_US);
        for (uint8_t col = 0; col < INT_KEY_COL_COUNT; col++){
            uint8_t raw_level = (uint8_t)gpio_get_level((gpio_num_t)s_int_key_col_pins[col]);
            int_key_update_cell(row, col, raw_level);
        }
    }
    int_key_release_rows();
}

INT_KEYS_t int_key_get_now_key(void){
    INT_KEYS_t now_key = s_int_key_cache;
    s_int_key_cache = KEY_NO;
    return now_key;
};
