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

#include <fstream>

class File {
  private:
  std::ifstream stream;
  bool is_zip; //Change this to an enum later
  std::size_t fsize, count;
  void *zipfile;
  bool zip_sub_open;

  void zip(const char *filename);

  public:
  File(const char *filename);
  ~File();
  void rewind();
  bool is_open();
  void close();
  std::size_t size() const { return fsize; };
  void read(char *buffer, std::size_t amount);
  std::size_t gcount() const { return count; }
  bool fail() const { return stream.fail(); }
};
