#include <stdint.h>

// Options to control how MicroPython is built

// Use the minimal starting configuration (disables all optional features).
#define MICROPY_CONFIG_ROM_LEVEL                (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)
//#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_CORE_FEATURES)

// Compiler configuration
#define MICROPY_ENABLE_COMPILER                 (1)

// Python internal features
#define MICROPY_ERROR_REPORTING                 (MICROPY_ERROR_REPORTING_NONE)

// Fine control over Python builtins, classes, modules, etc.
#define MICROPY_PY_SYS                          (1)
#define MICROPY_PY_SYS_PLATFORM "gba"
#define MICROPY_HW_BOARD_NAME "Nintendo Game Boy Advance"
#define MICROPY_HW_MCU_NAME "ARM7TDMI"

// Type definitions for the specific machine

#define MICROPY_PY_MICROPYTHON (0)
#define MICROPY_PY_MATH (1)

#define MICROPY_FLOAT_IMPL (MICROPY_FLOAT_IMPL_FLOAT)

typedef long mp_off_t;

// Need to provide a declaration/definition of alloca()
#include <alloca.h>
