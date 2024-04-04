/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <inttypes.h>

#define LINK_ADDRESS 0x80300000
#define STACK_PAGES  1
/*
 * Mapping the kernel itself using 2MB megapage. Assume
 * 1 us enough for now. We could be using up to five
 * page table levels (we won't know until we try
 * enabling!).
 */
#define EARLY_PAGES  4

#define R_RISCV_RELATIVE 0x3
typedef struct elf64_rela {
  uint64_t r_offset;
  uint64_t r_info;
  uint64_t r_addend;
} elf64_rela;

/*
 * We can have up to 5 levels on RISC-V, let's call
 * these L0 (which contains entries mapping 4KiB pages) to
 * L4 (which contains entries mapping 256TiB petapages).
 *
 * Each page level is indexed by PG_TAB_SHIFT (i.e. 512 entries).
 */
#define PAGE_SHIFT                 12
#define PAGE_SIZE                  (1UL << PAGE_SHIFT)
#define PAGE_MASK                  (PAGE_SIZE - 1)
#define PAGE_MASK_INVERSE          (~PAGE_MASK)
#define PG_TAB_SHIFT               9
#define PG_TAB_ENTRIES             (1 << PG_TAB_SHIFT)
#define PG_TAB_MASK                (PG_TAB_ENTRIES - 1)
#define PG_LEVEL_SHIFT(level)      (((level) * PG_TAB_SHIFT) + PAGE_SHIFT)
#define PG_LEVEL_SIZE(level)       (1UL << PG_LEVEL_SHIFT(level))
#define PG_LEVEL0_SHIFT            PG_LEVEL_SHIFT(0)
#define PG_LEVEL1_SHIFT            PG_LEVEL_SHIFT(1)
#define PG_LEVEL2_SHIFT            PG_LEVEL_SHIFT(2)
#define PG_LEVEL3_SHIFT            PG_LEVEL_SHIFT(3)
#define PG_LEVEL4_SHIFT            PG_LEVEL_SHIFT(4)
#define PG_LEVEL_OFFSET(level, va) ((va) >> PG_LEVEL_SHIFT(level)) & PG_TAB_MASK)
#define PG_ENT_PPN_MASK            0x3FFFFFFFFFFC00
/*
 * PPN is in bits [10:53].
 */
#define PG_ENT_PPN_SHIFT           2
#define PG_ENT_VALID               (1UL << 0)
#define PG_ENT_READ                (1UL << 1)
#define PG_ENT_WRITE               (1UL << 2)
#define PG_ENT_EXECUTE             (1UL << 3)

void sbi_putchar(char ch);
int printf(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));


