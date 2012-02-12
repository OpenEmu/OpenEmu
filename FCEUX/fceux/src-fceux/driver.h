#ifndef __DRIVER_H_
#define __DRIVER_H_

#include <stdio.h>
#include <string>
#include <iosfwd>

#include "types.h"
#include "git.h"
#include "file.h"

FCEUFILE* FCEUD_OpenArchiveIndex(ArchiveScanRecord& asr, std::string& fname, int innerIndex);
FCEUFILE* FCEUD_OpenArchive(ArchiveScanRecord& asr, std::string& fname, std::string* innerFilename);

//This makes me feel dirty for some reason.
void FCEU_printf(char *format, ...);
#define FCEUI_printf FCEU_printf

//Video interface
void FCEUD_SetPalette(unsigned char index, unsigned char r, unsigned char g, unsigned char b);
void FCEUD_GetPalette(unsigned char i, unsigned char *r, unsigned char *g, unsigned char *b);

//Displays an error.  Can block or not.
void FCEUD_PrintError(const char *s);
void FCEUD_Message(const char *s);

void FCEUI_ResetNES(void);
void FCEUI_PowerNES(void);

void FCEUI_NTSCSELHUE(void);
void FCEUI_NTSCSELTINT(void);
void FCEUI_NTSCDEC(void);
void FCEUI_NTSCINC(void);
void FCEUI_GetNTSCTH(int *tint, int *hue);
void FCEUI_SetNTSCTH(int n, int tint, int hue);

void FCEUI_SetInput(int port, ESI type, void *ptr, int attrib);
void FCEUI_SetInputFC(ESIFC type, void *ptr, int attrib);

//tells the emulator whether a fourscore is attached
void FCEUI_SetInputFourscore(bool attachFourscore);
//tells whether a fourscore is attached
bool FCEUI_GetInputFourscore();
//tells whether the microphone is used
bool FCEUI_GetInputMicrophone();

//New interface functions

//0 to keep 8-sprites limitation, 1 to remove it
void FCEUI_DisableSpriteLimitation(int a);

//name=path and file to load.  returns null if it failed
FCEUGI *FCEUI_LoadGame(const char *name, int OverwriteVidMode);

//same as FCEUI_LoadGame, except that it can load from a tempfile. 
//name is the logical path to open; archiveFilename is the archive which contains name
FCEUGI *FCEUI_LoadGameVirtual(const char *name, int OverwriteVidMode);

//general purpose emulator initialization. returns true if successful
bool FCEUI_Initialize();

//Closes currently loaded game
void FCEUI_CloseGame(void);

//Deallocates all allocated memory.  Call after FCEUI_Emulate() returns.
void FCEUI_Kill(void);

//Enable/Disable game genie. a=true->enabled
void FCEUI_SetGameGenie(bool a);

//Set video system a=0 NTSC, a=1 PAL
void FCEUI_SetVidSystem(int a);

//Convenience function; returns currently emulated video system(0=NTSC, 1=PAL).
int FCEUI_GetCurrentVidSystem(int *slstart, int *slend);

//First and last scanlines to render, for ntsc and pal emulation.
void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall);

//Sets the base directory(save states, snapshots, etc. are saved in directories below this directory.
void FCEUI_SetBaseDirectory(std::string const & dir);

//Tells FCE Ultra to copy the palette data pointed to by pal and use it.
//Data pointed to by pal needs to be 64*3 bytes in length.
void FCEUI_SetPaletteArray(uint8 *pal);

//Sets up sound code to render sound at the specified rate, in samples
//per second.  Only sample rates of 44100, 48000, and 96000 are currently supported.
//If "Rate" equals 0, sound is disabled.
void FCEUI_Sound(int Rate);
void FCEUI_SetSoundVolume(uint32 volume);
void FCEUI_SetTriangleVolume(uint32 volume);
void FCEUI_SetSquare1Volume(uint32 volume);
void FCEUI_SetSquare2Volume(uint32 volume);
void FCEUI_SetNoiseVolume(uint32 volume);
void FCEUI_SetPCMVolume(uint32 volume);

void FCEUD_SoundToggle(void);

int FCEUI_SelectState(int, int);
extern void FCEUI_SelectStateNext(int);

//"fname" overrides the default save state filename code if non-NULL.
void FCEUI_SaveState(const char *fname);
uint32_t FCEUI_LoadState(const char *fname);

//at the minimum, you should call FCEUI_SetInput, FCEUI_SetInputFC, and FCEUI_SetInputFourscore
//you may also need to maintain your own internal state
void FCEUD_SetInput(bool fourscore, bool microphone, ESI port0, ESI port1, ESIFC fcexp);

int32 FCEUI_GetDesiredFPS(void);
void FCEU_DispMessage(const char *format, int disppos, ...);

int FCEUI_DecodePAR(const char *code, int *a, int *v, int *c, int *type);
int FCEUI_DecodeGG(const char *str, int *a, int *v, int *c);
int FCEUI_AddCheat(const char *name, uint32 addr, uint8 val, int compare, int type);
int FCEUI_DelCheat(uint32 which);
int FCEUI_ToggleCheat(uint32 which);

int32 FCEUI_CheatSearchGetCount(void);
void FCEUI_CheatSearchGetRange(uint32 first, uint32 last, int (*callb)(uint32 a, uint8 last, uint8 current));
void FCEUI_CheatSearchGet(int (*callb)(uint32 a, uint8 last, uint8 current, void *data), void *data);
void FCEUI_CheatSearchBegin(void);
void FCEUI_CheatSearchEnd(int type, uint8 v1, uint8 v2);
void FCEUI_ListCheats(int (*callb)(char *name, uint32 a, uint8 v, int compare, int s, int type, void *data), void *data);

int FCEUI_GetCheat(uint32 which, char **name, uint32 *a, uint8 *v, int *compare, int *s, int *type);
int FCEUI_SetCheat(uint32 which, const char *name, int32 a, int32 v, int compare,int s, int type);

void FCEUI_CheatSearchShowExcluded(void);
void FCEUI_CheatSearchSetCurrentAsOriginal(void);

//.rom
#define FCEUIOD_ROMS    0	//Roms
#define FCEUIOD_NV      1	//NV = nonvolatile. save data.	
#define FCEUIOD_STATES  2	//savestates	
#define FCEUIOD_FDSROM  3	//disksys.rom
#define FCEUIOD_SNAPS   4	//screenshots
#define FCEUIOD_CHEATS  5	//cheats
#define FCEUIOD_MOVIES  6	//.fm2 files
#define FCEUIOD_MEMW    7	//memory watch fiels
#define FCEUIOD_BBOT    8	//basicbot, obsolete
#define FCEUIOD_MACRO   9	//macro files - tasedit, currently not implemented
#define FCEUIOD_INPUT   10	//input presets
#define FCEUIOD_AVI		12	//default file for avi output
#define FCEUIOD__COUNT  13	//base directory override?

void FCEUI_SetDirOverride(int which, char *n);

void FCEUI_VSUniToggleDIPView(void);
void FCEUI_VSUniToggleDIP(int w);
uint8 FCEUI_VSUniGetDIPs(void);
void FCEUI_VSUniSetDIP(int w, int state);
void FCEUI_VSUniCoin(void);

void FCEUI_FDSInsert(void); //mbg merge 7/17/06 changed to void fn(void) to make it an EMUCMDFN
//int FCEUI_FDSEject(void);
void FCEUI_FDSSelect(void);

int FCEUI_DatachSet(const uint8 *rcode);

//indicates whether input aids should be drawn (such as crosshairs, etc; usually in fullscreen mode)
bool FCEUD_ShouldDrawInputAids();

///A callback that the emu core uses to poll the state of a given emulator command key
typedef int TestCommandState(int cmd);
///Signals the emu core to poll for emulator commands and take actions
void FCEUI_HandleEmuCommands(TestCommandState* testfn);

//new merge-era driver routines here:

///called when fceu changes something in the video system you might be interested in
void FCEUD_VideoChanged();

enum EFCEUI
{
	FCEUI_QUICKSAVE, FCEUI_QUICKLOAD, FCEUI_SAVESTATE, FCEUI_LOADSTATE,
	FCEUI_NEXTSAVESTATE,FCEUI_PREVIOUSSAVESTATE,FCEUI_VIEWSLOTS,
	FCEUI_RESET, FCEUI_POWER
};

//checks whether an EFCEUI is valid right now
bool FCEU_IsValidUI(EFCEUI ui);

#endif //__DRIVER_H_
