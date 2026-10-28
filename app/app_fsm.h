#ifndef APP_FSM_H
#define APP_FSM_H

#include "..\data_types.h"

// 功能枚举
typedef enum{
    PAGE_NONE = 0,  // 表示无有效页面，固定放在枚举首位
    PAGE_MAIN,
    PAGE_WIFI,
    PAGE_LORA,
    PAGE_SAVE,
    PAGE_SAVE_READY,
    PAGE_SAVE_ERROR,
    PAGE_UART_TX,
    PAGE_COUNT      // 页面数量边界，固定放在枚举末尾
} APP_FSM_PAGE_t;

// 选项
typedef struct{
    const char *text;           // 选项名
    APP_FSM_PAGE_t next_page;   // 跳转至页面
    void (*funk)(void);         // 跳转至功能
} APP_FSM_ITEM_t;

// 页面
typedef struct{
    const char *title;          // 界面标题
    APP_FSM_PAGE_t back_id;     // 父级页面id
    const APP_FSM_ITEM_t *items;// 页面下选项
    uint8_t item_count;         // 页面下选项数量
} APP_FSM_MENU_PAGE_t;

APP_FSM_PAGE_t app_fsm_get_now_page(void);
uint8_t app_fsm_get_now_cursor(void);

#endif // APP_FSM_H
