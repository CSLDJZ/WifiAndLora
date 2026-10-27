#include "app_fsm.h"

static APP_FSM_PAGE_t s_app_fsm_page;
static const APP_FSM_ITEM_t s_app_fsm_main_items[] = {
    {"WiFi", PAGE_WIFI, NULL},
    {"LoRa", PAGE_LORA, NULL},
    {"Save", PAGE_SAVE, NULL},
    {"Out data to PC", PAGE_UART_TX, NULL}
};

#define APP_FSM_ITEM_COUNT(a) (sizeof(a) / sizeof((a)[0]))
static const APP_FSM_MENU_PAGE_t s_app_fsm_pages[] = {
    [PAGE_MAIN] = {"Main", PAGE_NONE, s_app_fsm_main_items, APP_FSM_ITEM_COUNT(s_app_fsm_main_items)}
};

void app_fsm_main(void){
    switch (s_app_fsm_page){
    case :
        /* code */
        break;
    
    case :
        break;

    case :
        break;

    case :
        break;
    }
}
