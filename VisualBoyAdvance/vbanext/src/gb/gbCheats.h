#ifndef GBCHEATS_H
#define GBCHEATS_H

#include "../System.h"

struct gbXxCheat {
  char cheatDesc[100];
  char cheatCode[20];
};

struct gbCheat {
  char cheatCode[20];
  char cheatDesc[32];
  uint16_t address;
  int code;
  uint8_t compare;
  uint8_t value;
  bool enabled;
};

void gbCheatsSaveGame(gzFile);
void gbCheatsReadGame(gzFile, int);
void gbCheatsReadGameSkip(gzFile, int);
void gbCheatsSaveCheatList(const char *);
bool gbCheatsLoadCheatList(const char *);
bool gbCheatReadGSCodeFile(const char *);

bool gbAddGsCheat(const char *, const char*);
bool gbAddGgCheat(const char *, const char*);
void gbCheatRemove(int);
void gbCheatRemoveAll();
void gbCheatEnable(int);
void gbCheatDisable(int);
uint8_t gbCheatRead(uint16_t);
void gbCheatWrite(bool);
bool gbVerifyGsCode(const char *code);
bool gbVerifyGgCode(const char *code);


extern int gbCheatNumber;
extern gbCheat gbCheatList[100];
extern bool gbCheatMap[0x10000];

#endif // GBCHEATS_H
