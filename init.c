#include "init.h"
#include <stddef.h>

// 初始化函数注册表，按数组顺序依次执行。
Init init_table[] = {
    /*code*/
};
#define INIT_COUNT (sizeof(init_table) / sizeof(init_table[0]))

bool init_all(void){
    for (size_t i = 0; i != INIT_COUNT; i++){
        if (!init_table[i].func()) return false;
    }
    return true;
}
