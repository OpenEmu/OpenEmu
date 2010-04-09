/***************************************************************************
Copyright (C) 2007 by Nach
http://nsrt.edgeemu.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License version 2 for more details.

You should have received a copy of the GNU General Public License
version 2 along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
***************************************************************************/

#include "file.h"

#include <cstring>

namespace zlib {
#include "unzip/unzip.h"
}

using namespace std;
using namespace zlib;

static const unsigned int MAX_FILE_NAME = 512;

File::File(const char *filename) : stream(filename, ios::in | ios::binary), is_zip(false), fsize(0), count(0)
{
  if (stream)
  {
    char temp[4];
    stream.read(temp, sizeof(temp));

    //check for standard zip 'magic number'
    if ((temp[0] == 'P') && (temp[1] == 'K') && (temp[2] == 3) && (temp[3] == 4))
    {
      stream.close();
      is_zip = true;
      zip(filename);
    }
    else
    {
      stream.seekg(0, ios::end);
      fsize = stream.tellg();
      stream.seekg(0, ios::beg);
    }
  }
}

void File::zip(const char *filename)
{
  zipfile = unzOpen(filename);
  if (zipfile)
  {
    zip_sub_open = false;

    unz_file_info cFileInfo;
    char ourFile[MAX_FILE_NAME] = { '\n' };

    for (int cFile = unzGoToFirstFile((unzFile)zipfile); cFile == UNZ_OK; cFile = unzGoToNextFile((unzFile)zipfile))
    {
      //Temporary char array for file name
      char cFileName[MAX_FILE_NAME];

      //Gets info on current file, and places it in cFileInfo
      unzGetCurrentFileInfo((unzFile)zipfile, &cFileInfo, cFileName, MAX_FILE_NAME, 0, 0, 0, 0);

      //Check for largest file which should be the ROM
      if ((size_t)cFileInfo.uncompressed_size > fsize)
      {
        strcpy(ourFile, cFileName);
        fsize = (size_t)cFileInfo.uncompressed_size;
      }
    }

    if (ourFile[0] != '\n')
    {
      //Sets current file to the file we liked before
      unzLocateFile((unzFile)zipfile, ourFile, 1);

      if (unzOpenCurrentFile((unzFile)zipfile) == UNZ_OK)
      {
        zip_sub_open = true;
      }
    }

    if (!zip_sub_open)
    {
      unzClose((unzFile)zipfile);
      zipfile = 0;
    }
  }
}

File::~File()
{
  close();
}

void File::rewind()
{
  if (is_open())
  {
    if (!is_zip)
    {
      stream.seekg(0, ios::beg);
    }
    else
    {
      unzCloseCurrentFile((unzFile)zipfile);
      unzOpenCurrentFile((unzFile)zipfile);
    }
  }
}

bool File::is_open()
{
  if (!is_zip)
  {
    return(stream.is_open());
  }
  return(zipfile && zip_sub_open);
}

void File::close()
{
  if (is_open())
  {
    if (!is_zip)
    {
      stream.close();
    }
    else
    {
      unzOpenCurrentFile((unzFile)zipfile);
      unzClose((unzFile)zipfile);
      zipfile = 0;
      zip_sub_open = false;
    }
  }
}

void File::read(char *buffer, size_t amount)
{
  if (is_open())
  {
    if (!is_zip)
    {
      stream.read(buffer, amount);
      count = stream.gcount();
    }
    else
    {
      count = (size_t)unzReadCurrentFile((unzFile)zipfile, buffer, amount);
    }
  }
  else
  {
    count = 0;
  }
}
