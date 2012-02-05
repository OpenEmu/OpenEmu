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

#ifndef __COMPRESSION_RANGECODER_H
#define __COMPRESSION_RANGECODER_H

#include "inbyte.h"

namespace NCompression {
namespace NArithmetic {

const UINT32 kNumTopBits = 24;
const UINT32 kTopValue = (1 << kNumTopBits);

class CRangeDecoder
{
public:
  NStream::CInByte m_Stream;
  UINT32 m_Range;
  UINT32 m_Code;
  UINT32 m_Word;
  void Normalize()
  {
    while (m_Range < kTopValue)
    {
      m_Code = (m_Code << 8) | m_Stream.ReadByte();
      m_Range <<= 8;
    }
  }

  void Init(ISequentialInStream *aStream)
  {
    m_Stream.Init(aStream);
    m_Code = 0;
    m_Range = UINT32(-1);
    for(int i = 0; i < 5; i++)
      m_Code = (m_Code << 8) | m_Stream.ReadByte();
  }

  UINT32 GetThreshold(UINT32 aTotal)
  {
    return (m_Code) / ( m_Range /= aTotal);
  }

  void Decode(UINT32 aStart, UINT32 aSize, UINT32 aTotal)
  {
    m_Code -= aStart * m_Range;
    m_Range *= aSize;
    Normalize();
  }

  /*
  UINT32 DecodeDirectBitsDiv(UINT32 aNumTotalBits)
  {
    m_Range >>= aNumTotalBits;
    UINT32 aThreshold = m_Code / m_Range;
    m_Code -= aThreshold * m_Range;

    Normalize();
    return aThreshold;
  }

  UINT32 DecodeDirectBitsDiv2(UINT32 aNumTotalBits)
  {
    if (aNumTotalBits <= kNumBottomBits)
      return DecodeDirectBitsDiv(aNumTotalBits);
    UINT32 aResult = DecodeDirectBitsDiv(aNumTotalBits - kNumBottomBits) << kNumBottomBits;
    return (aResult | DecodeDirectBitsDiv(kNumBottomBits));
  }
  */

  UINT32 DecodeDirectBits(UINT32 aNumTotalBits)
  {
    UINT32 aRange = m_Range;
    UINT32 aCode = m_Code;
    UINT32 aResult = 0;
    for (UINT32 i = aNumTotalBits; i > 0; i--)
    {
      aRange >>= 1;
      /*
      aResult <<= 1;
      if (aCode >= aRange)
      {
        aCode -= aRange;
        aResult |= 1;
      }
      */
      UINT32 t = (aCode - aRange) >> 31;
      aCode -= aRange & (t - 1);
      // aRange = aRangeTmp + ((aRange & 1) & (1 - t));
      aResult = (aResult << 1) | (1 - t);

      if (aRange < kTopValue)
      {
        aCode = (aCode << 8) | m_Stream.ReadByte();
        aRange <<= 8;
      }
    }
    m_Range = aRange;
    m_Code = aCode;
    return aResult;
  }

  UINT32 DecodeBit(UINT32 aSize0, UINT32 aNumTotalBits)
  {
    UINT32 aNewBound = (m_Range >> aNumTotalBits) * aSize0;
    UINT32 aSymbol;
    if (m_Code < aNewBound)
    {
      aSymbol = 0;
      m_Range = aNewBound;
    }
    else
    {
      aSymbol = 1;
      m_Code -= aNewBound;
      m_Range -= aNewBound;
    }
    Normalize();
    return aSymbol;
  }

  UINT64 GetProcessedSize() {return m_Stream.GetProcessedSize(); }
};

}}

#endif
