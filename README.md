# esx-boot

Some WiP work on esx-boot, mostly (only?) in the context of using
as a bootloader for ACRN on RISC-V.

## Prerequisites

* sbsign (sbsigntool on Ubuntu)
* toolchain, along with libbfd and libiberty (https://github.com/riscv-collab/riscv-gnu-toolchain works,
  or install whatever you want + binutils-multiarch-dev)

First, run `./configure` to generate `env/toolchain.mk`.

Provided you use the riscv-collab toolchain, this should work for
paths in your `toolchain.mk` file:

```
...
HOST_LIBBFDINC  := /path/to/riscv-gnu-toolchain/build-gdb-newlib/bfd
HOST_LIBBFD     := /path/to/riscv-gnu-toolchain/build-gdb-newlib/bfd/.libs/libbfd.a -lzstd -lz
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

With a distro toolchain + multiarch binutils:

```
...
HOST_LIBBFDINC  := /usr/include
HOST_LIBBFD     := -lbfd-multiarch
HOST_LIBERTY    := -liberty
HOST_LIBCRYPTO  := -lcrypto
...
GCCROOT := '/'
CC      := riscv64-linux-gnu-gcc
LD      := riscv64-linux-gnu-ld
AR      := riscv64-linux-gnu-ar
OBJCOPY := riscv64-linux-gnu-objcopy
...
```

My `toolchain.mk` is provided under `env/toolchain.mk.andreiw`. This
is what I use to test uefi64, uefi32, com32 and uefiarm64 builds. YMMV.

## Building

```
$ make uefiriscv64 VERBOSE=0 DEBUG=0
```
