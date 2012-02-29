#ifndef UTIL_H
#define UTIL_H

#include "System.h"

enum IMAGE_TYPE {
  IMAGE_UNKNOWN = -1,
  IMAGE_GBA     = 0,
  IMAGE_GB      = 1
};

// save game
typedef struct {
  void *address;
  int size;
} variable_desc;

bool utilWritePNGFile(const char *, int, int, u8 *);
bool utilWriteBMPFile(const char *, int, int, u8 *);
void utilApplyIPS(const char *ips, u8 **rom, int *size);
bool utilIsGBAImage(const char *);
bool utilIsGBImage(const char *);
bool utilIsGzipFile(const char *);
void utilStripDoubleExtension(const char *, char *);
IMAGE_TYPE utilFindType(const char *);
IMAGE_TYPE utilFindType(const char *, char (&)[2048]);
u8 *utilLoad(const char *, bool (*)(const char*), u8 *, int &);

void utilPutDword(u8 *, u32);
void utilPutWord(u8 *, u16);
void utilWriteData(gzFile, variable_desc *);
void utilReadData(gzFile, variable_desc *);
void utilReadDataSkip(gzFile, variable_desc *);
int utilReadInt(gzFile);
void utilWriteInt(gzFile, int);
gzFile utilGzOpen(const char *file, const char *mode);
gzFile utilMemGzOpen(char *memory, int available, const char *mode);
int utilGzWrite(gzFile file, const voidp buffer, unsigned int len);
int utilGzRead(gzFile file, voidp buffer, unsigned int len);
int utilGzClose(gzFile file);
z_off_t utilGzSeek(gzFile file, z_off_t offset, int whence);
long utilGzMemTell(gzFile file);
void utilGBAFindSave(const u8 *, const int);
void utilUpdateSystemColorMaps(bool lcd = false);
bool utilFileExists( const char *filename );

#endif // UTIL_H
