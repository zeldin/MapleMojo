#include "serial.h"
#include "maple_command.h"
#include "maple_function.h"

#include <stdio.h>
#include <stdlib.h>

static const char *function_name(int func)
{
  switch(func) {
  case MAPLE_FUNCTION_CONTROLLER : return "controller";
  case MAPLE_FUNCTION_MEMORY_CARD: return "memory card";
  case MAPLE_FUNCTION_LCD_DISPLAY: return "LCD display";
  case MAPLE_FUNCTION_CLOCK      : return "clock";
  case MAPLE_FUNCTION_MICROPHONE : return "microphone";
  case MAPLE_FUNCTION_AR_GUN     : return "AR-gun";
  case MAPLE_FUNCTION_KEYBOARD   : return "keyboard";
  case MAPLE_FUNCTION_LIGHT_GUN  : return "light gun";
  case MAPLE_FUNCTION_PURU_PURU  : return "puru puru";
  case MAPLE_FUNCTION_MOUSE      : return "mouse";
  }
  return "?";
}

static const char *area_code_name(uint8_t code, char *p)
{
  const char *ret = p;
  if (code == 0xff)
    return "all";
  else if(code == 0)
    return "none";
  if (code&(1<<0)) *p++ = 'U';
  if (code&(1<<1)) *p++ = 'J';
  if (code&(1<<2)) *p++ = 'A';
  if (code&(1<<3)) *p++ = 'E';
  if (code&(1<<4)) *p++ = '1';
  if (code&(1<<5)) *p++ = '2';
  if (code&(1<<6)) *p++ = '3';
  if (code&(1<<7)) *p++ = '4';
  *p = 0;
  return ret;
}

static const char *connector_direction_name(uint8_t dir)
{
  switch(dir) {
  case 0: return "top";
  case 1: return "bottom";
  case 2: return "left";
  case 3: return "right";
  }
  return "?";
}

static const char *keyboard_layout_name(uint8_t layout)
{
  switch(layout) {
  case 1: return "Japanese";
  case 2: return "US";
  case 3: return "European";
  }
  return "?";
}

static void print_controller_func_data(uint32_t data)
{
  if (data & 0x0f0f)
    printf("    Buttons :%s%s%s%s%s%s%s%s\n",
	   ((data&(1<<2))? " A":""),
	   ((data&(1<<1))? " B":""),
	   ((data&(1<<0))? " C":""),
	   ((data&(1<<11))? " D":""),
	   ((data&(1<<10))? " X":""),
	   ((data&(1<<9))? " Y":""),
	   ((data&(1<<8))? " Z":""),
	   ((data&(1<<3))? " START":""));
  if (data & 0x00f0)
    printf("    D-pad 1 :%s%s%s%s\n",
	   ((data&(1<<4))? " up":""),
	   ((data&(1<<5))? " down":""),
	   ((data&(1<<6))? " left":""),
	   ((data&(1<<7))? " right":""));
  if (data & 0xf000U)
    printf("    D-pad 2 :%s%s%s%s\n",
	   ((data&(1<<12))? " up":""),
	   ((data&(1<<13))? " down":""),
	   ((data&(1<<14))? " left":""),
	   ((data&(1<<15))? " right":""));
  if (data & 0x30000UL)
    printf("    Triggers:%s%s\n",
	   ((data&(1<<17))? " L":""),
	   ((data&(1<<16))? " R":""));
  if (data & 0xc0000UL)
    printf("    Analog 1:%s%s\n",
	   ((data&(1<<18))? " horizontal":""),
	   ((data&(1<<19))? " vertical":""));
  if (data & 0x300000UL)
    printf("    Analog 2:%s%s\n",
	   ((data&(1<<20))? " horizontal":""),
	   ((data&(1<<21))? " vertical":""));
}

static void print_memory_card_func_data(uint32_t data)
{
  printf("    #partitions = %u\n", (unsigned)(((data>>24)&0xff)+1));
  printf("    block size  = %u\n", (unsigned)((((data>>16)&0xff)+1)<<5));
  printf("    writecnt    = %u\n", (unsigned)((data>>12)&0xf));
  printf("    readcnt     = %u\n", (unsigned)((data>>8)&0xf));
  printf("    removable   = %s\n", (((data>>7)&1)? "yes" : "no"));
}

static void print_lcd_display_func_data(uint32_t data)
{
  printf("    #partitions = %u\n", (unsigned)(((data>>24)&0xff)+1));
  printf("    block size  = %u\n", (unsigned)((((data>>16)&0xff)+1)<<5));
  printf("    writecnt    = %u\n", (unsigned)((data>>12)&0xf));
  printf("    orientation = %s\n", (((data>>7)&1)? "vertical" : "horizontal"));
  printf("    polarity    = %s\n", (((data>>6)&1)? "while-on-black" : "black-on-white"));
}

static void print_keyboard_func_data(uint32_t data)
{
  printf("    layout = %u (%s)\n", (unsigned)((data>>24)&0xff),
	 keyboard_layout_name((data>>24)&0xff));
}

static void print_mouse_func_data(uint32_t data)
{
  if (data & 0x0f00)
    printf("    Buttons:%s%s%s%s\n",
	   ((data&(1<<10))? " A":""),
	   ((data&(1<<9))? " B":""),
	   ((data&(1<<8))? " C":""),
	   ((data&(1<<11))? " START":""));
  if (data & 0x00ff)
    printf("    Axis   :%s%s%s%s%s%s%s%s\n",
	   ((data&(1<<0))? " 1":""),
	   ((data&(1<<1))? " 2":""),
	   ((data&(1<<2))? " 3":""),
	   ((data&(1<<3))? " 4":""),
	   ((data&(1<<4))? " 5":""),
	   ((data&(1<<5))? " 6":""),
	   ((data&(1<<6))? " 7":""),
	   ((data&(1<<7))? " 8":""));
}

static void print_function_data(int func, uint32_t data)
{
  switch(func) {
  case MAPLE_FUNCTION_CONTROLLER : print_controller_func_data(data); break;
  case MAPLE_FUNCTION_MEMORY_CARD: print_memory_card_func_data(data); break;
  case MAPLE_FUNCTION_LCD_DISPLAY: print_lcd_display_func_data(data); break;
  case MAPLE_FUNCTION_KEYBOARD   : print_keyboard_func_data(data); break;
  case MAPLE_FUNCTION_MOUSE      : print_mouse_func_data(data); break;
  }
}

static void print_devinfo(const struct maple_dev_info *info)
{
  char buf[16];
  int func, funcnum = 0;
  printf(" func_codes = %08lx\n", (unsigned long)info->func_codes);
  for (func=31; func>=0; --func)
    if (info->func_codes & (1UL<<func)) {
      printf("  function %08lx (%s)\n", (unsigned long)(1UL<<func),
	     function_name(func));
      if (funcnum < 3) {
	printf("   function_data = %08lx\n",
	       (unsigned long)info->function_data[funcnum]);
	print_function_data(func, info->function_data[funcnum]);
      }
      funcnum++;
    }
  printf(" area_code = %d (%s)\n", (int)info->area_code,
	 area_code_name(info->area_code, buf));
  printf(" connector_direction = %d (%s)\n", (int)info->connector_direction,
	 connector_direction_name(info->connector_direction));
  printf(" product_name = %.30s\n", info->product_name);
  printf(" product_license = %.60s\n", info->product_license);
  if (info->version[0])
    printf(" version = %s\n", info->version);
  printf("\n");
}

int main()
{
  struct maple_dev_info *info[4][6];
  int i, j;

  maple_serial_open("/dev/ttyACM0");
  int r = maple_scan_all_ports(info);
  printf("r=%d\n", r);
  if (!r)
    for (i=0; i<4; i++)
      if (info[i][0]) {
	for (j=0; j<6; j++)
	  if (info[i][j]) {
	    printf("Unit %c%d\n", 'A'+i, j);
	    print_devinfo(info[i][j]);
	  }
      } else
	printf("Port %c - no connection\n", 'A'+i);
  maple_scan_all_ports_free(info);
  maple_serial_close();
  return 0;
}
