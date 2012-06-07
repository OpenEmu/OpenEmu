#include "mednafen/mednafen-types.h"
#include "mednafen/mednafen.h"
#include "mednafen/git.h"
#include "mednafen/general.h"
#include "mednafen/mednafen-driver.h"
#include "mednafen/netplay-driver.h"

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

// Stubs

void MDFND_Sleep(unsigned int time)
{
   usleep(time * 1000);
}

void MDFND_DispMessage(unsigned char *str)
{
   std::cerr << str;
}

void MDFND_Message(const char *str)
{
   std::cerr << str;
}

void MDFND_MidSync(const EmulateSpecStruct *)
{}

void MDFND_PrintError(const char* err)
{
   std::cerr << err;
}

MDFN_Thread *MDFND_CreateThread(int (*fn)(void *), void *data)
{
   pthread_t *thread = new pthread_t;

   pthread_create(thread, NULL, (void* (*)(void *))fn, data);

   return (MDFN_Thread*)thread;
}

void MDFND_SetMovieStatus(StateStatusStruct *) {}
void MDFND_SetStateStatus(StateStatusStruct *) {}

void MDFND_WaitThread(MDFN_Thread *thr, int *val)
{
   pthread_t *thread = (pthread_t*)thr;
   void *data;
   pthread_join(*thread, &data);

   if (val)
      *val = (intptr_t)data;

   delete thread;
}

void MDFND_KillThread(MDFN_Thread *thr)
{
   pthread_t *thread = (pthread_t*)thr;
   pthread_cancel(*thread);
   pthread_join(*thread, NULL);
   delete thread;
}

MDFN_Mutex *MDFND_CreateMutex()
{
   pthread_mutex_t *mutex = new pthread_mutex_t;
   pthread_mutex_init(mutex, NULL);
   return (MDFN_Mutex*)mutex;
}

void MDFND_DestroyMutex(MDFN_Mutex *lock)
{
   pthread_mutex_t *mutex = (pthread_mutex_t*)lock;
   pthread_mutex_destroy(mutex);
   delete mutex;
}

int MDFND_LockMutex(MDFN_Mutex *lock)
{
   pthread_mutex_t *mutex = (pthread_mutex_t*)lock;
   pthread_mutex_lock(mutex);
   return 0;
}

int MDFND_UnlockMutex(MDFN_Mutex *lock)
{
   pthread_mutex_t *mutex = (pthread_mutex_t*)lock;
   pthread_mutex_unlock(mutex);
   return 0;
}

int MDFND_SendData(const void*, uint32) { return 0; }
int MDFND_RecvData(void *, uint32) { return 0; }
void MDFND_NetplayText(const uint8*, bool) {}
void MDFND_NetworkClose() {}

uint32 MDFND_GetTime()
{
   struct timeval val;
   gettimeofday(&val, NULL);
   uint32_t ms = val.tv_sec * 1000 + val.tv_usec / 1000;

   static bool first = true;
   static uint32_t start_ms;
   if (first)
   {
      start_ms = ms;
      first = false;
   }

   return ms - start_ms;
}

