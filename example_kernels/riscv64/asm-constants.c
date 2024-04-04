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
  IDENTIFIER(STACK_PAGES);
  IDENTIFIER(EARLY_PAGES);

  IDENTIFIER(R_RISCV_RELATIVE);
  SIZEOF(elf64_rela);
  OFFSETOF(elf64_rela, r_offset);
  OFFSETOF(elf64_rela, r_info);
  OFFSETOF(elf64_rela, r_addend);

  IDENTIFIER(PG_BASE_SIZE);
  IDENTIFIER(PG_BASE_MASK);
  IDENTIFIER(PG_TAB_MASK);
  IDENTIFIER(PG_LEVEL0_SIZE);
  IDENTIFIER(PG_LEVEL1_SIZE);
  IDENTIFIER(PG_LEVEL2_SIZE);
  IDENTIFIER(PG_LEVEL3_SIZE);
  IDENTIFIER(PG_LEVEL4_SIZE);
  IDENTIFIER(PG_LEVEL0_MASK);
  IDENTIFIER(PG_LEVEL1_MASK);
  IDENTIFIER(PG_LEVEL2_MASK);
  IDENTIFIER(PG_LEVEL3_MASK);
  IDENTIFIER(PG_LEVEL4_MASK);
  IDENTIFIER(PG_LEVEL0_SHIFT);
  IDENTIFIER(PG_LEVEL1_SHIFT);
  IDENTIFIER(PG_LEVEL2_SHIFT);
  IDENTIFIER(PG_LEVEL3_SHIFT);
  IDENTIFIER(PG_LEVEL4_SHIFT);
  IDENTIFIER(PG_ENT_PPN_MASK);
  IDENTIFIER(PG_ENT_PPN_SHIFT);
  IDENTIFIER(PG_ENT_VALID);
  IDENTIFIER(PG_ENT_READ);
  IDENTIFIER(PG_ENT_WRITE);
  IDENTIFIER(PG_ENT_EXECUTE);
}


