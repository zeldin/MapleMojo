#include "maple.h"

#include "hardware.h"

uint8_t maple_transaction(const uint8_t* src, uint8_t* dest)
{
  uint16_t i, cnt = ((src[0]+1)<<2)+1;
  uint8_t avail;
  if (dest != NULL) {
    SET(SS, LOW);
    SPI.transfer(MAPLE_REG_INCTRL|MAPLE_WRITE_FLAG);
    SPI.transfer(MAPLE_INCTRL_FIFO_RESET|MAPLE_INCTRL_RUN);
    SET(SS, HIGH);
  }
  SET(SS, LOW);
  SPI.transfer(MAPLE_REG_OUTFIFO_FREE);
  avail = SPI.transfer(MAPLE_REG_OUTFIFO_FREE);
  SET(SS, HIGH);	
  if (avail > cnt)
    avail = cnt;
  cnt -= avail;
  SET(SS, LOW);
  SPI.transfer(MAPLE_REG_FIFO|MAPLE_WRITE_FLAG);
  while(avail--)
    SPI.transfer(*src++);
  SET(SS, HIGH);
  SET(SS, LOW);
  SPI.transfer(MAPLE_REG_OUTCTRL|MAPLE_WRITE_FLAG);
  if (cnt == 0) {
    SPI.transfer(MAPLE_OUTCTRL_MODE_START|MAPLE_OUTCTRL_MODE_END);
  } else {
    SPI.transfer(MAPLE_OUTCTRL_MODE_START);
    SET(SS, HIGH);	
    SET(SS, LOW);
    SPI.transfer(MAPLE_REG_OUTFIFO_CNT);
    for(;;) {
      avail = SPI.transfer(MAPLE_REG_OUTFIFO_CNT);
      if (avail) {
	SET(SS, HIGH);
	if (avail > cnt)
	  avail = cnt;
	cnt -= avail;
	SET(SS, LOW);
	SPI.transfer(MAPLE_REG_FIFO|MAPLE_WRITE_FLAG);
	while(avail--)
	  SPI.transfer(*src++);
	SET(SS, HIGH);
	SET(SS, LOW);
	if (cnt)
	  SPI.transfer(MAPLE_REG_OUTFIFO_CNT);
	else
	  break;
      }
    }
    SPI.transfer(MAPLE_REG_OUTCTRL|MAPLE_WRITE_FLAG);
    SPI.transfer(MAPLE_OUTCTRL_MODE_END);
  }
  SET(SS, HIGH);
  if (dest == NULL) {
    uint8_t outctl;
    SET(SS, LOW);
    SPI.transfer(MAPLE_REG_OUTCTRL);
    do {
      outctl = SPI.transfer(MAPLE_REG_OUTCTRL);
    } while(outctl & MAPLE_OUTCTRL_OUTPUT_EN);
    SET(SS, HIGH);
    return MAPLE_STATUS_OK;
  } else {
    uint8_t inctl;
    SET(SS, LOW);
    SPI.transfer(MAPLE_REG_INCTRL);
    inctl = SPI.transfer(MAPLE_REG_INFIFO_CNT);
    unsigned long start = millis();
    do {
      if (!(inctl & MAPLE_INCTRL_FIFO_READY)) {
	SPI.transfer(MAPLE_REG_INCTRL);
	inctl = SPI.transfer(MAPLE_REG_INFIFO_CNT);
      }
      while (inctl & MAPLE_INCTRL_FIFO_READY) {
	for (;;) {
	  avail = SPI.transfer(MAPLE_REG_INCTRL);
	  if (avail > MAPLE_BUFFER_SIZE-cnt)
	    avail = MAPLE_BUFFER_SIZE-cnt;
	  if (!avail)
	    break;
	  SPI.transfer(MAPLE_REG_FIFO);
	  if (avail > 1) {
	    --avail;
	    do {
	      dest[cnt++] = SPI.transfer(MAPLE_REG_FIFO);
	    } while(--avail);
	  }
	  dest[cnt++] = SPI.transfer(MAPLE_REG_INFIFO_CNT);
	}
	inctl = SPI.transfer(MAPLE_REG_INFIFO_CNT);
	if (cnt > MAPLE_MAX_PACKET_LENGTH)
	  break;
      }
      if (millis()-start > MAPLE_TIMEOUT_MS || cnt > MAPLE_MAX_PACKET_LENGTH)
	break;
    } while(inctl & MAPLE_INCTRL_RUN);
    SET(SS, HIGH);
    SET(SS, LOW);
    SPI.transfer(MAPLE_REG_INCTRL|MAPLE_WRITE_FLAG);
    SPI.transfer(MAPLE_INCTRL_FIFO_RESET|MAPLE_INCTRL_HALT);
    SET(SS, HIGH);
    if (inctl != (MAPLE_INCTRL_START_DETECT|MAPLE_INCTRL_END_DETECT)) {
      if (!(inctl & MAPLE_INCTRL_START_DETECT))
	return MAPLE_STATUS_NO_START_PATTERN;
      else if(inctl & MAPLE_INCTRL_FIFO_OVERF)
	return MAPLE_STATUS_FIFO_OVERFLOW;
      else if(inctl & MAPLE_INCTRL_FIFO_UNDERF)
	return MAPLE_STATUS_FIFO_UNDERFLOW;
      else if(cnt > MAPLE_MAX_PACKET_LENGTH)
	return MAPLE_STATUS_REPLY_TOO_LONG;
      else
	return MAPLE_STATUS_TIMEOUT;
    } else if (cnt < 5) {
      return MAPLE_STATUS_REPLY_TOO_SHORT;
    } else if (cnt != ((dest[0]+1)<<2)+1) {
      return MAPLE_STATUS_REPLY_BAD_COUNT;
    } else {
      return MAPLE_STATUS_OK;
    }
  }
}
