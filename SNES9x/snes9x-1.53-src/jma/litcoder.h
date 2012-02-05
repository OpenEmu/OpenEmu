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

#ifndef __LITERALCODER_H
#define __LITERALCODER_H

#include "aribitcd.h"
#include "rcdefs.h"

namespace NLiteral {

const int kNumMoveBits = 5;

class CDecoder2
{
  CMyBitDecoder<kNumMoveBits> m_Decoders[3][1 << 8];
public:
  void Init()
  {
    for (int i = 0; i < 3; i++)
      for (int j = 1; j < (1 << 8); j++)
        m_Decoders[i][j].Init();
  }

  BYTE DecodeNormal(CMyRangeDecoder *aRangeDecoder)
  {
    UINT32 aSymbol = 1;
    RC_INIT_VAR
    do
    {
      // aSymbol = (aSymbol << 1) | m_Decoders[0][aSymbol].Decode(aRangeDecoder);
      RC_GETBIT(kNumMoveBits, m_Decoders[0][aSymbol].m_Probability, aSymbol)
    }
    while (aSymbol < 0x100);
    RC_FLUSH_VAR
    return aSymbol;
  }

  BYTE DecodeWithMatchByte(CMyRangeDecoder *aRangeDecoder, BYTE aMatchByte)
  {
    UINT32 aSymbol = 1;
    RC_INIT_VAR
    do
    {
      UINT32 aMatchBit = (aMatchByte >> 7) & 1;
      aMatchByte <<= 1;
      // UINT32 aBit = m_Decoders[1 + aMatchBit][aSymbol].Decode(aRangeDecoder);
      // aSymbol = (aSymbol << 1) | aBit;
      UINT32 aBit;
      RC_GETBIT2(kNumMoveBits, m_Decoders[1 + aMatchBit][aSymbol].m_Probability, aSymbol,
          aBit = 0, aBit = 1)
      if (aMatchBit != aBit)
      {
        while (aSymbol < 0x100)
        {
          // aSymbol = (aSymbol << 1) | m_Decoders[0][aSymbol].Decode(aRangeDecoder);
          RC_GETBIT(kNumMoveBits, m_Decoders[0][aSymbol].m_Probability, aSymbol)
        }
        break;
      }
    }
    while (aSymbol < 0x100);
    RC_FLUSH_VAR
    return aSymbol;
  }
};

class CDecoder
{
  CDecoder2 *m_Coders;
  UINT32 m_NumPrevBits;
  UINT32 m_NumPosBits;
  UINT32 m_PosMask;
public:
  CDecoder(): m_Coders(0) {}
  ~CDecoder()  { Free(); }
  void Free()
  {
    delete []m_Coders;
    m_Coders = 0;
  }
  void Create(UINT32 aNumPosBits, UINT32 aNumPrevBits)
  {
    Free();
    m_NumPosBits = aNumPosBits;
    m_PosMask = (1 << aNumPosBits) - 1;
    m_NumPrevBits = aNumPrevBits;
    UINT32 aNumStates = 1 << (m_NumPrevBits + m_NumPosBits);
    m_Coders = new CDecoder2[aNumStates];
  }
  void Init()
  {
    UINT32 aNumStates = 1 << (m_NumPrevBits + m_NumPosBits);
    for (UINT32 i = 0; i < aNumStates; i++)
      m_Coders[i].Init();
  }
  UINT32 GetState(UINT32 aPos, BYTE aPrevByte) const
    { return ((aPos & m_PosMask) << m_NumPrevBits) + (aPrevByte >> (8 - m_NumPrevBits)); }
  BYTE DecodeNormal(CMyRangeDecoder *aRangeDecoder, UINT32 aPos, BYTE aPrevByte)
    { return m_Coders[GetState(aPos, aPrevByte)].DecodeNormal(aRangeDecoder); }
  BYTE DecodeWithMatchByte(CMyRangeDecoder *aRangeDecoder, UINT32 aPos, BYTE aPrevByte, BYTE aMatchByte)
    { return m_Coders[GetState(aPos, aPrevByte)].DecodeWithMatchByte(aRangeDecoder, aMatchByte); }
};

}

#endif
