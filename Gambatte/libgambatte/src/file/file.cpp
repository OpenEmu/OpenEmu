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

using namespace std;

static const unsigned int MAX_FILE_NAME = 512;

File::File(const char *filename) : stream(filename, ios::in | ios::binary), is_zip(false), fsize(0), count(0)
{
  if (stream)
  {
    stream.seekg(0, ios::end);
    fsize = stream.tellg();
    stream.seekg(0, ios::beg);
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
    stream.seekg(0, ios::beg);
  }
}

bool File::is_open()
{
  return(stream.is_open());
}

void File::close()
{
  if (is_open())
  {
    stream.close();
  }
}

void File::read(char *buffer, size_t amount)
{
  if (is_open())
  {
    stream.read(buffer, amount);
    count = stream.gcount();
  }
  else
  {
    count = 0;
  }
}
