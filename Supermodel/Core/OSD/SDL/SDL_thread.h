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

#ifndef SDL_THREAD_H
#define SDL_THREAD_H

typedef void *SDL_mutex;
typedef void *SDL_Thread;
typedef void *SDL_cond;
typedef void *SDL_sem;

SDL_mutex *SDL_CreateMutex(void);
SDL_Thread *SDL_CreateThread(int (*fn)(void *), void *data);
SDL_cond *SDL_CreateCond(void);
SDL_sem *SDL_CreateSemaphore(Uint32);
int SDL_LockMutex(SDL_mutex *m);
int SDL_UnlockMutex(SDL_mutex *m);
Uint32 SDL_SemValue(SDL_sem *sem);
int SDL_SemWait(SDL_sem *sem);
int SDL_SemPost(SDL_sem *sem);
int SDL_CondWait(SDL_cond *cond, SDL_mutex *m);
int SDL_CondSignal(SDL_cond *cond);
int SDL_CondBroadcast(SDL_cond *cond);
int SDL_mutexP(SDL_mutex *m);
int SDL_mutexV(SDL_mutex *m);
void SDL_DestroyMutex(SDL_mutex *m);
void SDL_DestroyCond(SDL_cond *m);
void SDL_DestroySemaphore(SDL_sem *m);
void SDL_KillThread(SDL_Thread *thr);
void SDL_WaitThread(SDL_Thread *thr, int *status);
Uint32 SDL_GetThreadID(SDL_Thread *thr);
#endif