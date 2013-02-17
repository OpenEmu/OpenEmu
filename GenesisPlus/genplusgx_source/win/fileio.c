/*
 *  fileio.c
 *
 *  Load a normal file, or ZIP/GZ archive into ROM buffer.
 *  Returns loaded ROM size (zero if an error occured)
 *  
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald
 *  modified by Eke-Eke (Genesis Plus GX)
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
#include <zlib.h>

static int check_zip(char *filename);

int load_archive(char *filename, unsigned char *buffer, int maxsize, char *extension)
{
  int size = 0;
  
  if(check_zip(filename))
  {
    unz_file_info info;
    int ret = 0;
    char fname[256];

    /* Attempt to open the archive */
    unzFile *fd = unzOpen(filename);
    if (!fd) return 0;

    /* Go to first file in archive */
    ret = unzGoToFirstFile(fd);
    if(ret != UNZ_OK)
    {
      unzClose(fd);
      return 0;
    }

    /* Get file informations and update filename */
    ret = unzGetCurrentFileInfo(fd, &info, fname, 256, NULL, 0, NULL, 0);
    if(ret != UNZ_OK)
    {
      unzClose(fd);
      return 0;
    }

    /* Compressed filename extension */
    if (extension)
    {
      strncpy(extension, &fname[strlen(fname) - 3], 3);
      extension[3] = 0;
    }

    /* Open the file for reading */
    ret = unzOpenCurrentFile(fd);
    if(ret != UNZ_OK)
    {
      unzClose(fd);
      return 0;
    }

    /* Retrieve uncompressed file size */
    size = info.uncompressed_size;
    if(size > maxsize)
    {
      size = maxsize;
    }

    /* Read (decompress) the file */
    ret = unzReadCurrentFile(fd, buffer, size);
    if(ret != size)
    {
      unzCloseCurrentFile(fd);
      unzClose(fd);
      return 0;
    }

    /* Close the current file */
    ret = unzCloseCurrentFile(fd);
    if(ret != UNZ_OK)
    {
      unzClose(fd);
      return 0;
    }

    /* Close the archive */
    ret = unzClose(fd);
    if(ret != UNZ_OK) return 0;
  }
  else
  {
    /* Open file */
    gzFile *gd = gzopen(filename, "rb");
    if (!gd) return 0;

    /* Read file data */
    size = gzread(gd, buffer, maxsize);

    /* filename extension */
    if (extension)
    {
      strncpy(extension, &filename[strlen(filename) - 3], 3);
      extension[3] = 0;
    }

    /* Close file */
    gzclose(gd);
  }

  /* Return loaded ROM size */
  return size;
}

/*
    Verifies if a file is a ZIP archive or not.
    Returns: 1= ZIP archive, 0= not a ZIP archive
*/
static int check_zip(char *filename)
{
  uint8 buf[2];
  FILE *fd = fopen(filename, "rb");
  if(!fd) return (0);
  fread(buf, 2, 1, fd);
  fclose(fd);
  if(memcmp(buf, "PK", 2) == 0) return (1);
  return (0);
}
