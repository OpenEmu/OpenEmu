#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include "types.h"
#include "git.h"

/* Video interface */
void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b);
void FCEUD_GetPalette(uint8 i,uint8 *r, uint8 *g, uint8 *b);

/* Displays an error.  Can block or not. */
void FCEUD_PrintError(const char *s);
void FCEUD_Message(const char *s);

void ResetNES(void);
void PowerNES(void);

void FCEUI_NTSCSELHUE(void);
void FCEUI_NTSCSELTINT(void);
void FCEUI_NTSCDEC(void);
void FCEUI_NTSCINC(void);
void FCEUI_GetNTSCTH(int *tint, int *hue);
void FCEUI_SetNTSCTH(int n, int tint, int hue);

void FCEUI_SetInput(int port, int type, void *ptr, int attrib);
void FCEUI_SetInputFC(int type, void *ptr, int attrib);
void FCEUI_DisableFourScore(int s);

#define SI_NONE      0
#define SI_GAMEPAD   1
#define SI_ZAPPER    2
#define SI_POWERPADA  3
#define SI_POWERPADB  4
#define SI_ARKANOID   5
#define SI_MOUSE   6

#define SIFC_NONE      0
#define SIFC_ARKANOID  1
#define SIFC_SHADOW      2
#define SIFC_4PLAYER    3
#define SIFC_FKB      4
#define SIFC_SUBORKB    5
#define SIFC_HYPERSHOT  6
#define SIFC_MAHJONG  7
#define SIFC_QUIZKING  8
#define SIFC_FTRAINERA  9
#define SIFC_FTRAINERB  10
#define SIFC_OEKAKIDS  11
#define SIFC_BWORLD      12
#define SIFC_TOPRIDER  13

#define SIS_NONE  0
#define SIS_DATACH  1
#define SIS_NWC    2
#define SIS_VSUNISYSTEM  3
#define SIS_NSF    4

/* New interface functions */

/* 0 to keep 8-sprites limitation, 1 to remove it */
void FCEUI_DisableSpriteLimitation(int a);

/* name=path and file to load.  returns 0 on failure, 1 on success */
FCEUGI *FCEUI_LoadGame(const char *name);

#ifdef COPYFAMI
/* Fake UNIF board to start new CFHI instance */
FCEUGI *FCEUI_CopyFamiStart();
#endif

/* allocates memory.  0 on failure, 1 on success. */
int FCEUI_Initialize(void);

/* Emulates a frame. */
void FCEUI_Emulate(uint8 **, int32 **, int32 *);

/* Closes currently loaded game */
void FCEUI_CloseGame(void);

/* Enable/Disable game genie. a=0 disable, a=1 enable */
void FCEUI_SetGameGenie(int a);

/* Set video system a=0 NTSC, a=1 PAL */
void FCEUI_SetVidSystem(int a);

/* Convenience function; returns currently emulated video system(0=NTSC, 1=PAL).  */
int FCEUI_GetCurrentVidSystem(int *slstart, int *slend);

/* First and last scanlines to render, for ntsc and pal emulation. */
void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall);

/* Sets the base directory(save states, snapshots, etc. are saved in directories
   below this directory. */
void FCEUI_SetBaseDirectory(char *dir);

/* Tells FCE Ultra to copy the palette data pointed to by pal and use it.
   Data pointed to by pal needs to be 64*3 bytes in length.
*/
void FCEUI_SetPaletteArray(uint8 *pal);

/* Sets up sound code to render sound at the specified rate, in samples
   per second.  Only sample rates of 44100, 48000, and 96000 are currently
   supported.
   If "Rate" equals 0, sound is disabled.
*/
void FCEUI_Sound(int Rate);
void FCEUI_SetSoundVolume(uint32 volume);

int FCEUI_DecodePAR(const char *code, uint16 *a, uint8 *v, int *c, int *type);
int FCEUI_DecodeGG(const char *str, uint16 *a, uint8 *v, int *c);
int FCEUI_AddCheat(const char *name, uint32 addr, uint8 val, int compare, int type);
int FCEUI_DelCheat(uint32 which);
int FCEUI_ToggleCheat(uint32 which);

int FCEUI_GetCheat(uint32 which, char **name, uint32 *a, uint8 *v, int *compare, int *s, int *type);
int FCEUI_SetCheat(uint32 which, const char *name, int32 a, int32 v, int compare,int s, int type);

#define FCEUIOD_STATE   0
#define FCEUIOD_SNAPS   1
#define FCEUIOD_NV      2
#define FCEUIOD_CHEATS  3
#define FCEUIOD_MISC    4
#define FCEUIOD__COUNT  6

void FCEUI_SetDirOverride(int which, const char *n);

uint32 FCEUI_CRC32(uint32 crc, uint8 *buf, uint32 len);

void FCEUI_VSUniToggleDIPView(void);
void FCEUI_VSUniToggleDIP(int w);
uint8 FCEUI_VSUniGetDIPs(void);
void FCEUI_VSUniSetDIP(int w, int state);
void FCEUI_VSUniCoin(void);

int FCEUI_FDSInsert(int oride);
int FCEUI_FDSEject(void);
void FCEUI_FDSSelect(void);

int FCEUI_DatachSet(const uint8 *rcode);

#ifdef  __cplusplus
}
#endif

