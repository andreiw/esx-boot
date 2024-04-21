/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

#include "pboot.h"

static pfn_t pfn_base;
static size_t pfn_total;
static size_t pfn_free;
static uint64_t *pfns_map;

static bool pmem_pfn_is_free (pfn_t pfn)
{
   pfn_t offset;
   uint64_t *pfn_chunk;

   assert (pfn >= pfn_base);
   if (pfn < pfn_base) {
      return false;
   }

   offset = pfn - pfn_base;
   assert (offset < pfn_total);
   if (offset >= pfn_total) {
      return false;
   }

   pfn_chunk = pfns_map + (offset / TYPE_BITS(uint64_t));
   offset %= TYPE_BITS(uint64_t);
   return (*pfn_chunk & (1UL << offset)) == 1;
}

static void pmem_pfn_mark (pfn_t pfn, bool used)
{
   pfn_t offset;
   uint64_t *pfn_chunk;

   offset = pfn - pfn_base;
   assert (offset < pfn_total);
   pfn_chunk = pfns_map + (offset / TYPE_BITS(uint64_t));

   offset %= TYPE_BITS(uint64_t);
   assert ((*pfn_chunk & (1UL << offset)) == used);
   if (used) {
      *pfn_chunk &= ~(1UL << offset);
      pfn_free--;
   } else {
      *pfn_chunk |= (1UL << offset);
      pfn_free++;
   }
}

int pmem_init(efi_info_t *efi_info)
{
   int status;
   e820_range_t *mmap;
   size_t count;
   size_t i;
   pfn_t start;
   pfn_t end;

   status = get_memory_map(0, &mmap, &count, efi_info);
   if (status != ERR_SUCCESS) {
      Log(LOG_ERR, "%s: get_memory_map: %s", __FUNCTION__,
          error_str[status]);
      return status;
   }
   e820_mmap_merge(mmap, &count);
   start = E820_BASE(&mmap[0]) >> PFN_SHIFT;
   end = (E820_BASE(&mmap[count - 1]) +
          E820_LENGTH(&mmap[count - 1]) - 1) >> PFN_SHIFT;

   Log(LOG_INFO, "First PFN: 0x%lx", start);
   Log(LOG_INFO, "Last PFN: 0x%lx", end);
   pfn_base = start;
   pfn_total = end - pfn_base + 1;

   pfns_map = malloc (ROUNDUP(pfn_total, TYPE_BITS(*pfns_map)) /
                        sizeof(*pfns_map));
   memset (pfns_map, 0, ROUNDUP(pfn_total, TYPE_BITS(*pfns_map)) /
           sizeof(*pfns_map));
   if (pfns_map == NULL) {
      status = ERR_OUT_OF_RESOURCES;
      Log(LOG_ERR, "%s: pfns_map: %s", __FUNCTION__,
          error_str[status]);
      return status;
   }

   for (i = 0; i < count; i++) {
      start = E820_BASE(&mmap[i]) >> PFN_SHIFT;
      end = ((E820_BASE(&mmap[i]) + E820_LENGTH(&mmap[i]) - 1) >> PFN_SHIFT) + 1;

      if (mmap[i].type == E820_TYPE_AVAILABLE) {
         while (start < end) {
            assert (!pmem_pfn_is_free (start));
            pmem_pfn_mark (start, false);
            start++;
         }
      }
   }

   Log(LOG_INFO, "Free PFNs: %lu/%lu", pfn_free, pfn_total);

   /*
    * FIXME: initialize free list here.
    */

   free_memory_map(mmap, efi_info);
   return ERR_SUCCESS;
}
