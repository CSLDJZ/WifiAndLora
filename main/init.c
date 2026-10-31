#include "init.h"
#include <stddef.h>

#include "int\int_timer.h"

// 注册表项要求返回 bool，定时器初始化不返回错误码
static bool int_timer_init_entry(void){
    int_timer_init();
    return true;
}

// 初始化函数注册表，按数组顺序依次执行。
Init init_table[] = {
    {"1ms GPTimer", int_timer_init_entry},
};
#define INIT_COUNT (sizeof(init_table) / sizeof(init_table[0]))

bool init_all(void){
    for (size_t i = 0; i != INIT_COUNT; i++){
        if (!init_table[i].func()) return false;
    }
    return true;
}
