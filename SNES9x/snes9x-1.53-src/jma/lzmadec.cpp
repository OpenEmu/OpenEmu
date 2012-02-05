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

#include "portable.h"
#include "lzmadec.h"

#define RETURN_E_OUTOFMEMORY_IF_FALSE(x) { if (!(x)) return E_OUTOFMEMORY; }

namespace NCompress {
namespace NLZMA {

HRESULT CDecoder::SetDictionarySize(UINT32 aDictionarySize)
{
  if (aDictionarySize > (1 << kDicLogSizeMax))
    return E_INVALIDARG;

  UINT32 aWindowReservSize = MyMax(aDictionarySize, UINT32(1 << 21));

  if (m_DictionarySize != aDictionarySize)
  {
    m_OutWindowStream.Create(aDictionarySize, kMatchMaxLen, aWindowReservSize);
    m_DictionarySize = aDictionarySize;
  }
  return S_OK;
}

HRESULT CDecoder::SetLiteralProperties(
    UINT32 aLiteralPosStateBits, UINT32 aLiteralContextBits)
{
  if (aLiteralPosStateBits > 8)
    return E_INVALIDARG;
  if (aLiteralContextBits > 8)
    return E_INVALIDARG;
  m_LiteralDecoder.Create(aLiteralPosStateBits, aLiteralContextBits);
  return S_OK;
}

HRESULT CDecoder::SetPosBitsProperties(UINT32 aNumPosStateBits)
{
  if (aNumPosStateBits > NLength::kNumPosStatesBitsMax)
    return E_INVALIDARG;
  UINT32 aNumPosStates = 1 << aNumPosStateBits;
  m_LenDecoder.Create(aNumPosStates);
  m_RepMatchLenDecoder.Create(aNumPosStates);
  m_PosStateMask = aNumPosStates - 1;
  return S_OK;
}

CDecoder::CDecoder():
  m_DictionarySize((UINT32)-1)
{
  Create();
}

HRESULT CDecoder::Create()
{
  for(int i = 0; i < kNumPosModels; i++)
  {
    RETURN_E_OUTOFMEMORY_IF_FALSE(
        m_PosDecoders[i].Create(kDistDirectBits[kStartPosModelIndex + i]));
  }
  return S_OK;
}


HRESULT CDecoder::Init(ISequentialInStream *anInStream,
    ISequentialOutStream *anOutStream)
{
  m_RangeDecoder.Init(anInStream);

  m_OutWindowStream.Init(anOutStream);

  int i;
  for(i = 0; i < kNumStates; i++)
  {
    for (UINT32 j = 0; j <= m_PosStateMask; j++)
    {
      m_MainChoiceDecoders[i][j].Init();
      m_MatchRepShortChoiceDecoders[i][j].Init();
    }
    m_MatchChoiceDecoders[i].Init();
    m_MatchRepChoiceDecoders[i].Init();
    m_MatchRep1ChoiceDecoders[i].Init();
    m_MatchRep2ChoiceDecoders[i].Init();
  }

  m_LiteralDecoder.Init();

  // m_RepMatchLenDecoder.Init();

  for (i = 0; (UINT32) i < kNumLenToPosStates; i++)
    m_PosSlotDecoder[i].Init();

  for(i = 0; i < kNumPosModels; i++)
    m_PosDecoders[i].Init();

  m_LenDecoder.Init();
  m_RepMatchLenDecoder.Init();

  m_PosAlignDecoder.Init();
  return S_OK;

}

HRESULT CDecoder::CodeReal(ISequentialInStream *anInStream,
    ISequentialOutStream *anOutStream,
    const UINT64 *anInSize, const UINT64 *anOutSize)
{
  if (anOutSize == NULL)
    return E_INVALIDARG;

  Init(anInStream, anOutStream);

  CState aState;
  aState.Init();
  bool aPeviousIsMatch = false;
  BYTE aPreviousByte = 0;
  UINT32 aRepDistances[kNumRepDistances];
  for(UINT32 i = 0 ; i < kNumRepDistances; i++)
    aRepDistances[i] = 0;

  UINT64 aNowPos64 = 0;
  UINT64 aSize = *anOutSize;
  while(aNowPos64 < aSize)
  {
    UINT64 aNext = MyMin(aNowPos64 + (1 << 18), aSize);
    while(aNowPos64 < aNext)
    {
      UINT32 aPosState = UINT32(aNowPos64) & m_PosStateMask;
      if (m_MainChoiceDecoders[aState.m_Index][aPosState].Decode(&m_RangeDecoder) == (UINT32) kMainChoiceLiteralIndex)
      {
        // aCounts[0]++;
        aState.UpdateChar();
        if(aPeviousIsMatch)
        {
          BYTE aMatchByte = m_OutWindowStream.GetOneByte(0 - aRepDistances[0] - 1);
          aPreviousByte = m_LiteralDecoder.DecodeWithMatchByte(&m_RangeDecoder,
              UINT32(aNowPos64), aPreviousByte, aMatchByte);
          aPeviousIsMatch = false;
        }
        else
          aPreviousByte = m_LiteralDecoder.DecodeNormal(&m_RangeDecoder,
              UINT32(aNowPos64), aPreviousByte);
        m_OutWindowStream.PutOneByte(aPreviousByte);
        aNowPos64++;
      }
      else
      {
        aPeviousIsMatch = true;
        UINT32 aDistance, aLen;
        if(m_MatchChoiceDecoders[aState.m_Index].Decode(&m_RangeDecoder) ==
            (UINT32) kMatchChoiceRepetitionIndex)
        {
          if(m_MatchRepChoiceDecoders[aState.m_Index].Decode(&m_RangeDecoder) == 0)
          {
            if(m_MatchRepShortChoiceDecoders[aState.m_Index][aPosState].Decode(&m_RangeDecoder) == 0)
            {
              aState.UpdateShortRep();
              aPreviousByte = m_OutWindowStream.GetOneByte(0 - aRepDistances[0] - 1);
              m_OutWindowStream.PutOneByte(aPreviousByte);
              aNowPos64++;
              // aCounts[3 + 4]++;
              continue;
            }
            // aCounts[3 + 0]++;
            aDistance = aRepDistances[0];
          }
          else
          {
            if(m_MatchRep1ChoiceDecoders[aState.m_Index].Decode(&m_RangeDecoder) == 0)
            {
              aDistance = aRepDistances[1];
              aRepDistances[1] = aRepDistances[0];
              // aCounts[3 + 1]++;
            }
            else
            {
              if (m_MatchRep2ChoiceDecoders[aState.m_Index].Decode(&m_RangeDecoder) == 0)
              {
                // aCounts[3 + 2]++;
                aDistance = aRepDistances[2];
              }
              else
              {
                // aCounts[3 + 3]++;
                aDistance = aRepDistances[3];
                aRepDistances[3] = aRepDistances[2];
              }
              aRepDistances[2] = aRepDistances[1];
              aRepDistances[1] = aRepDistances[0];
            }
            aRepDistances[0] = aDistance;
          }
          aLen = m_RepMatchLenDecoder.Decode(&m_RangeDecoder, aPosState) + kMatchMinLen;
          // aCounts[aLen]++;
          aState.UpdateRep();
        }
        else
        {
          aLen = kMatchMinLen + m_LenDecoder.Decode(&m_RangeDecoder, aPosState);
          aState.UpdateMatch();
          UINT32 aPosSlot = m_PosSlotDecoder[GetLenToPosState(aLen)].Decode(&m_RangeDecoder);
          // aCounts[aPosSlot]++;
          if (aPosSlot >= (UINT32) kStartPosModelIndex)
          {
            aDistance = kDistStart[aPosSlot];
            if (aPosSlot < (UINT32) kEndPosModelIndex)
              aDistance += m_PosDecoders[aPosSlot - kStartPosModelIndex].Decode(&m_RangeDecoder);
            else
            {
              aDistance += (m_RangeDecoder.DecodeDirectBits(kDistDirectBits[aPosSlot] -
                  kNumAlignBits) << kNumAlignBits);
              aDistance += m_PosAlignDecoder.Decode(&m_RangeDecoder);
            }
          }
          else
            aDistance = aPosSlot;


          aRepDistances[3] = aRepDistances[2];
          aRepDistances[2] = aRepDistances[1];
          aRepDistances[1] = aRepDistances[0];

          aRepDistances[0] = aDistance;
          // UpdateStat(aLen, aPosSlot);
        }
        if (aDistance >= aNowPos64)
          throw E_INVALIDDATA;
        m_OutWindowStream.CopyBackBlock(aDistance, aLen);
        aNowPos64 += aLen;
        aPreviousByte = m_OutWindowStream.GetOneByte(0 - 1);
      }
    }
  }
  return Flush();
}

HRESULT CDecoder::Code(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream, const UINT64 *anInSize, const UINT64 *anOutSize)
{
  try {
     return CodeReal(anInStream, anOutStream, anInSize, anOutSize);
  } catch (HRESULT& e) {
     return e;
  } catch (...) {
     return E_FAIL;
  }
}

HRESULT CDecoder::ReadCoderProperties(ISequentialInStream *anInStream)
{
  UINT32 aNumPosStateBits;
  UINT32 aLiteralPosStateBits;
  UINT32 aLiteralContextBits;
  UINT32 aDictionarySize;

   UINT32 aProcessesedSize;

  BYTE aByte;
  RETURN_IF_NOT_S_OK(anInStream->Read(&aByte, sizeof(aByte), &aProcessesedSize));
  if (aProcessesedSize != sizeof(aByte))
    return E_INVALIDARG;

  aLiteralContextBits = aByte % 9;
  BYTE aRemainder = aByte / 9;
  aLiteralPosStateBits = aRemainder % 5;
  aNumPosStateBits = aRemainder / 5;

  UINT8 uint_buffer[UINT_SIZE];
  RETURN_IF_NOT_S_OK(anInStream->Read(uint_buffer, sizeof(aDictionarySize), &aProcessesedSize));
  aDictionarySize = charp_to_uint(uint_buffer);

  if (aProcessesedSize != sizeof(aDictionarySize))
    return E_INVALIDARG;

  RETURN_IF_NOT_S_OK(SetDictionarySize(aDictionarySize));
  RETURN_IF_NOT_S_OK(SetLiteralProperties(aLiteralPosStateBits, aLiteralContextBits));
  RETURN_IF_NOT_S_OK(SetPosBitsProperties(aNumPosStateBits));

  return S_OK;
}

}}
