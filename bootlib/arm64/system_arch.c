/*******************************************************************************
 * Copyright (c) 2017-2019,2022 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * system_arch.c -- Various architecture-specific system routines.
 */

#include <bootlib.h>

/*-- system_arch_blacklist_memory ----------------------------------------------
 *
 *      Blacklist architecture-specific memory ranges.
 *
 * Parameters
 *      None.
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
int system_arch_blacklist_memory(void)
{
   return ERR_SUCCESS;
}
