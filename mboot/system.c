/*******************************************************************************
 * Copyright (c) 2008-2017,2020 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * system.c -- Various system routines.
 */

#include <cpu.h>
#include <boot_services.h>
#include "mboot.h"

int dump_firmware_info(void)
{
   const char *manufacturer;
   const char *product;
   const char *bios_ver;
   const char *bios_date;

   firmware_t firmware;
   int status;

   status = get_firmware_info(&firmware);
   if (status != ERR_SUCCESS) {
      Log(LOG_ERR, "Firmware detection failure.\n");
      return status;
   }

#if FORCE_STACK_SMASH
   Log(LOG_WARNING, "about to smash stack");
   (&firmware)[1] = firmware;
   Log(LOG_WARNING, "smashed stack");
#endif

   switch (firmware.interface) {
      case FIRMWARE_INTERFACE_EFI:
         Log(LOG_DEBUG, "%s v%u.%u (%s, Rev.%u)\n",
             (firmware.version.efi.major > 1) ? "UEFI" : "EFI",
             (uint32_t)firmware.version.efi.major,
             (uint32_t)firmware.version.efi.minor,
             ((firmware.vendor != NULL) ? firmware.vendor : "Unknown vendor"),
             firmware.revision);
         break;
      case FIRMWARE_INTERFACE_COM32:
         Log(LOG_DEBUG, "COM32 v%u.%u (%s)\n",
             (uint32_t)firmware.version.com32.major,
             (uint32_t)firmware.version.com32.minor,
             ((firmware.vendor != NULL) ?
              firmware.vendor : "Unknown derivative"));
         break;
      default:
         Log(LOG_WARNING, "Unknown firmware\n");
   }

   sys_free(firmware.vendor);

   if (smbios_get_platform_info(&manufacturer, &product, &bios_ver,
                                &bios_date) == ERR_SUCCESS) {
      SANITIZE_STRP(manufacturer);
      SANITIZE_STRP(product);
      SANITIZE_STRP(bios_ver);
      SANITIZE_STRP(bios_date);
      Log(LOG_DEBUG, "'%s' by '%s', firmware version '%s', built on '%s'\n",
          product, manufacturer, bios_ver, bios_date);
   }

   return ERR_SUCCESS;
}

/*-- firmware_shutdown ---------------------------------------------------------
 *
 *     Shutdown the boot services:
 *       - Get the run-time E820 memory map (and request some extra memory for
 *         converting it later to the possibly bigger ESXBootInfo or Multiboot
 *         format).
 *
 *       - Record some EFI-specific information if possible.
 *
 *       - Claim that we no longer need the firmware boot services.
 *
 *       - Disable hardware interrupts. Since firmware services have been shut
 *         down, it is no longer necessary to run firmware interrupt handlers.
 *         After this function is called, it is safe to clobber the IDT and GDT.
 *
 * Parameters
 *      OUT mmap:     pointer to the freshly allocated memory map
 *      OUT count:    number of entries in the memory map
 *      OUT efi_info: EFI information if available
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *
 * Side Effects
 *      No call to the boot services may be done after a call to this function.
 *----------------------------------------------------------------------------*/
int firmware_shutdown(e820_range_t **mmap, size_t *count, efi_info_t *efi_info)
{
   size_t desc_extra_mem;
   int status;

   if (boot_mmap_desc_size() > sizeof (e820_range_t)) {
      desc_extra_mem = boot_mmap_desc_size() - sizeof (e820_range_t);
   } else {
      desc_extra_mem = 0;
   }

   status = exit_boot_services(desc_extra_mem, mmap, count, efi_info);
   if (status != ERR_SUCCESS) {
      Log(LOG_ERR, "Failed to shutdown the boot services.\n");
      return status;
   }

   CLI();

   Log(LOG_DEBUG, "Scanning system tables...");

   e820_mmap_merge(*mmap, count);

   status = system_blacklist_memory(*mmap, *count);
   if (status != ERR_SUCCESS) {
      Log(LOG_ERR, "Error scanning system memory: %s\n", error_str[status]);
      return status;
   }

   return ERR_SUCCESS;
}
