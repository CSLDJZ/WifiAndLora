#ifndef APP_FSM_H
#define APP_FSM_H

#include <stdio.h>

#include "..\data_types.h"

#include "app_data_proc.h"
#include "app_view.h"
#include "app_io.h"

typedef enum{
    PAGE_NONE = 0,  // 表示无有效页面，固定放在枚举首位。
    PAGE_MAIN,
    PAGE_WIFI,
    PAGE_LORA,
    PAGE_SAVE,
    PAGE_UART_TX,
    PAGE_COUNT      // 页面数量边界，固定放在枚举末尾。
} APP_FSM_PAGE_t;

typedef struct{
    const char *text;
    APP_FSM_PAGE_t next_page;
    void (*funk)(void); 
} APP_FSM_ITEM_t;

typedef struct{
    const char *title;
    APP_FSM_PAGE_t back_id;
    const APP_FSM_ITEM_t *items;
    uint_8 item_count;
} APP_FSM_MENU_PAGE_t;

#endif // APP_FSM_H
