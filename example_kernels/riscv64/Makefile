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
HFLAGS += -I./ -Donly_riscv64 $(BUILD_FLAGS)
CFLAGS += -Os -Wall -Werror -fno-common -mcmodel=medany -ffreestanding -march=rv64gc -fno-pie -fno-pic $(HFLAGS)
ASFLAGS = $(CFLAGS) -D__ASSEMBLER__
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

kernel.ld: kernel.lds asm-constants.h
	$(CC) -xc -E -P $(CFLAGS) -o $@ $<
	$(ECHO) rebuilding $@

asm-constants.h: asm-constants.c
	$(ECHO) rebuilding $@
	$(CC) -MD $(CFLAGS) -S $< -o $@
	mv $@ $@.p1
	$(ECHO) \#ifndef __ASSEMBLER__ > $@.p2
	grep \#define $@.p1 >> $@.p2
	$(ECHO) \#else >> $@.p2
	grep equ $@.p1 >> $@.p2
	$(ECHO) \#endif >> $@.p2
	mv $@.p2 $@
	rm -f $@.p1
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
	$(CC) -MD $(ASFLAGS) $< -c -o $@
	cp $*.d $*.P; \
   sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
   -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
   rm -f $*.d

kernel.elf: kernel.ld asm-constants.h entry.o early-ebi.o early-relocs.o early-pages.o early-mmu.o kernel.o printf.o sbi.o trap.o
	$(LINKER) $(CFLAGS) $(LDFLAGS) -o $@ -T $^

-include *.P
