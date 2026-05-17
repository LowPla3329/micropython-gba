#include <stdio.h>
#include <stddef.h>
#include <gba_console.h>

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        iprintf("%c", str[i]);
    }
}

void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (str[i] == '\n') {
            iprintf("\r");
        }
        iprintf("%c", str[i]);
    }
}