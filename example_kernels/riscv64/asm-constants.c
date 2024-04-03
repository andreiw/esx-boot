/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include "kernel.h"
#include "../../include/esxbootinfo.h"

#define C(str, constant) asm volatile ("#define " str " %0\n\r" :: "i" ((uintptr_t)constant) :)

#define IDENTIFIER(x) C(#x, x)
#define SIZEOF(x) C("SZ_" #x, sizeof(x))
#define OFFSETOF(s, f) C("OF_" #s "_" #f, offsetof(s, f))

void dummy (void)
{
  IDENTIFIER(LINK_ADDRESS);
  IDENTIFIER(R_RISCV_RELATIVE);
  SIZEOF(elf64_rela);
  OFFSETOF(elf64_rela, r_offset);
  OFFSETOF(elf64_rela, r_info);
  OFFSETOF(elf64_rela, r_addend);
  IDENTIFIER(PAGE_SIZE);
  IDENTIFIER(STACK_PAGES);
  IDENTIFIER(EARLY_PAGES);
}


