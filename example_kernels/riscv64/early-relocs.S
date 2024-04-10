/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"
#include "asm-aabi-decls.sinc"

/*
 * A kernel built with .rela.dyn relocations requires the relocs
 * to be "run", even if we're not relocating it from the linked
 * address.
 */

   .section .rodata
debug_asciz string_addend,      "\nExtra addend = 0x"
debug_asciz string_rela_start,  "\n_rela_start  = 0x"
debug_asciz string_rela_end,    "\n_rela_end    = 0x"
debug_asciz string_rela_offset, "\nr_offset = 0x"
debug_asciz string_rela_info,   "r_info = 0x"
debug_asciz string_rela_addend, "r_addend = 0x"
debug_asciz string_rela_fixed,  "fixed -> 0x"
debug_asciz string_rela_skipped,"skipped?"

   .section .text
/*
 * "run" through the available relocations, applying
 * an additional addend (e.g. if the intention is to run
 * at loaded address) passed as a parameter.
 *
 * Inputs:
 *    a0: additional addend to apply.
 */
aabi_defun_global name=relocs_run, nargs=1, nrets=0
/*
 * a0 is the extra addend.
 */
   debug_string_reg str=string_addend, reg=a0
   la   t0, _rela_start
   la   t1, _rela_end
1: beq  t0, t1, relocs_done
   ld   t2, OF_elf64_rela_r_offset(t0)
   ld   t3, OF_elf64_rela_r_info(t0)
   ld   t4, OF_elf64_rela_r_addend(t0)
   debug_string_reg str=string_rela_offset, reg=t2
   debug_string_reg str=string_rela_info,   reg=t3
   debug_string_reg str=string_rela_addend, reg=t4
   addi t0, t0, SZ_elf64_rela
   li   t5, R_RISCV_RELATIVE
   bne  t3, t5, rela_skip
rela_fix:
   add  t2, t2, a0
   ld   t3, (t2)
   add  t3, t3, t4
   add  t3, t3, a0
   sd   t3, (t2)
   debug_string_reg str=string_rela_fixed, reg=t2
   j    1b
rela_skip:
   debug_string string_rela_skipped
   j    1b
relocs_done:
aabi_retfun
