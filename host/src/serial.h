
#include <stdint.h>
#include <unistd.h>

extern int maple_serial_open(const char *portname);
extern void maple_serial_close(void);
extern ssize_t maple_serial_write(const uint8_t *data, size_t len);
extern ssize_t maple_serial_read(uint8_t *data, size_t len);
