/*******************************************************************************
 * Copyright (c) 2022 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * esxbootinfo_arch.c -- Arch-specific portions of ESXBootInfo.
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <esxbootinfo.h>
#include <stdbool.h>
#include "mboot.h"

/*
 * When loading an ELF "anywhere", use a 2MB alignment.
 */
#define ELF_EXEC_ALLOC_ALIGNMENT 0x200000

/*-- esxbootinfo_arch_v1_supported_req_flags -----------------------------------
 *
 *      Extra arch-specific supported required flags.
 *
 * Parameters
 *      None.
 *
 * Results
 *      0.
 *----------------------------------------------------------------------------*/
uint32_t esxbootinfo_arch_v1_supported_req_flags(void)
{
   return 0;
}

/*-- esxbootinfo_arch_check_kernel----------------------------------------------
 *
 *      Extra arch-specific kernel checks.
 *
 * Parameters
 *      IN mbh: ESXBootInfo header.
 *
 * Results
 *      False if kernel is not supported (with error logged).
 *----------------------------------------------------------------------------*/
bool esxbootinfo_arch_check_kernel(UNUSED_PARAM(ESXBootInfo_Header *mbh))
{
   if (mbh->magic == ESXBOOTINFO_MAGIC_V1) {
      boot.kernel_load_align = ELF_EXEC_ALLOC_ALIGNMENT;
   }

   return true;
}
