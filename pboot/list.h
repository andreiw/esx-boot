/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

#pragma once

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
   struct list_head name = LIST_HEAD_INIT(name)

struct list_head {
   struct list_head *next, *prev;
};

/*
 * INIT_LIST_HEAD - Initialize a list_head structure
 * @list: list_head structure to be initialized.
 *
 * Initializes the list_head to point to itself.  If it is a list header,
 * the result is an empty list.
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
   list->next = list;
   list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
   next->prev = new;
   new->next = next;
   new->prev = prev;
   prev->next = new;
}

/*
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
   __list_add(new, head, head->next);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
   next->prev = prev;
   prev->next = next;
}

/*
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
   __list_del(entry->prev, entry->next);
}
