
# All files are under A (short for ARCH): I'm lazy
A := arch-$(ARCH)

CFLAGS += -Itools

OBJ-y += $A/cygwin-startup.o \
	$A/main-loop.o \
	$A/cygwin-io.o \
	$A/cygwin-conf.o \
	lib/cmdline.o \
	lib/conf.o \
	lib/libc-functions.o \
	lib/drop.o \
	lib/div64.o

# The user can set TIME=, but we pick cygwin time by default
TIME ?= cygwin

include time-$(TIME)/Makefile

# cygwin time operations are always included as a fallback
ifneq ($(TIME),cygwin)
include time-cygwin/Makefile
endif
CFLAGS += -Itime-cygwin

all: $(TARGET)

# to build the target, we need -lstd again, in case we call functions that
# were not selected yet (e.g., pp_init_globals() ).
$(TARGET): $(TARGET).o
	$(CC) -Wl,-Map,$(TARGET).map2 -o $@ $(TARGET).o -lrt 

