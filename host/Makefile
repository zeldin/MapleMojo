
SRCDIR?=$(dir $(lastword $(filter-out %.d,$(MAKEFILE_LIST))))
SRC := $(SRCDIR)src
O = obj

COMMON = $(SRCDIR)../common

libmaplemojo_SOURCES := \
	$(SRC)/serial.c \
	$(SRC)/maple_transaction.c \
	$(SRC)/maple_command.c \
	$(SRC)/maple_function.c

libmaplemojo_OBJS := \
	$(addprefix $O/,$(addsuffix .o,$(basename $(notdir $(libmaplemojo_SOURCES)))))

SAMPLES := \
	$(O)/lsmaple

all: $(O)/libmaplemojo.a $(SAMPLES)

$(O)/libmaplemojo.a : $(libmaplemojo_OBJS) | $O
	ar cru $@ $(libmaplemojo_OBJS)

$O/%.o : $(SRC)/%.c | $O
	gcc -c -I$(COMMON) -o $@ -MMD -MF $@.d $<

$O/% : $(SRCDIR)samples/%.c $(O)/libmaplemojo.a | $O
	gcc -I$(SRC) -I$(COMMON) -o $@ -MMD -MF $@.d $< $(O)/libmaplemojo.a

$O/libmaplemojo.a : $(OBJS)

$O:
	mkdir -p $@

clean:
	rm -rf $O

-include $(addprefix $O/,$(addsuffix .o.d,$(basename $(notdir $(libmaplemojo_SOURCES)))))
-include $(addprefix $O/,$(addsuffix .d,$(notdir $(SAMPLES))))
