#ifndef GBGLOBALS_H
#define GBGLOBALS_H

extern int gbRomSizeMask;
extern int gbRomSize;
extern int gbRamSize;
extern int gbRamSizeMask;
extern int gbTAMA5ramSize;

extern bool useBios;
extern bool skipBios;
extern uint8_t *bios;
extern bool skipSaveGameBattery;
extern bool skipSaveGameCheats;

extern uint8_t *gbRom;
extern uint8_t *gbRam;
extern uint8_t *gbVram;
extern uint8_t *gbWram;
extern uint8_t *gbMemory;
extern uint16_t *gbLineBuffer;
extern uint8_t *gbTAMA5ram;

extern uint8_t *gbMemoryMap[16];

extern int gbFrameSkip;
extern uint16_t gbColorFilter[32768];
extern int gbColorOption;
extern int gbPaletteOption;
extern int gbEmulatorType;
extern int gbBorderOn;
extern int gbBorderAutomatic;
extern int gbCgbMode;
extern int gbSgbMode;
extern int gbWindowLine;
extern int gbSpeed;
extern uint8_t gbBgp[4];
extern uint8_t gbObp0[4];
extern uint8_t gbObp1[4];
extern uint16_t gbPalette[128];
extern bool gbScreenOn;
extern bool gbDrawWindow;
extern uint8_t gbSCYLine[300];
// gbSCXLine is used for the emulation (bug) of the SX change
// found in the Artic Zone game.
extern uint8_t gbSCXLine[300];
// gbBgpLine is used for the emulation of the
// Prehistorik Man's title screen scroller.
extern uint8_t gbBgpLine[300];
extern uint8_t gbObp0Line [300];
extern uint8_t gbObp1Line [300];
// gbSpritesTicks is used for the emulation of Parodius' Laser Beam.
extern uint8_t gbSpritesTicks[300];

extern uint8_t register_LCDC;
extern uint8_t register_LY;
extern uint8_t register_SCY;
extern uint8_t register_SCX;
extern uint8_t register_WY;
extern uint8_t register_WX;
extern uint8_t register_VBK;
extern uint8_t oldRegister_WY;

extern int emulating;
extern bool genericflashcardEnable;

extern int gbBorderLineSkip;
extern int gbBorderRowSkip;
extern int gbBorderColumnSkip;
extern int gbDmaTicks;

extern void gbRenderLine();
extern void gbDrawSprites(bool);

extern uint8_t (*gbSerialFunction)(uint8_t);

#endif // GBGLOBALS_H
