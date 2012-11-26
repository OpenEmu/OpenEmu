#ifndef __MDFN_MEDNAFEN_DRIVER_H
#define __MDFN_MEDNAFEN_DRIVER_H

#include <stdio.h>
#include <vector>
#include <string>

#include "settings-common.h"

extern std::vector<MDFNGI *>MDFNSystems;

/* Indent stdout newlines +- "indent" amount */
void MDFN_indent(int indent);
void MDFN_printf(const char *format, ...) throw() MDFN_FORMATSTR(printf, 1, 2);

#define MDFNI_printf MDFN_printf

/* Displays an error.  Can block or not. */
void MDFND_PrintError(const char *s);
void MDFND_Message(const char *s);

uint32 MDFND_GetTime(void);
void MDFND_Sleep(uint32 ms);

// Synchronize virtual time to actual time using members of espec:
//
//  MasterCycles and MasterCyclesALMS (coupled with MasterClock of MDFNGI)
//   and/or
//  SoundBuf, SoundBufSize, and SoundBufSizeALMS
//
// ...and after synchronization, update the data pointed to by the pointers passed to MDFNI_SetInput().
// DO NOT CALL MDFN_* or MDFNI_* functions from within MDFND_MidSync().
// Calling MDFN_printf(), MDFN_DispMessage(),and MDFND_PrintError() are ok, though.
//
// If you do not understand how to implement this function, you can leave it empty at first, but know that doing so
// will subtly break at least one PC Engine game(Takeda Shingen), and raise input latency on some other PC Engine games.
void MDFND_MidSync(const EmulateSpecStruct *espec);

/* Being threading support. */
// Mostly based off SDL's prototypes and semantics.
// Driver code should actually define MDFN_Thread and MDFN_Mutex.

struct MDFN_Thread;
struct MDFN_Mutex;

MDFN_Thread *MDFND_CreateThread(int (*fn)(void *), void *data);
void MDFND_WaitThread(MDFN_Thread *thread, int *status);
void MDFND_KillThread(MDFN_Thread *thread);

MDFN_Mutex *MDFND_CreateMutex(void);
void MDFND_DestroyMutex(MDFN_Mutex *mutex);
int MDFND_LockMutex(MDFN_Mutex *mutex);
int MDFND_UnlockMutex(MDFN_Mutex *mutex);

/* End threading support. */

void MDFNI_Reset(void);
void MDFNI_Power(void);

/* path = path of game/file to load.  returns NULL on failure. */
MDFNGI *MDFNI_LoadGame(const char *force_module, const char *path);

MDFNGI *MDFNI_LoadCD(const char *sysname, const char *devicename);

// Call this function as early as possible, even before MDFNI_Initialize()
bool MDFNI_InitializeModules(const std::vector<MDFNGI *> &ExternalSystems);

/* allocates memory.  0 on failure, 1 on success. */
/* Also pass it the base directory to load the configuration file. */
int MDFNI_Initialize(const char *basedir, const std::vector<MDFNSetting> &DriverSettings);

/* Sets the base directory(save states, snapshots, etc. are saved in directories
   below this directory. */
void MDFNI_SetBaseDirectory(const char *dir);

/* Call only when a game is loaded. */
int MDFNI_NetplayStart(uint32 local_players, const std::string &nickname, const std::string &game_key, const std::string &connect_password);

/* Emulates a frame. */
void MDFNI_Emulate(EmulateSpecStruct *espec);

/* Closes currently loaded game */
void MDFNI_CloseGame(void);

/* Deallocates all allocated memory.  Call after MDFNI_Emulate() returns. */
void MDFNI_Kill(void);

void MDFN_DispMessage(const char *format, ...) throw() MDFN_FORMATSTR(printf, 1, 2);
#define MDFNI_DispMessage MDFN_DispMessage

uint32 MDFNI_CRC32(uint32 crc, uint8 *buf, uint32 len);

// NES hackish function.  Should abstract in the future.
int MDFNI_DatachSet(const uint8 *rcode);

void MDFNI_DoRewind(void);

void MDFNI_SetLayerEnableMask(uint64 mask);

void MDFNI_SetInput(int port, const char *type, void *ptr, uint32 dsize);

//int MDFNI_DiskInsert(int oride);
//int MDFNI_DiskEject(void);
//int MDFNI_DiskSelect(void);

// Arcade-support functions
// We really need to reexamine how we should abstract this, considering the initial state of the DIP switches,
// and moving the DIP switch drawing code to the driver side.
void MDFNI_ToggleDIP(int which);
void MDFNI_InsertCoin(void);
void MDFNI_ToggleDIPView(void);

// Disk/Disc-based system support functions
void MDFNI_DiskSelect(int which);
void MDFNI_DiskSelect();
void MDFNI_DiskInsert();
void MDFNI_DiskEject();

// New removable media interface(TODO!)
//
#if 0

struct MediumInfoStruct
{
 const char *name;		// More descriptive name, "Al Gore's Grand Adventure, Disk 1 of 7" ???
				// (remember, Do utf8->utf32->utf8 for truncation for display)
 const char *set_member_name;	// "Disk 1 of 4, Side A", "Disk 3 of 4, Side B", "Disc 2 of 5" ???? (Disk M of N, where N is related to the number of entries 
				// in the structure???)
};

struct DriveInfoStruct
{
 const char *name;
 const char *description;
 const MediumInfoStruct *possible_media;
 //bool 
 //const char *eject_state_name;	// Like "Lid Open", or "Tray Ejected"
 //const char *insert_state_name;	// Like "
};

 // Entry point
 DriveInfoStruct *Drives;

void MDFNI_SetDriveMedium(unsigned drive_index, unsigned int medium_index, unsigned state_id);
#endif


bool MDFNI_StartAVRecord(const char *path, double SoundRate);
void MDFNI_StopAVRecord(void);

bool MDFNI_StartWAVRecord(const char *path, double SoundRate);
void MDFNI_StopWAVRecord(void);

void MDFNI_DumpModulesDef(const char *fn);


#endif
