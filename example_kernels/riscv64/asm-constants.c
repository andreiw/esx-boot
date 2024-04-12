/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include "kernel.h"
#include "../../include/esxbootinfo.h"

#define C(str, constant) \
   asm volatile ("#define " str " %0\n\r" :: "i" ((uintptr_t)constant) :); \
   asm volatile (".equ " str ", %0\n\r" :: "i" ((uintptr_t)constant) :)

#define IS_TYPE_UNSIGNED(type) ((type)0 - 1 > 0)

#define IDENTIFIER(x) C(#x, x)
#define SIZE(x) C("SZ_" #x, sizeof(x))
#define FIELD_SIZE(t, f) C("SZ_" #t "_" #f, sizeof(((t *) 0)->f))
#define FIELD_OFFSET(t, f) C("OF_" #t "_" #f, offsetof(t, f))
/*
 * For a field in a struct, define the size, offset and signedness.
 * The latter is helpful in as macros to simplify loading correctly.
 */
#define FIELD(t, f) \
   FIELD_SIZE(t, f); \
   FIELD_OFFSET(t, f); \
   C("IU_" #t "_" #f, IS_TYPE_UNSIGNED(typeof(((t *) 0)->f)))

void dummy (void)
{
  IDENTIFIER(LINK_ADDRESS);
  IDENTIFIER(STACK_PAGES);
  IDENTIFIER(EARLY_PAGES);

  IDENTIFIER(R_RISCV_RELATIVE);
  SIZE(elf64_rela);
  FIELD(elf64_rela, r_offset);
  FIELD(elf64_rela, r_info);
  FIELD(elf64_rela, r_addend);

  FIELD(ESXBootInfo, numESXBootInfoElmt);
  FIELD_OFFSET(ESXBootInfo, elmts);
  FIELD(ESXBootInfo_Elmt, type);
  FIELD(ESXBootInfo_Elmt, elmtSize);
  IDENTIFIER(ESXBOOTINFO_SERIAL_CON_TYPE);
  FIELD(ESXBootInfo_SerialCon, base);
  FIELD(ESXBootInfo_SerialCon, space);
  IDENTIFIER(ESXBOOTINFO_SERIAL_CON_SPACE_MMIO);

  IDENTIFIER(SATP_SV39);
  IDENTIFIER(SATP_SV48);
  IDENTIFIER(SATP_SV57);

  IDENTIFIER(PG_BASE_SHIFT);
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


