#include <stdint.h>

#include "int\int_key.h"

#include "app\app_fsm.h"

// 主循环任务结构体
typedef struct {
    uint16_t now_tick;  // 当前计时
    uint16_t max_tick;  // 调度周期
    uint8_t is_can_run; // 是否可运行
    void (*funk)(void); // 任务函数
} MAIN_WORK_t;
// 主循环任务注册表
static MAIN_WORK_t s_main_work_list[] = {
    {0, 10, 0, int_key_main},
    {0, 10, 0, app_fsm_main},
};
#define MAIN_WORK_LIST_COUNT(a) (sizeof(a) / sizeof((a)[0]))

extern "C" void app_main(void){
    while (1){
        for (uint8_t i = 0; i < MAIN_WORK_LIST_COUNT(s_main_work_list); i++){
            if (s_main_work_list[i].is_can_run){
                s_main_work_list[i].is_can_run = 0;
                s_main_work_list[i].funk();
            }
        }
    }
}
