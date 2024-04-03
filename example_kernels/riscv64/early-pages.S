/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"

        .section .data
early_pages_next: .long _early_pages_start

        .section .rodata
debug_asciz string_early_pages, "\nNo early pages!"

        .section .text

/*
 * Allocates an early page, if possible.
 *
 * Inputs: None.
 * Outputs:
 *    a0: VA of allocated page.
 * Clobbers: Modifies early_page_next. Panics if no allocation
 e           possible.
 */
defun_global early_page_alloc
   push a1
   push a2
   push a3     
   la   a1, early_pages_next
   ld   a0, (a1)
   la   a2, _early_pages_end
   bne  a0, a2, 1f
   fatal string_early_pages   
1:
   li   a3, PAGE_SIZE
   add  a2, a0, a3
   sd   a2, (a1)
   pop  a3
   pop  a2
   pop  a1
retfun
