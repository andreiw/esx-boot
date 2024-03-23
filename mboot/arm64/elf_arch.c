/*******************************************************************************
 * Copyright (c) 2017-2018,2022 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * elf_arch.c -- Architecture-specific ELF handling.
 */

#include <elf.h>
#include "mboot.h"

/*-- elf_arch_supported --------------------------------------------------------
 *
 *      Validates ELF header against architecture requirements.
 *
 * Parameters
 *      IN buffer: pointer to the ELF binary buffer
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
int elf_arch_supported(void *buffer)
{
   Elf_CommonEhdr *ehdr = buffer;

   if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
      return ERR_BAD_ARCH;
   }

   if (Elf_CommonEhdrGetMachine(ehdr) != EM_AARCH64 ||
       ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
      return ERR_BAD_ARCH;
   }

   if (Elf_CommonEhdrGetType(ehdr) != ET_EXEC &&
       Elf_CommonEhdrGetType(ehdr) != ET_DYN) {
      return WARNING(ERR_NOT_EXECUTABLE);
   }

   return ERR_SUCCESS;
}

/*-- elf_arch_alloc_option -----------------------------------------------------
 *
 *      Option to use in runtime_alloc by elf_alloc_anywhere.
 *
 * Results
 *      ALLOC_ANY or ALLOC_32BIT.
 *----------------------------------------------------------------------------*/
int elf_arch_alloc_option(void)
{
   return ALLOC_ANY;
}
