#
# SPDX-License-Identifier: BSD-3-Clause
#

CC =  riscv64-linux-gnu-gcc
CFLAGS  += -Os -Wall -Werror -fno-common -mcmodel=medany -ffreestanding -march=rv64gc -fno-pie -fno-pic -I./
LDFLAGS += -nostdlib -Wl,--build-id=none

.PHONY: clean all

all: kernel.elf

clean:
	rm -f kernel.elf kernel.ld asm-constants.h sym-stubs.c *.P *.o

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
