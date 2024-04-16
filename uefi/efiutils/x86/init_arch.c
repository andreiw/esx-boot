/*******************************************************************************
 * Copyright (c) 2018 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * init_arch.c -- Architecture-specific EFI Firmware init/cleanup functions.
 */

#include <bootlib.h>
#include "efi_private.h"

/*-- sanitize_page_tables ------------------------------------------------------
 *
 *      Validate and transform MMU configuration to the state expected by
 *      allocate_page_tables and relocate_page_tables1/2.
 *
 *      This does nothing on x86.
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
   return ERR_SUCCESS;
}
