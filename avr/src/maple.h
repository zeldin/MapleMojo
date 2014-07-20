
/*
 * maple.h
 *
 *  Created on: Jul 17, 2014
 *      Author: Marcus Comstedt
 */

#ifndef MAPLE_H_
#define MAPLE_H_

#include <stdint.h>

#include "hardware.h"

#define MAPLE_STATUS_OK               0x00
#define MAPLE_STATUS_NO_START_PATTERN 0x02
#define MAPLE_STATUS_FIFO_OVERFLOW    0x03
#define MAPLE_STATUS_FIFO_UNDERFLOW   0x04
#define MAPLE_STATUS_REPLY_TOO_LONG   0x05
#define MAPLE_STATUS_TIMEOUT          0x06
#define MAPLE_STATUS_REPLY_TOO_SHORT  0x07
#define MAPLE_STATUS_REPLY_BAD_COUNT  0x08

#define MAPLE_MAX_PACKET_LENGTH 1025 /* 256 words + checksum */
#define MAPLE_BUFFER_SIZE 1039

#define MAPLE_TIMEOUT_MS 100

#define MAPLE_REG_VERSION      0
#define MAPLE_REG_SCRATCHPAD   1
#define MAPLE_REG_CLOCKDIV     2
#define MAPLE_REG_PORTSEL      3
#define MAPLE_REG_OUTCTRL      4
#define MAPLE_REG_INCTRL       5
#define MAPLE_REG_OUTFIFO_CNT  6
#define MAPLE_REG_OUTFIFO_FREE 7
#define MAPLE_REG_INFIFO_CNT   8
#define MAPLE_REG_INFIFO_FREE  9
#define MAPLE_REG_FIFO         10

#define MAPLE_WRITE_FLAG       0x80

#define MAPLE_OUTCTRL_FIFO_READY   0x80
#define MAPLE_OUTCTRL_FIFO_OVERF   0x40
#define MAPLE_OUTCTRL_FIFO_UNDERF  0x20
#define MAPLE_OUTCTRL_FIFO_RESET   0x10
#define MAPLE_OUTCTRL_OUTPUT_EN    0x04
#define MAPLE_OUTCTRL_MODE_END     0x02
#define MAPLE_OUTCTRL_MODE_START   0x01

#define MAPLE_INCTRL_FIFO_READY   0x80
#define MAPLE_INCTRL_FIFO_OVERF   0x40
#define MAPLE_INCTRL_FIFO_UNDERF  0x20
#define MAPLE_INCTRL_FIFO_RESET   0x10
#define MAPLE_INCTRL_START_DETECT 0x08
#define MAPLE_INCTRL_END_DETECT   0x04
#define MAPLE_INCTRL_HALT         0x02
#define MAPLE_INCTRL_RUN          0x01

static __inline void maple_set_reg(uint8_t reg, uint8_t value)
{
  SET(SS, LOW);
  SPI.transfer(reg|MAPLE_WRITE_FLAG);
  SPI.transfer(value);
  SET(SS, HIGH);
}

uint8_t maple_transaction(const uint8_t* src, uint8_t* dest);

#endif /* MAPLE_H_ */
