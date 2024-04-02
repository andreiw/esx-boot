#
# SPDX-License-Identifier: BSD-3-Clause
#

ENV_FILE=old_build_env
-include $(ENV_FILE)

BUILD_FLAGS =
ifeq ($(DEBUG), 1)
BUILD_FLAGS += -DDEBUG
endif
BUILD_ENV = "OLD_BUILD_FLAGS=$(BUILD_FLAGS)"

ifneq ($(BUILD_FLAGS),$(OLD_BUILD_FLAGS))
	PRETARGET=clean_env
else
	PRETARGET=log_env
endif

CC =  riscv64-linux-gnu-gcc
CFLAGS  += -Os -Wall -Werror -fno-common -mcmodel=medany -ffreestanding -march=rv64gc -fno-pie -fno-pic -I./ $(BUILD_FLAGS)
LDFLAGS += -nostdlib -Wl,--build-id=none

.PHONY: clean all

all: $(PRETARGET) kernel.elf

clean:
	rm -f kernel.elf kernel.ld asm-constants.h sym-stubs.c *.P *.o

log_env:
	@echo Resuming build with env \"$(BUILD_FLAGS)\"

log_clean_env:
	@echo Cleaning due to env change \(was \"$(OLD_BUILD_FLAGS)\" now \"$(BUILD_FLAGS)\"\)

clean_env: log_clean_env clean
	@echo $(BUILD_ENV) > $(ENV_FILE)

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

kernel.elf: kernel.ld asm-constants.h entry.o kernel.o printf.o
	./ldcc_wrapper $(CC) $(CFLAGS) $(LDFLAGS) -o $@ -T $^

-include *.P
