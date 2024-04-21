/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * pboot.c -- mystery meat, for now.
 *
 *   pboot [-sS]
 *
 *      OPTIONS
 *         -S <1...4>     Set the default serial port (1=COM1, 2=COM2, 3=COM3,
 *                        4=COM4, 0xNNNN=hex I/O port address ).
 *         -s <BAUDRATE>  Set the serial port speed to BAUDRATE (in bits per
 *                        second).
 */

#include "pboot.h"
#include "zforth.h"
#include "asm-constants.h"

static int serial_com;
static int serial_speed;
static bool log_verbose;
static efi_info_t efi_info;

#define DEFAULT_PROG_NAME       "pboot.efi"

/*-- pboot_parse_args ---------------------------------------------------------
 *
 *      Parse command line options.
 *
 * Parameters
 *      IN argc: number of command line arguments
 *      IN argv: pointer to the command line arguments array
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
static int pboot_parse_args(int argc, char **argv)
{
   int opt;

   if (argc == 0 || argv == NULL || argv[0] == NULL) {
      return ERR_INVALID_PARAMETER;
   }

   serial_com = DEFAULT_SERIAL_COM;
   serial_speed = DEFAULT_SERIAL_BAUDRATE;

   if (argc > 1) {
      optind = 1;
      do {
         opt = getopt(argc, argv, "s:S:V");
         switch (opt) {
            case -1:
               break;
            case 'S':
               serial_com = strtol(optarg, NULL, 0);
               break;
            case 's':
               if (!is_number(optarg)) {
                  return ERR_SYNTAX;
               }
              serial_speed = atoi(optarg);
               break;
            case 'V':
               log_verbose = true;
               break;
            case '?':
            default:
               return ERR_SYNTAX;
         }
      } while (opt != -1);
   }

   return ERR_SUCCESS;
}

/*-- main ---------------------------------------------------------------------
 *
 *      pboot main.
 *
 * Parameters
 *      IN argc: number of command line arguments
 *      IN argv: pointer to the command line arguments array
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
   int status;
   e820_range_t *e820_mmap;
   size_t e820_count;
   uint64_t stvec;

   status = pboot_parse_args(argc, argv);
   if (status != ERR_SUCCESS) {
      Log(LOG_EMERG, "%s: pboot_parse_args: %s", __FUNCTION__, error_str[status]);
      PANIC();
   }

   status = log_init(log_verbose);
   if (status != ERR_SUCCESS) {
      Log(LOG_EMERG, "%s: log_init: %s", __FUNCTION__, error_str[status]);
      PANIC();
   }

   status = serial_log_init(serial_com, serial_speed);
   if (status != ERR_SUCCESS) {
      Log(LOG_EMERG, "%s: serial_log_init: %s", __FUNCTION__,
          error_str[status]);
      PANIC();
   }

   /*
    * No need to log everything twice via serial/video.
    */
   log_unsubscribe(firmware_print);

   check_efi_quirks(&efi_info);

   status = pmem_init(&efi_info);
   if (status != ERR_SUCCESS) {
      Log(LOG_EMERG, "%s: pmem_init failed: %s", __FUNCTION__,
          error_str[status]);
      PANIC();
   }

   status = mon_init();
   if (status != ERR_SUCCESS) {
      Log(LOG_EMERG, "%s mon_init failed: %s",
          __FUNCTION__, error_str[status]);
      PANIC();
   }

   Log(LOG_INFO, "Exiting boot services");
   status = exit_boot_services(0, &e820_mmap, &e820_count, &efi_info);
   if (status != ERR_SUCCESS) {
      Log(LOG_EMERG, "%s: exit_boot_services: %s", __FUNCTION__,
          error_str[status]);
      PANIC();
   }

   e820_mmap_merge(e820_mmap, &e820_count);
   {
      size_t i;

      for (i = 0; i < e820_count; i++) {
         uint64_t base;
         uint64_t len;
         uint64_t limit;

         base = E820_BASE(&e820_mmap[i]);
         len = E820_LENGTH(&e820_mmap[i]);
         limit = base + len - 1;
         Log(LOG_DEBUG, "E820[%zu]: type 0x%08x 0x%016lx - 0x%016lx",
             i,  e820_mmap[i].type, base, limit);
      }
   }

   CSR_READ(stvec, stvec);
   Log(LOG_EMERG, "stvec is 0x%lx\n", stvec);

   mon_enter ();

   return status;
}
