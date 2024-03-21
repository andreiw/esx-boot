# esx-boot

Some WiP work on esx-boot, mostly (only?) in the context of using
as a bootloader for ACRN on RISC-V.

## Prerequisites

* sbsign (sbsigntool on Ubuntu)
* toolchain, along with libbfd and libiberty (https://github.com/riscv-collab/riscv-gnu-toolchain works)

First, run `./configure` to generate `env/toolchain.mk`.

Provided you use the riscv-collab toolchain, this should work for
paths in your `toolchain.mk` file:

```
...
HOST_LIBBFDINC  :=/path/to/riscv-gnu-toolchain/build-gdb-newlib/bfd
HOST_LIBBFD     :=/path/to/riscv-gnu-toolchain/build-gdb-newlib/bfd/.libs/libbfd.a -lzstd -lz
HOST_LIBERTY    := /path/to/riscv-gnu-toolchain/build-gdb-newlib/libiberty/libiberty.a
HOST_LIBCRYPTO  := -lcrypto
...
GCCROOT := '/path/to'
CC      :=/path/to/bin/riscv64-unknown-elf-gcc
LD      :=/path/to/bin/riscv64-unknown-elf-ld
AR      :=/path/to/bin/riscv64-unknown-elf-ar
OBJCOPY :=/path/to/bin/riscv64-unknown-elf-objcopy
...
```

## Building

```
$ make uefiriscv64 VERBOSE=0 DEBUG=0
```
