#ifndef GB_H
#define GB_H

#define C_FLAG 0x10
#define H_FLAG 0x20
#define N_FLAG 0x40
#define Z_FLAG 0x80

typedef union {
  struct {
#ifdef WORDS_BIGENDIAN
    u8 B1, B0;
#else
    u8 B0,B1;
#endif
  } B;
  u16 W;
} gbRegister;

extern gbRegister AF, BC, DE, HL, SP, PC;
extern u16 IFF;
int gbDis(char *, u16);

bool gbLoadRom(const char *);
bool gbUpdateSizes();
void gbEmulate(int);
void gbWriteMemory(register u16, register u8);
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

extern struct EmulatedSystem GBSystem;

#endif // GB_H
