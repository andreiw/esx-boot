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
   bnez \temp, 1f
   /*
    * Need to allocate.
    */   
   mmu_entry_new_tab entry_va=\entry_va, entry=\temp
   /*
    * Entry exists (is not zero), so extract
    * the address inside. Since this code runs
    * with 1:1 mapping, VA == PA.
    */
1: mv   \next_tab_va, \temp
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
 * Given a page table entry
 */
.macro mmu_map_mega entry_va, ma, temp
   li   \temp, PAGE_MASK_INVERSE
   and  \temp, \temp, \ma
   srli \temp, \temp, PG_ENT_PPN_SHIFT
   ori  \temp, \temp, PG_ENT_VALID
   ori  \temp, \temp, PG_ENT_READ
   ori  \temp, \temp, PG_ENT_WRITE
   ori  \temp, \temp, PG_ENT_EXECUTE
   sd   \temp, (\entry_va)
.endm

   .section .rodata
debug_asciz string_mapped1  "\nMapped 0x"
debug_asciz string_mapped2  "with PTE 0x"

   .section .text
defun_global mmu_init
   push a0
   push a1
   push a2
   push a3
   /*
    * L5 (root).
    */
   jal  early_page_alloc
   mv   a3, a0
   /*
    * Map the kernel using 1 megapage, with VA == PA.
    */
   la   a1, entry
   mmu_va_2_tab tab_va=a0, va=a1, level=4, temp=a2, next_tab_va=a0
   mmu_va_2_tab tab_va=a0, va=a1, level=3, temp=a2, next_tab_va=a0
   mmu_va_2_tab tab_va=a0, va=a1, level=2, temp=a2, next_tab_va=a0
   mmu_va_2_entry tab_va=a0, va=a1, level=1, temp=a2, entry_va=a0
   mmu_map_mega entry_va=a0, ma=a1, temp=a2
   debug_string_reg str=string_mapped1, reg=a1
   debug_string_dereg_val64 str=string_mapped2, reg=a0
   pop a3
   pop a2
   pop a1
   pop a0
retfun
