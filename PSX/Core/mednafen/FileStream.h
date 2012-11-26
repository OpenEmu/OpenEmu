/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// TODO/WIP

#ifndef __MDFN_FILESTREAM_H
#define __MDFN_FILESTREAM_H

#include "Stream.h"
#include "FileWrapper.h"

class FileStream : public Stream
{
 public:

 enum
 {
  MODE_READ = FileWrapper::MODE_READ,
  MODE_WRITE = FileWrapper::MODE_WRITE,
  MODE_WRITE_SAFE = FileWrapper::MODE_WRITE_SAFE
 };

 FileStream(const char *path, const int mode);
 virtual ~FileStream();

 virtual uint64 attributes(void);

 virtual uint8 *map(void);
 virtual void unmap(void);

 virtual uint64 read(void *data, uint64 count, bool error_on_eos = true);
 virtual void write(const void *data, uint64 count);
 virtual void seek(int64 offset, int whence);
 virtual int64 tell(void);
 virtual int64 size(void);
 virtual void close(void);

 private:
 FileWrapper fw;
 const int OpenedMode;
};



#endif
