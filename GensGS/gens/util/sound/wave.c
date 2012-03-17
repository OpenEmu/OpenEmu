/*========================================================================== 
* 
* Copyright (C) 1995-1996 Microsoft Corporation. All Rights Reserved. 
* 
* File: wave.c 
* Content: Wave library routines. 
* This file is used for loading/saving waves, and reading and 
* writing waves in smaller blocks. 
* Uses WaveOpenFile, WaveReadFile and WaveCloseReadFile for 
* single block access to reading wave files. 
* Uses WaveCreateFile, WaveWriteFile, WaveCloseWriteFile for 
* single block access for writing wave files. 
* Uses WaveLoadFile to load a whole wave file into memory. 
* Uses WaveSaveFile  to save a whole wave file into memory. 
* 
***************************************************************************/

/* PROTOTYPES */

#include <stdio.h>
#include "emulator/g_main.hpp"
#include "wave.h"


/* This routine will create a wave file for writing. This will automatically overwrite any 
existing file with the same name, so be careful and check before hand!!! 
pszFileName - Pointer to filename to write. 
phmmioOut - Pointer to HMMIO handle that is used for further writes 
pwfxDest - Valid waveformatex destination structure. 
pckOut - Pointer to be set with the MMCKINFO. 
pckOutRIFF - Pointer to be set with the RIFF info. 
*/


#if 0
int
WaveCreateFile (TCHAR * pszFileName,	// (IN) 
		HMMIO * phmmioOut,	// (OUT) 
		WAVEFORMATEX * pwfxDest,	// (IN) 
		MMCKINFO * pckOut,	// (OUT) 
		MMCKINFO * pckOutRIFF	// (OUT) 
  )
{
  int nError;			// Return value. 
  DWORD dwFactChunk;		// Contains the actual fact chunk. Garbage until WaveCloseWriteFile. 
  MMCKINFO ckOut1;
  char Name[128] = "";
  char ext[12] = "_000.wav";
  int num = -1, i, j;

  do
    {
      if (num++ > 99999)
	return (20);

      ext[0] = '_';
      i = 1;

      j = num / 10000;
      if (j)
	ext[i++] = '0' + j;
      j = (num / 1000) % 10;
      if (j)
	ext[i++] = '0' + j;
      j = (num / 100) % 10;
      ext[i++] = '0' + j;
      j = (num / 10) % 10;
      ext[i++] = '0' + j;
      j = num % 10;
      ext[i++] = '0' + j;
      ext[i++] = '.';
      ext[i++] = 'w';
      ext[i++] = 'a';
      ext[i++] = 'v';
      ext[i] = 0;

      if ((strlen (pszFileName) + strlen (ext)) > 127)
	return (21);

      strcpy (Name, pszFileName);
      strcat (Name, ext);
    }
  while (mmioOpen (Name, NULL, MMIO_EXIST) == (HMMIO) TRUE);

  dwFactChunk = (DWORD) - 1;
  nError = 0;

  *phmmioOut =
    mmioOpen (Name, NULL, MMIO_ALLOCBUF | MMIO_READWRITE | MMIO_CREATE);

  if (*phmmioOut == NULL)
    {
      nError = ER_CANNOTWRITE;
      goto ERROR_CANNOT_WRITE;	// cannot save WAVE file 
    }

  /* Create the output file RIFF chunk of form type 'WAVE'. */

  pckOutRIFF->fccType = mmioFOURCC ('W', 'A', 'V', 'E');
  pckOutRIFF->cksize = 0;

  if ((nError =
       mmioCreateChunk (*phmmioOut, pckOutRIFF, MMIO_CREATERIFF)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  /* We are now descended into the 'RIFF' chunk we just created. 
   * Now create the 'fmt ' chunk. Since we know the size of this chunk, 
   * specify it in the MMCKINFO structure so MMIO doesn't have to seek 
   * back and set the chunk size after ascending from the chunk. 
   */

  pckOut->ckid = mmioFOURCC ('f', 'm', 't', ' ');
  pckOut->cksize = sizeof (PCMWAVEFORMAT);	// we know the size of this ck. 

  if ((nError = mmioCreateChunk (*phmmioOut, pckOut, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  /* Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. */

  if (pwfxDest->wFormatTag == WAVE_FORMAT_PCM)
    {
      if (mmioWrite (*phmmioOut, (HPSTR) pwfxDest, sizeof (PCMWAVEFORMAT)) !=
	  sizeof (PCMWAVEFORMAT))
	{
	  nError = ER_CANNOTWRITE;
	  goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
	}
    }
  else
    {
      // Write the variable length size. 

      if ((UINT)
	  mmioWrite (*phmmioOut, (HPSTR) pwfxDest,
		     sizeof (*pwfxDest) + pwfxDest->cbSize) !=
	  (sizeof (*pwfxDest) + pwfxDest->cbSize))
	{
	  nError = ER_CANNOTWRITE;
	  goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
	}
    }

  /* Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk. */

  if ((nError = mmioAscend (*phmmioOut, pckOut, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  // Now create the fact chunk, not required for PCM but nice to have. This is filled 
  // in when the close routine is called. 

  ckOut1.ckid = mmioFOURCC ('f', 'a', 'c', 't');
  ckOut1.cksize = 0;

  if ((nError = mmioCreateChunk (*phmmioOut, &ckOut1, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  if (mmioWrite (*phmmioOut, (HPSTR) & dwFactChunk, sizeof (dwFactChunk)) !=
      sizeof (dwFactChunk))
    {
      nError = ER_CANNOTWRITE;
      goto ERROR_CANNOT_WRITE;
    }

  // Now ascend out of the fact chunk... 

  if ((nError = mmioAscend (*phmmioOut, &ckOut1, 0)) != 0)
    {
      nError = ER_CANNOTWRITE;	// cannot write file, probably 
      goto ERROR_CANNOT_WRITE;
    }

  goto DONE_CREATE;

ERROR_CANNOT_WRITE:
  // Maybe delete the half-written file? Ah forget it for now, its good to leave the 
  // file there for debugging... 

DONE_CREATE:
  return (nError);
}

/* This routine has to be called before any data is written to the wave
output file, via wavewritefile. This sets up the data to write, and creates the data chunk. 
*/

int
WaveStartDataWrite (HMMIO * phmmioOut,	// (IN) 
		    MMCKINFO * pckOut,	// (IN) 
		    MMIOINFO * pmmioinfoOut	// (OUT) 
  )
{
  int nError;

  nError = 0;

  /* Create the 'data' chunk that holds the waveform samples. */

  pckOut->ckid = mmioFOURCC ('d', 'a', 't', 'a');
  pckOut->cksize = 0;

  if ((nError = mmioCreateChunk (*phmmioOut, pckOut, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  if ((nError = mmioGetInfo (*phmmioOut, pmmioinfoOut, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;
    }

  goto CLEANUP;

ERROR_CANNOT_WRITE:

CLEANUP:
  return (nError);
}

/* This routine will write out data to a wave file. 
hmmioOut - Handle to hmmioOut filled by WaveCreateFile 
cbWrite - Number bytes to write out. 
pbSrc - Pointer to source. 
pckOut - pointer to ckOut filled by WaveCreateFile 
cbActualWrite - Pointer of number of actual bytes written. 
pmmioinfoOut - Pointer to mmioinfoOut filled by WaveCreateFile. 

Returns 0 if successful, else the error code. 
*/

int
WaveWriteFile (HMMIO hmmioOut,	// (IN) 
	       UINT cbWrite,	// (IN) 
	       BYTE * pbSrc,	// (IN) 
	       MMCKINFO * pckOut,	// (IN) 
	       UINT * cbActualWrite,	// (OUT) 
	       MMIOINFO * pmmioinfoOut	// (IN) 
  )
{
  int nError;
  UINT cT;

  nError = 0;

  *cbActualWrite = 0;

//      mmioWrite(&hmmioOut, (HPSTR) pbSrc, cbWrite);

  for (cT = 0; cT < cbWrite; cT++)
    {
      if (pmmioinfoOut->pchNext == pmmioinfoOut->pchEndWrite)
	{
	  pmmioinfoOut->dwFlags |= MMIO_DIRTY;

	  if ((nError =
	       mmioAdvance (hmmioOut, pmmioinfoOut, MMIO_WRITE)) != 0)
	    {
	      goto ERROR_CANNOT_WRITE;
	    }
	}

      *((BYTE *) pmmioinfoOut->pchNext)++ = *((BYTE *) pbSrc + cT);
      (*cbActualWrite)++;
    }

ERROR_CANNOT_WRITE:
  // What to do here? Well, for now, nothing, just return that error. (maybe delete the 
  // file later? 

  return (nError);
}


/* This routine will close a wave file used for writing. Returns 0 if successful, else 
the error code. 
phmmioOut - Pointer to mmio handle for saving. 
pckOut - Pointer to the MMCKINFO for saving. 
pckOutRiff - Pointer to the riff MMCKINFO for saving. 
pmmioinfoOut- Pointer to mmioinfo for saving. 
cSamples - # of samples saved, for the fact chunk. For PCM, this isn't used but 
will be written anyway, so this can be zero as long as programs ignore 
this field when they load PCM formats. 
*/

int
WaveCloseWriteFile (HMMIO * phmmioOut,	// (IN) 
		    MMCKINFO * pckOut,	// (IN) 
		    MMCKINFO * pckOutRIFF,	// (IN) 
		    MMIOINFO * pmmioinfoOut,	// (IN) 
		    DWORD cSamples	// (IN) 
  )
{
  int nError;

  nError = 0;

  if (*phmmioOut == NULL)
    return (0);

  pmmioinfoOut->dwFlags |= MMIO_DIRTY;

  if ((nError = mmioSetInfo (*phmmioOut, pmmioinfoOut, 0)) != 0)
    {
      // cannot flush, probably... 
      goto ERROR_CANNOT_WRITE;
    }

  /* Ascend the output file out of the 'data' chunk -- this will cause 
   * the chunk size of the 'data' chunk to be written. 
   */

  if ((nError = mmioAscend (*phmmioOut, pckOut, 0)) != 0)
    goto ERROR_CANNOT_WRITE;	// cannot write file, probably 

  // Do this here instead...

  if ((nError = mmioAscend (*phmmioOut, pckOutRIFF, 0)) != 0)
    goto ERROR_CANNOT_WRITE;	// cannot write file, probably 

  nError = mmioSeek (*phmmioOut, 0, SEEK_SET);

  if ((nError = (int) mmioDescend (*phmmioOut, pckOutRIFF, NULL, 0)) != 0)
    goto ERROR_CANNOT_WRITE;

  nError = 0;

  pckOut->ckid = mmioFOURCC ('f', 'a', 'c', 't');

  if ((nError =
       mmioDescend (*phmmioOut, pckOut, pckOutRIFF, MMIO_FINDCHUNK)) == 0)
    {
      // If it didn't fail, write the fact chunk out, if it failed, not critical, just 
      // assert (below). 

      nError = mmioWrite (*phmmioOut, (HPSTR) & cSamples, sizeof (DWORD));
      nError = mmioAscend (*phmmioOut, pckOut, 0);
      nError = 0;
    }
  else
    {
      nError = 0;
//              ASSERT(FALSE); 
    }

  /* Ascend the output file out of the 'RIFF' chunk -- this will cause 
   * the chunk size of the 'RIFF' chunk to be written. 
   */

  if ((nError = mmioAscend (*phmmioOut, pckOutRIFF, 0)) != 0)
    goto ERROR_CANNOT_WRITE;	// cannot write file, probably 

ERROR_CANNOT_WRITE:
  if (*phmmioOut != NULL)
    {
      mmioClose (*phmmioOut, 0);
      *phmmioOut = NULL;
    }

  return (nError);
}

/* This routine will copy from a source wave file to a destination wave file all those useless chunks 
(well, the ones useless to conversions, etc --> apparently people use them!). The source will be 
seeked to the begining, but the destination has to be at a current pointer to put the new chunks. 
This will also seek back to the start of the wave riff header at the end of the routine. 

phmmioIn - Pointer to input mmio file handle. 
pckIn - Pointer to a nice ckIn to use. 
pckInRiff - Pointer to the main riff. 
phmmioOut - Pointer to output mmio file handle. 
pckOut - Pointer to nice ckOut to use. 
pckOutRiff - Pointer to the main riff. 

Returns 0 if successful, else the error code. If this routine fails, it still attemps to seek back to 
the start of the wave riff header, though this too could be unsuccessful. 
*/

int
WaveCopyUselessChunks (HMMIO * phmmioIn,
		       MMCKINFO * pckIn,
		       MMCKINFO * pckInRiff,
		       HMMIO * phmmioOut,
		       MMCKINFO * pckOut, MMCKINFO * pckOutRiff)
{
  int nError;

  nError = 0;

  // First seek to the stinking start of the file, not including the riff header... 

  if ((nError =
       mmioSeek (*phmmioIn, pckInRiff->dwDataOffset + sizeof (FOURCC),
		 SEEK_SET)) == -1)
    {
      nError = ER_CANNOTREAD;
      goto ERROR_IN_PROC;
    }

  nError = 0;

  while (mmioDescend (*phmmioIn, pckIn, pckInRiff, 0) == 0)
    {
      // quickly check for corrupt RIFF file--don't ascend past end! 

      if ((pckIn->dwDataOffset + pckIn->cksize) >
	  (pckInRiff->dwDataOffset + pckInRiff->cksize))
	goto ERROR_IN_PROC;

      switch (pckIn->ckid)
	{
	  // explicitly skip these... 

	case mmioFOURCC ('f', 'm', 't', ' '):
	  break;

	case mmioFOURCC ('d', 'a', 't', 'a'):
	  break;

	case mmioFOURCC ('f', 'a', 'c', 't'):
	  break;

	case mmioFOURCC ('J', 'U', 'N', 'K'):
	  break;

	case mmioFOURCC ('P', 'A', 'D', ' '):
	  break;

	case mmioFOURCC ('c', 'u', 'e', ' '):
	  break;

	  // copy chunks that are OK to copy 

	case mmioFOURCC ('p', 'l', 's', 't'):

	  // although without the 'cue' chunk, it doesn't make much sense 

	  riffCopyChunk (*phmmioIn, *phmmioOut, pckIn);
	  break;

	case mmioFOURCC ('D', 'I', 'S', 'P'):
	  riffCopyChunk (*phmmioIn, *phmmioOut, pckIn);
	  break;

	  // don't copy unknown chunks 
	default:
	  break;
	}

      // step up to prepare for next chunk.. 

      mmioAscend (*phmmioIn, pckIn, 0);
    }

ERROR_IN_PROC:
  {
    int nErrorT;

    // Seek back to riff header

    nErrorT =
      mmioSeek (*phmmioIn, pckInRiff->dwDataOffset + sizeof (FOURCC),
		SEEK_SET);
  }

  return (nError);
}

/** BOOL RIFFAPI riffCopyChunk(HMMIO hmmioSrc, HMMIO hmmioDst, const LPMMCKINFO lpck) 
* 
* DESCRIPTION: 
* 
* 
* ARGUMENTS: 
* (LPWAVECONVCB lpwc, LPMMCKINFO lpck) 
* 
* RETURN (BOOL NEAR PASCAL): 
* 
* 
* NOTES: 
* 
** */

BOOL
riffCopyChunk (HMMIO hmmioSrc, HMMIO hmmioDst, const LPMMCKINFO lpck)
{
  MMCKINFO ck;
  HPSTR hpBuf;

  hpBuf = (HPSTR) GlobalAllocPtr (GHND, lpck->cksize);

  if (!hpBuf)
    return (FALSE);

  ck.ckid = lpck->ckid;
  ck.cksize = lpck->cksize;

  if (mmioCreateChunk (hmmioDst, &ck, 0))
    goto rscc_Error;

  if (mmioRead (hmmioSrc, hpBuf, lpck->cksize) != (LONG) lpck->cksize)
    goto rscc_Error;

  if (mmioWrite (hmmioDst, hpBuf, lpck->cksize) != (LONG) lpck->cksize)
    goto rscc_Error;

  if (mmioAscend (hmmioDst, &ck, 0))
    goto rscc_Error;

  if (hpBuf)
    GlobalFreePtr (hpBuf);

  return (TRUE);

rscc_Error:

  if (hpBuf)
    GlobalFreePtr (hpBuf);

  return (FALSE);
}				/* RIFFSupCopyChunk() */


/* This routine saves a wave file in currently in memory. 
pszFileName - FileName to save to. Automatically overwritten, be careful! 
cbSize - Size in bytes to write. 
cSamples - # of samples to write, used to make the fact chunk. (if !PCM) 
pwfxDest - Pointer to waveformatex structure. 
pbData - Pointer to the data. 
*/

int
WaveSaveFile (TCHAR * pszFileName,	// (IN) 
	      UINT cbSize,	// (IN) 
	      DWORD cSamples,	// (IN) 
	      WAVEFORMATEX * pwfxDest,	// (IN) 
	      BYTE * pbData	// (IN) 
  )
{
  HMMIO hmmioOut;
  MMCKINFO ckOut;
  MMCKINFO ckOutRIFF;
  MMIOINFO mmioinfoOut;
  UINT cbActualWrite;
  int nError;

  if ((nError =
       WaveCreateFile (pszFileName, &hmmioOut, pwfxDest, &ckOut,
		       &ckOutRIFF)) != 0)
    {
      goto ERROR_SAVING;
    }

  if ((nError = WaveStartDataWrite (&hmmioOut, &ckOut, &mmioinfoOut)) != 0)
    {
      goto ERROR_SAVING;
    }

  if ((nError =
       WaveWriteFile (hmmioOut, cbSize, pbData, &ckOut, &cbActualWrite,
		      &mmioinfoOut)) != 0)
    {
      goto ERROR_SAVING;
    }

  if ((nError =
       WaveCloseWriteFile (&hmmioOut, &ckOut, &ckOutRIFF, &mmioinfoOut,
			   cSamples)) != 0)
    {
      goto ERROR_SAVING;
    }

ERROR_SAVING:
  return (nError);
}


int
GYMCreateFile (TCHAR * pszFileName,	// (IN) 
	       HMMIO * phmmioOut,	// (OUT) 
	       WAVEFORMATEX * pwfxDest,	// (IN) 
	       MMCKINFO * pckOut,	// (OUT) 
	       MMCKINFO * pckOutRIFF	// (OUT) 
  )
{
  int nError;			// Return value. 
  DWORD dwFactChunk;		// Contains the actual fact chunk. Garbage until WaveCloseWriteFile. 
  MMCKINFO ckOut1;
  char Name[128] = "";
  char ext[12] = "_000.wav";
  int num = -1, i, j;

  do
    {
      if (num++ > 99999)
	return (20);

      ext[0] = '_';
      i = 1;

      j = num / 10000;
      if (j)
	ext[i++] = '0' + j;
      j = (num / 1000) % 10;
      if (j)
	ext[i++] = '0' + j;
      j = (num / 100) % 10;
      ext[i++] = '0' + j;
      j = (num / 10) % 10;
      ext[i++] = '0' + j;
      j = num % 10;
      ext[i++] = '0' + j;
      ext[i++] = '.';
      ext[i++] = 'w';
      ext[i++] = 'a';
      ext[i++] = 'v';
      ext[i] = 0;

      if ((strlen (pszFileName) + strlen (ext)) > 127)
	return (21);

      strcpy (Name, pszFileName);
      strcat (Name, ext);
    }
  while (mmioOpen (Name, NULL, MMIO_EXIST) == (HMMIO) TRUE);

  dwFactChunk = (DWORD) - 1;
  nError = 0;

  *phmmioOut =
    mmioOpen (Name, NULL, MMIO_ALLOCBUF | MMIO_READWRITE | MMIO_CREATE);

  if (*phmmioOut == NULL)
    {
      nError = ER_CANNOTWRITE;
      goto ERROR_CANNOT_WRITE;	// cannot save WAVE file 
    }

  /* Create the output file RIFF chunk of form type 'WAVE'. */

  pckOutRIFF->fccType = mmioFOURCC ('W', 'A', 'V', 'E');
  pckOutRIFF->cksize = 0;

  if ((nError =
       mmioCreateChunk (*phmmioOut, pckOutRIFF, MMIO_CREATERIFF)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  /* We are now descended into the 'RIFF' chunk we just created. 
   * Now create the 'fmt ' chunk. Since we know the size of this chunk, 
   * specify it in the MMCKINFO structure so MMIO doesn't have to seek 
   * back and set the chunk size after ascending from the chunk. 
   */

  pckOut->ckid = mmioFOURCC ('f', 'm', 't', ' ');
  pckOut->cksize = sizeof (PCMWAVEFORMAT);	// we know the size of this ck. 

  if ((nError = mmioCreateChunk (*phmmioOut, pckOut, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  /* Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. */

  if (pwfxDest->wFormatTag == WAVE_FORMAT_PCM)
    {
      if (mmioWrite (*phmmioOut, (HPSTR) pwfxDest, sizeof (PCMWAVEFORMAT)) !=
	  sizeof (PCMWAVEFORMAT))
	{
	  nError = ER_CANNOTWRITE;
	  goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
	}
    }
  else
    {
      // Write the variable length size. 

      if ((UINT)
	  mmioWrite (*phmmioOut, (HPSTR) pwfxDest,
		     sizeof (*pwfxDest) + pwfxDest->cbSize) !=
	  (sizeof (*pwfxDest) + pwfxDest->cbSize))
	{
	  nError = ER_CANNOTWRITE;
	  goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
	}
    }

  /* Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk. */

  if ((nError = mmioAscend (*phmmioOut, pckOut, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  // Now create the fact chunk, not required for PCM but nice to have. This is filled 
  // in when the close routine is called. 

  ckOut1.ckid = mmioFOURCC ('f', 'a', 'c', 't');
  ckOut1.cksize = 0;

  if ((nError = mmioCreateChunk (*phmmioOut, &ckOut1, 0)) != 0)
    {
      goto ERROR_CANNOT_WRITE;	// cannot write file, probably 
    }

  if (mmioWrite (*phmmioOut, (HPSTR) & dwFactChunk, sizeof (dwFactChunk)) !=
      sizeof (dwFactChunk))
    {
      nError = ER_CANNOTWRITE;
      goto ERROR_CANNOT_WRITE;
    }

  // Now ascend out of the fact chunk... 

  if ((nError = mmioAscend (*phmmioOut, &ckOut1, 0)) != 0)
    {
      nError = ER_CANNOTWRITE;	// cannot write file, probably 
      goto ERROR_CANNOT_WRITE;
    }

  goto DONE_CREATE;

ERROR_CANNOT_WRITE:
  // Maybe delete the half-written file? Ah forget it for now, its good to leave the 
  // file there for debugging... 

DONE_CREATE:
  return (nError);
}

#endif
