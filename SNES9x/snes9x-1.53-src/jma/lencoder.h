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

#ifndef __LENCODER_H
#define __LENCODER_H

#include "btreecd.h"

namespace NLength {

const UINT32 kNumPosStatesBitsMax = 4;
const int kNumPosStatesMax = (1 << kNumPosStatesBitsMax);


const int kNumPosStatesBitsEncodingMax = 4;
const int kNumPosStatesEncodingMax = (1 << kNumPosStatesBitsEncodingMax);


const int kNumMoveBits = 5;

const int kNumLenBits = 3;
const int kNumLowSymbols = 1 << kNumLenBits;
const int kNumMidBits = 3;
const int kNumMidSymbols = 1 << kNumMidBits;

const int kNumHighBits = 8;

const int kNumSymbolsTotal = kNumLowSymbols + kNumMidSymbols + (1 << kNumHighBits);

const int kNumSpecSymbols = kNumLowSymbols + kNumMidSymbols;

class CDecoder
{
  CMyBitDecoder<kNumMoveBits> m_Choice;
  CBitTreeDecoder<kNumMoveBits, kNumLenBits>  m_LowCoder[kNumPosStatesMax];
  CMyBitDecoder<kNumMoveBits> m_Choice2;
  CBitTreeDecoder<kNumMoveBits, kNumMidBits>  m_MidCoder[kNumPosStatesMax];
  CBitTreeDecoder<kNumMoveBits, kNumHighBits> m_HighCoder;
  UINT32 m_NumPosStates;
public:
  void Create(UINT32 aNumPosStates)
    { m_NumPosStates = aNumPosStates; }
  void Init()
  {
    m_Choice.Init();
    for (UINT32 aPosState = 0; aPosState < m_NumPosStates; aPosState++)
    {
      m_LowCoder[aPosState].Init();
      m_MidCoder[aPosState].Init();
    }
    m_Choice2.Init();
    m_HighCoder.Init();
  }
  UINT32 Decode(CMyRangeDecoder *aRangeDecoder, UINT32 aPosState)
  {
    if(m_Choice.Decode(aRangeDecoder) == 0)
      return m_LowCoder[aPosState].Decode(aRangeDecoder);
    else
    {
      UINT32 aSymbol = kNumLowSymbols;
      if(m_Choice2.Decode(aRangeDecoder) == 0)
        aSymbol += m_MidCoder[aPosState].Decode(aRangeDecoder);
      else
      {
        aSymbol += kNumMidSymbols;
        aSymbol += m_HighCoder.Decode(aRangeDecoder);
      }
      return aSymbol;
    }
  }

};

}


#endif
