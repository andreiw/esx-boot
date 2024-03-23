#*******************************************************************************
# Copyright (c) 2008-2012,2015,2017-2020,2022 VMware, Inc.  All rights reserved.
# Copyright (c) 2024, Intel Corporation. All rights reserved.
# SPDX-License-Identifier: GPL-2.0
#*******************************************************************************

#
# Tool chain definitions.
# Example for building with the default tools on the local machine.
#

#=============================================================
# Host definitions. Used to compile tools that run on the
# host during the build, such as uefi/elf2efi and bios/relocs.
#=============================================================

HOST_CC         := gcc
HOST_CFLAGS     :=
#
# Known to work with libbfd 2.17.50. libbfd
# used should be aware of all architecture types
# used in build.
#
HOST_LIBBFDINC  := /usr/include
HOST_LIBBFD     := -lbfd
HOST_LIBERTY    := -liberty
HOST_LIBCRYPTO  := -lcrypto

#=============================================================
# Target definitions.
#=============================================================

# Either python2 or python3 can be used
PYTHON  := /usr/bin/python3
GETKEYS_PYTHONPATH := $(PYTHONPATH)

ifeq ($(BUILDENV),uefiriscv64)
GCCROOT := <Path to riscv64 gcc sysroot>
CC      := <Path to riscv64 gcc>
LD      := <Path to riscv64 ld>
AR      := <Path to riscv64 ar>
OBJCOPY := <Path to riscv64 objcopy>
else ifeq ($(BUILDENV),uefiarm64)
GCCROOT := <Path to aarch64 gcc sysroot>
CC      := <Path to aarch64 gcc>
LD      := <Path to aarch64 ld>
AR      := <Path to aarch64 ar>
OBJCOPY := <Path to aarch64 objcopy>
else
#
# Known to work with gcc 4.8.0, nasm 2.01, and binutils 2.22
#
GCCROOT := '/'
CC      := gcc
LD      := ld
AR      := ar
OBJCOPY := objcopy
AS      := nasm
endif
