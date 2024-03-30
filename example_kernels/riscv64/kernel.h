/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <inttypes.h>

#define LINK_ADDRESS 0x80300000

#define R_RISCV_RELATIVE 0x3

typedef struct elf64_rela {
  uint64_t r_offset;
  uint64_t r_info;
  uint64_t r_addend;
} elf64_rela;

void sbi_putchar(char ch);
int printf(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));


