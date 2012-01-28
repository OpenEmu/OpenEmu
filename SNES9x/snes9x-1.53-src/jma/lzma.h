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

#include "lencoder.h"

#ifndef __LZMA_H
#define __LZMA_H

namespace NCompress {
namespace NLZMA {

const UINT32 kNumRepDistances = 4;

const BYTE kNumStates = 12;

const BYTE kLiteralNextStates[kNumStates] = {0, 0, 0, 0, 1, 2, 3, 4,  5,  6,   4, 5};
const BYTE kMatchNextStates[kNumStates]   = {7, 7, 7, 7, 7, 7, 7, 10, 10, 10, 10, 10};
const BYTE kRepNextStates[kNumStates]     = {8, 8, 8, 8, 8, 8, 8, 11, 11, 11, 11, 11};
const BYTE kShortRepNextStates[kNumStates]= {9, 9, 9, 9, 9, 9, 9, 11, 11, 11, 11, 11};

class CState
{
public:
  BYTE m_Index;
  void Init()
    { m_Index = 0; }
  void UpdateChar()
    { m_Index = kLiteralNextStates[m_Index]; }
  void UpdateMatch()
    { m_Index = kMatchNextStates[m_Index]; }
  void UpdateRep()
    { m_Index = kRepNextStates[m_Index]; }
  void UpdateShortRep()
    { m_Index = kShortRepNextStates[m_Index]; }
};

class CBaseCoder
{
protected:
  CState m_State;
  BYTE m_PreviousByte;
  bool m_PeviousIsMatch;
  UINT32 m_RepDistances[kNumRepDistances];
  void Init()
  {
    m_State.Init();
    m_PreviousByte = 0;
    m_PeviousIsMatch = false;
    for(UINT32 i = 0 ; i < kNumRepDistances; i++)
      m_RepDistances[i] = 0;
  }
};

const int kNumPosSlotBits = 6;
const int kDicLogSizeMax = 28;
const int kDistTableSizeMax = kDicLogSizeMax * 2;

extern UINT32 kDistStart[kDistTableSizeMax];
const BYTE kDistDirectBits[kDistTableSizeMax] =
{
  0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
  10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
  20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26
};

const UINT32 kNumLenToPosStates = 4;
inline UINT32 GetLenToPosState(UINT32 aLen)
{
  aLen -= 2;
  if (aLen < kNumLenToPosStates)
    return aLen;
  return kNumLenToPosStates - 1;
}

const int kMatchMinLen = 2;

const int kMatchMaxLen = kMatchMinLen + NLength::kNumSymbolsTotal - 1;

const int kNumAlignBits = 4;
const int kAlignTableSize = 1 << kNumAlignBits;
const UINT32 kAlignMask = (kAlignTableSize - 1);

const int kStartPosModelIndex = 4;
const int kEndPosModelIndex = 14;
const int kNumPosModels = kEndPosModelIndex - kStartPosModelIndex;

const int kNumFullDistances = 1 << (kEndPosModelIndex / 2);


const int kMainChoiceLiteralIndex = 0;
const int kMainChoiceMatchIndex = 1;

const int kMatchChoiceDistanceIndex= 0;
const int kMatchChoiceRepetitionIndex = 1;

const int kNumMoveBitsForMainChoice = 5;
const int kNumMoveBitsForPosCoders = 5;

const int kNumMoveBitsForAlignCoders = 5;

const int kNumMoveBitsForPosSlotCoder = 5;

const int kNumLitPosStatesBitsEncodingMax = 4;
const int kNumLitContextBitsMax = 8;

}}

#endif
