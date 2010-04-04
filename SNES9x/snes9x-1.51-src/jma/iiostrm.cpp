/*
Copyright (C) 2005-2006 NSRT Team ( http://nsrt.edgeemu.com )
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )
Copyright (C) 2001-4 Igor Pavlov ( http://www.7-zip.org )

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License version 2.1 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "portable.h"
#include "iiostrm.h"
#include "crc32.h"

HRESULT ISequentialInStream_Array::Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  if (aSize > size)
  {
    aSize = size;
  }

  *aProcessedSize = aSize;
  memcpy(aData, data, aSize);
  size -= aSize;
  data += aSize;
  return(S_OK);
}

HRESULT ISequentialOutStream_Array::Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  if (aSize > size)
  {
    overflow = true;
    aSize = size;
  }

  *aProcessedSize = aSize;
  memcpy(data, aData, aSize);
  size -= aSize;
  data += aSize;
  total += aSize;
  return(S_OK);
}

HRESULT ISequentialInStream_String::Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  if (aSize > data.size())
  {
    aSize = data.size();
  }

  *aProcessedSize = aSize;
  memcpy(aData, data.c_str(), aSize);
  data.erase(0, aSize);
  return(S_OK);
}

HRESULT ISequentialOutStream_String::Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  *aProcessedSize = aSize;
  data.append((const char *)aData, aSize);
  total += aSize;
  return(S_OK);
}

HRESULT ISequentialInStream_Istream::Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  data.read((char *)aData, aSize);
  *aProcessedSize = data.gcount();
  return(S_OK);
}

HRESULT ISequentialOutStream_Ostream::Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  *aProcessedSize = aSize;
  data.write((char *)aData, aSize);
  total += aSize;
  return(S_OK);
}



HRESULT ISequentialInStreamCRC32_Array::Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  ISequentialInStream_Array::Read(aData, aSize, aProcessedSize);
  crc32 = CRC32lib::CRC32((const unsigned char *)aData, *aProcessedSize, ~crc32);
  return(S_OK);
}

HRESULT ISequentialOutStreamCRC32_Array::Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  ISequentialOutStream_Array::Write(aData, aSize, aProcessedSize);
  crc32 = CRC32lib::CRC32((const unsigned char *)aData, *aProcessedSize, ~crc32);
  return(S_OK);
}

HRESULT ISequentialInStreamCRC32_String::Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  ISequentialInStream_String::Read(aData, aSize, aProcessedSize);
  crc32 = CRC32lib::CRC32((const unsigned char *)aData, *aProcessedSize, ~crc32);
  return(S_OK);
}

HRESULT ISequentialOutStreamCRC32_String::Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  ISequentialOutStream_String::Write(aData, aSize, aProcessedSize);
  crc32 = CRC32lib::CRC32((const unsigned char *)aData, *aProcessedSize, ~crc32);
  return(S_OK);
}

HRESULT ISequentialInStreamCRC32_Istream::Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  ISequentialInStream_Istream::Read(aData, aSize, aProcessedSize);
  crc32 = CRC32lib::CRC32((const unsigned char *)aData, *aProcessedSize, ~crc32);
  return(S_OK);
}

HRESULT ISequentialOutStreamCRC32_Ostream::Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize)
{
  ISequentialOutStream_Ostream::Write(aData, aSize, aProcessedSize);
  crc32 = CRC32lib::CRC32((const unsigned char *)aData, *aProcessedSize, ~crc32);
  return(S_OK);
}
