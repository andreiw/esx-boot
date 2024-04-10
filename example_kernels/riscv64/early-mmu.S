/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"
#include "asm-aabi-decls.sinc"

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
   macro_arg_is_not \entry, \entry_va, "entry cannot be entry_va"
   aabi_call name=early_page_alloc, nargs=0, nrets=1
   .if \entry != a0
      mv   \entry, a0
   .endif
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

/*
 * Enable the MMU, mapping the kernel (only).
 *
 * Automatically detects Sv57, Sv48 and Sv39.
 */
aabi_defun_global name=mmu_init, nargs=0, nrets=0
   push s11
   push s10
   push s9
   push s8
   /*
    * L5 (root) in s11.
    */
   aabi_call name=early_page_alloc, nargs=0, nrets=1
   mv   s11, a0
   /*
    * Map the kernel between _start and _end using megapages,
    * with VA == PA.
    */
   la   s10, _start
   la   s9, _end
map_megapage:
   mv   s8, s11
   debug_string_reg str=string_mapped1, reg=s10
   mmu_va_2_tab tab_va=s8, va=s10, level=4, temp=a0, next_tab_va=s8
   mmu_va_2_tab tab_va=s8, va=s10, level=3, temp=a0, next_tab_va=s8
   mmu_va_2_tab tab_va=s8, va=s10, level=2, temp=a0, next_tab_va=s8
   mmu_va_2_entry tab_va=s8, va=s10, level=1, temp=a0, entry_va=s8
   mmu_map_mega entry_va=s8, ma=s10, temp=a0
   debug_string_dereg_val64 str=string_mapped2, reg=s8
   li   a0, PG_LEVEL1_SIZE
   add  s10, s10, a0
   blt  s10, s9, map_megapage
   /*
    * We don't know how many levels the CPU supports. So we probe.
    *
    * First, build up the SATP values.
    *
    * s11 is the Sv57 (L4) root SATP value.
    * s10 is the Sv48 (L3) root SATP value.
    * s9 is the Sv39 (L2) root SATP value.
    */
   srli s11, s11, PG_BASE_SHIFT
   addi s10, s11, 1
   addi s9, s10, 1
   li   a0, SATP_SV57
   or   s11, s11, a0
   li   a0, SATP_SV48
   or   s10, s10, a0
   li   a0, SATP_SV39
   or   s9, s9, a0
   debug_string_reg str=string_satp57, reg=s11
   debug_string_reg str=string_satp48, reg=s10
   debug_string_reg str=string_satp39, reg=s9
   /*
    * Try setting SATP. If the value doesn't stick, try the
    * next value.
    */
   csrw satp, s11
   csrr a0, satp
   beq  a0, s11, done
   csrw satp, s10
   csrr a0, satp
   beq  a0, s10, done
   csrw satp, s9
   csrr a0, satp
   beq  a0, s9, done
   fatal string_no_satp
done:
   sfence.vma
   debug_string_reg str=string_satp, reg=a0
   pop s8
   pop s9
   pop s10
   pop s11
aabi_retfun
