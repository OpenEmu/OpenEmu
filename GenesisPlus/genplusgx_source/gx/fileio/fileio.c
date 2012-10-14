/*
 *  fileio.c
 *
 *  Load a normal file, or ZIP/GZ archive into ROM buffer.
 *  Returns loaded ROM size (zero if an error occured).
 *  
 *  Copyright Eke-Eke (2007-2012), based on original work from Softdev (2006)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "file_load.h"
#include "gui.h"

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

int load_archive(char *filename, unsigned char *buffer, int maxsize, char *extension)
{
  int size = 0;
  char in[CHUNKSIZE];
  char msg[64] = "Unable to open file";

  /* Open file */
  FILE *fd = fopen(filename, "rb");

  /* Master System & Game Gear BIOS are optional files */
  if (!strcmp(filename,MS_BIOS_US) || !strcmp(filename,MS_BIOS_EU) || !strcmp(filename,MS_BIOS_JP) || !strcmp(filename,GG_BIOS))
  {
    /* disable all messages */
    SILENT = 1;
  }
  
  /* Mega CD BIOS are required files */
  if (!strcmp(filename,CD_BIOS_US) || !strcmp(filename,CD_BIOS_EU) || !strcmp(filename,CD_BIOS_JP)) 
  {
    sprintf(msg,"Unable to open %s", filename + 14);
  }

  if (!fd)
  {
    GUI_WaitPrompt("Error", msg);
    SILENT = 0;
    return 0;
  }

  /* Read first chunk */
  fread(in, CHUNKSIZE, 1, fd);

  /* Detect Zip file */
  if (memcmp(in, "PK", 2) == 0)
  {
    /* Inflate buffer */
    char out[CHUNKSIZE];

    /* PKZip header pointer */
    PKZIPHEADER *pkzip = (PKZIPHEADER *) in;

    /* Retrieve uncompressed ROM size */
    size = FLIP32(pkzip->uncompressedSize);

    /* Check ROM size */
    if (size > maxsize)
    {
      fclose(fd);
      GUI_WaitPrompt("Error","File is too large");
      SILENT = 0;
      return 0;
    }

    sprintf (msg, "Unzipping %d bytes ...", size);
    GUI_MsgBoxUpdate("Information",msg);

    /* Initialize zip stream */
    z_stream zs;
    memset (&zs, 0, sizeof (z_stream));
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = 0;
    zs.next_in = Z_NULL;
    int res = inflateInit2(&zs, -MAX_WBITS);

    if (res != Z_OK)
    {
      fclose(fd);
      GUI_WaitPrompt("Error","Unable to unzip file");
      SILENT = 0;
      return 0;
    }

    /* Compressed filename offset */
    int offset = sizeof (PKZIPHEADER) + FLIP16(pkzip->filenameLength);
 
    if (extension)
    {
      memcpy(extension, &in[offset - 3], 3);
      extension[3] = 0;
    }

   
    /* Initial Zip buffer offset */
    offset += FLIP16(pkzip->extraDataLength);
    zs.next_in = (Bytef *)&in[offset];

    /* Initial Zip remaining chunk size */
    zs.avail_in = CHUNKSIZE - offset;

    /* Start unzipping file */
    do
    {
      /* Inflate data until output buffer is empty */
      do
      {
        zs.avail_out = CHUNKSIZE;
        zs.next_out = (Bytef *) out;
        res = inflate(&zs, Z_NO_FLUSH);

        if (res == Z_MEM_ERROR)
        {
          inflateEnd(&zs);
          fclose(fd);
          GUI_WaitPrompt("Error","Unable to unzip file");
          SILENT = 0;
          return 0;
        }

        offset = CHUNKSIZE - zs.avail_out;
        if (offset)
        {
          memcpy(buffer, out, offset);
          buffer += offset;
        }
      }
      while (zs.avail_out == 0);

      /* Read next chunk of zipped data */
      fread(in, CHUNKSIZE, 1, fd);
      zs.next_in = (Bytef *)&in[0];
      zs.avail_in = CHUNKSIZE;
    }
    while (res != Z_STREAM_END);
    inflateEnd (&zs);
  }
  else
  {
    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* size limit */
    if(size > maxsize)
    {
      fclose(fd);
      GUI_WaitPrompt("Error","File is too large");
      SILENT = 0;
      return 0;
    }

    sprintf((char *)msg,"Loading %d bytes ...", size);
    GUI_MsgBoxUpdate("Information", (char *)msg);

    /* filename extension */
    if (extension)
    {
      memcpy(extension, &filename[strlen(filename) - 3], 3);
      extension[3] = 0;
    }

    /* Read into buffer */
    int left = size;
    while (left > CHUNKSIZE)
    {
      fread(buffer, CHUNKSIZE, 1, fd);
      buffer += CHUNKSIZE;
      left -= CHUNKSIZE;
    }

    /* Read remaining bytes */
    fread(buffer, left, 1, fd);
  }

  /* Close file */
  fclose(fd);

  /* Return loaded ROM size */
  SILENT = 0;
  return size;
}
