#ifndef __MDFN_MEDNAFEN_DRIVER_H
#define __MDFN_MEDNAFEN_DRIVER_H

#include <stdio.h>
#include <vector>
#include <string>

#include "settings-common.h"

extern MDFNGI *MDFNSystems[];
extern unsigned int MDFNSystemCount;

/* Indent stdout newlines +- "indent" amount */
void MDFN_indent(int indent);
void MDFN_printf(const char *format, ...);
#define MDFNI_printf MDFN_printf

/* Displays an error.  Can block or not. */
void MDFND_PrintError(const char *s);
void MDFND_Message(const char *s);

void MDFNI_Reset(void);
void MDFNI_Power(void);

/* Called from the physical CD disc reading code. */
bool MDFND_ExitBlockingLoop(void);

/* name=path and file to load.  returns 0 on failure, 1 on success */
MDFNGI *MDFNI_LoadGame(const char *name);

MDFNGI *MDFNI_LoadCD(const char *sysname, const char *devicename);


/* allocates memory.  0 on failure, 1 on success. */
/* Also pass it the base directory to load the configuration file. */
int MDFNI_Initialize(char *dir, const std::vector<MDFNSetting> &);

/* Call only when a game is loaded. */
int MDFNI_NetplayStart(uint32 local_players, uint32 netmerge, const std::string &nickname, const std::string &game_key, const std::string &connect_password);


#include "nes/ppu/palette-driver.h"

/* Emulates a frame. */
void MDFNI_Emulate(EmulateSpecStruct *espec);

/* Closes currently loaded game */
void MDFNI_CloseGame(void);

/* Deallocates all allocated memory.  Call after MDFNI_Emulate() returns. */
void MDFNI_Kill(void);

/* Sets the base directory(save states, snapshots, etc. are saved in directories
   below this directory. */
void MDFNI_SetBaseDirectory(char *dir);

void MDFNI_SaveSnapshot(void);

void MDFN_DispMessage(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
#define MDFNI_DispMessage MDFN_DispMessage

uint32 MDFNI_CRC32(uint32 crc, uint8 *buf, uint32 len);

int MDFNI_NSFChange(int amount);
int MDFNI_NSFGetInfo(uint8 *name, uint8 *artist, uint8 *copyright, int maxlen);

int MDFNI_DatachSet(const uint8 *rcode);

void MDFNI_DoRewind(void);

void MDFNI_SetPixelFormat(int rshift, int gshift, int bshift, int ashift);

void MDFNI_Sound(int Rate);

void MDFNI_SetSoundVolume(uint32 volume);

void MDFNI_ToggleLayer(int);

void MDFNI_SetInput(int port, const char *type, void *ptr, uint32 dsize);

int MDFNI_DiskInsert(int oride);

int MDFNI_DiskEject(void);

int MDFNI_DiskSelect(void);

#endif
