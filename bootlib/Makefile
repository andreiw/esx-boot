#*******************************************************************************
# Copyright (c) 2008-2013,2016,2018-2019,2021,2023 VMware, Inc.
# All rights reserved.
# Copyright (c) 2024, Intel Corporation. All rights reserved.
# SPDX-License-Identifier: GPL-2.0
#*******************************************************************************

#
# Boot utils Library Makefile
#

TOPDIR       := ..
include common.mk

#
# 0log.c must precede log.c.  Otherwise the list is in alphabetical
# order just for neatness.
#
SRC         := \
	       0log.c        \
               acpi.c        \
               alloc.c       \
               e820.c        \
               error.c       \
               fb.c          \
               fbcon.c       \
               file.c        \
               gpt.c         \
               gzip.c        \
               log.c         \
               mbr.c         \
               net.c         \
               parse.c       \
               serial.c      \
               smbios.c      \
               system.c      \
               $(IARCH)/system_arch.c\
               sort.c        \
               string.c      \
               video.c       \
               volume.c      \

ifneq ($(IARCH),x86)
SRC += fdt.c
endif

BASENAME    := boot
TARGETTYPE  := lib
INC         := $(ZLIB_INC) $(LIBFAT_INC) $(UEFIPROTO)

include rules.mk
