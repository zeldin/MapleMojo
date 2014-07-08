ifeq ($(origin _),command line)

XILINXBIN := /opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64

SRC := $(SRCDIR)source

maplemojo_DEVICE := xc6slx9-2tqg144
maplemojo_TOP_MODULE := maplemojo_top

maplemojo_VERILOG_SOURCES := \
	$(SRC)/maplemojo_top.v \
	$(SRC)/maple_interface.v \
	$(SRC)/maple_out.v \
	$(SRC)/maple_ports.v \
	$(SRC)/regaccess.v \
	$(SRC)/clock_divider.v \
	$(SRC)/read_only_reg.v \
	$(SRC)/read_write_reg.v \
	$(SRC)/spi_slave.v \
	$(SRC)/ocpin.v

maplemojo_CONSTRAINT_FILES := \
	$(SRC)/maplemojo.ucf

all : maplemojo.bit

.SECONDARY:

include $(SRCDIR)xilinx.mk

else

# Run make in object directory

SRCDIR?=$(dir $(lastword $(MAKEFILE_LIST)))
SUB_SRCDIR:=$(if $(filter /%,$(SRCDIR)),,../)$(SRCDIR)
O=obj
.DEFAULT_GOAL:=dummy

%: | $O
	@$(MAKE) --no-print-directory -C $O -f $(SUB_SRCDIR)/Makefile SRCDIR=$(SUB_SRCDIR) _= $(if $(MAKECMDGOALS),$@,)

clean:
	rm -rf $O

$O:
	mkdir -p $@

endif
