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

#ifndef __RCDEFS_H
#define __RCDEFS_H

#include "aribitcd.h"
#include "ariconst.h"

#define RC_INIT_VAR                            \
  UINT32 aRange = aRangeDecoder->m_Range;      \
  UINT32 aCode = aRangeDecoder->m_Code;

#define RC_FLUSH_VAR                          \
  aRangeDecoder->m_Range = aRange;            \
  aRangeDecoder->m_Code = aCode;

#define RC_NORMALIZE                                    \
    if (aRange < NCompression::NArithmetic::kTopValue)               \
    {                                                              \
      aCode = (aCode << 8) | aRangeDecoder->m_Stream.ReadByte();   \
      aRange <<= 8; }

#define RC_GETBIT2(aNumMoveBits, aProb, aModelIndex, Action0, Action1)                        \
    {UINT32 aNewBound = (aRange >> NCompression::NArithmetic::kNumBitModelTotalBits) * aProb; \
    if (aCode < aNewBound)                               \
    {                                                             \
      Action0;                                                    \
      aRange = aNewBound;                                         \
      aProb += (NCompression::NArithmetic::kBitModelTotal - aProb) >> aNumMoveBits;          \
      aModelIndex <<= 1;                                          \
    }                                                             \
    else                                                          \
    {                                                             \
      Action1;                                                    \
      aRange -= aNewBound;                                        \
      aCode -= aNewBound;                                          \
      aProb -= (aProb) >> aNumMoveBits;                           \
      aModelIndex = (aModelIndex << 1) + 1;                       \
    }}                                                             \
    RC_NORMALIZE

#define RC_GETBIT(aNumMoveBits, aProb, aModelIndex) RC_GETBIT2(aNumMoveBits, aProb, aModelIndex, ; , ;)

#endif
