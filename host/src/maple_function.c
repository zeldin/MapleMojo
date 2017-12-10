#include "maple_function.h"
#include "maple_command.h"
#include "maple_error_codes.h"

#include <string.h>
#include <stdlib.h>

int maple_function_check(const struct maple_dev_info *info, int func, uint32_t *func_data)
{
  if (info == NULL || func < 0 || func > 31)
    return MAPLE_ERROR_BAD_ARGUMENT;
  int f, funcnum = 0;
  for (f=31; f>=0; --f)
    if (info->func_codes & (1UL<<f)) {
      uint32_t f_data  = 0;
      if (funcnum < 3) {
	f_data = info->function_data[funcnum];
      }
      if (f == func) {
	if (func_data != NULL)
	  *func_data = f_data;
	return MAPLE_OK;
      }
      funcnum++;
    }

  return MAPLE_ERROR_FUNCTION_CODE_UNSUPPORTED;
}
