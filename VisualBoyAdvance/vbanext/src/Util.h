#ifndef UTIL_H
#define UTIL_H

#include "System.h"

enum {
	IMAGE_UNKNOWN,
	IMAGE_GBA,
	IMAGE_GB
};

// save game
typedef struct {
	void *address;
	int size;
} variable_desc;

void utilApplyIPS(const char *ips, uint8_t **rom, int *size);
bool utilIsGBAImage(const char *);
bool utilIsGBImage(const char *);
bool utilIsGzipFile(const char *);
bool utilIsZipFile(const char *);
void utilStripDoubleExtension(const char *, char *);
uint32_t utilFindType(const char *);
uint8_t *utilLoad(const char *, bool (*)(const char*), uint8_t *, int &);

void utilPutDword(uint8_t *, uint32_t);
void utilPutWord(uint8_t *, uint16_t);
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

void utilGBAFindSave(const uint8_t *, const int);

void utilUpdateSystemColorMaps();
bool utilFileExists( const char *filename );

#ifdef __LIBSNES__
void utilWriteIntMem(uint8_t *& data, int);
void utilWriteMem(uint8_t *& data, const void *in_data, unsigned size);
void utilWriteDataMem(uint8_t *& data, variable_desc *);

int utilReadIntMem(const uint8_t *& data);
void utilReadMem(void *buf, const uint8_t *& data, unsigned size);
void utilReadDataMem(const uint8_t *& data, variable_desc *);
#endif

#endif // UTIL_H
