#ifndef _FCEUH
#define _FCEUH

extern int fceuindbg;
extern int newppu;
void ResetGameLoaded(void);

#define DECLFR(x) uint8 x (uint32 A)
#define DECLFW(x) void x (uint32 A, uint8 V)

void SetReadHandler(int32 start, int32 end, readfunc func);
void SetWriteHandler(int32 start, int32 end, writefunc func);
writefunc GetWriteHandler(int32 a);
readfunc GetReadHandler(int32 a);

int AllocGenieRW(void);
void FlushGenieRW(void);

void FCEU_ResetVidSys(void);

void ResetMapping(void);
void ResetNES(void);
void PowerNES(void);

void FCEUI_Autosave(void);

extern uint64 timestampbase;
extern uint32 MMC5HackVROMMask;
extern uint8 *MMC5HackExNTARAMPtr;
extern int MMC5Hack;
extern uint8 *MMC5HackVROMPTR;
extern uint8 MMC5HackCHRMode;
extern uint8 MMC5HackSPMode;
extern uint8 MMC50x5130;
extern uint8 MMC5HackSPScroll;
extern uint8 MMC5HackSPPage;


#define GAME_MEM_BLOCK_SIZE 131072

extern  uint8  *RAM;            //shared memory modifications
extern  uint8  *GameMemBlock;   //shared memory modifications

extern readfunc ARead[0x10000];
extern writefunc BWrite[0x10000];

enum GI {
	GI_RESETM2	=1,
	GI_POWER =2,
	GI_CLOSE =3,
	GI_RESETSAVE = 4
};

extern void (*GameInterface)(GI h);
extern void (*GameStateRestore)(int version);


#include "git.h"
extern FCEUGI *GameInfo;
extern int GameAttributes;

extern uint8 PAL;

typedef struct {
	int PAL;
	int SoundVolume;		//Master volume
	int TriangleVolume;
	int Square1Volume;
	int Square2Volume;
	int NoiseVolume;
	int PCMVolume;
	bool GameGenie;

	//the currently selected first and last rendered scanlines.
	int FirstSLine;
	int LastSLine;

	//Driver-supplied user-selected first and last rendered scanlines.
	//Usr*SLine[0] is for NTSC, Usr*SLine[1] is for PAL.
	int UsrFirstSLine[2];
	int UsrLastSLine[2];

	uint32 SndRate;
} FCEUS;

//the number of scanlines in the currently selected configuration
#define TotalScanLines() (FSettings.LastSLine - FSettings.FirstSLine + 1)

int FCEU_TextScanlineOffset(int y);
int FCEU_TextScanlineOffsetFromBottom(int y);

extern FCEUS FSettings;

bool CheckFileExists(const char* filename);	//Receives a filename (fullpath) and checks to see if that file exists

void FCEU_PrintError(const char *format, ...);
void FCEU_printf(const char *format, ...);
void FCEU_DispMessage(const char *format, int disppos, ...);
void FCEU_TogglePPU();

void SetNESDeemph(uint8 d, int force);

extern uint8 pale;
extern uint8 vsdip;

#define JOY_A		1
#define JOY_B		2
#define JOY_SELECT      4
#define JOY_START       8
#define JOY_UP		0x10
#define JOY_DOWN        0x20
#define JOY_LEFT        0x40
#define JOY_RIGHT       0x80

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif
