// TODO/WIP

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

#include "mednafen.h"
#include "Stream.h"
#include "FileStream.h"

#include <trio/trio.h>


FileStream::FileStream(const char *path, const int mode) : fw(path, mode), OpenedMode(mode)
{

}

FileStream::~FileStream()
{

}

uint64 FileStream::attributes(void)
{
 uint64 ret = ATTRIBUTE_SEEKABLE;

 switch(OpenedMode)
 {
  case MODE_READ:
	ret |= ATTRIBUTE_READABLE;
	break;

  case MODE_WRITE_SAFE:
  case MODE_WRITE:
	ret |= ATTRIBUTE_WRITEABLE;
	break;
 }

 return ret;
}

uint8 *FileStream::map(void)
{
 //return fw.map();
 return(NULL);
}

void FileStream::unmap(void)
{
 //fw.unmap();
}


uint64 FileStream::read(void *data, uint64 count, bool error_on_eos)
{
 return fw.read(data, count, error_on_eos);
}

void FileStream::write(const void *data, uint64 count)
{
 fw.write(data, count);
}

void FileStream::seek(int64 offset, int whence)
{
 fw.seek(offset, whence);
}

int64 FileStream::tell(void)
{
 return fw.tell();
}

int64 FileStream::size(void)
{
 return fw.size();
}

void FileStream::close(void)
{
 fw.close();
}
