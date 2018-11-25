#ifndef MAPLE_COMMAND_H_
#define MAPLE_COMMAND_H_

#include <stdint.h>

struct maple_dev_info {
  uint32_t func_codes;
  uint32_t function_data[3];
  uint8_t area_code;
  uint8_t connector_direction;
  char product_name[30];
  char product_license[60];
  uint16_t standby_power;
  uint16_t max_power;
  char version[];
};

struct maple_mem_info {
  uint16_t last_block;
  uint16_t dunno1;
  uint16_t root_loc;
  uint16_t fat_loc;
  uint16_t fat_size;
  uint16_t dir_loc;
  uint16_t dir_size;
  uint16_t icon_shape;
  uint16_t num_user_blocks;
  uint16_t dunno2;
  uint16_t dunno3;
  uint16_t dunno4;
};

int maple_probe(uint8_t address);

int maple_scan_port(uint8_t port, struct maple_dev_info *info[6]);

int maple_scan_all_ports(struct maple_dev_info *info[4][6]);

void maple_scan_port_free(struct maple_dev_info *info[6]);

void maple_scan_all_ports_free(struct maple_dev_info *info[4][6]);

int maple_get_dev_info(uint8_t address, struct maple_dev_info **info);

int maple_get_dev_info_extended(uint8_t address, struct maple_dev_info **info);

int maple_get_mem_info(uint8_t address, uint32_t func, uint8_t pt, struct maple_mem_info *info);

int maple_block_read(uint8_t address, uint32_t func, uint8_t pt, uint8_t phase, uint16_t block, uint8_t *buf, uint16_t size);

#define maple_address(port, unit) (((port)<<6)|((unit)>0?(((1<<(unit))>>1)&0x1f):0x20))

#endif /* MAPLE_COMMAND_H_ */
