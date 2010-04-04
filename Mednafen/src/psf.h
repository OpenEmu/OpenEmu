#ifndef _PSF_H
#define _PSF_H

typedef struct __PSFTAG
{
 char *key;
 char *value;
 struct __PSFTAG *next;
} PSFTAG;

typedef struct {
        uint32 length;
        uint32 stop;
        uint32 fade;
        char *title,*artist,*game,*year,*genre,*psfby,*comment,*copyright;
        PSFTAG *tags;
} PSFINFO;

int MDFNPSF_Load(int desired_version, MDFNFILE *fp, PSFINFO **info, void (*datafunc)(void *data, uint32 len));
void MDFNPSF_Free(PSFINFO *info);

#endif
