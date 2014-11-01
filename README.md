MapleMojo
=========

This is an implementation of the Maple Bus used by SEGA
Dreamcast perihperhals, using the Mojo v3 FPGA board.

The FPGA bitstream implements send and receive on four
ports at configurable speeds, and is controlled with an
SPI interface.

The included AVR firmware used this SPI interface to
provide I/O of Maple packets via a CDC serial port.

There is also host software using this CDC serial port
to e.g. enumerate the currectly connected devices.

Hardware requirements
---------------------

In order to interface with Maple Bus units, appropriate
connecters need to be fitted to the Mojo board.  Pull
up resistors to +3.3V also need to be installed for each
connector going to a peripheral.  For a connector going to
the Dreamcast main unit, no pull ups should be mounted.

Mojo V3 SV1 pinout for connection to Maple Bus:

|Pin|Text |Connection                 |
|--:|-----|---------------------------|
| 1 |GND  |Common ground (pin 3)      |
| 2 |RAW  |+5V feed (pin 2)	      |
| 3 |GND  |Common ground (pin 3)      |
| 4 |+V   |+3.3V feed (for pull-up)   |
| 5 |SUS  |N/C			      |
| 6 |DONE |N/C			      |
| 7 |58   |Port A SDCKB (pin 5)	      |
| 8 |57   |Port A SDCKA (pin 1)	      |
| 9 |67   |Port B SDCKB (pin 5)	      |
|10 |66   |Port B SDCKA (pin 1)	      |
|11 |75   |Port C SDCKB (pin 5)	      |
|12 |74   |Port C SDCKA (pin 1)	      |
|13 |79   |Port D SDCKB (pin 5)	      |
|14 |78   |Port D SDCKA (pin 1)	      |
|15 |81   |Reserved for Port B pin 4  |
|16 |80   |Reserved for Port A pin 4  |
|17 |83   |Reserved for Port D pin 4  |
|18 |82   |Reserced for port C pin 4  |


Software requirements
---------------------

In order to build MapleMojo, the following is needed

* Xilinx ISE

* Arduino SDK

* [ino build tool](http://inotool.org/)

