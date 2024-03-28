#include <inttypes.h>
#include <stdlib.h>
#include "../../include/esxbootinfo.h"

#define EBH_FEATS 1
#define EBH_MAGIC ((uint32_t) (-(ESXBOOTINFO_MAGIC_V2 + sizeof (ebh) + EBH_FEATS)))

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
  }
};

void
c_main (ESXBootInfo *ebi)
{
  volatile uint8_t *uart_base;
  ESXBootInfo_SerialCon *con;
  const char *hello = "\n\r\n\rHello via UART!\n\r";

  uart_base = NULL;
  FOR_EACH_ESXBOOTINFO_ELMT_TYPE_DO(ebi, ESXBOOTINFO_SERIAL_CON_TYPE, con) {
    if (con->con_type == ESXBOOTINFO_SERIAL_CON_TYPE_NS16550 &&
        con->space ==  ESXBOOTINFO_SERIAL_CON_SPACE_MMIO &&
        con->offset_scaling == 1 &&
        con->access == ESXBOOTINFO_SERIAL_CON_ACCESS_8) {
      uart_base = (void *) con->base;
    }
  } FOR_EACH_ESXBOOTINFO_ELMT_TYPE_DONE(ebi, con);

  if (uart_base != NULL) {
    while (*hello != '\0') {
      *uart_base = *hello++;
    }
  }
}
