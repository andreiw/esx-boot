/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

#include "pboot.h"
#include "zforth.h"
#include "zf_dict.h"

#define IBUF_SIZE PAGE_SIZE

static uart_t *uart;
static char *ibuf;
static unsigned ibuf_index;

zf_cell zf_host_parse_num(const char *buf)
{
   char *end;
   zf_cell v = strtol(buf, &end, 0);
   if(*end != '\0') {
      zf_abort(ZF_ABORT_NOT_A_WORD);
   }
   return v;
}

zf_input_state zf_host_sys(zf_syscall_id id, const char *input)
{
   unsigned i;
   char num[2 + (sizeof(uint64_t) * 2) + 1];

   switch((int)id) {
   case ZF_SYSCALL_EMIT: {
      uart_putc(uart, (char)zf_pop());
   } break;
   case ZF_SYSCALL_PRINT: {
      snprintf(num, sizeof(num), ZF_CELL_FMT, zf_pop());
      for (i = 0; i < sizeof(num) && num[i] != '\0'; i++) {
         uart_putc(uart, num[i]);
      }

   } break;
   case ZF_SYSCALL_TELL: {
      char *buf;
      zf_cell len = zf_pop();
      zf_cell addr = zf_pop();
      if (addr >= ZF_DICT_SIZE - len) {
         zf_abort(ZF_ABORT_OUTSIDE_MEM);
      }
      buf = (char *) zf_dump(NULL) + addr;
      while (len--) {
         uart_putc(uart, *buf++);
      }
   } break;
   }

   return 0;
}

/*
 * Tracing output
 */

void zf_host_trace(const char *format, va_list va)
{
   char *buffer;
   int len;

   len = vsnprintf(NULL, 0, format, va);
   if (len < 0) {
      return;
   }

   buffer = malloc(len + 1);
   if (buffer == NULL) {
      return;
   }

   len = vsnprintf(buffer, len + 1, format, va);

   if (len != -1) {
      Log(LOG_DEBUG, "%s", buffer);
   }
   free(buffer);
}


/*-- mon_init -----------------------------------------------------------------
 *
 *      Initialize debug monitor.
 *
 * Results
 *      ERR_SUCCESS, or a generic error status.
 *----------------------------------------------------------------------------*/
int mon_init(void)
{
   uart = serial_get_uart();

   ibuf_index = 0;
   ibuf = malloc(PAGE_SIZE);
   if (ibuf == NULL) {
      Log(LOG_EMERG, "mon_ibuf allocation failed");
      return ERR_OUT_OF_RESOURCES;
   }

   zf_init(1);
   zf_bootstrap();
   zf_eval(": . 1 sys ;");
   if (zf_eval((void *) zf_dict_zf) != ZF_OK) {
     Log(LOG_ERR, "Dictionary eval error\n");
   }

   return ERR_SUCCESS;
}

static void mon_prompt (void)
{
   zf_eval("br .\" ok \" dsp @ . 62 emit br");
}

void mon_enter(void)
{
   int status;

   mon_prompt();
   do {
     char c;
     status = uart_getc(uart, &c);
     if (status != ERR_SUCCESS) {
        continue;
     }

     if (c == '\b' || c == 0x7f) {
        if (ibuf_index != 0) {
           ibuf_index--;
           uart_putc(uart, '\b');
           uart_putc(uart, ' ');
           uart_putc(uart, '\b');
        }
        continue;
     } else if (c == '\n' || c == '\r') {
        zf_result r;

        uart_putc(uart, '\r');
        uart_putc(uart, '\n');
        ibuf[ibuf_index] = '\0';
        r = zf_eval(ibuf);
        if(r != ZF_OK) {
           Log(LOG_ERR, "Eval error");
        }
        ibuf_index = 0;
        mon_prompt();
        continue;
     }

     if (ibuf_index == (IBUF_SIZE - 1)) {
        continue;
     }

     if (iscntrl(c)) {
        continue;
     }

     uart_putc(uart, c);
     ibuf[ibuf_index] = c;
     ibuf_index++;
   } while (1);
}
