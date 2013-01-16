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

#include "workqueue.h"
#include "api/callbacks.h"

#include <SDL.h>
#include <SDL_thread.h>

#define WORKQUEUE_THREADS 1

struct workqueue_mgmt_globals {
    struct list_head work_queue;
    struct list_head thread_queue;
    struct list_head thread_list;
    SDL_mutex *lock;
};

struct workqueue_thread {
    SDL_Thread *thread;
    SDL_cond *work_avail;
    struct list_head list;
    struct list_head list_mgmt;
};

static struct workqueue_mgmt_globals workqueue_mgmt;

static void workqueue_dismiss(struct work_struct *work)
{
}

static struct work_struct *workqueue_get_work(struct workqueue_thread *thread)
{
    int found = 0;
    struct work_struct *work;

    while (1) {
        SDL_LockMutex(workqueue_mgmt.lock);
        list_del_init(&thread->list);
        if (!list_empty(&workqueue_mgmt.work_queue)) {
            found = 1;
            work = list_first_entry(&workqueue_mgmt.work_queue, struct work_struct, list);
            list_del_init(&work->list);
        } else {
            list_add(&thread->list, &workqueue_mgmt.thread_queue);
	    SDL_CondWait(thread->work_avail, workqueue_mgmt.lock);
        }
        SDL_UnlockMutex(workqueue_mgmt.lock);

        if (found)
            break;
    }

    return work;
}

static int workqueue_thread_handler(void *data)
{
    struct workqueue_thread *thread = data;
    struct work_struct *work;

    while (1) {
        work = workqueue_get_work(thread);
        if (work->func == workqueue_dismiss) {
            free(work);
            break;
        }

        work->func(work);
    }

    return 0;
}

int workqueue_init(void)
{
    size_t i;
    struct workqueue_thread *thread;

    memset(&workqueue_mgmt, 0, sizeof(workqueue_mgmt));
    INIT_LIST_HEAD(&workqueue_mgmt.work_queue);
    INIT_LIST_HEAD(&workqueue_mgmt.thread_queue);
    INIT_LIST_HEAD(&workqueue_mgmt.thread_list);

    workqueue_mgmt.lock = SDL_CreateMutex();
    if (!workqueue_mgmt.lock) {
        DebugMessage(M64MSG_ERROR, "Could not create workqueue management");
        return -1;
    }

    SDL_LockMutex(workqueue_mgmt.lock);
    for (i = 0; i < WORKQUEUE_THREADS; i++) {
        thread = malloc(sizeof(*thread));
        if (!thread) {
            DebugMessage(M64MSG_ERROR, "Could not create workqueue thread management data");
            SDL_UnlockMutex(workqueue_mgmt.lock);
            return -1;
        }

        memset(thread, 0, sizeof(*thread));
        list_add(&thread->list_mgmt, &workqueue_mgmt.thread_list);
        INIT_LIST_HEAD(&thread->list);
        thread->work_avail = SDL_CreateCond();
        if (!thread->work_avail) {
            DebugMessage(M64MSG_ERROR, "Could not create workqueue thread work_avail condition");
            SDL_UnlockMutex(workqueue_mgmt.lock);
            return -1;
        }

#if SDL_VERSION_ATLEAST(2,0,0)
        thread->thread = SDL_CreateThread(workqueue_thread_handler, "m64pwq", thread);
#else
        thread->thread = SDL_CreateThread(workqueue_thread_handler, thread);
#endif
        if (!thread->thread) {
            DebugMessage(M64MSG_ERROR, "Could not create workqueue thread handler");
            SDL_UnlockMutex(workqueue_mgmt.lock);
            return -1;
        }
    }
    SDL_UnlockMutex(workqueue_mgmt.lock);

    return 0;
}

void workqueue_shutdown(void)
{
    size_t i;
    int status;
    struct work_struct *work;
    struct workqueue_thread *thread, *safe;

    for (i = 0; i < WORKQUEUE_THREADS; i++) {
        work = malloc(sizeof(*work));
        init_work(work, workqueue_dismiss);
        queue_work(work);
    }

    list_for_each_entry_safe(thread, safe, &workqueue_mgmt.thread_list, struct workqueue_thread, list_mgmt) {
        list_del(&thread->list_mgmt);
        SDL_WaitThread(thread->thread, &status);
        SDL_DestroyCond(thread->work_avail);
        free(thread);
    }

    if (!list_empty(&workqueue_mgmt.work_queue))
        DebugMessage(M64MSG_WARNING, "Stopped workqueue with work still pending");
 
    SDL_DestroyMutex(workqueue_mgmt.lock);
}

int queue_work(struct work_struct *work)
{
    struct workqueue_thread *thread;

    SDL_LockMutex(workqueue_mgmt.lock);
    list_add_tail(&work->list, &workqueue_mgmt.work_queue);
    if (!list_empty(&workqueue_mgmt.thread_queue)) {
        thread = list_first_entry(&workqueue_mgmt.thread_queue, struct workqueue_thread, list);
        list_del_init(&thread->list);

        SDL_CondSignal(thread->work_avail);
    }
    SDL_UnlockMutex(workqueue_mgmt.lock);

    return 0;
}
