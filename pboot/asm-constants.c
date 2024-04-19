/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include "pboot.h"

#define C(str, constant) \
   __asm__ volatile ("#define " str " %0\n\r" :: "i" ((uintptr_t)constant) :); \
   __asm__ volatile (".equ " str ", %0\n\r" :: "i" ((uintptr_t)constant) :)

#define IS_TYPE_UNSIGNED(type) ((type)0 - 1 > 0)

#define IDENTIFIER(x) C(#x, x)
#define SIZE(x) C("SZ_" #x, sizeof(x))
#define FIELD_SIZE(t, f) C("SZ_" #t "_" #f, sizeof(((t *) 0)->f))
#define FIELD_OFFSET(t, f) C("OF_" #t "_" #f, offsetof(t, f))
/*
 * For a field in a struct, define the size, offset and signedness.
 * The latter is helpful in as macros to simplify loading correctly.
 */
#define FIELD(t, f) \
   FIELD_SIZE(t, f); \
   FIELD_OFFSET(t, f); \
   C("IU_" #t "_" #f, IS_TYPE_UNSIGNED(typeof(((t *) 0)->f)))

void dummy (void)
{
#define DUMMY 1
   IDENTIFIER(DUMMY);
}
