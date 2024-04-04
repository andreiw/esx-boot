#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef V
   # V not set: Terse messages; can be hard to debug makefile issues.
   MAKEFLAGS += -s
else
ifeq (0,$(V))
   # V=0: Same as VERBOSE not set.
   MAKEFLAGS += -s
else
   # V=1 (or other value): Maximally verbose messages.
endif
endif

ENV_FILE=old_build_env
-include $(ENV_FILE)

BUILD_ENV =
BUILD_FLAGS =
ifeq ($(DEBUG), 1)
	BUILD_ENV += DEBUG
	BUILD_FLAGS += -DDEBUG
	WITH_BRING_UP_SYM_STUBS=1
endif
ifeq ($(WITH_BRING_UP_SYM_STUBS), 1)
	BUILD_ENV += WITH_BRING_UP_SYM_STUBS
	BUILD_FLAGS += -DWITH_BRING_UP_SYM_STUBS
endif

ifneq ($(BUILD_ENV),$(OLD_BUILD_ENV))
	PRETARGET=clean_env
else
	PRETARGET=log_env
endif

ECHO = @echo
CC =  riscv64-linux-gnu-gcc
CFLAGS  += -Os -Wall -Werror -fno-common -mcmodel=medany -ffreestanding -march=rv64gc -fno-pie -fno-pic -I./ $(BUILD_FLAGS)
LINKER = $(CC)
ifeq ($(WITH_BRING_UP_SYM_STUBS), 1)
	LINKER=./ldcc_wrapper $(CC)
endif
LDFLAGS += -nostdlib -Wl,--build-id=none

.PHONY: clean all

all: $(PRETARGET) kernel.elf
	@file kernel.elf

clean:
	rm -f kernel.elf kernel.ld asm-constants.h sym-stubs.c *.P *.o

log_env:
	$(ECHO) Resuming build with env \"$(BUILD_ENV)\"

log_clean_env:
	$(ECHO) Cleaning due to env change \(was \"$(OLD_BUILD_ENV)\" now \"$(BUILD_ENV)\"\)

clean_env: log_clean_env clean
	$(ECHO) "OLD_BUILD_ENV=$(BUILD_ENV)" > $(ENV_FILE)

kernel.lds: asm-constants.h

kernel.ld: kernel.lds
	$(CC) -xc -E -P $(CFLAGS) -o $@ $<

asm-constants.h: asm-constants.c
	$(CC) -MD $(CFLAGS) -S $< -o $@
	grep \#define $@ > $@.h
	mv $@.h $@
	cp $*.d $*.P; \
   sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
   -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
   rm -f $*.d

%.o: %.c
	$(CC) -MD $(CFLAGS) $< -c -o $@
	cp $*.d $*.P; \
   sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
   -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
   rm -f $*.d

%.o: %.S
	$(CC) -MD $(CFLAGS) $< -c -o $@
	cp $*.d $*.P; \
   sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
   -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
   rm -f $*.d

kernel.elf: kernel.ld asm-constants.h entry.o early-pages.o early-mmu.S kernel.o printf.o
	$(LINKER) $(CFLAGS) $(LDFLAGS) -o $@ -T $^

-include *.P
