/*
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

#ifndef __STREAM_INBYTE_H
#define __STREAM_INBYTE_H

#include "iiostrm.h"

namespace NStream {

class CInByte
{
  UINT64 m_ProcessedSize;
  BYTE *m_BufferBase;
  UINT32 m_BufferSize;
  BYTE *m_Buffer;
  BYTE *m_BufferLimit;
  ISequentialInStream* m_Stream;
  bool m_StreamWasExhausted;

  bool ReadBlock();

public:
  CInByte(UINT32 aBufferSize = 0x100000);
  ~CInByte();

  void Init(ISequentialInStream *aStream);

  bool ReadByte(BYTE &aByte)
    {
      if(m_Buffer >= m_BufferLimit)
        if(!ReadBlock())
          return false;
      aByte = *m_Buffer++;
      return true;
    }
  BYTE ReadByte()
    {
      if(m_Buffer >= m_BufferLimit)
        if(!ReadBlock())
          return 0x0;
      return *m_Buffer++;
    }
  void ReadBytes(void *aData, UINT32 aSize, UINT32 &aProcessedSize)
    {
      for(aProcessedSize = 0; aProcessedSize < aSize; aProcessedSize++)
        if (!ReadByte(((BYTE *)aData)[aProcessedSize]))
          return;
    }
  bool ReadBytes(void *aData, UINT32 aSize)
    {
      UINT32 aProcessedSize;
      ReadBytes(aData, aSize, aProcessedSize);
      return (aProcessedSize == aSize);
    }
  UINT64 GetProcessedSize() const { return m_ProcessedSize + (m_Buffer - m_BufferBase); }
};

}

#endif
