CC =  riscv64-linux-gnu-gcc
CFLAGS  += -Os -fno-common -fomit-frame-pointer -mcmodel=medany -ffreestanding -nostdinc -march=rv64gc -fno-pie -fno-pic -I../../include
LDFLAGS += -static -nostdlib -Wl,--build-id=none

.PHONY: clean all

all: kernel.elf

clean:
	rm -f kernel.elf kernel.ld

kernel.ld: kernel.lds kernel.h
	$(CC) -xc -E -P $(CFLAGS) -o $@ $<

kernel.elf: kernel.ld entry.S
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -T $^

