
MODEL = MojoV3

INO = ino
INO_FLAGS = -m $(MODEL)

FW = firmware.hex

SRC := $(SRCDIR)src
O = obj
BUILD_FW := .build/$(MODEL)/$(FW)

maplemojo_SOURCES := \
	$(SRC)/maplemojo.ino \
	$(SRC)/flash.cpp \
	$(SRC)/flash.h \
	$(SRC)/maple.cpp \
	$(SRC)/maple.h \
	$(SRC)/ring_buffer.h \
	$(SRC)/hardware.h

all : $O/$(FW)

$O/$(FW) : $(BUILD_FW) | $O
	cp $< $@

$(BUILD_FW) : $(maplemojo_SOURCES)
	$(INO) build $(INO_FLAGS)

upload: $(BUILD_FW)
	$(INO) upload $(INO_FLAGS)

clean:
	$(INO) clean

$O:
	mkdir -p $@
