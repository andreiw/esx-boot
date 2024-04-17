/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdlib.h>
#include "kernel.h"
#include "../../include/esxbootinfo.h"

#define EBH_FEATS 3
#define EBH_MAGIC ((uint32_t) (-(ESXBOOTINFO_MAGIC_V2 + sizeof (ebh) + EBH_FEATS)))
/*
 * 2MiB (megapage) alignment for kernel. 4MiB alignment for modules just because.
 */
#define LOAD_ALIGNMENT PG_LEVEL1_SIZE
#define MODULE_ALIGNMENT 2 * PG_LEVEL1_SIZE

struct ebh_s {
  ESXBootInfo_Header_V2 header;
  ESXBootInfo_FeatSerialCon feat_serial_con;
  ESXBootInfo_FeatLoadOptions feat_load_options;
  ESXBootInfo_FeatCpuMode   feat_cpu_mode;
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
      ESXBOOTINFO_FEAT_LOAD_OPTIONS_TYPE,
      ESXBOOTINFO_FEAT_REQUIRED,
      sizeof (ESXBootInfo_FeatLoadOptions),
      ESXBOOTINFO_FEAT_LOAD_OPTIONS_CONTIG,
      LOAD_ALIGNMENT,
      MODULE_ALIGNMENT
   },
   {
      ESXBOOTINFO_FEAT_CPU_MODE_TYPE,
      ESXBOOTINFO_FEAT_REQUIRED,
      sizeof (ESXBootInfo_FeatCpuMode)
   }
};

const char *hello = "Hello via UART!\n";

#ifdef WITH_BRING_UP_SYM_STUBS
extern void some_missing_symbol (void);
extern void another_missing_symbol (void);
#endif /* WITH_BRING_UP_SYM_STUBS */

void
c_main (ESXBootInfo *ebi)
{
   ESXBootInfo_Elmt *elmt;
   volatile uint8_t *uart_base;

   uart_base = NULL;
   FOR_EACH_ESXBOOTINFO_ELMT_DO (ebi, elmt) {
      switch (elmt->type) {
      case ESXBOOTINFO_SERIAL_CON_TYPE: {
         ESXBootInfo_SerialCon *con = (void *) elmt;
         if (con->con_type == ESXBOOTINFO_SERIAL_CON_TYPE_NS16550 &&
             con->space == ESXBOOTINFO_SERIAL_CON_SPACE_MMIO &&
             con->offset_scaling == 1 &&
             con->access == ESXBOOTINFO_SERIAL_CON_ACCESS_8) {
            uart_base = (void *) con->base;
            printf ("Found compatible UART at %p\n", uart_base);
         }
         break;
      }
      case ESXBOOTINFO_EFI_TYPE: {
         ESXBootInfo_Efi *efi = (void *) elmt;
         printf ("UEFI System Table at 0x%lx\n", efi->efi_systab);
         break;
      }
      case ESXBOOTINFO_CPU_MODE_TYPE: {
         ESXBootInfo_CpuMode *cpu = (void *) elmt;
         printf ("Hart ID 0x%lx\n", cpu->hart_id);
         break;
      }
      case ESXBOOTINFO_MODULE_TYPE: {
         ESXBootInfo_Module *module = (void *) elmt;
         if (module->numRanges != 0) {
            printf ("Module at 0x%lx\n", module->ranges[0].startPageNum << PG_BASE_SHIFT);
         }
         break;
      }
      default:
      }
   } FOR_EACH_ESXBOOTINFO_ELMT_DONE(ebi, elmt);
   printf ("Command line: %s\n", (char *) ebi->cmdline);

#ifdef WITH_BRING_UP_SYM_STUBS
   some_missing_symbol ();
   another_missing_symbol ();
#endif /* WITH_BRING_UP_SYM_STUBS */

   if (uart_base != NULL) {
      while (*hello != '\0') {
         *uart_base = *hello++;
      }
   }
}
