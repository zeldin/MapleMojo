
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

int maple_scan_port(uint8_t port, struct maple_dev_info *info[6]);

int maple_scan_all_ports(struct maple_dev_info *info[4][6]);

void maple_scan_port_free(struct maple_dev_info *info[6]);

void maple_scan_all_ports_free(struct maple_dev_info *info[4][6]);

int maple_get_dev_info(uint8_t address, struct maple_dev_info **info);

int maple_get_dev_info_extended(uint8_t address, struct maple_dev_info **info);


