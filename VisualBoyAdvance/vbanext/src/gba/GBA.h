#ifndef GBA_H
#define GBA_H

#include "../System.h"

#define BITS_16 0
#define BITS_32 1

#define SAVE_GAME_VERSION_1 1
#define SAVE_GAME_VERSION_2 2
#define SAVE_GAME_VERSION_3 3
#define SAVE_GAME_VERSION_4 4
#define SAVE_GAME_VERSION_5 5
#define SAVE_GAME_VERSION_6 6
#define SAVE_GAME_VERSION_7 7
#define SAVE_GAME_VERSION_8 8
#define SAVE_GAME_VERSION_9 9
#define SAVE_GAME_VERSION_10 10
#define SAVE_GAME_VERSION  SAVE_GAME_VERSION_10

typedef struct {
  uint8_t *address;
  uint32_t mask;
} memoryMap;

typedef union {
  struct {
#ifdef LSB_FIRST
    uint8_t B0;
    uint8_t B1;
    uint8_t B2;
    uint8_t B3;
#else
    uint8_t B3;
    uint8_t B2;
    uint8_t B1;
    uint8_t B0;
#endif
  } B;
  struct {
#ifdef LSB_FIRST
    uint16_t W0;
    uint16_t W1;
#else
    uint16_t W1;
    uint16_t W0;
#endif
  } W;
#ifdef LSB_FIRST
	uint32_t I;
#else
	volatile uint32_t I;
#endif
} reg_pair;

typedef struct 
{
	reg_pair reg[45];
	bool busPrefetch;
	bool busPrefetchEnable;
	uint32_t busPrefetchCount;
	uint32_t armNextPC;
} bus_t;

typedef struct
{
	uint8_t * paletteRAM;
	uint16_t DISPCNT;
	uint16_t DISPSTAT;
	int layerSettings;
	int layerEnable;
	int layerEnableDelay;
	int lcdTicks;
} graphics_t;

extern bus_t bus;
extern graphics_t graphics;

#ifndef NO_GBA_MAP
extern memoryMap map[256];
#endif

extern uint8_t biosProtected[4];

extern uint64_t joy;
extern void (*cpuSaveGameFunc)(uint32_t,uint8_t);

extern bool CPUReadGSASnapshot(const char *);
extern bool CPUReadGSASPSnapshot(const char *);
extern bool CPUWriteGSASnapshot(const char *, const char *, const char *, const char *);
extern bool CPUWriteBatteryFile(const char *);
extern bool CPUReadBatteryFile(const char *);
extern bool CPUExportEepromFile(const char *);
extern bool CPUImportEepromFile(const char *);
extern bool CPUReadMemState(char *, int);
extern bool CPUReadState(const char *);
#ifdef __LIBSNES__
extern bool CPUReadState_libgba(const uint8_t*, unsigned);
#endif
extern bool CPUWriteMemState(char *, int);
#ifdef __LIBSNES__
extern unsigned CPUWriteState_libgba(uint8_t*, unsigned);
#endif
extern bool CPUWriteState(const char *);
extern int CPULoadRom(const char *);
extern void doMirroring(bool);
extern void CPUUpdateRegister(uint32_t, uint16_t);
extern void applyTimer ();
extern void CPUInit(const char *,bool);
extern void CPUReset();
#ifdef USE_FRAMESKIP
extern void CPULoop(int ticks);
#else
extern void CPULoop();
#endif
extern void CPUCheckDMA(int,int);

#define R13_IRQ  18
#define R14_IRQ  19
#define SPSR_IRQ 20
#define R13_USR  26
#define R14_USR  27
#define R13_SVC  28
#define R14_SVC  29
#define SPSR_SVC 30
#define R13_ABT  31
#define R14_ABT  32
#define SPSR_ABT 33
#define R13_UND  34
#define R14_UND  35
#define SPSR_UND 36
#define R8_FIQ   37
#define R9_FIQ   38
#define R10_FIQ  39
#define R11_FIQ  40
#define R12_FIQ  41
#define R13_FIQ  42
#define R14_FIQ  43
#define SPSR_FIQ 44

#ifdef USE_CHEATS
#include "Cheats.h"
#endif
#include "Globals.h"
#include "EEprom.h"
#include "Flash.h"

#endif // GBA_H
