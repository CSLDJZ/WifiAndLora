#ifndef INIT_H
#define INIT_H

#include <stdbool.h>

// 初始化函数注册表，按数组顺序依次执行。
typedef bool (*InitFunc)(void);
typedef struct {
    const char *help;
    InitFunc func;
} Init;

#ifdef __cplusplus
extern "C" {
#endif

bool init_all(void);

#ifdef __cplusplus
}
#endif

#endif // INIT_H
