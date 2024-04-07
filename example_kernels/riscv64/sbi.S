/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"

   .section .rodata
string_bye:   .asciz            "\nCalling shutdown...\n"

   .section .text
/*
 * Prints a char using SBI console.
 *
 * Inputs:
 *    a0: Character to print.
 */
aabi_defun_global name=sbi_putchar, nargs=1, nrets=0
   li   a6, 0
   li   a7, 1
   ecall
aabi_retfun

/*
 * Prints a string using SBI console.
 *
 * Inputs:
 *    a0: ASCII string to print.
 */
aabi_defun_global name=sbi_putstring, nargs=1, nrets=0
/*
 * This function is a bit special, because
 * it is used by the debug_ macros, which
 * are designed to be used anywhere (we have
 * a stack). a0 is already saved by the
 * debug_string* caller. The rest are saved
 * here. Under C ABI none would have to be
 * saved.
 */
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
aabi_retfun

/*
 * Prints a 64-bit unsigned integer using hex chars,
 * followed by a single space char.
 *
 * Inputs:
 *    a0: Value to print.
 */
aabi_defun_global name=sbi_puthex64, nargs=1, nrets=0
/*
 * This function is a bit special, because
 * it is used by the debug_ macros, which
 * are designed to be used anywhere (we have
 * a stack). a0 is already saved by the
 * debug_string* caller. The rest are saved
 * here. Under C ABI none would have to be
 * saved.
 */
   push    a1
   push    a2
   push    a3
   push    a6
   push    a7
   mv      a1, a0
   li      a2, 60
   li      a3, '9'
   li      a6, 0
   li      a7, 1
1: srl     a0, a1, a2
   andi    a0, a0, 0xf
   addi    a0, a0, '0'
   bleu    a0, a3, 2f
   addi    a0, a0, 'a' - ('9' + 1)
2: ecall
   addi    a2, a2, -4
   bgez    a2, 1b
   li      a0, ' '
   ecall
   pop     a7
   pop     a6
   pop     a3
   pop     a2
   pop     a1
aabi_retfun

/*
 * Shutdown the system.
 *
 * Outputs: Doesn't return.
 */
aabi_defun_global name=sbi_shutdown, nargs=0, nrets=0
   la   a0, string_bye
   aabi_call name=sbi_putstring, nargs=1, nrets=0
   li   a6, 0
   li   a7, 8
   ecall
aabi_retfun
