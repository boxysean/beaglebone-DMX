#CROSS_COMPILE?=arm-arago-linux-gnueabi-

#include libprussdrv/Makefile

LIBDIR?=lib
INCDIR?=include
BINDIR?=bin
SRCDIR?=src
PASM?=pasm/pasm

CFLAGS+= -Wall -I$(INCDIR) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a
LDFLAGS+=-L$(LIBDIR) -lprussdrv -lpthread
OBJDIR=obj

TARGET=$(BINDIR)/dmx
P_TARGET=$(BINDIR)/dmx.bin

_DEPS = libprussdrv.a libprussdrvd.a
DEPS = $(patsubst %,$(LIBDIR)/%,$(_DEPS))

_OBJ = dmx.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

all: $(TARGET) $(P_TARGET)

$(LIBDIR)/%.a:
	make --directory=libprussdrv

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(OBJDIR)
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	@mkdir -p $(BINDIR)
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

# add dependency to pasm being compiled...
$(BINDIR)/%.bin: $(SRCDIR)/%.p
	@mkdir -p $(BINDIR)
	${PASM} -b $< $(basename $@)

.PHONY: clean

clean:
	rm -rf $(OBJDIR) $(BINDIR) $(LIBDIR)
