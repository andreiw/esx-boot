/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <assert.h>
#include <inttypes.h>

#define DECLARE_AABI_CALL(ret_type, name, nargs, nrets) \
  static_assert(nrets <= 2, "aabi call " #name " returns too much"); \
  ret_type aabi_##name##_##nargs##_##nrets
#define AABI_CALL(name, nargs, nrets) aabi_##name##_##nargs##_##nrets

#define LINK_ADDRESS 0x80300000UL
#define STACK_PAGES  1
/*
 * Mapping the kernel itself using 2MB megapages. Assume
 * 1 megapage is enough for now. We could be using up to five
 * page table levels (we won't know until we try
 * enabling!).
 *
 * Also map the UART using a 4K page. This could eat another 4
 * pages (worst case).
 */
#define EARLY_PAGES  (4 + 4)

#define R_RISCV_RELATIVE 0x3
typedef struct elf64_rela {
  uint64_t r_offset;
  uint64_t r_info;
  uint64_t r_addend;
} elf64_rela;

#define SATP_MODE_BARE  0UL
#define SATP_MODE_SV39  8UL
#define SATP_MODE_SV48  9UL
#define SATP_MODE_SV57  10UL
#define SATP_MODE_SHIFT 60
#define SATP_SV39       (SATP_MODE_SV39 << SATP_MODE_SHIFT)
#define SATP_SV48       (SATP_MODE_SV48 << SATP_MODE_SHIFT)
#define SATP_SV57       (SATP_MODE_SV57 << SATP_MODE_SHIFT)

/*
 * We can have up to 5 levels on RISC-V, let's call
 * these L0 (which contains entries mapping 4KiB pages) to
 * L4 (which contains entries mapping 256TiB petapages).
 *
 * Each page level is indexed by PG_TAB_SHIFT (i.e. 512 entries).
 */
#define PG_BASE_SHIFT              12UL
#define PG_BASE_SIZE               (1UL << PG_BASE_SHIFT)
#define PG_BASE_MASK               (PG_BASE_SIZE - 1)
#define PG_BASE_MASK_INVERSE       (~PG_BASE_MASK)
#define PG_TAB_SHIFT               9UL
#define PG_TAB_ENTRIES             (1UL << PG_TAB_SHIFT)
#define PG_TAB_MASK                (PG_TAB_ENTRIES - 1)
#define PG_LEVEL_SHIFT(level)      (((level) * PG_TAB_SHIFT) + PG_BASE_SHIFT)
#define PG_LEVEL_SIZE(level)       (1UL << PG_LEVEL_SHIFT(level))
#define PG_LEVEL_MASK(level)       (PG_LEVEL_SIZE(level) - 1)
#define PG_LEVEL0_SHIFT            PG_LEVEL_SHIFT(0)
#define PG_LEVEL1_SHIFT            PG_LEVEL_SHIFT(1)
#define PG_LEVEL2_SHIFT            PG_LEVEL_SHIFT(2)
#define PG_LEVEL3_SHIFT            PG_LEVEL_SHIFT(3)
#define PG_LEVEL4_SHIFT            PG_LEVEL_SHIFT(4)
#define PG_LEVEL0_SIZE             PG_LEVEL_SIZE(0)
#define PG_LEVEL1_SIZE             PG_LEVEL_SIZE(1)
#define PG_LEVEL2_SIZE             PG_LEVEL_SIZE(2)
#define PG_LEVEL3_SIZE             PG_LEVEL_SIZE(3)
#define PG_LEVEL4_SIZE             PG_LEVEL_SIZE(4)
#define PG_LEVEL0_MASK             PG_LEVEL_MASK(0)
#define PG_LEVEL1_MASK             PG_LEVEL_MASK(1)
#define PG_LEVEL2_MASK             PG_LEVEL_MASK(2)
#define PG_LEVEL3_MASK             PG_LEVEL_MASK(3)
#define PG_LEVEL4_MASK             PG_LEVEL_MASK(4)
#define PG_LEVEL_OFFSET(level, va) ((va) >> PG_LEVEL_SHIFT(level)) & PG_TAB_MASK)
#define PG_ENT_PPN_MASK            0x3FFFFFFFFFFC00UL
/*
 * PPN is in bits [10:53].
 */
#define PG_ENT_PPN_SHIFT           2
#define PG_ENT_VALID               (1UL << 0)
#define PG_ENT_READ                (1UL << 1)
#define PG_ENT_WRITE               (1UL << 2)
#define PG_ENT_EXECUTE             (1UL << 3)

DECLARE_AABI_CALL(void, sbi_putchar, 1, 0)(char ch);
int printf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
