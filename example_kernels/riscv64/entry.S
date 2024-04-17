/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"
#include "asm-env.sinc"
#include "asm-aabi-decls.sinc"

   .section .rodata
debug_asciz string_magic,       "\nESXBootInfo magic = 0x"
debug_asciz string_ebi,         "\nESXBootInfo struct = 0x"
debug_asciz string_link_va,     "\nLink VA = 0x"
debug_asciz string_load_va,     "\nLoad VA = 0x"
debug_asciz string_stack,       "\nStack pointer = 0x"
debug_asciz string_uart_base    "\nMMIO UART = 0x"
debug_asciz string_go_c,        "\nCalling C...\n"
string_panic: .asciz            "\nPanic RA=0x"

   .section .text
/*
 * Entry point into the "kernel".
 *
 * Loaded anywhere that's aligned to the expose FEAT_LOAD_OPTIONS
 * (see kernel.c) - _NOT_ at the linked address.
 *
 * MMU may be on, in which case VA may != PA, but
 * mboot always hands off with VA == PA.
 *
 * If MMU enabled:
 *  - If VA != PA, VA = PA + ESXBootInfo_CpuMode::pa_2_va_addend.
 *  - ESXBootInfo structure is mapped. Addresses embedded in the
 *    EBI structrs don't have to be mapped, but the elmts[] array
 *    should be traversable without fault.
 *
 * Inputs:
 *    a0: ESXBOOTINFO_MAGIC_V2
 *    a1: virtual address of the ESXBootInfo structure.
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
 * Exception handling.
 */
  aabi_call name=trap_init, nargs=0, nrets=0

/*
 * Initialize MMU with identity mapping of the kernel.
 */
  aabi_call name=mmu_init, nargs=0, nrets=0

/*
 * Example of locating an EBI structure, in this case
 * the UART described by ESXBootInfo_SerialCon.
 */
   mv   a0, s10
   li   a1, ESXBOOTINFO_SERIAL_CON_TYPE
   aabi_call name=ebi_find, nargs=2, nrets=1
   beqz a0, 1f
   /*
    * Found ESXBootInfo_SerialCon.
    */
   l_asmc_field dest=t0, name=ESXBootInfo_SerialCon_space, base=a0
   li   t1, ESXBOOTINFO_SERIAL_CON_SPACE_MMIO
   bne  t0, t1, 1f
   /*
    * It's an MMIO one - map it identity.
    */
   l_asmc_field dest=a0, name=ESXBootInfo_SerialCon_base, base=a0
   debug_string_reg str=string_uart_base, reg=a0
   aabi_call name=mmu_map_device, nargs=1, nrets=0
1:

/*
 * Call C.
 */
   mv   t0, sp
   debug_string_reg str=string_stack, reg=t0
   debug_string string_go_c
   mv   a0, s10
   cabi_call c_main

/*
 * Break on purpose.
 */
   ebreak

/*
 * Fatal error handling.
 *
 * Outputs: Doesn't return.
 */
aabi_defun_global name=panic, nargs=0, nrets=0
   mv  a1, ra
   la  a0, string_panic
   aabi_call name=sbi_putstring, nargs=1, nrets=0
   mv  a0, a1
   aabi_call name=sbi_puthex64, nargs=1, nrets=0
   aabi_call name=sbi_shutdown, nargs=0, nrets=0
aabi_retfun
