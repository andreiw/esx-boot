#*******************************************************************************
# Copyright (c) 2024, Intel Corporation. All rights reserved.
# SPDX-License-Identifier: GPL-2.0
#*******************************************************************************

#
# pboot Makefile
#

TOPDIR      := ..
include common.mk

INC         += ./ $(ODIR)
SRC         := pboot.c \
               zforth.c \
               setjmp.S \
               mon.c

BASENAME    := pboot
TARGETTYPE  := app
LIBS        := $(BOOTLIB) $(ENV_LIB)
CFLAGS      += -Wno-unused-parameter -Wno-clobbered

include rules.mk

$(ODIR)/zf_dict.h: zf_dict.zf
	echo Generating $@
	xxd -i $< > $@

$(ODIR)/asm-constants.h: asm-constants.c
	echo rebuilding $@
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

$(TARGET): $(ODIR)/asm-constants.h $(ODIR)/zf_dict.h
