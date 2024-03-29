/** @file
  RISC-V Boot Protocol mandatory for RISC-V UEFI platforms.

  @par Revision Reference:
  The protocol specification can be found at
  https://github.com/riscv-non-isa/riscv-uefi

  Copyright (c) 2022, Ventana Micro Systems Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef RISCV_BOOT_PROTOCOL_H_
#define RISCV_BOOT_PROTOCOL_H_

typedef struct _RISCV_EFI_BOOT_PROTOCOL RISCV_EFI_BOOT_PROTOCOL;

#define RISCV_EFI_BOOT_PROTOCOL_REVISION  0x00010000
#define RISCV_EFI_BOOT_PROTOCOL_LATEST_VERSION \
        RISCV_EFI_BOOT_PROTOCOL_REVISION

#define RISCV_EFI_BOOT_PROTOCOL_GUID \
   { \
   0xccd15fec, 0x6f73, 0x4eec, { 0x83, 0x95, 0x3e, 0x69, 0xe4, 0xb9, 0x40, 0xbf } \
   }

typedef
EFI_STATUS
(EFIAPI *EFI_GET_BOOT_HARTID)(
  IN RISCV_EFI_BOOT_PROTOCOL   *This,
  OUT UINTN                    *BootHartId
  );

typedef struct _RISCV_EFI_BOOT_PROTOCOL {
  UINT64                 Revision;
  EFI_GET_BOOT_HARTID    GetBootHartId;
} RISCV_EFI_BOOT_PROTOCOL;

#endif
