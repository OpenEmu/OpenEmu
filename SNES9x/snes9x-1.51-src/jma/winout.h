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

#ifndef __STREAM_WINDOWOUT_H
#define __STREAM_WINDOWOUT_H

#include "iiostrm.h"

namespace NStream {
namespace NWindow {

// m_KeepSizeBefore: how mach BYTEs must be in buffer before m_Pos;
// m_KeepSizeAfter: how mach BYTEs must be in buffer after m_Pos;
// m_KeepSizeReserv: how mach BYTEs must be in buffer for Moving Reserv;
//                    must be >= aKeepSizeAfter; // test it

class COut
{
  BYTE  *m_Buffer;
  UINT32 m_Pos;
  UINT32 m_PosLimit;
  UINT32 m_KeepSizeBefore;
  UINT32 m_KeepSizeAfter;
  UINT32 m_KeepSizeReserv;
  UINT32 m_StreamPos;

  UINT32 m_WindowSize;
  UINT32 m_MoveFrom;

  ISequentialOutStream *m_Stream;

  virtual void MoveBlockBackward();
public:
  COut(): m_Buffer(0), m_Stream(0) {}
  virtual ~COut();
  void Create(UINT32 aKeepSizeBefore,
      UINT32 aKeepSizeAfter, UINT32 aKeepSizeReserv = (1<<17));
  void SetWindowSize(UINT32 aWindowSize);

  void Init(ISequentialOutStream *aStream, bool aSolid = false);
  HRESULT Flush();

  UINT32 GetCurPos() const { return m_Pos; }
  const BYTE *GetPointerToCurrentPos() const { return m_Buffer + m_Pos;};

  void CopyBackBlock(UINT32 aDistance, UINT32 aLen)
  {
    if (m_Pos >= m_PosLimit)
      MoveBlockBackward();
    BYTE *p = m_Buffer + m_Pos;
    aDistance++;
    for(UINT32 i = 0; i < aLen; i++)
      p[i] = p[i - aDistance];
    m_Pos += aLen;
  }

  void PutOneByte(BYTE aByte)
  {
    if (m_Pos >= m_PosLimit)
      MoveBlockBackward();
    m_Buffer[m_Pos++] = aByte;
  }

  BYTE GetOneByte(UINT32 anIndex) const
  {
    return m_Buffer[m_Pos + anIndex];
  }

  BYTE *GetBuffer() const { return m_Buffer; }
};

}}

#endif
