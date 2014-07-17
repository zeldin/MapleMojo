#include "hardware.h"
#include "ring_buffer.h"
#include <SPI.h>
#include "flash.h"
#include "maple.h"

typedef enum {
  IDLE,
  READ_SIZE,
  WRITE_TO_FLASH,
  WRITE_TO_FPGA,
  VERIFY_FLASH,
  LOAD_FROM_FLASH
} 
loaderState_t;

typedef enum {
  WAIT, START_LOAD, LOAD, SERVICE
} 
taskState_t;

uint8_t loadBuffer[256];
volatile taskState_t taskState = SERVICE;

/* This is where you should add your own code! Feel free to edit anything here. 
   This function will work just like the Arduino loop() function in that it will
   be called over and over. You should try to not delay too long in the loop to 
   allow the Mojo to enter loading mode when requested. */
void userLoop() {
}

/* this is used to undo any setup you did in initPostLoad */
void disablePostLoad() {
  ADCSRA = 0; // disable ADC
  UCSR1B = 0; // disable serial port
  SPI.end();  // disable SPI
  SET(CCLK, LOW);
  OUT(PROGRAM);
  SET(PROGRAM, LOW); // reset the FPGA
  IN(INIT);
  SET(INIT, HIGH); // pullup on INIT
}

/* Here you can do some setup before entering the userLoop loop */
void initPostLoad() {
  Serial.flush();

  // Setup all the SPI pins
  SET(CS_FLASH, HIGH);
  OUT(SS);
  SET(SS, HIGH);
  SPI_Setup(); // enable the SPI Port

  // set progam as an input so that it's possible to use a JTAG programmer with the Mojo
  IN(PROGRAM);

  // the FPGA looks for CCLK to be high to know the AVR is ready for data
  SET(CCLK, HIGH);
}

void setup() {
  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  OUT(CS_FLASH);
  SET(CS_FLASH, HIGH);
  OUT(CCLK);
  OUT(PROGRAM);

  /* Disable digital inputs on analog pins */
  DIDR0 = 0xF3;
  DIDR2 = 0x03;

  Serial.begin(115200); // Baud rate does nothing

  sei(); // enable interrupts

  loadFromFlash(); // load on power up
  initPostLoad();
}

void loop() {
  static loaderState_t state = IDLE;
  static int8_t destination;
  static int8_t verify;
  static uint32_t byteCount;
  static uint32_t transferSize;

  int16_t w;
  uint8_t bt;
  uint8_t buffIdx;

  switch (taskState) {
  case WAIT:
    break;
  case START_LOAD: // command to enter loader mode
    disablePostLoad(); // setup peripherals
    taskState = LOAD; // enter loader mode
    state = IDLE; // in idle state
    break;
  case LOAD:
    w = Serial.read();
    bt = (uint8_t) w;
    if (w >= 0) { // if we have data
      switch (state) {
      case IDLE: // in IDLE we are waiting for a command from the PC
        byteCount = 0;
        transferSize = 0;
        if (bt == 'F') { // write to flash
          destination = 0; // flash
          verify = 0; // don't verify
          state = READ_SIZE;
          Serial.write('R'); // signal we are ready
        }
        if (bt == 'V') { // write to flash and verify
          destination = 0; // flash
          verify = 1; // verify
          state = READ_SIZE;
          Serial.write('R'); // signal we are ready
        }
        if (bt == 'R') { // write to RAM
          destination = 1; // ram
          state = READ_SIZE;
          Serial.write('R'); // signal we are ready
        }
        if (bt == 'E') { //erase
          eraseFlash();
          Serial.write('D'); // signal we are done
        }
        break;
      case READ_SIZE: // we need to read in how many bytes the config data is
        transferSize |= ((uint32_t) bt << (byteCount++ * 8));
        if (byteCount > 3) {
          byteCount = 0;
          if (destination) {
            state = WRITE_TO_FPGA;
            initLoad(); // get the FPGA read for a load
            startLoad(); // start the load
          } 
          else {
            state = WRITE_TO_FLASH;
            eraseFlash();
          }
          Serial.write('O'); // signal the size was read
        }
        break;
      case WRITE_TO_FLASH:
        // we can only use the batch write for even addresses
        // so address 5 is written as a single byte
        if (byteCount == 0)
          writeByteFlash(5, bt);

        buffIdx = (byteCount++ - 1) % 256;
        loadBuffer[buffIdx] = bt;

        if (buffIdx == 255 && byteCount != 0)
          writeFlash(byteCount + 5 - 256, loadBuffer, 256); // write blocks of 256 bytes at a time for speed

        if (byteCount == transferSize) { // the last block to write

          if (buffIdx != 255) // finish the partial block write
            writeFlash(byteCount + 5 - (buffIdx + 1), loadBuffer,
            buffIdx + 1);

          delayMicroseconds(50); // these are necciary to get reliable writes
          uint32_t size = byteCount + 5;
          for (uint8_t k = 0; k < 4; k++) {
            writeByteFlash(k + 1, (size >> (k * 8)) & 0xFF); // write the size of the config data to the flash
            delayMicroseconds(50);
          }
          delayMicroseconds(50);
          writeByteFlash(0, 0xAA); // 0xAA is used to signal the flash has valid data
          Serial.write('D'); // signal we are done
          Serial.flush(); // make sure it sends
          if (verify) {
            state = VERIFY_FLASH;
          } 
          else {
            state = LOAD_FROM_FLASH;
          }
        }
        break;
      case WRITE_TO_FPGA:
        sendByte(bt); // just send the byte!
        if (++byteCount == transferSize) { // if we are done
          sendExtraClocks(); // send some extra clocks to make sure the FPGA is happy
          state = IDLE;
          taskState = SERVICE; // enter user mode
          initPostLoad();
          Serial.write('D'); // signal we are done
        }
        break;
      case VERIFY_FLASH:
        if (bt == 'S') {
          byteCount += 5;
          for (uint32_t k = 0; k < byteCount; k += 256) { // dump all the flash data
            uint16_t s;
            if (k + 256 <= byteCount) {
              s = 256;
            } 
            else {
              s = byteCount - k;
            }
            readFlash(loadBuffer, k, s); // read blocks of 256
            uint16_t br = Serial.write((uint8_t*) loadBuffer, s); // dump them to the serial port
            k -= (256 - br); // if all the bytes weren't sent, resend them next round
            delay(10); // needed to prevent errors in some computers running Windows (give it time to process the data?)
          }
          state = LOAD_FROM_FLASH;
        }
        break;
      case LOAD_FROM_FLASH:
        if (bt == 'L') {
          loadFromFlash(); // load 'er up!
          Serial.write('D'); // loading done
          state = IDLE;
          taskState = SERVICE;
          initPostLoad();
        }
        break;
      }
    }

    break;
  case SERVICE:
    userLoop(); // loop the user code
    break;
  } 
}

/* This is called when any control lines on the serial port are changed. 
 It requires a modification to the Arduino core code to work.         
 
 This looks for 5 pulses on the DTR line within 250ms. Checking for 5 
 makes sure that false triggers won't happen when the serial port is opened. */
void lineStateEvent(unsigned char linestate)
{
  static unsigned long start = 0; 
  static uint8_t falling = 0;
  if (!(linestate & LINESTATE_DTR)) {
    if ((millis() - start) < 250) {
      if (++falling >= 5)
        taskState = START_LOAD;
    } 
    else {
      start = millis();
      falling = 1;
    }
  }
}

inline void enableDataBus() {
  FPGA_BUS_DDR = 0xFF;
}

void initLoad() {
  SET(INIT, HIGH);
  SET(CCLK, LOW);
  SET(PROGRAM, LOW);
  enableDataBus();
} 

void startLoad() {
  SET(CCLK, LOW);
  SET(PROGRAM, LOW);
  delay(1);
  SET(PROGRAM, HIGH);
  while (!VALUE(INIT));
}

void sendByte(uint8_t b) {
  FPGA_BUS_PORT = b;
  SET(CCLK, HIGH);
  SET(CCLK, LOW);
}

void sendExtraClocks() {
  FPGA_BUS_PORT = 0xff;
  for (int i = 0; i < 10; i++) {
    SET(CCLK, HIGH);
    SET(CCLK, LOW);
  }
}

void loadFromFlash() {
  uint32_t lastAddr = 0;

  initLoad();
  startLoad();

  readFlash(loadBuffer, 0, 5);

  if (loadBuffer[0] != 0xaa){
    return;
  }

  for (uint8_t k = 0; k < 4; k++) {
    lastAddr |= (uint32_t) loadBuffer[k + 1] << (k * 8);
  }

  uint32_t curAddr = 5;

  while (curAddr + 256 < lastAddr) {
    readFlash(loadBuffer, curAddr, 256);
    enableDataBus();
    for (uint16_t i = 0; i < 256; i++) {
      sendByte(loadBuffer[i]);
    }
    curAddr += 256;
  }

  if (curAddr < lastAddr) {
    uint8_t rem = lastAddr - curAddr;
    readFlash(loadBuffer, curAddr, rem);
    enableDataBus();
    for (uint8_t i = 0; i < rem; i++) {
      sendByte(loadBuffer[i]);
    }
  }

  sendExtraClocks();
}

