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

#include "winout.h"

namespace NStream {
namespace NWindow {

void COut::Create(UINT32 aKeepSizeBefore, UINT32 aKeepSizeAfter, UINT32 aKeepSizeReserv)
{
  m_Pos = 0;
  m_PosLimit = aKeepSizeReserv + aKeepSizeBefore;
  m_KeepSizeBefore = aKeepSizeBefore;
  m_KeepSizeAfter = aKeepSizeAfter;
  m_KeepSizeReserv = aKeepSizeReserv;
  m_StreamPos = 0;
  m_MoveFrom = m_KeepSizeReserv;
  m_WindowSize = aKeepSizeBefore;
  UINT32 aBlockSize = m_KeepSizeBefore + m_KeepSizeAfter + m_KeepSizeReserv;
  delete []m_Buffer;
  m_Buffer = new BYTE[aBlockSize];
}

COut::~COut()
{
  delete []m_Buffer;
}

void COut::SetWindowSize(UINT32 aWindowSize)
{
  m_WindowSize = aWindowSize;
  m_MoveFrom = m_KeepSizeReserv + m_KeepSizeBefore - aWindowSize;
}

void COut::Init(ISequentialOutStream *aStream, bool aSolid)
{
  m_Stream = aStream;

  if(aSolid)
    m_StreamPos = m_Pos;
  else
  {
    m_Pos = 0;
    m_PosLimit = m_KeepSizeReserv + m_KeepSizeBefore;
    m_StreamPos = 0;
  }
}

HRESULT COut::Flush()
{
  UINT32 aSize = m_Pos - m_StreamPos;
  if(aSize == 0)
    return S_OK;
  UINT32 aProcessedSize;
  HRESULT aResult = m_Stream->Write(m_Buffer + m_StreamPos, aSize, &aProcessedSize);
  if (aResult != S_OK)
    return aResult;
  if (aSize != aProcessedSize)
    return E_FAIL;
  m_StreamPos = m_Pos;
  return S_OK;
}

void COut::MoveBlockBackward()
{
  HRESULT aResult = Flush();
  if (aResult != S_OK)
    throw aResult;
  memmove(m_Buffer, m_Buffer + m_MoveFrom, m_WindowSize + m_KeepSizeAfter);
  m_Pos -= m_MoveFrom;
  m_StreamPos -= m_MoveFrom;
}

}}
