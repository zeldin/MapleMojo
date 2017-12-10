#include "serial.h"
#include "maple_transaction.h"
#include "maple_error_codes.h"

#include <string.h>

int maple_transaction_sg(const struct maple_header *header,
			 const struct maple_sg_block *block, unsigned nblock,
			 struct maple_header *rheader,
			 const struct maple_sg_rblock *rblock, unsigned nrblock)
{
  unsigned i;
  size_t payload_size = 0, rbufsize = 0;
  uint8_t r, csum = 0;
  uint8_t buf[1025], *bp, *rbuf;
  for(i=0; i<4; i++)
    csum ^= (buf[i] = ((const uint8_t *)header)[i]);
  for(bp = &buf[4], i=0; i<nblock; i++) {
    const uint8_t *data = block[i].data;
    size_t len = block[i].len;
    payload_size += len;
    if (payload_size > 1020)
      return MAPLE_ERROR_PAYLOAD_TOO_LARGE;
    while(len--)
      csum ^= (*bp++ = *data++);
  }
  if (payload_size != header->payload_words*4)
    return MAPLE_ERROR_PAYLOAD_SIZE_MISMATCH;
  *bp = csum;
  if (maple_serial_write(buf, payload_size+5) != payload_size+5)
    return MAPLE_ERROR_INTERNAL_WRITE_FAILURE;
  if (maple_serial_read(&r, 1) != 1)
    return MAPLE_ERROR_INTERNAL_READ_STATUS_FAILURE;
  if (r != 0)
    return MAPLE_STATUS_TO_ERROR((int)r);
  if (maple_serial_read(buf, 4) != 4)
    return MAPLE_ERROR_INTERNAL_READ_HEADER_FAILURE;
  csum = 0;
  payload_size = buf[0]*4;
  for(i=0; i<4; i++)
    csum ^= (((uint8_t *)rheader)[i] = buf[i]);
  bp = buf+4;
  while (nrblock--) {
    rbuf = rblock->data;
    if ((rbufsize = (*rblock++).len))
      break;
  }
  if (payload_size) {
    size_t i = 0;
    if (maple_serial_read(bp, payload_size) != payload_size)
      return MAPLE_ERROR_INTERNAL_READ_PAYLOAD_FAILURE;
    for (i = 0; i < payload_size; i++) {
      uint8_t b = bp[i^3];
      csum ^= b;
      if (rbufsize) {
	*rbuf++ = b;
	if (!--rbufsize)
	  while (nrblock--) {
	    rbuf = rblock->data;
	    if ((rbufsize = (*rblock++).len))
	      break;
	  }
      }
    }
    bp += payload_size;
  }
  if (maple_serial_read(bp, 1) != 1)
    return MAPLE_ERROR_INTERNAL_READ_CHECKSUM_FAILURE;
  if (*bp != csum)
    return MAPLE_ERROR_CHECKSUM_MISMATCH;
  if (buf[3]&0x80)
    return -128|(int8_t)buf[3];
  else
    return MAPLE_OK;
}

int maple_transaction_0(const struct maple_header *header,
			struct maple_header *rheader,
			uint8_t *rbuf, size_t rbufsize)
{
  struct maple_sg_rblock rblocks[] = {
    { rbuf, rbufsize }
  };
  return maple_transaction_sg(header, NULL, 0, rheader, rblocks, 1);
}

int maple_transaction_1(const struct maple_header *header,
			const uint8_t *payload, size_t payload_size,
			struct maple_header *rheader,
			uint8_t *rbuf1, size_t rbuf1size,
			uint8_t *rbuf2, size_t rbuf2size)
{
  struct maple_sg_block blocks[] = {
    { payload, payload_size }
  };
  struct maple_sg_rblock rblocks[] = {
    { rbuf1, rbuf1size },
    { rbuf2, rbuf2size }
  };
  return maple_transaction_sg(header, blocks, 1, rheader, rblocks, 2);
}

int maple_transaction_2(const struct maple_header *header,
			const uint8_t *payload1, size_t payload1_size,
			const uint8_t *payload2, size_t payload2_size,
			struct maple_header *rheader,
			uint8_t *rbuf, size_t rbufsize)
{
  struct maple_sg_block blocks[] = {
    { payload1, payload1_size },
    { payload2, payload2_size }
  };
  struct maple_sg_rblock rblocks[] = {
    { rbuf, rbufsize }
  };
  return maple_transaction_sg(header, blocks, 2, rheader, rblocks, 1);
}
