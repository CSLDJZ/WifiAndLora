#include <stddef.h>
#include <stdint.h>

#include "esp_attr.h"
#include "freertos/FreeRTOS.h"

#include "init.h"
#include "int\int_key.h"
#include "int\int_timer.h"

#include "app\app_fsm.h"

// 主循环任务结构体
typedef struct {
    uint16_t now_tick;  // 当前计时
    uint16_t max_tick;  // 调度周期
    uint8_t is_can_run; // 是否可运行
    void (*funk)(void); // 任务函数
} MAIN_WORK_t;
// 主循环任务注册表，now_tick / max_tick 的单位为 ms
static MAIN_WORK_t s_main_work_list[] = {
    {0, 10, 0, int_key_main},
    {0, 10, 0, app_fsm_main},
};
#define MAIN_WORK_LIST_COUNT(a) (sizeof(a) / sizeof((a)[0]))

// 中断和主循环共享任务标志，读写都需进入临界区
static portMUX_TYPE s_main_work_mux = portMUX_INITIALIZER_UNLOCKED;

/**
 * @brief 1ms 定时器中断：累加各任务计时，到期后置位可运行标志
 */
static void IRAM_ATTR main_tick(void){
    portENTER_CRITICAL_ISR(&s_main_work_mux);
    for (size_t i = 0; i < MAIN_WORK_LIST_COUNT(s_main_work_list); i++){
        if (++s_main_work_list[i].now_tick >= s_main_work_list[i].max_tick){
            s_main_work_list[i].now_tick = 0;
            s_main_work_list[i].is_can_run = 1;
        }
    }
    portEXIT_CRITICAL_ISR(&s_main_work_mux);
}

extern "C" void app_main(void){
    int_timer_set_tick_callback(main_tick);
    init_all();

    while (1){
        for (uint8_t i = 0; i < MAIN_WORK_LIST_COUNT(s_main_work_list); i++){
            bool is_can_run = false;
            portENTER_CRITICAL(&s_main_work_mux);
            if (s_main_work_list[i].is_can_run){
                s_main_work_list[i].is_can_run = 0;
                is_can_run = true;
            }
            portEXIT_CRITICAL(&s_main_work_mux);

            if (is_can_run){
                s_main_work_list[i].funk();
            }
        }
    }
}

