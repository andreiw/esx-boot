/*******************************************************************************
 * Copyright (c) 2018,2020-2022 VMware, Inc.  All rights reserved.
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * esxbootinfo_arch.c -- Arch-specific portions of ESXBootInfo.
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <esxbootinfo.h>
#include <stdbool.h>
#include <cpu.h>
#include <bootlib.h>

/*-- esxbootinfo_arch_v1_supported_req_flags -----------------------------------
 *
 *      Extra arch-specific supported required flags.
 *
 * Parameters
 *      None.
 *
 * Results
 *      ESXBOOTINFO_FLAG_ARM64_MODE0.
 *----------------------------------------------------------------------------*/
int esxbootinfo_arch_v1_supported_req_flags(void)
{
   return ESXBOOTINFO_FLAG_ARM64_MODE0;
}

/*-- esxbootinfo_arch_check_kernel----------------------------------------------
 *
 *      Extra arch-specific kernel checks.
 *
 * Parameters
 *      IN mbh: ESXBootInfo header.
 *
 * Results
 *      False if kernel is not supported (with error logged).
 *----------------------------------------------------------------------------*/
bool esxbootinfo_arch_check_kernel(ESXBootInfo_Header *mbh)
{
   ESXBOOTINFO_ARM64_MODE kernel_mode = 0;

   if (mbh->magic == ESXBOOTINFO_MAGIC_V1) {
      kernel_mode = mbh->v1.flags & (ESXBOOTINFO_FLAG_ARM64_MODE0 |
                                     ESXBOOTINFO_FLAG_ARM64_MODE1);
   } else {
      ESXBootInfo_FeatArm64Mode *arm64;

      FOR_EACH_ESXBOOTINFO_FEAT_TYPE_DO(&mbh->v2,
                                        ESXBOOTINFO_FEAT_ARM64_MODE_TYPE,
                                        arm64) {

         if (arm64->feat_size > sizeof (ESXBootInfo_FeatArm64Mode)) {
            Log(LOG_ERR, "Unsupported FEAT_ARM64_MODE size");
            return false;
         }

         kernel_mode = arm64->flags & (ESXBOOTINFO_FLAG_ARM64_MODE0 |
                                       ESXBOOTINFO_FLAG_ARM64_MODE1);
         break;
      } FOR_EACH_ESXBOOTINFO_FEAT_TYPE_DONE(&mbh->v2, arm64);
   }

   switch (kernel_mode) {
      case ESXBOOTINFO_ARM64_MODE_EL2:
         if (!el_is_hyp()) {
            Log(LOG_ERR, "System (EL1) incompatible with kernel (EL2 non-VHE).\n");
            return false;
         }
         /*
          * Some CPUs can only run in VHE mode, and thus we'll enter
          * esxboot with the E2H bit set in HCR_EL2. These CPUs can't
          * run OSes in pure v8.0 mode.
          */
         if (vhe_enabled()) {
            Log(LOG_ERR, "System (EL2 VHE-only) incompatible with kernel (EL2 non-VHE).\n");
            return false;
         }
         break;

      case ESXBOOTINFO_ARM64_MODE_EL1:
         if (el_is_hyp()) {
            Log(LOG_ERR, "System (EL2) incompatible with kernel (EL1).\n");
            return false;
         }
         break;

      case ESXBOOTINFO_ARM64_MODE_UNIFIED:
         /*
          * Whatever the current EL or VHE support, VMK will be able to
          * support it.
          */
         break;

      case ESXBOOTINFO_ARM64_MODE_EL1_VHE:
         if (el_is_hyp() && !vhe_supported()) {
            Log(LOG_ERR, "System (EL2 non-VHE) incompatible with kernel (EL1 or EL2 VHE).\n");
            return false;
         }
         break;
   }

   return true;
}
