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

#include <vector>
#include <stdint.h>
#include <cstddef>

namespace gambatte {

class File {
  // Memory backed file.
  struct
  {
	  std::vector<uint8_t> data;
	  size_t size;
	  size_t ptr;
	  size_t count;
  } mem;

  public:
  File(const void *buffer, std::size_t size); // Memory backed file.
  ~File();
  void rewind();
  bool is_open();
  void close();
  std::size_t size() const { return mem.size; };
  void read(char *buffer, std::size_t amount);
  std::size_t gcount() const { return mem.count; }
  bool fail() const { return false; }
};

}
