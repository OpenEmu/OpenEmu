/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - util.h                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Mupen64plus development team                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

#define LIST_HEAD(list) \
    struct list_head list = { &(list), &(list) }

static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

static inline void list_add(struct list_head *new_item, struct list_head *head)
{
    struct list_head *next = head->next;

    next->prev = new_item;
    new_item->next = next;
    new_item->prev = head;
    head->next = new_item;
}

static inline void list_add_tail(struct list_head *new_item, struct list_head *head)
{
    struct list_head *prev = head->prev;

    prev->next = new_item;
    new_item->next = head;
    new_item->prev = prev;
    head->prev = new_item;
}

static inline void list_del(struct list_head *entry)
{
    struct list_head *next = entry->next;
    struct list_head *prev = entry->prev;

    next->prev = prev;
    prev->next = next;
}

static inline void list_del_init(struct list_head *entry)
{
    list_del(entry);
    INIT_LIST_HEAD(entry);
}

static inline int list_empty(const struct list_head *head)
{
    return (head->next == head);
}

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_entry(pos, head, type, member) \
    for (pos = list_entry((head)->next, type, member); \
         &pos->member != (head); \
         pos = list_entry(pos->member.next, type, member))

#define list_for_each_safe(pos, safe, head) \
    for (pos = (head)->next, safe = pos->next; pos != (head); \
         pos = safe, safe = pos->next)

#define list_for_each_entry_safe(pos, safe, head, type, member) \
    for (pos = list_entry((head)->next, type, member), \
         safe = list_entry(pos->member.next, type, member); \
         &pos->member != (head); \
         pos = safe, \
         safe = list_entry(safe->member.next, type, member))

#ifdef __cplusplus
}
#endif

#endif
