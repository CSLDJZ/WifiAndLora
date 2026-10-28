#include "app_fsm.h"

#include <stdio.h>

#include "app_data_proc.h"
#include "app_view.h"
#include "app_io.h"

#include "..\int\int_key.h"

// 页面位置枚举
static APP_FSM_PAGE_t g_app_fsm_now_page;
// 光标
static uint8_t g_app_fsm_now_cursor;
// 主界面选项
static const APP_FSM_ITEM_t s_app_fsm_main_items[] = {
    {"WiFi", PAGE_WIFI, NULL},
    {"LoRa", PAGE_LORA, NULL},
    {"Save", PAGE_SAVE, NULL},
    {"Out data to PC", PAGE_UART_TX, NULL}
};
// WIFI检测界面选项
static const APP_FSM_ITEM_t s_app_fsm_wifi_items[] = {
    {"Save", PAGE_SAVE, NULL},
    {"Back", PAGE_MAIN, NULL}
};
// LORA检测界面选项
static const APP_FSM_ITEM_t s_app_fsm_lora_items[] = {
    {"Save", PAGE_SAVE, NULL},
    {"Back", PAGE_MAIN, NULL}
};
// 保存界面选项
static const APP_FSM_ITEM_t s_app_fsm_save_items[] = {
    {"Downloading...", PAGE_NONE, NULL}
};
// 保存完成界面选项
static const APP_FSM_ITEM_t s_app_fsm_save_ready_items[] = {
    {"Ready", PAGE_NONE, NULL},
    {"Back", PAGE_MAIN, NULL}
};
// 保存错误界面选项
static const APP_FSM_ITEM_t s_app_fsm_save_error_items[] = {
    {"Error", PAGE_NONE, NULL},
    {"Back", PAGE_MAIN, NULL}
};

#define APP_FSM_ITEM_COUNT(a) (sizeof(a) / sizeof((a)[0]))
// 界面注册表
static const APP_FSM_MENU_PAGE_t s_app_fsm_pages[] = {
    [PAGE_MAIN] = {"Main", PAGE_NONE, s_app_fsm_main_items, APP_FSM_ITEM_COUNT(s_app_fsm_main_items)},
    [PAGE_WIFI] = {"Wifi", PAGE_MAIN, s_app_fsm_wifi_items, APP_FSM_ITEM_COUNT(s_app_fsm_wifi_items)},
    [PAGE_LORA] = {"Lora", PAGE_MAIN, s_app_fsm_lora_items, APP_FSM_ITEM_COUNT(s_app_fsm_lora_items)},
    [PAGE_SAVE] = {"Save", PAGE_MAIN, s_app_fsm_save_items, APP_FSM_ITEM_COUNT(s_app_fsm_save_items)},
    [PAGE_SAVE_READY] = {"Save ready", PAGE_SAVE, s_app_fsm_save_ready_items, APP_FSM_ITEM_COUNT(s_app_fsm_save_ready_items)},
    [PAGE_SAVE_ERROR] = {"Save error", PAGE_SAVE, s_app_fsm_save_error_items, APP_FSM_ITEM_COUNT(s_app_fsm_save_error_items)}
};

/**
 * @brief 页面状态接口
 * 
 * @return APP_FSM_PAGE_t 页面id
 */
APP_FSM_PAGE_t app_fsm_get_now_page(void){
    return g_app_fsm_now_page;
};
/**
 * @brief 光标状态接口
 * 
 * @return uint8_t 光标位置
 */
uint8_t app_fsm_get_now_cursor(void){
    return g_app_fsm_now_cursor;
};

void app_fsm_key(void){
    APP_FSM_MENU_PAGE_t s_now_page = s_app_fsm_pages[g_app_fsm_now_page];
    uint8_t s_max_scroll = s_now_page.item_count;

    INT_KEYS_t s_now_key = int_key_get_now_key();
    switch (s_now_key){
    case KEY_UP:
        g_app_fsm_now_cursor = (g_app_fsm_now_cursor - 1) % s_max_scroll;
        break;
    
    case KEY_DOWN:
        g_app_fsm_now_cursor = (g_app_fsm_now_cursor + 1) % s_max_scroll;
        break;

    case KEY_LEFT:
        if (s_now_page.back_id != PAGE_NONE){
            g_app_fsm_now_page = s_now_page.back_id;
        }
        break;

    case KEY_RIGHT:
        if (s_now_page.items[g_app_fsm_now_cursor].funk != NULL){
            s_now_page.items[g_app_fsm_now_cursor].funk();
        }
        if (s_now_page.items[g_app_fsm_now_cursor].next_page != PAGE_NONE){
            g_app_fsm_now_page = s_now_page.items[g_app_fsm_now_cursor].next_page;
        }
        break;

    case KEY_NO:
        // 留空，暂时无意义
        break;
    }
};
