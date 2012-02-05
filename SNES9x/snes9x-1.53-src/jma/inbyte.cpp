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

#include "inbyte.h"

namespace NStream{

CInByte::CInByte(UINT32 aBufferSize):
  m_BufferBase(0),
  m_BufferSize(aBufferSize)
{
  m_BufferBase = new BYTE[m_BufferSize];
}

CInByte::~CInByte()
{
  delete []m_BufferBase;
}

void CInByte::Init(ISequentialInStream *aStream)
{
  m_Stream = aStream;
  m_ProcessedSize = 0;
  m_Buffer = m_BufferBase;
  m_BufferLimit = m_Buffer;
  m_StreamWasExhausted = false;
}

bool CInByte::ReadBlock()
{
  if (m_StreamWasExhausted)
    return false;
  m_ProcessedSize += (m_Buffer - m_BufferBase);
  UINT32 aNumProcessedBytes;
  HRESULT aResult = m_Stream->Read(m_BufferBase, m_BufferSize, &aNumProcessedBytes);
  if (aResult != S_OK)
    throw aResult;
  m_Buffer = m_BufferBase;
  m_BufferLimit = m_Buffer + aNumProcessedBytes;
  m_StreamWasExhausted = (aNumProcessedBytes == 0);
  return (!m_StreamWasExhausted);
}

}
