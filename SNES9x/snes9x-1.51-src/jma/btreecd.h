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

#ifndef __BITTREECODER_H
#define __BITTREECODER_H

#include "aribitcd.h"
#include "rcdefs.h"


//////////////////////////
// CBitTreeDecoder

template <int aNumMoveBits, UINT32 m_NumBitLevels>
class CBitTreeDecoder
{
  CMyBitDecoder<aNumMoveBits> m_Models[1 << m_NumBitLevels];
public:
  void Init()
  {
    for(UINT32 i = 1; i < (1 << m_NumBitLevels); i++)
      m_Models[i].Init();
  }
  UINT32 Decode(CMyRangeDecoder *aRangeDecoder)
  {
    UINT32 aModelIndex = 1;
    RC_INIT_VAR
    for(UINT32 aBitIndex = m_NumBitLevels; aBitIndex > 0; aBitIndex--)
    {
      // aModelIndex = (aModelIndex << 1) + m_Models[aModelIndex].Decode(aRangeDecoder);
      RC_GETBIT(aNumMoveBits, m_Models[aModelIndex].m_Probability, aModelIndex)
    }
    RC_FLUSH_VAR
    return aModelIndex - (1 << m_NumBitLevels);
  };
};

////////////////////////////////
// CReverseBitTreeDecoder

template <int aNumMoveBits>
class CReverseBitTreeDecoder2
{
  CMyBitDecoder<aNumMoveBits> *m_Models;
  UINT32 m_NumBitLevels;
public:
  CReverseBitTreeDecoder2(): m_Models(0) { }
  ~CReverseBitTreeDecoder2() { delete []m_Models; }
  bool Create(UINT32 aNumBitLevels)
  {
    m_NumBitLevels = aNumBitLevels;
    m_Models = new CMyBitDecoder<aNumMoveBits>[1 << aNumBitLevels];
    return (m_Models != 0);
  }
  void Init()
  {
    UINT32 aNumModels = 1 << m_NumBitLevels;
    for(UINT32 i = 1; i < aNumModels; i++)
      m_Models[i].Init();
  }
  UINT32 Decode(CMyRangeDecoder *aRangeDecoder)
  {
    UINT32 aModelIndex = 1;
    UINT32 aSymbol = 0;
    RC_INIT_VAR
    for(UINT32 aBitIndex = 0; aBitIndex < m_NumBitLevels; aBitIndex++)
    {
      // UINT32 aBit = m_Models[aModelIndex].Decode(aRangeDecoder);
      // aModelIndex <<= 1;
      // aModelIndex += aBit;
      // aSymbol |= (aBit << aBitIndex);
      RC_GETBIT2(aNumMoveBits, m_Models[aModelIndex].m_Probability, aModelIndex, ; , aSymbol |= (1 << aBitIndex))
    }
    RC_FLUSH_VAR
    return aSymbol;
  };
};
////////////////////////////
// CReverseBitTreeDecoder2

template <int aNumMoveBits, UINT32 m_NumBitLevels>
class CReverseBitTreeDecoder
{
  CMyBitDecoder<aNumMoveBits> m_Models[1 << m_NumBitLevels];
public:
  void Init()
  {
    for(UINT32 i = 1; i < (1 << m_NumBitLevels); i++)
      m_Models[i].Init();
  }
  UINT32 Decode(CMyRangeDecoder *aRangeDecoder)
  {
    UINT32 aModelIndex = 1;
    UINT32 aSymbol = 0;
    RC_INIT_VAR
    for(UINT32 aBitIndex = 0; aBitIndex < m_NumBitLevels; aBitIndex++)
    {
      // UINT32 aBit = m_Models[aModelIndex].Decode(aRangeDecoder);
      // aModelIndex <<= 1;
      // aModelIndex += aBit;
      // aSymbol |= (aBit << aBitIndex);
      RC_GETBIT2(aNumMoveBits, m_Models[aModelIndex].m_Probability, aModelIndex, ; , aSymbol |= (1 << aBitIndex))
    }
    RC_FLUSH_VAR
    return aSymbol;
  }
};



#endif
