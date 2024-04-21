/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <bootlib.h>
#include <boot_services.h>
#include <uart.h>
#include <cpu.h>
#include <bootlib.h>
#include "assert.h"

#define TYPE_BITS(type) (sizeof(type) * 8)

static inline void PANIC(void)
{
   for (;;) {
      HLT();
   }
}

#define NOT_REACHED() PANIC()

int mon_init(void);
void mon_enter(void);

#define PFN_SHIFT 12
typedef uint64_t pfn_t;
int pmem_init(efi_info_t *efi_info);
