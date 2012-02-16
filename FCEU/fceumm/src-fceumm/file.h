#ifndef _FCEU_FILE_H
#define _FCEU_FILE_H

typedef struct {
  void *fp;       /* FILE* or ptr to ZIPWRAP*/
  uint32 type;    /* 0=normal file, 1=gzip, 2=zip*/
} FCEUFILE;

FCEUFILE *FCEU_fopen(const char *path, const char *ipsfn, char *mode, char *ext);
int FCEU_fclose(FCEUFILE*);
uint64 FCEU_fread(void *ptr, size_t size, size_t nmemb, FCEUFILE*);
uint64 FCEU_fwrite(void *ptr, size_t size, size_t nmemb, FCEUFILE*);
int FCEU_fseek(FCEUFILE*, long offset, int whence);
int FCEU_read32le(uint32 *Bufo, FCEUFILE*);
int FCEU_read16le(uint16 *Bufo, FCEUFILE*);
int FCEU_fgetc(FCEUFILE*);
uint64 FCEU_fgetsize(FCEUFILE*);

#endif
