#include <stdint.h>
#include <cstdlib>

uint8_t *gbMemoryMap[16];

int gbRomSizeMask = 0;
int gbRomSize = 0;
int gbRamSizeMask = 0;
int gbRamSize = 0;
int gbTAMA5ramSize = 0;

uint8_t *gbMemory = NULL;
uint8_t *gbVram = NULL;
uint8_t *gbRom = NULL;
uint8_t *gbRam = NULL;
uint8_t *gbWram = NULL;
uint16_t *gbLineBuffer = NULL;
uint8_t *gbTAMA5ram = NULL;

uint16_t gbPalette[128];
uint8_t gbBgp[4]  = { 0, 1, 2, 3};
uint8_t gbObp0[4] = { 0, 1, 2, 3};
uint8_t gbObp1[4] = { 0, 1, 2, 3};
int gbWindowLine = -1;

bool genericflashcardEnable = false;
int gbCgbMode = 0;

uint16_t gbColorFilter[32768];
int gbColorOption = 0;
int gbPaletteOption = 0;
int gbEmulatorType = 0;
int gbBorderOn = 1;
int gbBorderAutomatic = 0;
int gbBorderLineSkip = 160;
int gbBorderRowSkip = 0;
int gbBorderColumnSkip = 0;
int gbDmaTicks = 0;

uint8_t (*gbSerialFunction)(uint8_t) = NULL;
