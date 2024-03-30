#
# SPDX-License-Identifier: BSD-3-Clause
#

CC =  riscv64-linux-gnu-gcc
CFLAGS  += -Os -Wall -Werror -fno-common -fomit-frame-pointer -mcmodel=medany -ffreestanding -march=rv64gc -fno-pie -fno-pic -I./
LDFLAGS += -nostdlib -Wl,--build-id=none

.PHONY: clean all

all: kernel.elf

clean:
	rm -f kernel.elf kernel.ld asm-constants.s asm-constants.h

kernel.ld: kernel.lds asm-constants.h
	$(CC) -xc -E -P $(CFLAGS) -o $@ $<

%.s: %.c
	$(CC) $(CFLAGS) -S $< -o $@

asm-constants.h: asm-constants.s
	grep \#define $< > $@

kernel.elf: kernel.ld asm-constants.h entry.S kernel.c printf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -T $^
