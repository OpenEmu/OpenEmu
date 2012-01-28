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

#ifndef __LZARITHMETIC_DECODER_H
#define __LZARITHMETIC_DECODER_H

#include "winout.h"
#include "lzma.h"
#include "lencoder.h"
#include "litcoder.h"

namespace NCompress {
namespace NLZMA {

typedef CMyBitDecoder<kNumMoveBitsForMainChoice> CMyBitDecoder2;

class CDecoder
{
  NStream::NWindow::COut m_OutWindowStream;
  CMyRangeDecoder m_RangeDecoder;

  CMyBitDecoder2 m_MainChoiceDecoders[kNumStates][NLength::kNumPosStatesMax];
  CMyBitDecoder2 m_MatchChoiceDecoders[kNumStates];
  CMyBitDecoder2 m_MatchRepChoiceDecoders[kNumStates];
  CMyBitDecoder2 m_MatchRep1ChoiceDecoders[kNumStates];
  CMyBitDecoder2 m_MatchRep2ChoiceDecoders[kNumStates];
  CMyBitDecoder2 m_MatchRepShortChoiceDecoders[kNumStates][NLength::kNumPosStatesMax];

  CBitTreeDecoder<kNumMoveBitsForPosSlotCoder, kNumPosSlotBits> m_PosSlotDecoder[kNumLenToPosStates];

  CReverseBitTreeDecoder2<kNumMoveBitsForPosCoders> m_PosDecoders[kNumPosModels];
  CReverseBitTreeDecoder<kNumMoveBitsForAlignCoders, kNumAlignBits> m_PosAlignDecoder;
  // CBitTreeDecoder2<kNumMoveBitsForPosCoders> m_PosDecoders[kNumPosModels];
  // CBitTreeDecoder<kNumMoveBitsForAlignCoders, kNumAlignBits> m_PosAlignDecoder;

  NLength::CDecoder m_LenDecoder;
  NLength::CDecoder m_RepMatchLenDecoder;

  NLiteral::CDecoder m_LiteralDecoder;

  UINT32 m_DictionarySize;

  UINT32 m_PosStateMask;

  HRESULT Create();

  HRESULT Init(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream);

  HRESULT Flush() {  return m_OutWindowStream.Flush(); }

  HRESULT CodeReal(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream, const UINT64 *anInSize, const UINT64 *anOutSize);

public:

  CDecoder();

  HRESULT Code(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream, const UINT64 *anInSize, const UINT64 *anOutSize);
  HRESULT ReadCoderProperties(ISequentialInStream *anInStream);

  HRESULT SetDictionarySize(UINT32 aDictionarySize);
  HRESULT SetLiteralProperties(UINT32 aLiteralPosStateBits, UINT32 aLiteralContextBits);
  HRESULT SetPosBitsProperties(UINT32 aNumPosStateBits);
};

}}

#endif
