/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"
#include "asm-aabi-decls.sinc"

   .section .rodata
debug_asciz string_magic,       "\nESXBootInfo magic  = 0x"
debug_asciz string_ebi,         "\nESXBootInfo struct = 0x"
debug_asciz string_link_va,     "\nLink VA            = 0x"
debug_asciz string_load_va,     "\nLoad VA            = 0x"
debug_asciz string_stack,       "\nStack pointer      = 0x"
debug_asciz string_go_c,        "\nCalling C...\n"
string_panic: .asciz            "\nPanic RA=0x"

   .section .text
/*
 * Entry point into the "kernel".
 *
 * MMU may be on (with VA == PA).
 *
 * Inputs:
 *    a0: ESXBOOTINFO_MAGIC_V2
 *    a1: physical address of the ESXBootInfo structure.
 */
   .global entry
entry:
/*
 * Code below extensively uses stack, so set this up ASAP.
 */
   la   sp, _stack_end
   mv   t0, sp
   debug_string_reg   str=string_stack, reg=t0

/*
 * Save inputs.
 */
   mv   s11, a0
   debug_string_reg   str=string_magic, reg=s11
   mv   s10, a1
   debug_string_reg   str=string_ebi, reg=s10

/*
 * Run relocs. The kernel could be loaded anywhere with
 * the requested alignment, but expects to run at LINK_ADDRESS,
 * so the computed "extra addend" is &entry - LINK_ADDRESS.
 */
   li   t0, LINK_ADDRESS
   debug_string_reg   str=string_link_va, reg=t0
   la   t1, entry
   debug_string_sym   str=string_load_va, sym=entry
   sub  t1, t1, t0
   mv   a0, t1
   aabi_call name=relocs_run, nargs=1, nrets=0

/*
 * Clear .bss.
 */
   la   t0, _bss_start
   la   t1, _bss_end
1: beq  t0, t1, 2f
   sb   zero, 0(t0)
   addi t0, t0, 1
   j    1b
2:

/*
 * Call C.
 */
   mv   t0, sp
   debug_string_reg str=string_stack, reg=t0
   debug_string string_go_c
   mv   a0, s10
   cabi_call c_main
   aabi_call name=sbi_shutdown, nargs=0, nrets=0

/*
 * Fatal error handling.
 *
 * Outputs: Doesn't return.
 * Clobbers: a0, a1.
 */
aabi_defun_global name=panic, nargs=0, nrets=0
   mv  a1, ra
   la  a0, string_panic
   aabi_call name=sbi_putstring, nargs=1, nrets=0
   mv  a0, a1
   aabi_call name=sbi_puthex64, nargs=1, nrets=0
   aabi_call name=sbi_shutdown, nargs=0, nrets=0
aabi_retfun
