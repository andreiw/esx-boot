/*******************************************************************************
 * Copyright (c) 2022 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * init_arch.c -- Architecture-specific EFI Firmware init/cleanup functions.
 */

#include <bootlib.h>
#include "efi_private.h"
#include <Protocol/RiscVBootProtocol.h>

uint64_t RiscVBootHartId;

/*-- sanitize_page_tables ------------------------------------------------------
 *
 *      Validate and transform MMU configuration to the state expected by
 *      allocate_page_tables and relocate_page_tables1/2.
 *
 *      Ensuring we have the full 4 levels of page tables present.
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
int sanitize_page_tables(void)
{
   return ERR_SUCCESS;
}

/*-- efi_arch_init ------------------------------------------------------------
 *
 *      Arch-specific initialization.
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
int efi_arch_init(void)
{
   EFI_STATUS status;
   RISCV_EFI_BOOT_PROTOCOL *BootProtocol;
   EFI_GUID BootProtocolGuid = RISCV_EFI_BOOT_PROTOCOL_GUID;

   status = LocateProtocol(&BootProtocolGuid, (void **) &BootProtocol);
   if (!EFI_ERROR(status)) {
     BootProtocol->GetBootHartId (BootProtocol, &RiscVBootHartId);
   }

   /*
    * If the above fails for any reason, I guess we leave it as 0.
    */

   return ERR_SUCCESS;
}
