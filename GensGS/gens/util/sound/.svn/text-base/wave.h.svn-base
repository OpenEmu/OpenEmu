/*========================================================================== 
 * 
 *  Copyright (C) 1995-1997 Microsoft Corporation. All Rights Reserved. 
 * 
 *  File:wave.h 
 *  Content:wave header 
 * 
 ***************************************************************************/ 
#ifndef __WAVE_INCLUDED__
#define __WAVE_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#define WAVEVERSION 1

#ifndef ER_MEM
#define ER_MEM 0xe000
#endif

#ifndef ER_CANNOTOPEN
#define ER_CANNOTOPEN 0xe100
#endif

#ifndef ER_NOTWAVEFILE
#define ER_NOTWAVEFILE 0xe101
#endif

#ifndef ER_CANNOTREAD
#define ER_CANNOTREAD 0xe102
#endif

#ifndef ER_CORRUPTWAVEFILE
#define ER_CORRUPTWAVEFILE 0xe103
#endif

#ifndef ER_CANNOTWRITE
#define ER_CANNOTWRITE 0xe104
#endif

#if 0
int WaveCreateFile(char *, HMMIO *, WAVEFORMATEX *, MMCKINFO *, MMCKINFO *);
int WaveStartDataWrite(HMMIO *, MMCKINFO *, MMIOINFO *);
int WaveWriteFile(HMMIO, UINT, BYTE *, MMCKINFO *, UINT *, MMIOINFO *);
int WaveCloseWriteFile(HMMIO *, MMCKINFO *, MMCKINFO *, MMIOINFO *, DWORD);

int WaveSaveFile(char *, UINT, DWORD, WAVEFORMATEX *, BYTE *);

int WaveCopyUselessChunks(HMMIO *, MMCKINFO *, MMCKINFO *, HMMIO *, MMCKINFO *, MMCKINFO *);
BOOL riffCopyChunk(HMMIO, HMMIO, const LPMMCKINFO);
#endif

#ifdef __cplusplus
};
#endif

#endif
