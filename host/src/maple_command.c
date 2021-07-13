#include "maple_transaction.h"
#include "maple_command.h"
#include "maple_command_codes.h"
#include "maple_error_codes.h"
#include "maple_endian.h"

#include <string.h>
#include <stdlib.h>

static int make_dev_info(const struct maple_header *header,
			 const uint8_t *buf, struct maple_dev_info **info)
{
  int verlen;
  struct maple_dev_info *ibuf;
  if (header->command == MAPLE_RESPONSE_DEVINFO)
    verlen = 0;
  else if(header->command == MAPLE_RESPONSE_DEVINFOX)
    verlen = header->payload_words*4-sizeof(struct maple_dev_info);
  else {
    *info = NULL;
    return MAPLE_ERROR_UNEXPECTED_RESPONSE;
  }
  if (header->payload_words*4 < sizeof(struct maple_dev_info)) {
    *info = NULL;
    return MAPLE_ERROR_INVALID_RESPONSE_LENGTH;
  }
  *info = ibuf = malloc(sizeof(struct maple_dev_info)+verlen+1);
  if (ibuf == NULL)
    return MAPLE_ERROR_OUT_OF_MEMORY;
  memcpy(ibuf, buf, sizeof(*ibuf));
  if (verlen)
    memcpy(ibuf->version, buf+sizeof(*ibuf), verlen);
  ibuf->version[verlen] = 0;
#if MAPLE_HOST_BIG_ENDIAN
  ibuf->standby_power = maple_bswap16(ibuf->standby_power);
  ibuf->max_power = maple_bswap16(ibuf->max_power);
#else
  ibuf->func_codes = maple_bswap32(ibuf->func_codes);
  ibuf->function_data[0] = maple_bswap32(ibuf->function_data[0]);
  ibuf->function_data[1] = maple_bswap32(ibuf->function_data[1]);
  ibuf->function_data[2] = maple_bswap32(ibuf->function_data[2]);
#endif
  return MAPLE_OK;
}

int maple_probe(uint8_t address)
{
  struct maple_header rheader, header = {
    0, address&0xc0, (address&0xc0)|0x20, MAPLE_COMMAND_REQUEST_DEVINFO
  };
  int r = maple_transaction_0(&header, &rheader, NULL, 0);
  if (!r) {
    if ((rheader.src_address & address & 0x3f) != (address & 0x3f))
      return MAPLE_ERROR_SUBUNIT_DOES_NOT_EXIST;
  }
  return r;
}

int maple_scan_port(uint8_t port, struct maple_dev_info *info[6])
{
  if (port > 3)
    return MAPLE_ERROR_BAD_ARGUMENT;
  struct maple_header rheader, header = {
    0, port<<6, (port<<6)|0x20, MAPLE_COMMAND_REQUEST_DEVINFO
  };
  uint8_t rbuf[1024];
  int i;
  int r = maple_transaction_0(&header, &rheader, rbuf, sizeof(rbuf));
  if (r) {
    for (i=0; i<6; i++)
      info[i] = NULL;
  }
  else {
    uint8_t subunits;
    r = make_dev_info(&rheader, rbuf, &info[0]);
    subunits = (r? 0 : (rheader.src_address&0x1f))<<1;
    for (i=1; i<6 && !r; i++)
      if (subunits & (1<<i)) {
	header.dest_address = (header.dest_address&0xc0)|((1<<i)>>1);
	r = maple_transaction_0(&header, &rheader, rbuf, sizeof(rbuf));
	if (!r)
	  r = make_dev_info(&rheader, rbuf, &info[i]);
	if (r)
	  info[i] = NULL;
      } else
	info[i] = NULL;
    while (i<6)
      info[i++] = NULL;
  }
  return r;
}

int maple_scan_all_ports(struct maple_dev_info *info[4][6])
{
  int i, j, r = 0;
  for (i=0; i<4 && !r; i++) {
    r = maple_scan_port(i, info[i]);
    if (r == MAPLE_ERROR_NO_START_PATTERN && info[i][0] == NULL)
      r = MAPLE_OK;
  }
  while (i<4) {
    for (j=0; j<6; j++)
      info[i][j] = NULL;
    i++;
  }
  return r;
}

void maple_scan_port_free(struct maple_dev_info *info[6])
{
  int i;
  for (i=0; i<6; i++)
    if (info[i])
      free(info[i]);
}

void maple_scan_all_ports_free(struct maple_dev_info *info[4][6])
{
  int i;
  for (i=0; i<4; i++)
    maple_scan_port_free(info[i]);
}

int maple_get_dev_info(uint8_t address, struct maple_dev_info **info)
{
  int r = (address & 0x20)? MAPLE_OK : maple_probe(address);
  struct maple_header rheader, header = {
    0, address&0xc0, address, MAPLE_COMMAND_REQUEST_DEVINFO
  };
  uint8_t rbuf[1024];
  if (!r)
    r = maple_transaction_0(&header, &rheader, rbuf, sizeof(rbuf));
  if (r)
    *info = NULL;
  else
    r = make_dev_info(&rheader, rbuf, info);
  return r;
}

int maple_get_dev_info_extended(uint8_t address, struct maple_dev_info **info)
{
  int r = (address & 0x20)? MAPLE_OK : maple_probe(address);
  struct maple_header rheader, header = {
    0, address&0xc0, address, MAPLE_COMMAND_REQUEST_DEVINFOX
  };
  uint8_t rbuf[1024];
  if (!r)
    r = maple_transaction_0(&header, &rheader, rbuf, sizeof(rbuf));
  if (r)
    *info = NULL;
  else
    r = make_dev_info(&rheader, rbuf, info);
  return r;
}

int maple_get_mem_info(uint8_t address, uint32_t func, uint8_t pt, struct maple_mem_info *info)
{
  struct maple_header rheader, header = {
    2, address&0xc0, address, MAPLE_COMMAND_GET_MEMINFO
  };
  union {
    uint32_t u32[2];
    uint8_t u8[8];
  } payload = {
    .u32 = {
#if MAPLE_HOST_BIG_ENDIAN
	maple_bswap32(func),
	maple_bswap32(pt<<24)
#else
	func,
	pt<<24
#endif
      }
  };
  uint32_t funcecho;
  int r = maple_transaction_1(&header, payload.u8, sizeof(payload),
			      &rheader, (uint8_t *)&funcecho, sizeof(funcecho),
			      (uint8_t *)info, (info? sizeof(struct maple_mem_info) : 0));
  if (!r) {
    if (rheader.command != MAPLE_RESPONSE_DATA)
      r = MAPLE_ERROR_UNEXPECTED_RESPONSE;
    else if (rheader.payload_words*4 < sizeof(funcecho)+sizeof(struct maple_mem_info))
      r = MAPLE_ERROR_INVALID_RESPONSE_LENGTH;
    else if(funcecho != maple_bswap32(payload.u32[0]))
      r = MAPLE_ERROR_UNEXPECTED_RESPONSE;
  }
#if MAPLE_HOST_BIG_ENDIAN
  if (info && !r) {
    info->last_block = maple_bswap32(info->last_block);
    info->dunno1 = maple_bswap16(info->dunno1);
    info->root_loc = maple_bswap16(info->root_loc);
    info->fat_loc = maple_bswap16(info->fat_loc);
    info->fat_size = maple_bswap16(info->fat_size);
    info->dir_loc = maple_bswap16(info->dir_loc);
    info->dir_size = maple_bswap16(info->dir_size);
    info->icon_shape = maple_bswap16(info->icon_shape);
    info->num_user_blocks = maple_bswap16(info->num_user_blocks);
    info->dunno2 = maple_bswap16(info->dunno2);
    info->dunno3 = maple_bswap16(info->dunno3);
    info->dunno4 = maple_bswap16(info->dunno4);
  }
#endif
  return r;
}

int maple_block_read(uint8_t address, uint32_t func, uint8_t pt, uint8_t phase, uint16_t block, uint8_t *buf, uint16_t size)
{
  struct maple_header rheader, header = {
    2, address&0xc0, address, MAPLE_COMMAND_BLOCK_READ
  };
  union {
    uint32_t u32[2];
    uint8_t u8[8];
  } payload = {
    .u32 = {
#if MAPLE_HOST_BIG_ENDIAN
	maple_bswap32(func),
	maple_bswap32((pt<<24)|(phase<<16)|block)
#else
	func,
	(pt<<24)|(phase<<16)|block
#endif
      }
  };
  uint32_t echo[2];
  int r = maple_transaction_1(&header, payload.u8, sizeof(payload),
			      &rheader, (uint8_t *)echo, sizeof(echo),
			      buf, size);
  if (!r) {
    if (rheader.command != MAPLE_RESPONSE_DATA)
      r = MAPLE_ERROR_UNEXPECTED_RESPONSE;
    else if (rheader.payload_words*4 != sizeof(echo)+size)
      r = MAPLE_ERROR_INVALID_RESPONSE_LENGTH;
    else if (echo[0] != maple_bswap32(payload.u32[0]) ||
	     echo[1] != maple_bswap32(payload.u32[1]))
      r = MAPLE_ERROR_UNEXPECTED_RESPONSE;
  }
  return r;
}
