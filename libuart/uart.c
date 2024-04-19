/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * Copyright (c) 2008-2012,2015,2020 VMware, Inc.  All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * uart.c -- Universal Asynchronous Receiver/Transmitter (UART) support
 */

#include <stdint.h>
#include <io.h>
#include <uart.h>

/*-- uart_getc -----------------------------------------------------------------
 *
 *      Read a character from a serial port.
 *
 * Parameters
 *      IN dev: pointer to a UART descriptor
 *      IN c:   pointer to memory, where to store the read character
 *
 * Results:
 *      ERR_SUCCESS:
 *      ERR_NOT_READY: no character
 *      ERR_UNSUPPORTED: uart_t implementation doesn't support reading
 *----------------------------------------------------------------------------*/
int uart_getc(const uart_t *dev, char *c)
{
  if (dev->getc != NULL) {
     return dev->getc(dev, c);
  }

  return ERR_UNSUPPORTED;
}

/*-- uart_putc -----------------------------------------------------------------
 *
 *      Write a character on a serial port.
 *
 * Parameters
 *      IN dev: pointer to a UART descriptor
 *      IN c:   character to be written
 *----------------------------------------------------------------------------*/
void uart_putc(const uart_t *dev, char c)
{
  if (dev->putc != NULL) {
     dev->putc(dev, c);
  }
}

/*-- uart_flags ----------------------------------------------------------------
 *
 *      Return UART flags to be used by upper layers.
 *
 * Parameters
 *      IN dev: pointer to a UART descriptor
 *
 * Results:
 *      UART flags to be used by upper layers.
 *----------------------------------------------------------------------------*/
uint32_t uart_flags(const uart_t *dev)
{
   return dev->flags;
}
