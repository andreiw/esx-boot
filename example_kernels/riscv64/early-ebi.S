/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"
#include "asm-aabi-decls.sinc"

/*
 * Given a pointer to the ESXBootInfo and an ESXBootInfo_Type,
 * return the address to the first matching ESXBootInfo_Elmt.
 *
 * Inputs:
 *    a0: address of ESXBootInfo.
 *    a1: ESXBootInfo_Type.
 * Outputs: address of matching ESXBootInfo_Elmt or 0.
 */
aabi_defun_global name=ebi_find, nargs=2, nrets=1
   l_asmc_field dest=t0, name=ESXBootInfo_numESXBootInfoElmt, base=a0
   addi t1, a0, OF_ESXBootInfo_elmts
   mv   a0, zero
1: /*
    & Any more elements left?
    */
   beqz t0, 3f
   /*
    * There is at least another element remaining, pointed
    * to by t1.
    */
   l_asmc_field dest=t2, name=ESXBootInfo_Elmt_type, base=t1
   beq  t2, a1, 2f
   /*
    * Type of element doesn't match, skip this element
    * and go to the next one.
    */
   l_asmc_field dest=t2, name=ESXBootInfo_Elmt_elmtSize, base=t1
   add  t1, t1, t2
   addi t0, t0, -1
   j    1b
2: /*
    * Have match!
    */
   mv   a0, t1
3: aabi_retfun
