SRCDIR?=$(dir $(lastword $(MAKEFILE_LIST)))

MODEL = MojoV3

ARDUINO_DIR = /usr/share/arduino
ARDUINO_HARDWARE_DIRS = $(ARDUINO_DIR)/hardware $(wildcard $(HOME)/Arduino/hardware)
ARDUINO_TOOLS_DIR = $(ARDUINO_DIR)/tools

ARDUINO_BUILDER = arduino-builder
ARDUINO_BUILDER_FLAGS = $(addprefix -hardware ,$(ARDUINO_HARDWARE_DIRS)) -tools $(ARDUINO_TOOLS_DIR) -fqbn "embeddedmicro:avr:MojoV3" -build-path "$(abspath $O)"

AVRDUDE = avrdude
UPLOAD_PORT = /dev/ttyACM0

FW = maplemojo.ino.hex

SRC := $(SRCDIR)src
O = obj

COMMON = $(SRCDIR)../common

FLAGS = -I$(COMMON) -ffunction-sections -fdata-sections -g -Os -w

maplemojo_SOURCES := \
	$(SRC)/maplemojo.ino \
	$(SRC)/flash.cpp \
	$(SRC)/flash.h \
	$(SRC)/maple.cpp \
	$(SRC)/maple.h \
	$(SRC)/hardware.h \
	$(COMMON)/maple_status.h

all : $O/$(FW)

$O/$(FW) : $(maplemojo_SOURCES) | $O
	$(ARDUINO_BUILDER) $(ARDUINO_BUILDER_FLAGS) -prefs=compiler.cpp.extra_flags='$(FLAGS)' -compile $<

upload: $O/$(FW)
	$(AVRDUDE) -patmega32u4 -cavr109 -P$(UPLOAD_PORT) -Uflash:w:"$^":i

clean:
	rm -rf $O

$O:
	mkdir -p $@
