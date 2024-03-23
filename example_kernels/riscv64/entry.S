#define ESXBOOTINFO_MAGIC     0x1BADB005
#define ESXBOOTINFO_ALIGNMENT 8
#define ESXBOOTINFO_FLAGS     0

   .section .data
string_entry_ma: .asciz "\nEntry MA = 0x"
string_bye:      .asciz "\nCalling shutdown...\n"

   .section .text.entry

/*
 * Entry point into the "kernel".
 *
 * MMU may be on.
 *
 * Inputs:
 *    a0: ESXBOOTINFO_MAGIC
 *    a1: physical address of the ESXBootInfo structure.
 */
   .global entry
entry:
   j    stack_setup

/*
 * The ESXBootInfo_Header passed statically from kernel to bootloader,
 * must be located within the first ESXBOOTINFO_SEARCH bytes of
 * the lowest loaded ELF segment.
 */
   .balign ESXBOOTINFO_ALIGNMENT
   .long   ESXBOOTINFO_MAGIC
   .long   ESXBOOTINFO_FLAGS
   .long   (-(ESXBOOTINFO_MAGIC + ESXBOOTINFO_FLAGS))
   .long   0 /* reserved[0] */
   .long   0 /* reserved[1] */
   .long   0 /* min_width */
   .long   0 /* min_height */
   .long   0 /* min_depth */
   .long   0 /* mode_type */
   .long   0 /* width */
   .long   0 /* height */
   .long   0 /* depth */
   .long   0 /* rts_vaddr */
   .long   0 /* rts_size */
   .long   0 /* loadesx_version */
   .long   0 /* tpm_measure */

stack_setup:
   la   sp, stack_end

show_info:
   la   a0, string_entry_ma
   jal  sbi_putstring

   la   a0, entry
   jal  sbi_puthex64

shutdown:
   la   a0, string_bye
   jal  sbi_putstring
   call sbi_shutdown

   .section .text

/*
 * Prints a char using SBI console.
 *
 * Inputs:
 *    a0: Character to print.
 * Outputs: N/A.
 * Clobbers: a0, a1, a6, a7.
 */
sbi_putchar:
   li   a6, 0
   li   a7, 1
ecall:
   ecall
   ret

/*
 * Prints a string using SBI console.
 *
 * Inputs:
 *    a0: ASCII string to print.
 * Outputs: N/A.
 * Clobbers: a0, a1, a6, a7.
 */
sbi_putstring:
   mv      a1, a0
1: lbu     a0, (a1)
   beqz    a0, 2f
   li      a6, 0
   li      a7, 1
   ecall
   addi    a1, a1, 1
   j       1b
2: ret

/*
 * Prints a 64-bit unsigned integer using hex chars.
 *
 * Inputs:
 *    a0: Value to print.
 * Outputs: N/A.
 * Clobbers: a0, a1, a2, a3, a6, a7.
 */
sbi_puthex64:
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
   ret

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
   j    ecall

   .section .bss
   .align 12
stack:
   .fill 4096
stack_end:
