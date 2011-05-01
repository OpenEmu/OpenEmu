/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2007 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
 *  or direct your browser at http://www.gnu.org.
 */

#ifndef DVDISASTER_H
#define DVDISASTER_H

/* "Dare to be gorgeous and unique. 
 *  But don't ever be cryptic or otherwise unfathomable.
 *  Make it unforgettably great."
 *
 *  From "A Final Note on Style", 
 *  Amiga Intuition Reference Manual, 1986, p. 231
 */

/***
 *** I'm too lazy to mess with #include dependencies.
 *** Everything #includeable is rolled up herein...
 */

#include "../types.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/***
 *** dvdisaster.c
 ***/

void PrepareDeadSector(void);

void CreateEcc(void);
void FixEcc(void);
void Verify(void);

uint32 EDCCrc32(unsigned char*, int);

/***
 *** galois.c
 ***
 * This is currently the hardcoded GF(2**8).
 * int32 gives abundant space for the GF.
 * Squeezing it down to uint8 won't probably gain much,
 * so we implement this defensively here.
 *
 * Note that some performance critical stuff needs to
 * be #included from galois-inlines.h
 */  

/* Galois field parameters for 8bit symbol Reed-Solomon code */

#define GF_SYMBOLSIZE 8
#define GF_FIELDSIZE (1<<GF_SYMBOLSIZE)
#define GF_FIELDMAX (GF_FIELDSIZE-1)
#define GF_ALPHA0 GF_FIELDMAX

/* Lookup tables for Galois field arithmetic */

typedef struct _GaloisTables
{  int32 gfGenerator;  /* GF generator polynomial */ 
   int32 *indexOf;     /* log */
   int32 *alphaTo;     /* inverse log */
   int32 *encAlphaTo; /* inverse log optimized for encoder */
} GaloisTables;

/* Lookup and working tables for the ReedSolomon codecs */

typedef struct _ReedSolomonTables
{  GaloisTables *gfTables;/* from above */
   int32 *gpoly;        /* RS code generator polynomial */
   int32 fcr;           /* first consecutive root of RS generator polynomial */
   int32 primElem;      /* primitive field element */
   int32 nroots;        /* degree of RS generator polynomial */
   int32 ndata;         /* data bytes per ecc block */
} ReedSolomonTables;

GaloisTables* CreateGaloisTables(int32);
void FreeGaloisTables(GaloisTables*);

ReedSolomonTables *CreateReedSolomonTables(GaloisTables*, int32, int32, int);
void FreeReedSolomonTables(ReedSolomonTables*);

/*** 
 *** l-ec.c
 ***/

#define N_P_VECTORS   86      /* 43 16bit p vectors */
#define P_VECTOR_SIZE 26      /* using RS(26,24) ECC */

#define N_Q_VECTORS   52      /* 26 16bit q vectors */
#define Q_VECTOR_SIZE 45      /* using RS(45,43) ECC */

#define P_PADDING 229         /* padding values for */
#define Q_PADDING 210         /* shortened RS code  */

int PToByteIndex(int, int);
int QToByteIndex(int, int);
void ByteIndexToP(int, int*, int*);
void ByteIndexToQ(int, int*, int*);

void GetPVector(unsigned char*, unsigned char*, int);
void SetPVector(unsigned char*, unsigned char*, int);
void FillPVector(unsigned char*, unsigned char, int);
void AndPVector(unsigned char*, unsigned char, int);
void OrPVector(unsigned char*, unsigned char, int);

void GetQVector(unsigned char*, unsigned char*, int);
void SetQVector(unsigned char*, unsigned char*, int);
void FillQVector(unsigned char*, unsigned char, int);
void AndQVector(unsigned char*, unsigned char, int);
void OrQVector(unsigned char*, unsigned char, int);

int DecodePQ(ReedSolomonTables*, unsigned char*, int, int*, int);

int CountC2Errors(unsigned char*);

/***
 *** misc.c 
 ***/

char* sgettext(char*);
char* sgettext_utf8(char*);

int64 uchar_to_int64(unsigned char*);
void int64_to_uchar(unsigned char*, int64);

void CalcSectors(int64, int64*, int*);

/***
 *** recover-raw.c
 ***/

#define CD_RAW_SECTOR_SIZE 2352  
#define CD_RAW_C2_SECTOR_SIZE (2352+294)  /* main channel plus C2 vector */

int CheckEDC(unsigned char*, int);
int CheckMSF(unsigned char*, int);


int ValidateRawSector(unsigned char *frame, bool xaMode);
bool Init_LEC_Correct(void);
void Kill_LEC_Correct(void);


#endif				/* DVDISASTER_H */
