#include "mednafen/mednafen-types.h"
#include "mednafen/mednafen.h"
#include "mednafen/md5.h"
#include "mednafen/git.h"
#include "mednafen/general.h"
#include "mednafen/mednafen-driver.h"
#include "mednafen/netplay-driver.h"
#include "thread.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Stubs

void MDFND_Sleep(unsigned int time)
{
#ifdef _WIN32
   Sleep(time);
#else
   usleep(time * 1000);
#endif
}

extern std::string retro_base_directory;
extern std::string retro_base_name;
/*
#ifdef _WIN32
static void sanitize_path(std::string &path)
{
    size_t size = path.size();
    for (size_t i = 0; i < size; i++)
        if (path[i] == '/')
            path[i] = '\\';
}
#endif

// Use a simpler approach to make sure that things go right for libretro.
std::string MDFN_MakeFName(MakeFName_Type type, int id1, const char *cd1)
{
    std::string ret;
    switch (type)
    {
        case MDFNMKF_SAV:
            ret = retro_base_directory + std::string(PSS) + retro_base_name +
            std::string(".") + md5_context::asciistr(MDFNGameInfo->MD5, 0) + std::string(".") +
            std::string(cd1);
            break;
        case MDFNMKF_FIRMWARE:
            ret = std::string(cd1);
            break;
        default:
            break;
    }
    
#ifdef _WIN32
    sanitize_path(ret); // Because Windows path handling is mongoloid.
#endif
    return ret;
}
*/
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
   return (MDFN_Thread*)sthread_create((void (*)(void*))fn, data);
}

void MDFND_SetMovieStatus(StateStatusStruct *) {}
void MDFND_SetStateStatus(StateStatusStruct *) {}

void MDFND_WaitThread(MDFN_Thread *thr, int *val)
{
   sthread_join((sthread_t*)thr);

   if (val)
   {
      *val = 0;
      std::cerr << "WaitThread relies on return value." << std::endl;
   }
}

void MDFND_KillThread(MDFN_Thread *)
{
   std::cerr << "Killing a thread is a BAD IDEA!" << std::endl;
}

MDFN_Mutex *MDFND_CreateMutex()
{
   return (MDFN_Mutex*)slock_new();
}

void MDFND_DestroyMutex(MDFN_Mutex *lock)
{
   slock_free((slock_t*)lock);
}

int MDFND_LockMutex(MDFN_Mutex *lock)
{
   slock_lock((slock_t*)lock);
   return 0;
}

int MDFND_UnlockMutex(MDFN_Mutex *lock)
{
   slock_unlock((slock_t*)lock);
   return 0;
}

void MDFND_SendData(const void*, uint32) {}
void MDFND_RecvData(void *, uint32) {}
void MDFND_NetplayText(const uint8*, bool) {}
void MDFND_NetworkClose() {}
int MDFND_NetworkConnect() { return 0; }

uint32 MDFND_GetTime()
{
   static bool first = true;
   static uint32_t start_ms;

#ifdef _WIN32
   DWORD ms = timeGetTime();
   if (first)
   {
      start_ms = ms;
      first = false;
   }
#else
   struct timeval val;
   gettimeofday(&val, NULL);
   uint32_t ms = val.tv_sec * 1000 + val.tv_usec / 1000;

   if (first)
   {
      start_ms = ms;
      first = false;
   }
#endif

   return ms - start_ms;
}

