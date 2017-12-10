
#include <stdint.h>

#define MAPLE_FUNCTION_CONTROLLER  0
#define MAPLE_FUNCTION_MEMORY_CARD 1
#define MAPLE_FUNCTION_LCD_DISPLAY 2
#define MAPLE_FUNCTION_CLOCK       3
#define MAPLE_FUNCTION_MICROPHONE  4
#define MAPLE_FUNCTION_AR_GUN      5
#define MAPLE_FUNCTION_KEYBOARD    6
#define MAPLE_FUNCTION_LIGHT_GUN   7
#define MAPLE_FUNCTION_PURU_PURU   8
#define MAPLE_FUNCTION_MOUSE       9

struct maple_dev_info;

int maple_function_check(const struct maple_dev_info *info, int func, uint32_t *func_data);

