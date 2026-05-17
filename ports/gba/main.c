/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2021 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <string.h>

#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_video.h>

#include "py/compile.h"
#include "py/runtime.h"


static const char *demo_single_input =
    "print('hello world!', list(x + 1 for x in range(10)), end='eol\\n')";
/*
static const char *demo_file_input =
    "import micropython\n"
    "\n"
    "print(dir(micropython))\n"
    "\n"
    "for i in range(10):\n"
    "    print('iter {:08}'.format(i))";
*/

static const char *demo_file_input =
    "import sys\n"
    "print(sys.version,sys.implementation)\n"
    "print('hello world! i am a python running on gba')\n"    
    "print('Running: list(x + 1 for x in range(10))')\n"
    "print(list(x + 1 for x in range(10)), end='eol\\n')";

static void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // Compile, parse and execute the given string.
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // Uncaught exception: print it out.
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
}

// Called if an exception is raised outside all C exception-catching handlers.
void nlr_jump_fail(void *val) {
    for (;;) {
    }
}

#ifndef NDEBUG
// Used when debugging is enabled.
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    for (;;) {
    }
}
#endif


void clearScreen(){
    printf("\x1b[2J");
}

#define MAX_BUF 128

static char buffer[MAX_BUF];
static int buf_len = 0;
static int cursor = 0;
static int char_sel = 0;


static const char charset[] =
" abcdefghijklmnopqrstuvwxyz"
"0123456789+-*/=()'\"[]{}.,:_";

void run_micro_python(const char *src) {
    do_str(src, MP_PARSE_FILE_INPUT);
}


//TODO: separate keyboard to swkbd.c

static int up_repeat = 0;
static int down_repeat = 0;

#define REPEAT_DELAY 25
#define REPEAT_RATE   8

static bool repeat_key(u16 held, u16 down, u16 key, int *counter) {

    if (down & key) {
        *counter = REPEAT_DELAY;
        return true;
    }

    if (held & key) {

        if (--(*counter) <= 0) {
            *counter = REPEAT_RATE;
            return true;
        }
    }

    else {
        *counter = 0;
    }

    return false;
}

static void repl_input_step(void) {

    scanKeys();

    u16 k_down = keysDown();
    u16 k_held = keysHeld();

    if (k_down & KEY_LEFT) {
        if (cursor > 0) {
            cursor--;
        }
    }

    if (k_down & KEY_RIGHT) {
        if (cursor < buf_len) {
            cursor++;
        }
    }

    if (repeat_key(k_held, k_down, KEY_UP, &up_repeat)) {

        char_sel++;

        if (char_sel >= (int)sizeof(charset) - 1) {
            char_sel = 0;
        }
    }

    if (repeat_key(k_held, k_down, KEY_DOWN, &down_repeat)) {

        int len = (int)sizeof(charset) - 1;

        if (--char_sel < 0) {
            char_sel = len - 1;
        }
    }

    if (k_down & KEY_B) {

        if (cursor > 0 && buf_len > 0) {

            for (int i = cursor - 1; i < buf_len - 1; i++) {
                buffer[i] = buffer[i + 1];
            }

            buf_len--;
            cursor--;

            buffer[buf_len] = '\0';
        }
    }

    if (k_down & KEY_A) {

        char c = charset[char_sel];

        // R as shift key
        if ((keysHeld() & KEY_R) && c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }

        if (buf_len < MAX_BUF - 1) {

            for (int i = buf_len; i > cursor; i--) {
                buffer[i] = buffer[i - 1];
            }

            buffer[cursor] = c;

            cursor++;
            buf_len++;

            buffer[buf_len] = '\0';
        }
    }

    // START
    if (k_down & KEY_START) {

        buffer[buf_len++] = '\n';
        buffer[buf_len] = 0;

        clearScreen();


        run_micro_python(buffer);

        printf("\nPress A button to continue...");

        while (1) {

            scanKeys();

            u16 k1 = keysDown();

            if (k1 & KEY_A) {
                break;
            }

            VBlankIntrWait();
        }

        buf_len = 0;
        cursor = 0;

        memset(buffer, 0, sizeof(buffer));
    }
    if(k_down & KEY_SELECT){
        mp_deinit();
        VBlankIntrWait();
        mp_init();
    }
}
static void draw_ui(void) {
    clearScreen(); // clear screen (simple but slow)

    printf("%s\n", buffer);
    printf("CUR: %d CHAR: %c\n", cursor, charset[char_sel]);
}

void splash(){
        void waitForAbutton(){
        while(1){
            scanKeys();
            if(keysDown() & KEY_A){
                break;
            }
            VBlankIntrWait();
        }
    }
    iprintf(
        "Welcome to Python on GBA!\n"
        "by Low_Plankton_3329\n\n"
        "https://github.com/LowPla3329/"
        "micropython-gba\n\n"
        "Python version:\n"
    );
 
    mp_init();
    do_str("import sys\nprint(sys.version,sys.implementation)\n",
            MP_PARSE_FILE_INPUT);
    mp_deinit();
    iprintf("Press A button to continue...\n");
    waitForAbutton();
    clearScreen();

    iprintf(
        "[Controls]\n"
        "Left/Right: Move text cursor.\n"
        "Up/Down   : Select charactor.\n"
        "A button  : Insert charactor.\n"
        "B button  : Backspace key\n"
        "R button  : Shift key\n"
        "START     : Execute buffer!\n\n"
        "Press A button to continue..."
    );
    waitForAbutton();
    clearScreen();
}

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);
    consoleDemoInit();
    
    splash();

    mp_init();

    while (1) {
        VBlankIntrWait();
        repl_input_step();
        draw_ui();
    }

    return 0;
}