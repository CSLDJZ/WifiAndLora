#include "int_timer.h"

#include "driver/gptimer.h"
#include "esp_attr.h"

#define INT_TIMER_PERIOD_US (1000)

static gptimer_handle_t s_timer = NULL;
static INT_TIMER_TICK_CALLBACK_t s_tick_callback = NULL;

static bool IRAM_ATTR int_timer_on_alarm(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *){
    s_tick_callback();
    return false;
}

void int_timer_set_tick_callback(INT_TIMER_TICK_CALLBACK_t tick_callback){
    s_tick_callback = tick_callback;
}

void int_timer_init(void){
    gptimer_config_t timer_config = {};
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000 * 1000;

    gptimer_new_timer(&timer_config, &s_timer);

    gptimer_event_callbacks_t callbacks = {};
    callbacks.on_alarm = int_timer_on_alarm;
    gptimer_register_event_callbacks(s_timer, &callbacks, NULL);

    gptimer_alarm_config_t alarm_config = {};
    alarm_config.alarm_count = INT_TIMER_PERIOD_US;
    alarm_config.flags.auto_reload_on_alarm = true;
    gptimer_set_alarm_action(s_timer, &alarm_config);

    gptimer_enable(s_timer);
    gptimer_start(s_timer);
}

