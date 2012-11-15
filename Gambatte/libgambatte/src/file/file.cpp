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
#include <algorithm>

using namespace std;

namespace gambatte {

File::File(const void *buffer, std::size_t size)
{
  mem.ptr = 0;
  mem.size = size;
  mem.data.reserve(size);
  std::copy(reinterpret_cast<const uint8_t*>(buffer),
      reinterpret_cast<const uint8_t*>(buffer) + size,
      mem.data.begin());
}

File::~File()
{}

void File::rewind()
{
   mem.ptr = 0;
}

bool File::is_open()
{
  return true;
}

void File::close()
{}

void File::read(char *buffer, size_t amount)
{
   size_t max_read = std::min(amount, mem.size - mem.ptr);
   std::copy(reinterpret_cast<const char*>(&mem.data[mem.ptr]),
         reinterpret_cast<const char*>(&mem.data[mem.ptr + max_read]),
         buffer);
   mem.ptr += max_read;
   mem.count = max_read;
}

}
