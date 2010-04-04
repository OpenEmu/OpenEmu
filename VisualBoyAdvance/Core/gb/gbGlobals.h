#ifndef GBGLOBALS_H
#define GBGLOBALS_H

extern int gbRomSizeMask;
extern int gbRomSize;
extern int gbRamSize;
extern int gbRamSizeMask;
extern int gbTAMA5ramSize;

extern bool useBios;
extern bool skipBios;
extern u8 *bios;
extern bool skipSaveGameBattery;
extern bool skipSaveGameCheats;

extern u8 *gbRom;
extern u8 *gbRam;
extern u8 *gbVram;
extern u8 *gbWram;
extern u8 *gbMemory;
extern u16 *gbLineBuffer;
extern u8 *gbTAMA5ram;

extern u8 *gbMemoryMap[16];

extern int gbFrameSkip;
extern u16 gbColorFilter[32768];
extern int gbColorOption;
extern int gbPaletteOption;
extern int gbEmulatorType;
extern int gbBorderOn;
extern int gbBorderAutomatic;
extern int gbCgbMode;
extern int gbSgbMode;
extern int gbWindowLine;
extern int gbSpeed;
extern u8 gbBgp[4];
extern u8 gbObp0[4];
extern u8 gbObp1[4];
extern u16 gbPalette[128];
extern bool gbScreenOn;
extern bool gbDrawWindow;
extern u8 gbSCYLine[300];
// gbSCXLine is used for the emulation (bug) of the SX change
// found in the Artic Zone game.
extern u8 gbSCXLine[300];
// gbBgpLine is used for the emulation of the
// Prehistorik Man's title screen scroller.
extern u8 gbBgpLine[300];
extern u8 gbObp0Line [300];
extern u8 gbObp1Line [300];
// gbSpritesTicks is used for the emulation of Parodius' Laser Beam.
extern u8 gbSpritesTicks[300];

extern u8 register_LCDC;
extern u8 register_LY;
extern u8 register_SCY;
extern u8 register_SCX;
extern u8 register_WY;
extern u8 register_WX;
extern u8 register_VBK;
extern u8 oldRegister_WY;

extern int emulating;
extern bool genericflashcardEnable;

extern int gbBorderLineSkip;
extern int gbBorderRowSkip;
extern int gbBorderColumnSkip;
extern int gbDmaTicks;

extern void gbRenderLine();
extern void gbDrawSprites(bool);

extern u8 (*gbSerialFunction)(u8);

#endif // GBGLOBALS_H
