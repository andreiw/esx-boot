/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"

   .section .data
early_pages_next: .long _early_pages_start

   .section .rodata
debug_asciz string_early_pages, "\nOut of early pages!"

   .section .text
/*
 * Allocates an early page, if possible.
 *
 * On success, advances early_page_next.
 * Panics on failure.
 *
 * Outputs:
 *    a0: VA of allocated page.
 */
aabi_defun_global early_page_alloc, nargs=0, nrets=1
   la   t0, early_pages_next
   ld   a0, (t0)
   la   t1, _early_pages_end
   bne  a0, t1, 1f
   fatal string_early_pages   
1:
   li   t2, PG_BASE_SIZE
   add  t1, a0, t2
   sd   t1, (t0)
aabi_retfun
