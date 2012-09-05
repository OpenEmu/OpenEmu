/*
 Copyright (c) 2012 OpenEmu Team
 
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
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>

SDL_mutex *SDL_CreateMutex(void)
{
    //pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_t *m = new pthread_mutex_t;
    pthread_mutex_init(m, NULL);
    
    return (SDL_mutex*)m;
}

SDL_Thread *SDL_CreateThread(int (*fn)(void *), void *data)
{
    //pthread_t *thread = malloc(sizeof(pthread_t));
    pthread_t *thread = new pthread_t;
    
    pthread_create(thread, NULL, (void* (*)(void *))fn, data);
    
    return (SDL_Thread*)thread;
}

SDL_cond *SDL_CreateCond(void)
{
    //TODO
}

SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
    sem_t *semaphore = new sem_t;
    sem_init(semaphore, 0, initial_value);
    
    return (SDL_sem*)semaphore;
}

int SDL_LockMutex(SDL_mutex *m)
{
    return pthread_mutex_lock((pthread_mutex_t*)m);
}

int SDL_UnlockMutex(SDL_mutex *m)
{
    return pthread_mutex_unlock((pthread_mutex_t*)m);
}

Uint32 SDL_SemValue(SDL_sem *sem)
{
    //TODO
}

int SDL_SemWait(SDL_sem *sem)
{
    //TODO
}

int SDL_SemPost(SDL_sem *sem)
{
    //TODO
}

int SDL_CondWait(SDL_cond *cond, SDL_mutex *m)
{
    //TODO
}

int SDL_CondSignal(SDL_cond *cond)
{
    //TODO
}

int SDL_CondBroadcast(SDL_cond *cond)
{
    //TODO
}

int SDL_mutexP(SDL_mutex *m)
{
    //TODO
}

int SDL_mutexV(SDL_mutex *m)
{
    //TODO
}

void SDL_DestroyMutex(SDL_mutex *m)
{
    pthread_mutex_destroy((pthread_mutex_t*)m);
    free(m);
}

void SDL_DestroyCond(SDL_cond *m)
{
    pthread_cond_destroy((pthread_cond_t*)m);
    free(m);
}
void SDL_DestroySemaphore(SDL_sem *m)
{
    sem_destroy((sem_t *)m);
    free(m);
}

void SDL_KillThread(SDL_Thread *thr)
{
    pthread_t *thread = (pthread_t*)thr;
    pthread_cancel(*thread);
    pthread_join(*thread, NULL);
    free(thr);
}

void SDL_WaitThread(SDL_Thread *thr, int *status)
{
    pthread_t *thread = (pthread_t*)thr;
    void *data;
    pthread_join(*thread, &data);
    
    if (status)
        *status = (intptr_t)data;
    
    free(thr);
}

Uint32 SDL_GetThreadID(SDL_Thread *thr)
{
    Uint32 id;
    
    //TODO
    
    return(id);
}

Uint32 SDL_GetTicks(void)
{   // FIXME use mach_absolute_time
    static struct timeval start = {};
    struct timeval now;
    
    gettimeofday(&now, NULL);
    
    if (!start.tv_sec)
        start = now;
    
    return (now.tv_sec - start.tv_sec)*1000 + (now.tv_usec - start.tv_usec)/1000;
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
    //FIXME: we might need this to do something
}

char *SDL_GetError(void)
{
}