/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"

/*
 * Necessary as C processor statements in asm-constants.h
 * are applied *before* 'as' macros are evaluated.
 */
.equ PLS0, PG_LEVEL0_SHIFT
.equ PLS1, PG_LEVEL1_SHIFT
.equ PLS2, PG_LEVEL2_SHIFT
.equ PLS3, PG_LEVEL3_SHIFT
.equ PLS4, PG_LEVEL4_SHIFT

/*
 * Given a page table of \level, compute the address
 * of the page table entry corresponding to \va.
 *
 * Hint: \tab_va and \entry_va can be the same register.
 *
 * Inputs:
 *    \tab_va: Register containing page table VA.
 *    \va:     Register containing VA for lookup.
 *    \level:  Immediate between 0 and 4.
 * Outputs:  \entry_va, address of page table entry.
 * Clobbers: \temp, scratch register.
 */
.macro mmu_va_2_entry tab_va, va, level, temp, entry_va
   macro_arg_is_not \tab_va, \temp, "tab_va cannot be temp"
   srli \temp, \va, PLS\level
   /*
    * Compute entry index within \tab_ve.
    */
   andi \temp, \temp, PG_TAB_MASK
   /*
    * Each entry is 8 bytes.
    */
   slli \temp, \temp, 3
   add  \entry_va, \tab_va, \temp
.endm

/*
 * Given address of an empty page table entry,
 * fill it with a new allocated page table.
 *
 * Inputs:
 *    \entry_va: Address of a page table entry.
 * Output: \entry, filled page table entry.
 */
.macro mmu_entry_new_tab entry_va, entry
   macro_arg_is_not \entry, a0, "entry cannot be a0"
   macro_arg_is_not \entry, \entry_va, "entry cannot be entry_va"
   push a0
   jal  early_page_alloc
   mv   \entry, a0
   pop  a0
   srli \entry, \entry, PG_ENT_PPN_SHIFT
   ori  \entry, \entry, PG_ENT_VALID
   sd   \entry, (\entry_va)
.endm

/*
 * Given the address for page table entry pointing to
 * the next page table level, compute the address
 * of the described table.
 *
 * Hint: \va and \next_tab_va can be the same register.
 *
 * Inputs:
 *    \va: Register containing page table entry VA.
 * Outputs: \next_tab_va, Address of next level page table.
 * Clobbers: \temp, scratch register.
 */
.macro mmu_entry_2_tab entry_va, temp, next_tab_va
   macro_arg_is_not \entry_va, \temp, "entry_va cannot be temp"
   macro_arg_is_not \next_tab_va, \temp, "next_tab_va cannot be temp"
   ld   \temp, (\entry_va)
   bnez \temp, m\@_0
   /*
    * Need to allocate.
    */
   mmu_entry_new_tab entry_va=\entry_va, entry=\temp
   /*
    * Entry exists (is not zero), so extract
    * the address inside. Since this code runs
    * with 1:1 mapping, VA == PA.
    */
m\@_0:
   mv   \next_tab_va, \temp
   li   \temp, PG_ENT_PPN_MASK
   and  \next_tab_va, \next_tab_va, \temp
   slli \next_tab_va, \next_tab_va, PG_ENT_PPN_SHIFT
.endm

/*
 * Given a page table of \level, return the table of level - 1
 * corresponding to the \va.
 *
 * Hint: \tab_va and \next_tab_va can be the same registers.
 *
 * Inputs:
 *    \tab_va: Register containing page table VA.
 *    \va:     Register containing VA for lookup.
 *    \level:  Immediate between 0 and 4.
 * Outputs:  \next_tab_va, address of next level page table.
 * Clobbers: \temp, scratch register.
 */
.macro mmu_va_2_tab tab_va, va, level, temp, next_tab_va=table
   mmu_va_2_entry tab_va=\tab_va, va=\va, level=\level, temp=\temp, entry_va=\next_tab_va
   mmu_entry_2_tab entry_va=\next_tab_va, temp=\temp, next_tab_va=\next_tab_va
.endm

/*
 * Given an L1 page table entry, fill it with an RWX
 * mapping for \ma.
 *
 * Inputs:
 *    \entry_va: Register containing L1 page table VA.
 *    \ma:       Register containing 2MiB-aligned MA.
 * Clobbers: \temp, scratch register.
 */
.macro mmu_map_mega entry_va, ma, temp
   macro_arg_is_not \temp, \entry_va, "temp cannot be entry_va"
   macro_arg_is_not \temp, \ma, "temp cannot be ma"
#ifdef DEBUG
   li   \temp, PG_LEVEL1_MASK
   and  \temp, \temp, \ma
   beqz \temp, m\@_0
   fatal string_map_bad_align
m\@_0:
#endif /* DEBUG */
   mv   \temp, \ma
   srli \temp, \temp, PG_ENT_PPN_SHIFT
   ori  \temp, \temp, PG_ENT_VALID
   ori  \temp, \temp, PG_ENT_READ | PG_ENT_WRITE | PG_ENT_EXECUTE
   sd   \temp, (\entry_va)
.endm

   .section .rodata
debug_asciz string_map_bad_align "Unaligned MA for mapping"
debug_asciz string_mapped1       "\nMapping 0x"
debug_asciz string_mapped2       "with PTE 0x"
debug_asciz string_satp57        "\nSv57 SATP = 0x"
debug_asciz string_satp48        "\nSv48 SATP = 0x"
debug_asciz string_satp39        "\nSv39 SATP = 0x"
debug_asciz string_satp          "\nSwitched SATP to 0x"
debug_asciz string_no_satp       "\nUnsupported MMU"

   .section .text
defun_global mmu_init
   push a0
   push a1
   push a2
   push a3
   push a4
   /*
    * L5 (root) in a0.
    */
   jal  early_page_alloc
   la   a1, _start
   la   a2, _end
   /*
    * Map the kernel between _start and _end using megapages,
    * with VA == PA.
    */
map_megapage:
   mv   a3, a0
   debug_string_reg str=string_mapped1, reg=a1
   mmu_va_2_tab tab_va=a3, va=a1, level=4, temp=a4, next_tab_va=a3
   mmu_va_2_tab tab_va=a3, va=a1, level=3, temp=a4, next_tab_va=a3
   mmu_va_2_tab tab_va=a3, va=a1, level=2, temp=a4, next_tab_va=a3
   mmu_va_2_entry tab_va=a3, va=a1, level=1, temp=a4, entry_va=a3
   mmu_map_mega entry_va=a3, ma=a1, temp=a4
   debug_string_dereg_val64 str=string_mapped2, reg=a3
   li   a4, PG_LEVEL1_SIZE
   add  a1, a1, a4
   blt  a1, a2, map_megapage
   /*
    * We don't know how many levels the CPU supports. So we probe.
    *
    * First, build up the SATP values.
    *
    * a0 is the Sv57 (L4) root SATP value.
    * a1 is the Sv48 (L3) root SATP value.
    * a2 is the Sv39 (L2) root SATP value.
    */
   srli a0, a0, PG_BASE_SHIFT
   addi a1, a0, 1
   addi a2, a1, 1
   li   a3, SATP_SV57
   or   a0, a0, a3
   li   a3, SATP_SV48
   or   a1, a1, a3
   li   a3, SATP_SV39
   or   a2, a2, a3
   debug_string_reg str=string_satp57, reg=a0
   debug_string_reg str=string_satp48, reg=a1
   debug_string_reg str=string_satp39, reg=a2
   /*
    * Try setting SATP. If the value doesn't stick, try the
    * next value.
    */
   csrw satp, a0
   csrr a4, satp
   beq  a4, a0, done
   csrw satp, a1
   csrr a4, satp
   beq  a4, a1, done
   csrw satp, a2
   csrr a4, satp
   beq  a4, a2, done
   fatal string_no_satp
done:
   sfence.vma
   debug_string_reg str=string_satp, reg=a4
   pop a4
   pop a3
   pop a2
   pop a1
   pop a0
retfun
