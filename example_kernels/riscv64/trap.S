/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"
#include "asm-aabi-decls.sinc"

   .section .rodata
debug_asciz string_stvec, "\nSTVEC = 0x"
string_trap_scause: .asciz "\nTrap!\nSCAUSE = 0x"
string_trap_sstatus: .asciz "\nSSTATUS = 0x"
string_trap_stval: .asciz "\nSTVAL = 0x"
string_trap_sepc: .asciz "\nSEPC = 0x"
string_trap_ra: .asciz "\nRA = 0x"

   .section .text
/*
 * Aligned for 'direct' mode.
 */
   .global trap_handler
   .balign 4
trap_handler:
/*
 * Assumes we still have a working stack.
 */
    la   a0, string_trap_scause
    aabi_call name=sbi_putstring, nargs=1, nrets=0
    csrr a0, scause
    aabi_call name=sbi_puthex64, nargs=1, nrets=0

    la   a0, string_trap_sstatus
    aabi_call name=sbi_putstring, nargs=1, nrets=0
    csrr a0, sstatus
    aabi_call name=sbi_puthex64, nargs=1, nrets=0

    la   a0, string_trap_stval
    aabi_call name=sbi_putstring, nargs=1, nrets=0
    csrr a0, stval
    aabi_call name=sbi_puthex64, nargs=1, nrets=0

    la   a0, string_trap_ra
    aabi_call name=sbi_putstring, nargs=1, nrets=0
    mv   a0, ra
    aabi_call name=sbi_puthex64, nargs=1, nrets=0
    aabi_call name=sbi_shutdown, nargs=0, nrets=0

/*
 * Initializes exception handling.
 */
aabi_defun_global name=trap_init, nargs=0, nrets=0
    la   a0, trap_handler
    csrw stvec, a0
    debug_string_reg str=string_stvec, reg=a0
aabi_retfun
