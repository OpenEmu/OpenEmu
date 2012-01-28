/******************************************************************************
 *
 * unzip.c
 *
 *   Zip Support
 *
 *   Only partial support is included, in that only the first file within the archive
 *   is considered to be a ROM image.
 *
 *   Softdev (2006)
 *   Eke-Eke (2007,2008) 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************************/

#include "shared.h"
#include "gui.h"

/*
 * PKWare Zip Header - adopted into zip standard
 */
#define PKZIPID 0x504b0304
#define MAXROM 0x500000
#define ZIPCHUNK 2048

/*
 * Zip file header definition
 */
typedef struct
{
  unsigned int zipid __attribute__ ((__packed__));  // 0x04034b50
  unsigned short zipversion __attribute__ ((__packed__));
  unsigned short zipflags __attribute__ ((__packed__));
  unsigned short compressionMethod __attribute__ ((__packed__));
  unsigned short lastmodtime __attribute__ ((__packed__));
  unsigned short lastmoddate __attribute__ ((__packed__));
  unsigned int crc32 __attribute__ ((__packed__));
  unsigned int compressedSize __attribute__ ((__packed__));
  unsigned int uncompressedSize __attribute__ ((__packed__));
  unsigned short filenameLength __attribute__ ((__packed__));
  unsigned short extraDataLength __attribute__ ((__packed__));
} PKZIPHEADER;

/*
 * Zip files are stored little endian
 * Support functions for short and int types
 */
static inline u32 FLIP32 (u32 b)
{
  unsigned int c;
  c = (b & 0xff000000) >> 24;
  c |= (b & 0xff0000) >> 8;
  c |= (b & 0xff00) << 8;
  c |= (b & 0xff) << 24;
  return c;
}

static inline u16 FLIP16 (u16 b)
{
  u16 c;
  c = (b & 0xff00) >> 8;
  c |= (b & 0xff) << 8;
  return c;
}

/****************************************************************************
 * IsZipFile
 *
 * Returns TRUE when PKZIPID is first four characters of buffer
 ****************************************************************************/
int IsZipFile (char *buffer)
{
  unsigned int *check;
  check = (unsigned int *) buffer;
  if (check[0] == PKZIPID) return 1;
  return 0;
}

/*****************************************************************************
 * UnZipBuffer
 *
 ******************************************************************************/
int UnZipBuffer (unsigned char *outbuffer, FILE *fd, char *filename)
{
  PKZIPHEADER pkzip;
  int zipoffset = 0;
  int zipchunk = 0;
  char out[ZIPCHUNK];
  z_stream zs;
  int res;
  int bufferoffset = 0;
  int have = 0;
  char readbuffer[ZIPCHUNK];
  char msg[64];

  /*** Read Zip Header ***/
  fread(readbuffer, ZIPCHUNK,  1, fd);

  /*** Copy PKZip header to local, used as info ***/
  memcpy (&pkzip, &readbuffer, sizeof (PKZIPHEADER));

  if (FLIP32 (pkzip.uncompressedSize) > MAXROMSIZE)
  {
    GUI_WaitPrompt("Error","File is too large !");
    return 0;
  }

  sprintf (msg, "Unzipping %d bytes ...", FLIP32 (pkzip.uncompressedSize));
  GUI_MsgBoxOpen("Information",msg,1);

  /*** Prepare the zip stream ***/
  memset (&zs, 0, sizeof (z_stream));
  zs.zalloc = Z_NULL;
  zs.zfree = Z_NULL;
  zs.opaque = Z_NULL;
  zs.avail_in = 0;
  zs.next_in = Z_NULL;
  res = inflateInit2 (&zs, -MAX_WBITS);

  if (res != Z_OK)
  {
    GUI_WaitPrompt("Error","Unable to unzip file !");
    return 0;
  }

  /*** Get file name (first file) ***/
  int size = FLIP16 (pkzip.filenameLength);
  if (size > 255) size = 255;
  strncpy(filename, &readbuffer[sizeof(PKZIPHEADER)], size);
  filename[size] = 0;

  /*** Set ZipChunk for first pass ***/
  zipoffset = (sizeof (PKZIPHEADER) + size + FLIP16 (pkzip.extraDataLength));
  zipchunk = ZIPCHUNK - zipoffset;

  /*** Now do it! ***/
  do
  {
    zs.avail_in = zipchunk;
    zs.next_in = (Bytef *) &readbuffer[zipoffset];

    /*** Now inflate until input buffer is exhausted ***/
    do
    {
      zs.avail_out = ZIPCHUNK;
      zs.next_out = (Bytef *) &out;
      res = inflate (&zs, Z_NO_FLUSH);

      if (res == Z_MEM_ERROR)
      {
        inflateEnd (&zs);
        GUI_WaitPrompt("Error","Unable to unzip file !");
        return 0;
      }

      have = ZIPCHUNK - zs.avail_out;
      if (have)
      {
        /*** Copy to normal block buffer ***/
        memcpy (&outbuffer[bufferoffset], &out, have);
        bufferoffset += have;
      }
    }
    while (zs.avail_out == 0);

    /*** Readup the next 2k block ***/
    zipoffset = 0;
    zipchunk = ZIPCHUNK;
    fread(readbuffer, ZIPCHUNK, 1, fd);
  }
  while (res != Z_STREAM_END);

  inflateEnd (&zs);
  GUI_MsgBoxClose();

  if (res == Z_STREAM_END)
  {
    if (FLIP32 (pkzip.uncompressedSize) == (u32) bufferoffset)
      return bufferoffset;
    else
      return FLIP32 (pkzip.uncompressedSize);
  }

  return 0;
}



