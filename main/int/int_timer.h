#ifndef INT_TIMER_H
#define INT_TIMER_H

typedef void (*INT_TIMER_TICK_CALLBACK_t)(void);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 设置 1ms 中断回调，必须在 int_timer_init 之前调用。
 *
 * @param tick_callback 由定时器中断调用的回调函数，需放在 IRAM 中执行。
 */
void int_timer_set_tick_callback(INT_TIMER_TICK_CALLBACK_t tick_callback);

/**
 * @brief 启动 1ms 周期 GPTimer，每个周期调用已设置的 tick 回调。
 */
void int_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif // INT_TIMER_H
