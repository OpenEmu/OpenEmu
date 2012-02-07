#ifndef GLOBALS_H
#define GLOBALS_H

#include "../common/Types.h"
#include "GBA.h"

extern bool ioReadable[0x400];
extern bool N_FLAG;
extern bool C_FLAG;
extern bool Z_FLAG;
extern bool V_FLAG;
extern bool armState;
extern bool armIrqEnable;
extern int armMode;
extern int saveType;
extern bool useBios;
extern bool skipBios;
extern int frameSkip;
#ifdef USE_FRAMESKIP
extern bool speedup;
#endif
extern bool cpuIsMultiBoot;
extern bool speedHack;
extern int cpuSaveType;
extern bool cheatsEnabled;
extern bool mirroringEnable;
extern bool enableRtc;
extern bool skipSaveGameBattery; // skip battery data when reading save states
extern bool skipSaveGameCheats;  // skip cheat list data when reading save states

extern u8 *bios;
extern u8 *rom;
extern u8 *internalRAM;
extern u8 *workRAM;
extern u8 *vram;
extern u8 *pix;
extern u8 *oam;
extern u8 *ioMem;

extern u16 VCOUNT;
extern u16 MOSAIC;
extern u16 TM0D;
extern u16 TM0CNT;
extern u16 TM1D;
extern u16 TM1CNT;
extern u16 TM2D;
extern u16 TM2CNT;
extern u16 TM3D;
extern u16 TM3CNT;
extern u16 P1;
extern u16 IE;
extern u16 IF;
extern u16 IME;

#endif // GLOBALS_H
