/*******************************************************************************
 * Copyright (c) 2015-2023 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * esxbootinfo.h --
 *
 *       The ESXBootInfo boot loader interface is a redesign of the
 *       Multiboot interface(*).  It takes over some design concepts,
 *       but widens all address fields to 64 bits and omits obsolete
 *       features.
 *
 *       ESXBootInfo is built around a variable size array of elements,
 *       where each element has its own type and size.  The
 *       specification can easily be extended in the future in a
 *       compatible manner by adding more element types and sizes.
 *
 *       ESXBootInfo is architecture and platform-agnostic.
 *
 *       (* The Multiboot specification document can be found at
 *       http://www.gnu.org/software/grub/manual/multiboot/multiboot.html.)
 */

#ifndef ESXBOOTINFO_H_
#define ESXBOOTINFO_H_

/*
 * Define constant values across boot loaders
 */
#define ESXBOOTINFO_MAXCMDLINE   (4096)
#define ESXBOOTINFO_MAXMODNAME    (256)

/*
 * ESXBootInfo Header
 *
 * The ESXBootInfo Header must be 8-bytes aligned, and must fit entirely
 * within the first 8192 bytes of the lowest loaded ELF segment.
 */
#define ESXBOOTINFO_MAGIC_V1          0x1BADB005 /* header signature */
#define ESXBOOTINFO_MAGIC_V2          0x1BADB006 /* header signature */
#define ESXBOOTINFO_ALIGNMENT         8
#define ESXBOOTINFO_SEARCH            8192

/*
 * These flags expose OS requirements and support.
 *
 * Flag bits 0 - 15 indicate required features and the boot loader stops, if it
 * does not support all requirements. All non-specified features flags must be
 * 0. Flag 16 - 31 describes optional features. The boot loader continues, even
 * if it does not support all of the optional flags.
 */
#define ESXBOOTINFO_FLAG_ARM64_MODE0      (1 << 0)   /* bit 0 of ARM Mode */
#define ESXBOOTINFO_FLAG_VIDEO            (1 << 2)   /* Must pass video info to OS;
                                                        non-min video fields valid */
#define ESXBOOTINFO_FLAG_ARM64_MODE1      (1 << 16)  /* bit 1 of ARM Mode */
#define ESXBOOTINFO_FLAG_EFI_RTS_OLD      (1 << 17)  /* Reserved; do not redefine */
#define ESXBOOTINFO_FLAG_EFI_RTS          (1 << 18)  /* EFI RTS fields valid */
#define ESXBOOTINFO_FLAG_OS_PRIVATE       (1 << 19)  /* os_private field valid */
#define ESXBOOTINFO_FLAG_VIDEO_MIN        (1 << 20)  /* Video min fields valid */
#define ESXBOOTINFO_FLAG_TPM_MEASUREMENT  (1 << 21)  /* TPM measurement field valid */
#define ESXBOOTINFO_FLAG_MEMTYPE_SP       (1 << 22)  /* Can use specific purpose memory */

/*
 * ARM64 supports multiple image types identified by a mode which is
 * built as described by this table:
 *
 *         +-------+-------+---------+
 *         | Mode1 | Mode0 | Meaning |
 *         ===========================
 *         |   0   |   0   | v80 EL2 |
 *         +-------+-------+---------+
 *         |   0   |   1   | EL1     |
 *         +-------+-------+---------+
 *         |   1   |   0   | UNIFIED |
 *         +-------+-------+---------+
 *         |   1   |   1   | EL1+VHE |
 *         +-------+-------+---------+
 */
typedef enum {
   ESXBOOTINFO_ARM64_MODE_EL2     = 0x00000, /* Support v80 EL2 */
   ESXBOOTINFO_ARM64_MODE_EL1     = 0x00001, /* Support EL1 */
   ESXBOOTINFO_ARM64_MODE_UNIFIED = 0x10000, /* Support EL1, v80 EL2, VHE EL2 */
   ESXBOOTINFO_ARM64_MODE_EL1_VHE = 0x10001  /* Support EL1, VHE EL2 */
} ESXBOOTINFO_ARM64_MODE;

#define ESXBOOTINFO_VIDEO_GRAPHIC         0          /* Linear graphics mode */
#define ESXBOOTINFO_VIDEO_TEXT            1          /* EGA-standard text mode */

/*
 * These flags expose OS TPM measurement requests to the bootloader.
 *
 * The OS reports the measurement versions it supports, and the bootloader will
 * measure the highest version it supports from that set. The actual measured
 * version is reported through ESXBootInfo_Tpm.
 */
#define ESXBOOTINFO_TPM_MEASURE_NONE      0          /* No measurement */
#define ESXBOOTINFO_TPM_MEASURE_V1        (1 << 0)   /* Mods, cmdline, certs, tag. */

/*
 * Basic runtime watchdog types. Currently only support VMW-defined
 * VMW_RUNTIME_WATCHDOG_PROTOCOL, but in the future may support additional ways of
 * describing watchdog hardware.
 */
typedef enum {
   NONE,
   VMW_RUNTIME_WATCHDOG_PROTOCOL
} RUNTIME_WATCHDOG_BASIC_TYPE;

/*
 * ESXBootInfo_Header_V1 passed statically from kernel to bootloader.
 */
typedef struct ESXBootInfo_Header_V1 {
   uint32_t magic;           /* ESXBootInfo Header Magic */
   uint32_t flags;           /* Feature flags */
   uint32_t checksum;        /* (magic + flags + checksum) sum up to zero */
   /*
    * The "reserved" fields were added by mistake from original Multiboot.
    * Never used in ESX or its bootloaders. These are prime candidates for
    * adding future functionality in a backward-compatible fashion.
    */
   uint32_t reserved[2];
   uint32_t min_width;       /* Video: minimum horizontal resolution */
   uint32_t min_height;      /* Video: minimum vertical resolution */
   uint32_t min_depth;       /* Video: minimum bits per pixel (0 for text) */
   uint32_t mode_type;       /* Video: preferred mode (0=graphic, 1=text) */
   uint32_t width;           /* Video: preferred horizontal resolution */
   uint32_t height;          /* Video: preferred vertical resolution */
   uint32_t depth;           /* Video: preferred bits per pixel (0 for text) */
   uint64_t rts_vaddr;       /* Virtual address of UEFI Runtime Services */
   uint64_t rts_size;        /* For new-style RTS. */
   uint32_t os_private;      /* OS-private field, could be used by a more
                                OS-coupled loader (ex: loadesx kexec-like
                                functionality) */
   uint32_t tpm_measure;     /* TPM: determine what bootloader measures */
} __attribute__((packed)) ESXBootInfo_Header_V1;

typedef enum ESXBootInfo_FeatType {
   ESXBOOTINFO_FEAT_INVALID_TYPE,
   ESXBOOTINFO_FEAT_CPU_MODE_TYPE,
   ESXBOOTINFO_FEAT_VIDEO_TYPE,
   ESXBOOTINFO_FEAT_UEFI_TYPE,
   ESXBOOTINFO_FEAT_OS_PRIVATE_TYPE,
   ESXBOOTINFO_FEAT_TPM_TYPE,
   ESXBOOTINFO_FEAT_LOAD_ALIGN_TYPE,
   ESXBOOTINFO_FEAT_SERIAL_CON_TYPE,
   NUM_ESXBOOTINFO_FEAT_TYPE
} ESXBootInfo_FeatType;

#define ESXBOOTINFO_FEAT_REQUIRED (1 << 0) /* Feature must be supported by
                                              bootloader */

typedef struct ESXBootInfo_FeatElmt {
   ESXBootInfo_FeatType feat_type;
   uint32_t feat_flags;
   uint32_t feat_size;
} __attribute__((packed)) ESXBootInfo_FeatElmt;

typedef struct ESXBootInfo_FeatCpuMode {
   ESXBootInfo_FeatType feat_type;
   uint32_t feat_flags;
   uint32_t feat_size;
   /*
    * These flags are valid to be used here on AArch64:
    *    ESXBOOTINFO_FLAG_ARM64_MODE0
    *    ESXBOOTINFO_FLAG_ARM64_MODE1
    */
   uint32_t flags;
}  __attribute__((packed)) ESXBootInfo_FeatCpuMode;

typedef struct ESXBootInfo_FeatVideo {
   ESXBootInfo_FeatType feat_type;
   uint32_t feat_flags;
   uint32_t feat_size;
   /*
    * These flags are valid to be used here:
    *    ESXBOOTINFO_FLAG_VIDEO_MIN
    */
   uint32_t flags;
   uint32_t min_width;       /* minimum horizontal resolution */
   uint32_t min_height;      /* minimum vertical resolution */
   uint32_t min_depth;       /* minimum bits per pixel (0 for text) */
   uint32_t mode_type;       /* preferred mode (0=graphic, 1=text) */
   uint32_t width;           /* preferred horizontal resolution */
   uint32_t height;          /* preferred vertical resolution */
   uint32_t depth;           /* preferred bits per pixel (0 for text) */
} __attribute__((packed)) ESXBootInfo_FeatVideo;

typedef struct ESXBootInfo_FeatUefi {
   ESXBootInfo_FeatType feat_type;
   uint32_t feat_flags;
   uint32_t feat_size;
   /*
    * These flags are valid to be used here:
    *    ESXBOOTINFO_FLAG_EFI_RTS
    *    ESXBOOTINFO_FLAG_MEMTYPE_SP
    */
   uint32_t flags;
   uint64_t rts_vaddr;       /* Virtual address of UEFI Runtime Services */
   uint64_t rts_size;        /* For new-style RTS. */
} __attribute__((packed)) ESXBootInfo_FeatUefi;

typedef struct ESXBootInfo_FeatTpm {
   ESXBootInfo_FeatType feat_type;
   uint32_t feat_flags;
   uint32_t feat_size;
   uint32_t tpm_measure;     /* TPM: determine what bootloader measures */
} __attribute__((packed)) ESXBootInfo_FeatTpm;

typedef struct ESXBootInfo_FeatLoadAlign {
   ESXBootInfo_FeatType feat_type;
   uint32_t feat_flags;
   uint32_t feat_size;
   uint32_t load_align;
} __attribute__((packed)) ESXBootInfo_FeatLoadAlign;

typedef struct ESXBootInfo_FeatSerialCon {
   ESXBootInfo_FeatType feat_type;
   uint32_t feat_flags;
   uint32_t feat_size;
} __attribute__((packed)) ESXBootInfo_FeatSerialCon;

/*
 * ESXBootInfo_Header_V2 passed statically from kernel to bootloader.
 */
typedef struct ESXBootInfo_Header_V2 {
   uint32_t magic;           /* ESXBootInfo Header Magic */
   uint32_t length;          /* Entire length of the structure, including
                                the variable elements */
   uint32_t num_elmts;       /* Array size for elmts[] */
   uint32_t checksum;        /* (magic + length + num_eles + checksum)
                                sum up to zero */
   ESXBootInfo_FeatElmt elmts[0];
} __attribute__((packed)) ESXBootInfo_Header_V2;

#define FOR_EACH_ESXBOOTINFO_FEAT_DO(ebh, feat)                           \
   do {                                                                   \
      uint64_t __i;                                                       \
                                                                          \
      for (__i = 0,                                                       \
           feat = (__typeof__(feat)) &(ebh)->elmts[0];                    \
           __i < (ebh)->num_elmts;                                        \
           __i += 1,                                                      \
           feat = (__typeof__(feat))((uint8_t *)feat + feat->feat_size)) {

#define FOR_EACH_ESXBOOTINFO_FEAT_DONE(ebh, feat)                         \
      }                                                                   \
      feat = NULL;                                                        \
   } while (0)

#define FOR_EACH_ESXBOOTINFO_FEAT_TYPE_DO(ebh, featType, feat)            \
   FOR_EACH_ESXBOOTINFO_FEAT_DO(ebh, feat) {                              \
      if (feat->feat_type == featType) {

#define FOR_EACH_ESXBOOTINFO_FEAT_TYPE_DONE(ebh, feat)                    \
      }                                                                   \
   } FOR_EACH_ESXBOOTINFO_FEAT_DONE(ebh, feat)

typedef union ESXBootInfo_Header {
  uint32_t magic;
  ESXBootInfo_Header_V1 v1;
  ESXBootInfo_Header_V2 v2;
} ESXBootInfo_Header;

typedef enum ESXBootInfo_Type {
   ESXBOOTINFO_INVALID_TYPE,
   ESXBOOTINFO_MEMRANGE_TYPE,
   ESXBOOTINFO_MODULE_TYPE,
   ESXBOOTINFO_VBE_TYPE,
   ESXBOOTINFO_EFI_TYPE,
   ESXBOOTINFO_OS_PRIVATE1_TYPE,
   ESXBOOTINFO_OS_PRIVATE2_TYPE,
   ESXBOOTINFO_TPM_TYPE,
   ESXBOOTINFO_RWD_TYPE,
   ESXBOOTINFO_LOGBUFFER_TYPE,
   ESXBOOTINFO_SERIAL_CON_TYPE,
   ESXBOOTINFO_CPU_MODE_TYPE,
   NUM_ESXBOOTINFO_TYPE
} ESXBootInfo_Type;

typedef struct ESXBootInfo_Elmt {
   ESXBootInfo_Type type;
   uint64_t elmtSize;
} __attribute__((packed)) ESXBootInfo_Elmt;

typedef struct ESXBootInfo_MemRange {
   ESXBootInfo_Type type;
   uint64_t elmtSize;

   uint64_t startAddr;
   uint64_t len;
   uint32_t memType;
} __attribute__((packed)) ESXBootInfo_MemRange;

typedef struct ESXBootInfo_ModuleRange {
   uint64_t startPageNum;
   uint32_t numPages;
   uint32_t padding;
} __attribute__((packed)) ESXBootInfo_ModuleRange;

typedef struct ESXBootInfo_Module {
   ESXBootInfo_Type type;
   uint64_t elmtSize;

   uint64_t string;
   uint64_t moduleSize;

   uint32_t numRanges;
   ESXBootInfo_ModuleRange ranges[0];
} __attribute__((packed)) ESXBootInfo_Module;

typedef struct ESXBootInfo_RuntimeWdt {
   ESXBootInfo_Type type;
   uint64_t elmtSize;

   RUNTIME_WATCHDOG_BASIC_TYPE watchdogBasicType;
   int watchdogSubType;
   uint64_t base;
   uint64_t maxTimeout;
   uint64_t minTimeout;
   uint64_t timeout;
} __attribute__((packed)) ESXBootInfo_RuntimeWdt;

/* VBE flags */
#define ESXBOOTINFO_VBE_FB64              (1UL << 0)  /* fbBaseAddress in use */

typedef struct ESXBootInfo_Vbe {
   ESXBootInfo_Type type;
   uint64_t elmtSize;

   uint64_t vbe_control_info; // main VBE header
   uint64_t vbe_mode_info;    // current mode definition
   uint16_t vbe_mode;         // current mode index
   uint32_t vbe_flags;        // ESXBootInfo VBE flags.
   /*
    * The fb base address field in the vbe_mode_info structure is only
    * 32-bit wide. If ESXBOOTINFO_VBE_FB64 is set in vbe_flags, then the
    * fb base address is stored in fbBaseAddress.
    */
   uint64_t fbBaseAddress;
} __attribute__((packed)) ESXBootInfo_Vbe;

/* EFI flags */
#define ESXBOOTINFO_EFI_ARCH64            (1<<0)  /* 64-bit EFI */
#define ESXBOOTINFO_EFI_SECURE_BOOT       (1<<1)  /* EFI Secure Boot in progress */
#define ESXBOOTINFO_EFI_MMAP              (1<<2)  /* EFI memory map is valid */

typedef struct ESXBootInfo_Efi {
   ESXBootInfo_Type type;
   uint64_t elmtSize;

   uint32_t efi_flags;
   uint64_t efi_systab;         // EFI system table physical address

   /* Set if efi_flags & ESXBOOTINFO_EFI_MMAP */
   uint64_t efi_mmap;           // EFI memory map physical address
   uint32_t efi_mmap_num_descs; // Number of descriptors in EFI memory map
   uint32_t efi_mmap_desc_size; // Size of each descriptor
   uint32_t efi_mmap_version;   // Descriptor version
} __attribute__((packed)) ESXBootInfo_Efi;

typedef struct ESXBootInfo_LogBuffer {
   ESXBootInfo_Type type;
   uint64_t elmtSize;

   uint32_t bufferSize;
   uint64_t addr;
} __attribute__((packed)) ESXBootInfo_LogBuffer;

/* TPM Flags */
#define ESXBOOTINFO_TPM_EVENT_LOG_TRUNCATED     (1 << 0)
#define ESXBOOTINFO_TPM_EVENTS_MEASURED_V1      (1 << 1)

typedef struct ESXBootInfo_Tpm {
   ESXBootInfo_Type type;
   uint64_t elmtSize;

   uint32_t flags;

   uint32_t eventLogSize;
   uint8_t eventLog[0];
} __attribute__((packed)) ESXBootInfo_Tpm;

typedef enum ESXBootInfo_SerialConType {
   ESXBOOTINFO_SERIAL_CON_TYPE_NS16550,
   ESXBOOTINFO_SERIAL_CON_TYPE_PL011,
   ESXBOOTINFO_SERIAL_CON_TYPE_TMFIFO,
   ESXBOOTINFO_SERIAL_CON_TYPE_AAPL_S5L,
   ESXBOOTINFO_SERIAL_CON_TYPE_SBI,
} ESXBootInfo_SerialConType;

typedef enum ESXBootInfo_SerialConSpace {
   ESXBOOTINFO_SERIAL_CON_SPACE_IO_PORT,
   ESXBOOTINFO_SERIAL_CON_SPACE_MMIO,
} ESXBootInfo_SerialConSpace;

typedef enum ESXBootInfo_SerialConAccess {
   /*
    * These match ACPI Generic Address Structure (GAS) Access Size.
    */
   ESXBOOTINFO_SERIAL_CON_ACCESS_LEGACY = 0,
   ESXBOOTINFO_SERIAL_CON_ACCESS_8,
   ESXBOOTINFO_SERIAL_CON_ACCESS_16,
   ESXBOOTINFO_SERIAL_CON_ACCESS_32,
   ESXBOOTINFO_SERIAL_CON_ACCESS_64,
} ESXBootInfo_SerialConAccess;

typedef struct ESXBootInfo_SerialCon {
   ESXBootInfo_Type type;
   uint64_t elmtSize;
   ESXBootInfo_SerialConType  con_type;
   ESXBootInfo_SerialConSpace space;
   uint64_t base;
   /*
    * 'access' and 'offset_scaling' are only applicable
    * for port I/O and MMIO accesses.
    */
   uint8_t offset_scaling;
   ESXBootInfo_SerialConAccess access;
   /*
    * 'baud' is only applicable to actual U(S)ARTS.
    */
   uint32_t baud;
} __attribute__((packed)) ESXBootInfo_SerialCon;

typedef struct ESXBootInfo_CpuMode {
   ESXBootInfo_Type type;
   uint64_t elmtSize;
#if defined(only_riscv64)
   uint64_t hart_id;
#endif /* only_riscv64 */
} __attribute__((packed)) ESXBootInfo_CpuMode;

/*
 * ESXBootInfo passed from bootloader to kernel.
 *
 * x86/x64:
 *  EAX: ESXBootInfo magic.
 *  EBX: PA of ESXBootInfo data structure.
 *  State: 32-bit unpaged mode. See mboot/x86/trampoline.s
 *         for details.
 *
 * ARM64:
 *  x0: ESXBootInfo magic.
 *  x1: PA of ESXBootInfo data structure.
 *  State: MMU and caches may be on.
 *
 * RISCV64:
 *  a0: ESXBootInfo magic.
 *  a1: PA of ESXBootInfo data structure.
 *  State: MMU and caches may be on.
 *
 * The magic value reported matches the magic of the ESXBootInfo
 * header reported by the kernel.
 *
 * Some state at kernel entry depends on choice of reported
 * ESXBootInfo header.
 *
 * Version ESXBOOTINFO_MAGIC_V1:
 *  x86/x64: loaded at linked address.
 *  ARM64/RISCV64: loaded at any 2MB-aligned address.
 *
 * Version ESXBOOTINFO_MAGIC_V2:
 *  FEAT_LOAD_ALIGN missing: loaded at linked address.
 *  FEAT_LOAD_ALIGN load_align == 0: loaded at linked address.
 *  FEAT_LOAD_ALIGN load_align != 0: loaded at load_align-aligned
 *                                   address. On x86/x64, this
 *                                   address is below 4GiB.
 *
 * The ESXBootInfo data structure is used by the the boot loader
 * to communicate vital information to the operating system.
 * The operating system can use or ignore any parts of the structure
 * as it chooses; all information passed by the boot loader is
 * advisory only.
 *
 * The ESXBootInfo structure and its related substructures may be
 * placed anywhere in memory by the boot loader (with the exception
 * of the memory reserved for the kernel and boot modules, of course).
 * It is the operating system's responsibility to avoid overwriting
 * this memory until it is done using it.
 */

typedef struct ESXBootInfo {
   uint64_t cmdline;

   uint64_t numESXBootInfoElmt;
   ESXBootInfo_Elmt elmts[0];
} __attribute__((packed)) ESXBootInfo;

#define FOR_EACH_ESXBOOTINFO_ELMT_DO(ebi, elmt)                           \
   do {                                                                   \
      uint64_t __i;                                                       \
                                                                          \
      for (__i = 0,                                                       \
           elmt = (__typeof__(elmt)) &(ebi)->elmts[0];                    \
           __i < (ebi)->numESXBootInfoElmt;                               \
           __i += 1,                                                      \
           elmt = (__typeof__(elmt))((uint8_t *)elmt + elmt->elmtSize)) {

#define FOR_EACH_ESXBOOTINFO_ELMT_DONE(ebi, elmt)                         \
      }                                                                   \
      elmt = NULL;                                                        \
   } while (0)


#define FOR_EACH_ESXBOOTINFO_ELMT_TYPE_DO(ebi, elmtType, elmt)            \
   FOR_EACH_ESXBOOTINFO_ELMT_DO(ebi, elmt) {                              \
      if (elmt->type == elmtType) {

#define FOR_EACH_ESXBOOTINFO_ELMT_TYPE_DONE(ebi, elmt)                    \
      }                                                                   \
   } FOR_EACH_ESXBOOTINFO_ELMT_DONE(ebi, elmt)


#endif /* ESXBOOTINFO_H_ */
