/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

#include "pboot.h"
#include "list.h"
#include "efi.h"

static pfn_t pfn_base;
static size_t pfn_total;
static size_t pfn_free;
static uint64_t *pfns_map;

#define MAX_ORDERS 20

static struct list_head pfns_list_by_order[MAX_ORDERS];
static size_t pfns_free_by_order[MAX_ORDERS];

typedef struct {
   struct list_head entry;
   void *magic;
   unsigned order;
} free_page_t;

static bool pmem_pfn_is_free (pfn_t pfn)
{
   pfn_t offset;
   uint64_t *pfn_chunk;

   if (pfn < pfn_base) {
      return false;
   }

   offset = pfn - pfn_base;
   if (offset >= pfn_total) {
      return false;
   }

   pfn_chunk = pfns_map + (offset / TYPE_BITS(uint64_t));
   offset %= TYPE_BITS(uint64_t);
   return (*pfn_chunk & (1UL << offset)) != 0;
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

static void pmem_pfn_free(pfn_t pfn, unsigned order)
{
   pfn_t pfn_s;
   pfn_t pfn_e;
   pfn_t buddy_pfn;
   free_page_t *page;

   assert (order < MAX_ORDERS);
   assert ((pfn & ((1UL << order) - 1)) == 0);

   pfn_s = pfn;
   pfn_e = pfn + (1UL << order);

   while (pfn_s < pfn_e) {
      assert (!pmem_pfn_is_free (pfn_s));
      pmem_pfn_mark (pfn_s, false);
      pfn_s++;
   }

   while (order < (MAX_ORDERS - 1)) {
      /*
       * The buddy PFN is the base PFN of the buddy range.
       * If the range is available for coalescing, the base
       * PFN must be in the free map. We can then check the order
       * in the metadata structure (free_page_t) to ensure the rest
       * of the buddy range are free.
       */
      buddy_pfn = pfn ^ (1UL << order);

      if (!pmem_pfn_is_free (buddy_pfn)) {
         /*
          * Buddy range is definitely not free.
          */
         break;
      }

      page = (void *)(buddy_pfn << PFN_SHIFT);
      assert (page->magic == (void *) page);
      if (page->order != order) {
         /*
          * If buddy is on the free list, it needs to be the
          * same order, meaning the entire range of buddy PFNs
          * is free.
          */
         Log(LOG_INFO, "bpfn 0x%lx != order %u page->order 0x%x",
             buddy_pfn, order, page->order);
         break;
      }
      list_del (&page->entry);
      pfns_free_by_order[order]--;
      memset (page, 0, sizeof (*page));
      order++;
      pfn = MIN(pfn, buddy_pfn);
   }

   page = (void *)(pfn << PFN_SHIFT);
   page->magic = (void *) page;
   page->order = order;
   list_add (&page->entry, &pfns_list_by_order[order]);
   pfns_free_by_order[order]++;
}


int pmem_init(efi_info_t *efi_info)
{
   int status;
   unsigned i;
   pfn_t start;
   pfn_t end;
   size_t e820_count_unused;
   e820_range_t *e820_mmap;
   EFI_MEMORY_DESCRIPTOR *desc;

   for (i = 0; i < MAX_ORDERS; i++) {
      INIT_LIST_HEAD(&pfns_list_by_order[i]);
   }

   status = get_memory_map(0, &e820_mmap, &e820_count_unused, efi_info);
   if (status != ERR_SUCCESS) {
      Log(LOG_ERR, "%s: get_memory_map: %s", __FUNCTION__,
          error_str[status]);
      return status;
   }

   for (start = -1, end = 0, desc = efi_info->mmap, i = 0;
        i < efi_info->num_descs;
        i++, desc = (void *)((uintptr_t)desc + efi_info->desc_size)) {
      pfn_t s = desc->PhysicalStart >> PFN_SHIFT;
      pfn_t e = s + desc->NumberOfPages - 1;

      if (desc->Type != EfiConventionalMemory) {
         continue;
      }

      if (s < start) {
         start = s;
      }

      if (end < e) {
         end = e;
      }
   }

   Log(LOG_INFO, "First PFN: 0x%lx", start);
   Log(LOG_INFO, "Last PFN: 0x%lx", end);
   pfn_base = start;
   pfn_total = end - pfn_base + 1;

   Log(LOG_INFO, "Size of pfns_map 0x%lx",
       ROUNDUP(pfn_total, TYPE_BITS(*pfns_map)) /
       sizeof(*pfns_map));

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

   free_memory_map(e820_mmap, efi_info);
   status = get_memory_map(0, &e820_mmap, &e820_count_unused, efi_info);
   if (status != ERR_SUCCESS) {
      Log(LOG_ERR, "%s: get_memory_map: %s", __FUNCTION__,
          error_str[status]);
      return status;
   }

   for (start = -1, end = 0, desc = efi_info->mmap, i = 0;
        i < efi_info->num_descs;
        i++, desc = (void *)((uintptr_t) desc + efi_info->desc_size)) {
     start = desc->PhysicalStart >> PFN_SHIFT;
     end = start + desc->NumberOfPages - 1;

     if (desc->Type == EfiConventionalMemory) {
        while (start <= end) {
           unsigned order;

           for (order = MAX_ORDERS - 1;; order--) {
              pfn_t size = 1UL << order;

              if ((start & (size - 1)) == 0 &&
                  (start + size - 1) <= end) {
                 pmem_pfn_free (start, order);
                 start += size;
                 break;
              }
           }
        }
     }
   }

   Log(LOG_INFO, "Free PFNs: %lu/%lu", pfn_free, pfn_total);
   for (i = 0; i < MAX_ORDERS; i++) {
      Log(LOG_INFO, "Free PFNs order %u: %lu", i,
          pfns_free_by_order[i] * (1 << i));
   }


   free_memory_map(e820_mmap, efi_info);
   return ERR_SUCCESS;
}
