#ifndef INT_KEY_H
#define INT_KEY_H

typedef enum {
    KEY_NO = 0, // 表无输入
    KEY_UP,     //
    KEY_DOWN,   //
    KEY_LEFT,   // 又兼返回
    KEY_RIGHT   // 又兼前进
} INT_KEYS_t;

INT_KEYS_t int_key_get_now_key(void);

#endif
