#include "int_key.h"

INT_KEYS_t g_int_now_keys;

void int_key_main(void){
}

INT_KEYS_t int_key_get_now_key(void){
    return g_int_now_keys;
};
