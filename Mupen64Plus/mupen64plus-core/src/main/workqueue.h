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

#ifndef __WORKQUEUE_H__
#define __WORKQUEUE_H__

#include "list.h"

struct work_struct *work;
typedef void (*work_func_t)(struct work_struct *work);
struct work_struct {
    work_func_t func;
    struct list_head list;
};

static inline void init_work(struct work_struct *work, work_func_t func)
{
    INIT_LIST_HEAD(&work->list);
    work->func = func;
}

#ifdef M64P_PARALLEL

int workqueue_init(void);
void workqueue_shutdown(void);
int queue_work(struct work_struct *work);

#else

static inline int workqueue_init(void)
{
    return 0;
}

static inline void workqueue_shutdown(void)
{
}

static inline int queue_work(struct work_struct *work)
{
    work->func(work);
    return 0;
}

#endif

#endif
