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

int maple_scan_port(uint8_t port, struct maple_dev_info *info[6])
{
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
  struct maple_header rheader, header = {
    0, address&0xc0, address, MAPLE_COMMAND_REQUEST_DEVINFO
  };
  uint8_t rbuf[1024];
  int r = maple_transaction_0(&header, &rheader, rbuf, sizeof(rbuf));
  if (r)
    *info = NULL;
  else
    r = make_dev_info(&rheader, rbuf, info);
  return r;
}

int maple_get_dev_info_extended(uint8_t address, struct maple_dev_info **info)
{
  struct maple_header rheader, header = {
    0, address&0xc0, address, MAPLE_COMMAND_REQUEST_DEVINFOX
  };
  uint8_t rbuf[1024];
  int r = maple_transaction_0(&header, &rheader, rbuf, sizeof(rbuf));
  if (r)
    *info = NULL;
  else
    r = make_dev_info(&rheader, rbuf, info);
  return r;
}


