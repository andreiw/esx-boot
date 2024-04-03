/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "asm-constants.h"

OUTPUT_ARCH("riscv")
ENTRY(entry)

PHDRS {
   lo_rx PT_LOAD FLAGS (5); /* RX */
   lo_r  PT_LOAD FLAGS (4); /* R  */
   lo_rw PT_LOAD FLAGS (6); /* RW */
}

SECTIONS
{
   . = LINK_ADDRESS;
   .text (LINK_ADDRESS) :
   {
      *(.text.entry)
      *(.text .text.*)
   } : lo_rx

   .rodata :
   {
      *(.rodata .rodata.*)
   } : lo_r

   .rela   :
   {
      _rela_start = .;
      *(.rela)
      *(.rela.*)
      _rela_end = .;
   } : lo_rw

   .data   :
   {
      *(.data .data.*)
      *(.got)
   } : lo_rw

   .bss    :
   {
      _bss_start = .;
      *(.bss .bss.*)
      . = ALIGN(PAGE_SIZE);
      _stack_start = .;
      . += PAGE_SIZE * STACK_PAGES;
      _stack_end = .;
      _early_pages_start = .;
      . += PAGE_SIZE * EARLY_PAGES;
      _early_pages_end = .;
      _bss_end = .;
   } : lo_rw

   .got.plt : { *(.got.plt) }
   .plt :
   {
      *(.plt) *(.plt.*) *(.iplt) *(.igot .igot.plt)
   }
   .data.rel.ro : { *(.data.rel.ro) }
   ASSERT(SIZEOF(.got.plt) == 0 || SIZEOF(.got.plt) == 0x10, "Unexpected .got.plt size")
   ASSERT(SIZEOF(.plt) == 0, ".plt not 0")
   ASSERT(SIZEOF(.data.rel.ro) == 0, ".data.rel.ro not 0")

   /DISCARD/ : { *(.interp) *(.dynamic) *(.note.gnu.build-id) *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
