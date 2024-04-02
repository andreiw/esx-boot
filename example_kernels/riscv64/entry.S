/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"

/*
 * Given a link-time address in \reg, modify \reg
 * to reflect the load-time (actual) address, by
 * adding the computed addend from t4.
 */
.macro link_to_load reg
   add \reg, \reg, t4
.endm

   .section .rodata
string_magic:        .asciz "\nESXBootInfo magic  = 0x"
string_ebi:          .asciz "\nESXBootInfo struct = 0x"
string_link_va:      .asciz "\nLink VA            = 0x"
string_load_va:      .asciz "\nLoad VA            = 0x"
string_addend:       .asciz "\nAddend (link-load) = 0x"
string_rela_start:   .asciz "\n_rela_start        = 0x"
string_rela_end:     .asciz "\n_rela_end          = 0x"
string_rela_offset:  .asciz "\nr_offset = 0x"
string_rela_info:    .asciz " r_info = 0x"
string_rela_addend:  .asciz " r_addend = 0x"
string_rela_fixed:   .asciz " fixed -> 0x"
string_rela_skipped: .asciz " skipped?"
string_stack:        .asciz "\nStack pointer      = 0x"
string_go_c:         .asciz "\nCalling C...\n"
string_bye:          .asciz "\nCalling shutdown...\n"

   .section .text

/*
 * Entry point into the "kernel".
 *
 * MMU may be on.
 *
 * Inputs:
 *    a0: ESXBOOTINFO_MAGIC_V2
 *    a1: physical address of the ESXBootInfo structure.
 */
   .global entry
entry:
   mv   t0, a0
   mv   t1, a1
/*
 * Compute the addend (link-load) into t4.
 */
   li   t3, LINK_ADDRESS
   la   t4, entry
   sub  t4, t4, t3

/*
 * Code below extensively uses stack, so set this up ASAP.
 */
stack_setup:
   la   sp, stack_end

show_info:
   debug_string_reg   str=string_stack, reg=sp
   debug_string_reg   str=string_magic, reg=t0
   debug_string_reg   str=string_ebi, reg=t1
   debug_string_reg   str=string_link_va, reg=t3
   debug_string_sym   str=string_load_va, sym=entry
   debug_string_reg   str=string_addend, reg=t4
   debug_string_sym   str=string_rela_start, sym=_rela_start
   debug_string_sym   str=string_rela_end, sym=_rela_end

clear_bss:
   la   a0, _bss_start
   la   a1, _bss_end
1: beq  a0, a1, relocs_run
   sb   zero, 0(a0)
   addi a0, a0, 1
   j    1b

relocs_run:
   la   s0, _rela_start
   la   s1, _rela_end
1: beq  s0, s1, relocs_done
   ld   s2, OF_elf64_rela_r_offset(s0)
   ld   s3, OF_elf64_rela_r_info(s0)
   ld   s4, OF_elf64_rela_r_addend(s0)
   debug_string_reg str=string_rela_offset, reg=s2
   debug_string_reg str=string_rela_info,   reg=s3
   debug_string_reg str=string_rela_addend, reg=s4
   addi s0, s0, SZ_elf64_rela
   li   a0, R_RISCV_RELATIVE
   bne  s3, a0, rela_skip
rela_fix:
   link_to_load reg=s2
   ld   s3, (s2)
   add  s3, s3, s4
   link_to_load reg=s3
   sd   s3, (s2)
   debug_string_reg str=string_rela_fixed, reg=s2
   j    1b
rela_skip:
   debug_string str=string_rela_skipped
   j    1b

relocs_done:
   debug_string_reg str=string_stack, reg=sp
   debug_string     str=string_go_c
   mv   a0, t1
   call_c c_main

shutdown:
   debug_string str=string_bye
   call sbi_shutdown

   .section .text

/*
 * Prints a char using SBI console.
 *
 * Inputs:
 *    a0: Character to print.
 * Outputs: N/A.
 * Clobbers: Nothing.
 */
global_defun sbi_putchar
   push a0
   push a1
   push a6
   push a7
   li   a6, 0
   li   a7, 1
   ecall
   pop  a7
   pop  a6
   pop  a1
   pop  a0
retfun

/*
 * Prints a string using SBI console.
 *
 * Inputs:
 *    a0: ASCII string to print.
 * Outputs: N/A.
 * Clobbers: Nothing.
 */
defun sbi_putstring
   push    a0
   push    a1
   push    a6
   push    a7
   mv      a1, a0
1: lbu     a0, (a1)
   beqz    a0, 2f
   li      a6, 0
   li      a7, 1
   ecall
   addi    a1, a1, 1
   j       1b
2: pop     a7
   pop     a6
   pop     a1
   pop     a0
retfun

/*
 * Prints a 64-bit unsigned integer using hex chars.
 *
 * Inputs:
 *    a0: Value to print.
 * Outputs: N/A.
 * Clobbers: Nothing.
 */
defun sbi_puthex64
   push    a0
   push    a1
   push    a2
   push    a3
   push    a6
   push    a7
   mv      a1, a0
   li      a2, 60
   li      a3, 58
   li      a6, 0
   li      a7, 1
1: srl     a0, a1, a2
   andi    a0, a0, 0xf
   addi    a0, a0, 48
   bltu    a0, a3, 2f
   addi    a0, a0, 39
2: ecall
   addi    a2, a2, -4
   bgez    a2, 1b
   pop     a7
   pop     a6
   pop     a3
   pop     a2
   pop     a1
   pop     a0
retfun

/*
 * Shutdown the system.
 *
 * Inputs: N/A.
 * Outputs: Doesn't return.
 * Clobbers: N/A.
 */
sbi_shutdown:
   li   a6, 0
   li   a7, 8
   ecall

   .section .bss
   .align 12
stack:
   .fill 4096
stack_end:
