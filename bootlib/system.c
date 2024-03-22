/*******************************************************************************
 * Copyright (c) 2008-2017,2020 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * system.c -- Various system routines.
 */

#include <bootlib.h>

EXTERN int system_arch_blacklist_memory(void);

/*-- reserve_sysmem ------------------------------------------------------------
 *
 *      Blacklist system memory so it will not be used later for run-time
 *      relocations.
 *
 * Parameters
 *      IN name: memory region description string
 *      IN addr: pointer to the memory region to be blacklisted
 *      IN size: memory region size
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
static INLINE int reserve_sysmem(const char *name, void *addr, uint64_t size)
{
   Log(LOG_DEBUG, "%s found @ %p (%"PRIu64" bytes)\n", name, addr, size);

   return blacklist_runtime_mem(PTR_TO_UINT64(addr), size);
}

/*-- reserve_smbios_ranges -----------------------------------------------------
 *
 *      Register the SMBIOS memory.
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
static int reserve_smbios_ranges(void *eps_start, size_t eps_length,
                                 void *table_start, size_t table_length)
{
   int status;

   if (!is_valid_firmware_table(eps_start, eps_length)) {
      return reserve_sysmem("SMBIOS: invalid entry point structure",
                            eps_start, eps_length);
   }

   status = reserve_sysmem("SMBIOS: entry point structure", eps_start,
                           eps_length);
   if (status != ERR_SUCCESS) {
      return status;
   }

   status = reserve_sysmem("SMBIOS: table", table_start, table_length);
   if (status != ERR_SUCCESS) {
      return status;
   }

   return ERR_SUCCESS;
}

/*-- scan_smbios_memory --------------------------------------------------------
 *
 *      Register the SMBIOS memory.
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
static int scan_smbios_memory(void)
{
   void *eps_start;
   size_t eps_length;
   void *table_start;
   size_t table_length;
   int status;

   status = smbios_get_info(&eps_start, &eps_length, &table_start, &table_length);
   if (status == ERR_SUCCESS && eps_length != 0) {
      status = reserve_smbios_ranges(eps_start, eps_length, table_start,
                                     table_length);
      if (status != ERR_SUCCESS) {
         Log(LOG_ERR, "Failed to reserve legacy 32-bit SMBIOS ranges\n");
         return status;
      }
   }

   status = smbios_get_v3_info(&eps_start, &eps_length, &table_start, &table_length);
   if (status == ERR_SUCCESS && eps_length != 0) {
      status = reserve_smbios_ranges(eps_start, eps_length, table_start,
                                     table_length);
      if (status != ERR_SUCCESS) {
         Log(LOG_ERR, "Failed to reserve v3 64-bit SMBIOS ranges\n");
         return status;
      }
   }

   /* No SMBIOS found, do nothing. */
   return ERR_SUCCESS;
}

/*-- system_blacklist_memory ---------------------------------------------------
 *
 *      List all the memory ranges that may not be used by the bootloader.
 *
 * Parameters
 *      IN mmap:  pointer to the system memory map
 *      IN count: number of entries in the memory map
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
int system_blacklist_memory(e820_range_t *mmap, size_t count)
{
   int status;

   status = system_arch_blacklist_memory();
   if (status != ERR_SUCCESS) {
      return status;
   }

   status = scan_smbios_memory();
   if (status != ERR_SUCCESS) {
      return status;
   }

   status = e820_to_blacklist(mmap, count);
   if (status != ERR_SUCCESS) {
      return status;
   }

   return ERR_SUCCESS;
}
