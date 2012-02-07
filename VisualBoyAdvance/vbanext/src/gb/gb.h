#ifndef GB_H
#define GB_H

#define C_FLAG 0x10
#define H_FLAG 0x20
#define N_FLAG 0x40
#define Z_FLAG 0x80

typedef union {
  struct {
#ifdef LSB_FIRST
    uint8_t B0,B1;
#else
    uint8_t B1, B0;
#endif
  } B;
  uint16_t W;
} gbRegister;

bool gbLoadRom(const char *);
#ifdef USE_FRAMESKIP
void gbEmulate(int ticksToStop);
#else
void gbEmulate();
#endif
void gbWriteMemory(register uint16_t, register uint8_t);
void gbDrawLine();
bool gbIsGameboyRom(const char *);
void gbGetHardwareType();
void gbReset();
void gbCleanUp();
void gbCPUInit(const char *,bool);
bool gbWriteBatteryFile(const char *);
bool gbWriteBatteryFile(const char *, bool);
bool gbReadBatteryFile(const char *);
bool gbWriteSaveState(const char *);
bool gbWriteMemSaveState(char *, int);
bool gbReadSaveState(const char *);
bool gbReadMemSaveState(char *, int);
void gbSgbRenderBorder();
bool gbWritePNGFile(const char *);
bool gbWriteBMPFile(const char *);
bool gbReadGSASnapshot(const char *);

extern int gbHardware;

extern int gbJoymask[4];

#endif // GB_H
