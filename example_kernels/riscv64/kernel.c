/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdlib.h>
#include "kernel.h"
#include "../../include/esxbootinfo.h"

#define EBH_FEATS 2
#define EBH_MAGIC ((uint32_t) (-(ESXBOOTINFO_MAGIC_V2 + sizeof (ebh) + EBH_FEATS)))
/*
 * 2MiB (megapage) alignment.
 */
#define LOAD_ALIGNMENT 0x200000

struct ebh_s {
  ESXBootInfo_Header_V2 header;
  ESXBootInfo_FeatSerialCon feat_serial_con;
  ESXBootInfo_FeatLoadAlign feat_load_align;
} __attribute__((packed));

__attribute__((section(".text.ebi,\"a\"#")))
__attribute__((aligned(ESXBOOTINFO_ALIGNMENT)))
struct ebh_s ebh = {
   {
      ESXBOOTINFO_MAGIC_V2,
      sizeof (ebh),
      EBH_FEATS,
      EBH_MAGIC
   },
   {
      ESXBOOTINFO_FEAT_SERIAL_CON_TYPE,
      ESXBOOTINFO_FEAT_REQUIRED,
      sizeof (ESXBootInfo_FeatSerialCon)
   },
   {
      ESXBOOTINFO_FEAT_LOAD_ALIGN_TYPE,
      ESXBOOTINFO_FEAT_REQUIRED,
      sizeof (ESXBootInfo_FeatLoadAlign),
      LOAD_ALIGNMENT
   }
};

const char *hello = "Hello via UART!\n";

void
c_main (ESXBootInfo *ebi)
{
   ESXBootInfo_Elmt *elmt;
   volatile uint8_t *uart_base;

   uart_base = NULL;
   FOR_EACH_ESXBOOTINFO_ELMT_DO(ebi, elmt) {
      switch (elmt->type) {
      case ESXBOOTINFO_SERIAL_CON_TYPE: {
         ESXBootInfo_SerialCon *con = (void *) elmt;
         if (con->con_type == ESXBOOTINFO_SERIAL_CON_TYPE_NS16550 &&
             con->space ==  ESXBOOTINFO_SERIAL_CON_SPACE_MMIO &&
             con->offset_scaling == 1 &&
             con->access == ESXBOOTINFO_SERIAL_CON_ACCESS_8) {
            uart_base = (void *) con->base;
            printf("Found compatible UART at %p\n", uart_base);
         }
         break;
      }
      case ESXBOOTINFO_EFI_TYPE: {
         ESXBootInfo_Efi *efi = (void *) elmt;
         printf ("UEFI System Table at 0x%lx\n", efi->efi_systab);
         break;
      }
      default:
      }
   } FOR_EACH_ESXBOOTINFO_ELMT_DONE(ebi, elmt);
   printf("Command line: %s\n", (char *) ebi->cmdline);

   if (uart_base != NULL) {
      while (*hello != '\0') {
         *uart_base = *hello++;
      }
   }
}
