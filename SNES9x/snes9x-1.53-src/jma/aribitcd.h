#ifndef __COMPRESSION_BITCODER_H
#define __COMPRESSION_BITCODER_H

#include "rngcoder.h"

namespace NCompression {
namespace NArithmetic {

const int kNumBitModelTotalBits  = 11;
const UINT32 kBitModelTotal = (1 << kNumBitModelTotalBits);

const int kNumMoveReducingBits = 2;

/////////////////////////////
// CBitModel

template <int aNumMoveBits>
class CBitModel
{
public:
  UINT32 m_Probability;
  void UpdateModel(UINT32 aSymbol)
  {
    /*
    m_Probability -= (m_Probability + ((aSymbol - 1) & ((1 << aNumMoveBits) - 1))) >> aNumMoveBits;
    m_Probability += (1 - aSymbol) << (kNumBitModelTotalBits - aNumMoveBits);
    */
    if (aSymbol == 0)
      m_Probability += (kBitModelTotal - m_Probability) >> aNumMoveBits;
    else
      m_Probability -= (m_Probability) >> aNumMoveBits;
  }
public:
  void Init() { m_Probability = kBitModelTotal / 2; }
};

template <int aNumMoveBits>
class CBitDecoder: public CBitModel<aNumMoveBits>
{
public:
  UINT32 Decode(CRangeDecoder *aRangeDecoder)
  {
    UINT32 aNewBound = (aRangeDecoder->m_Range >> kNumBitModelTotalBits) * CBitModel<aNumMoveBits>::m_Probability;
    if (aRangeDecoder->m_Code < aNewBound)
    {
      aRangeDecoder->m_Range = aNewBound;
      CBitModel<aNumMoveBits>::m_Probability += (kBitModelTotal - CBitModel<aNumMoveBits>::m_Probability) >> aNumMoveBits;
      if (aRangeDecoder->m_Range < kTopValue)
      {
        aRangeDecoder->m_Code = (aRangeDecoder->m_Code << 8) | aRangeDecoder->m_Stream.ReadByte();
        aRangeDecoder->m_Range <<= 8;
      }
      return 0;
    }
    else
    {
      aRangeDecoder->m_Range -= aNewBound;
      aRangeDecoder->m_Code -= aNewBound;
      CBitModel<aNumMoveBits>::m_Probability -= (CBitModel<aNumMoveBits>::m_Probability) >> aNumMoveBits;
      if (aRangeDecoder->m_Range < kTopValue)
      {
        aRangeDecoder->m_Code = (aRangeDecoder->m_Code << 8) | aRangeDecoder->m_Stream.ReadByte();
        aRangeDecoder->m_Range <<= 8;
      }
      return 1;
    }
  }
};

}}


#endif
