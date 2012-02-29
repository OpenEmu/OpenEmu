#include <cstdlib>
#include "../common/Types.h"

u8 *gbMemoryMap[16];

int gbRomSizeMask = 0;
int gbRomSize = 0;
int gbRamSizeMask = 0;
int gbRamSize = 0;
int gbTAMA5ramSize = 0;

u8 *gbMemory = NULL;
u8 *gbVram = NULL;
u8 *gbRom = NULL;
u8 *gbRam = NULL;
u8 *gbWram = NULL;
u16 *gbLineBuffer = NULL;
u8 *gbTAMA5ram = NULL;

u16 gbPalette[128];
u8 gbBgp[4]  = { 0, 1, 2, 3};
u8 gbObp0[4] = { 0, 1, 2, 3};
u8 gbObp1[4] = { 0, 1, 2, 3};
int gbWindowLine = -1;

bool genericflashcardEnable = false;
int gbCgbMode = 0;

u16 gbColorFilter[32768];
bool gbColorOption = false;
int gbPaletteOption = 0;
int gbEmulatorType = 0;
bool gbBorderOn = true;
bool gbBorderAutomatic = false;
int gbBorderLineSkip = 160;
int gbBorderRowSkip = 0;
int gbBorderColumnSkip = 0;
int gbDmaTicks = 0;

u8 (*gbSerialFunction)(u8) = NULL;
