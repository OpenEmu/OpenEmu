/*
 Copyright (c) 2010 OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SDL.h"
#include "SDL_thread.h"
#import <OpenEmuBase/OETimingUtils.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

SDL_mutex *SDL_CreateMutex(void)
{
    pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(m, NULL);
    
    return (SDL_mutex*)m;
}

int SDL_LockMutex(SDL_mutex *m)
{
    return pthread_mutex_lock((pthread_mutex_t*)m);
}

int SDL_UnlockMutex(SDL_mutex *m)
{
    return pthread_mutex_unlock((pthread_mutex_t*)m);
}

void SDL_DestroyMutex(SDL_mutex *m)
{
    pthread_mutex_destroy((pthread_mutex_t*)m);
    free(m);
}

Uint32 SDL_GetTicks(void)
{
    return OEMonotonicTime();
}

void SDL_Quit(void)
{
}

void SDL_Delay(Uint32 ms)
{
    usleep(ms * 1000);
}

void SDL_PumpEvents(void)
{
}

void SDL_GL_SwapBuffers(void)
{
    NSLog(@"Mupen warning: Should not reach here");
}

SDL_cond *SDL_CreateCond(void)
{
    pthread_cond_t *cond = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(cond, NULL);
    
    return (SDL_cond*)cond;
}

int SDL_CondWait(SDL_cond *cond, SDL_mutex *mut)
{
    return pthread_cond_wait((pthread_cond_t*)cond, (pthread_mutex_t*)mut);
}

int SDL_CondSignal(SDL_cond *cond)
{
    return pthread_cond_signal((pthread_cond_t*)cond);
}

void SDL_DestroyCond(SDL_cond *cond)
{
    pthread_cond_destroy((pthread_cond_t*)cond);
    free(cond);
}

static struct {
    const char *thread_name;
    void *thread_context;
} sContext;

void *Fake_SDL_New_Thread(void *p)
{
    pthread_setname_np(sContext.thread_name);
    int (*fn)(void *) = p;
    return (void*)fn(sContext.thread_context);
}

SDL_Thread *SDL_CreateThread(int (*fn)(void *), const char *name, void *context)
{
    pthread_t *thread = malloc(sizeof(pthread_t));
    
    sContext.thread_name = name;
    sContext.thread_context = context;
    pthread_create(thread, NULL, Fake_SDL_New_Thread, fn);
    return (SDL_Thread*)thread;
}

void SDL_WaitThread(SDL_Thread *thread, int *status)
{
    void *_status;
    
    pthread_join(*((pthread_t*)thread), &_status);
    *status = (int)_status;
    free(thread);
}

SDL_sem *SDL_CreateSemaphore(int initial_value)
{
    sem_t *semaphore = (sem_t *)malloc(sizeof(sem_t));
    sem_init(semaphore, 0, initial_value);
    
    return (SDL_sem*)semaphore;
}

int SDL_SemPost(SDL_sem *sem)
{
    int retval;
    
    if ( ! sem ) {
        // Passed a NULL semaphore
        return -1;
    }
    
    retval = sem_post((sem_t *)sem);
    
    return retval;
}

int SDL_SemTryWait(SDL_sem *sem)
{
	int retval;
    
	if ( ! sem ) {
		// Passed a NULL semaphore
		return -1;
	}
    
	retval = 1;
    
    if ( sem_trywait((sem_t *)sem) == 0 ) {
		retval = 0;
	}
	return retval;
}
