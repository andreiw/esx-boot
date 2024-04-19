/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

#pragma once

#include "pboot.h"

#define assert(x) if (!(x)) { \
      Log(LOG_EMERG, "%s(L%u): ASSERT fail: %s", __FILE__, __LINE__, #x); \
      PANIC();                            \
   }
