/*
  Copyright (c) 2005, The Musepack Development Team
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.

  * Neither the name of the The Musepack Development Team nor the
  names of its contributors may be used to endorse or promote
  products derived from this software without specific prior
  written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/// \file mpc_decoder.c
/// Core decoding routines and logic.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mpcdec/mpcdec.h>
#include <mpcdec/internal.h>
#include <mpcdec/requant.h>
#include <mpcdec/huffman.h>

//SV7 tables
extern const HuffmanTyp*   mpc_table_HuffQ [2] [8];
extern const HuffmanTyp    mpc_table_HuffHdr  [10];
extern const HuffmanTyp    mpc_table_HuffSCFI [ 4];
extern const HuffmanTyp    mpc_table_HuffDSCF [16];


#ifdef MPC_SUPPORT_SV456
//SV4/5/6 tables
extern const HuffmanTyp*   mpc_table_SampleHuff [18];
extern const HuffmanTyp    mpc_table_SCFI_Bundle   [ 8];
extern const HuffmanTyp    mpc_table_DSCF_Entropie [13];
extern const HuffmanTyp    mpc_table_Region_A [16];
extern const HuffmanTyp    mpc_table_Region_B [ 8];
extern const HuffmanTyp    mpc_table_Region_C [ 4];

#endif

#ifndef MPC_LITTLE_ENDIAN
#define SWAP(X) mpc_swap32(X)
#else
#define SWAP(X) (X)
#endif

//------------------------------------------------------------------------------
// types
//------------------------------------------------------------------------------
enum
    {
        EQ_TAP = 13,                        // length of FIR filter for EQ
        DELAY = ((EQ_TAP + 1) / 2),         // delay of FIR
        FIR_BANDS = 4,                      // number of subbands to be FIR filtered
        MEMSIZE = MPC_DECODER_MEMSIZE,      // overall buffer size
        MEMSIZE2 = (MEMSIZE/2),             // size of one buffer
        MEMMASK = (MEMSIZE-1)
    };

//------------------------------------------------------------------------------
// forward declarations
//------------------------------------------------------------------------------
void mpc_decoder_read_bitstream_sv6(mpc_decoder *d, mpc_bool_t seeking);
void mpc_decoder_read_bitstream_sv7(mpc_decoder *d, mpc_bool_t seeking);
mpc_bool_t mpc_decoder_seek_sample(mpc_decoder *d, mpc_int64_t destsample);
void mpc_decoder_requantisierung(mpc_decoder *d, const mpc_int32_t Last_Band);

//------------------------------------------------------------------------------
// utility functions
//------------------------------------------------------------------------------
static mpc_int32_t f_read(mpc_decoder *d, void *ptr, mpc_int32_t size) 
{ 
    return d->r->read(d->r->data, ptr, size); 
}

static mpc_bool_t f_seek(mpc_decoder *d, mpc_int32_t offset) 
{ 
    return d->r->seek(d->r->data, offset); 
}

static mpc_int32_t f_read_dword(mpc_decoder *d, mpc_uint32_t * ptr, mpc_uint32_t count) 
{
    return f_read(d, ptr, count << 2) >> 2;
}

static void mpc_decoder_seek(mpc_decoder *d, mpc_uint32_t bitpos)
{
    f_seek(d, (bitpos>>5) * 4 + d->MPCHeaderPos);
    f_read_dword(d, d->Speicher, MEMSIZE);
    d->dword = SWAP(d->Speicher[d->Zaehler = 0]);
    d->pos = bitpos & 31;
    d->WordsRead = bitpos >> 5;
}

// jump desired number of bits out of the bitstream
static void mpc_decoder_bitstream_jump(mpc_decoder *d, const mpc_uint32_t bits)
{
    d->pos += bits;

    if (d->pos >= 32) {
        d->Zaehler = (d->Zaehler + (d->pos >> 5)) & MEMMASK;
        d->dword = SWAP(d->Speicher[d->Zaehler]);
        d->WordsRead += d->pos >> 5;
        d->pos &= 31;
    }
}

void mpc_decoder_update_buffer(mpc_decoder *d, mpc_uint32_t RING)
{
    if ((RING ^ d->Zaehler) & MEMSIZE2 ) {
        // update buffer
        f_read_dword(d, d->Speicher + (RING & MEMSIZE2), MEMSIZE2);
    }
}

//------------------------------------------------------------------------------
// huffman & bitstream functions
//------------------------------------------------------------------------------

/* F U N C T I O N S */

// resets bitstream decoding
static void
mpc_decoder_reset_bitstream_decode(mpc_decoder *d) 
{
    d->dword = 0;
    d->pos = 0;
    d->Zaehler = 0;
    d->WordsRead = 0;
}

// reports the number of read bits
static mpc_uint32_t
mpc_decoder_bits_read(mpc_decoder *d) 
{
    return 32 * d->WordsRead + d->pos;
}

// read desired number of bits out of the bitstream (max 31)
static mpc_uint32_t
mpc_decoder_bitstream_read(mpc_decoder *d, const mpc_uint32_t bits) 
{
    mpc_uint32_t out = d->dword;

    d->pos += bits;

    if (d->pos < 32) {
        out >>= (32 - d->pos);
    } else {
        d->dword = SWAP(d->Speicher[d->Zaehler = (d->Zaehler + 1) & MEMMASK]);
        d->pos -= 32;
        if (d->pos) {
            out <<= d->pos;
            out |= d->dword >> (32 - d->pos);
        }
        d->WordsRead++;
    }

    return out & ((1 << bits) - 1);
}

// basic huffman decoding routine
// works with maximum lengths up to max_length
static mpc_int32_t
mpc_decoder_huffman_decode(mpc_decoder *d, const HuffmanTyp *Table,
                           const mpc_uint32_t max_length)
{
    // load preview and decode
    mpc_uint32_t code = d->dword << d->pos;
    if (32 - d->pos < max_length)
        code |= SWAP(d->Speicher[(d->Zaehler + 1) & MEMMASK]) >> (32 - d->pos);

    while (code < Table->Code) Table++;

    // set the new position within bitstream without performing a dummy-read
    if ((d->pos += Table->Length) >= 32) {
        d->pos -= 32;
        d->dword = SWAP(d->Speicher[d->Zaehler = (d->Zaehler + 1) & MEMMASK]);
        d->WordsRead++;
    }

    return Table->Value;
}

// decode SCFI-bundle (sv4,5,6)
static void
mpc_decoder_scfi_bundle_read(mpc_decoder *d, const HuffmanTyp* Table,
                             mpc_int32_t* SCFI, mpc_bool_t* DSCF)
{
    mpc_uint32_t value = mpc_decoder_huffman_decode(d, Table, 6);

    *SCFI = value >> 1;
    *DSCF = value &  1;
}

static void
mpc_decoder_reset_v(mpc_decoder *d) 
{
    memset(d->V_L, 0, sizeof d->V_L);
    memset(d->V_R, 0, sizeof d->V_R);
}

static void
mpc_decoder_reset_synthesis(mpc_decoder *d) 
{
    mpc_decoder_reset_v(d);
}

static void
mpc_decoder_reset_y(mpc_decoder *d) 
{
    memset(d->Y_L, 0, sizeof d->Y_L);
    memset(d->Y_R, 0, sizeof d->Y_R);
}

static void
mpc_decoder_reset_globals(mpc_decoder *d) 
{
    mpc_decoder_reset_bitstream_decode(d);

    d->DecodedFrames  = 0;
    d->StreamVersion  = 0;
    d->MS_used        = 0;

    memset(d->Y_L             , 0, sizeof d->Y_L              );
    memset(d->Y_R             , 0, sizeof d->Y_R              );
    memset(d->SCF_Index_L     , 0, sizeof d->SCF_Index_L      );
    memset(d->SCF_Index_R     , 0, sizeof d->SCF_Index_R      );
    memset(d->Res_L           , 0, sizeof d->Res_L            );
    memset(d->Res_R           , 0, sizeof d->Res_R            );
    memset(d->SCFI_L          , 0, sizeof d->SCFI_L           );
    memset(d->SCFI_R          , 0, sizeof d->SCFI_R           );
    memset(d->DSCF_Flag_L     , 0, sizeof d->DSCF_Flag_L      );
    memset(d->DSCF_Flag_R     , 0, sizeof d->DSCF_Flag_R      );
    memset(d->Q               , 0, sizeof d->Q                );
    memset(d->MS_Flag         , 0, sizeof d->MS_Flag          );
    memset(d->seeking_table   , 0, sizeof d->seeking_table    );
}

// Frame decoding. Takes big endian 32 bits words as input
mpc_uint32_t
mpc_decoder_decode_frame(mpc_decoder *d, mpc_uint32_t *in_buffer,
                         mpc_uint32_t in_len, MPC_SAMPLE_FORMAT *out_buffer)
{
  unsigned int i;
  mpc_decoder_reset_bitstream_decode(d);
  if (in_len > sizeof(d->Speicher)) in_len = sizeof(d->Speicher);
  memcpy(d->Speicher, in_buffer, in_len);
  for (i = 0; i < (in_len + 3) / 4; i++)
    d->Speicher[i] = mpc_swap32(d->Speicher[i]);
  d->dword = SWAP(d->Speicher[0]);
  switch (d->StreamVersion) {
#ifdef MPC_SUPPORT_SV456
    case 0x04:
    case 0x05:
    case 0x06:
        mpc_decoder_read_bitstream_sv6(d, FALSE);
        break;
#endif
    case 0x07:
    case 0x17:
        mpc_decoder_read_bitstream_sv7(d, FALSE);
        break;
    default:
        return (mpc_uint32_t)(-1);
  }
  mpc_decoder_requantisierung(d, d->Max_Band);
  mpc_decoder_synthese_filter_float(d, out_buffer);
  return mpc_decoder_bits_read(d);
}

static mpc_uint32_t
mpc_decoder_decode_internal(mpc_decoder *d, MPC_SAMPLE_FORMAT *buffer) 
{
    mpc_uint32_t output_frame_length = MPC_FRAME_LENGTH;
    mpc_uint32_t FwdJumpInfo = 0;
    mpc_uint32_t  FrameBitCnt = 0;

    if (d->DecodedFrames >= d->OverallFrames) {
        return (mpc_uint32_t)(-1);                           // end of file -> abort decoding
    }

    // add seeking info
    if (d->seeking_table_frames < d->DecodedFrames &&
       (d->DecodedFrames & ((1 << d->seeking_pwr) - 1)) == 0) {
        d->seeking_table[d->DecodedFrames >> d->seeking_pwr] = mpc_decoder_bits_read(d);
        d->seeking_table_frames = d->DecodedFrames;
    }

    // read jump-info for validity check of frame
    FwdJumpInfo  = mpc_decoder_bitstream_read(d, 20);

    // decode data and check for validity of frame
    FrameBitCnt = mpc_decoder_bits_read(d);
    switch (d->StreamVersion) {
#ifdef MPC_SUPPORT_SV456
    case 0x04:
    case 0x05:
    case 0x06:
        mpc_decoder_read_bitstream_sv6(d, FALSE);
        break;
#endif
    case 0x07:
    case 0x17:
        mpc_decoder_read_bitstream_sv7(d, FALSE);
        break;
    default:
        return (mpc_uint32_t)(-1);
    }
    d->FrameWasValid = mpc_decoder_bits_read(d) - FrameBitCnt == FwdJumpInfo;

    // synthesize signal
    mpc_decoder_requantisierung(d, d->Max_Band);
    mpc_decoder_synthese_filter_float(d, buffer);

    d->DecodedFrames++;

    // cut off first MPC_DECODER_SYNTH_DELAY zero-samples
    if (d->DecodedFrames == d->OverallFrames  && d->StreamVersion >= 6) {        
        // reconstruct exact filelength
        mpc_int32_t  mod_block   = mpc_decoder_bitstream_read(d,  11);
        mpc_int32_t  FilterDecay;

        if (mod_block == 0) {
            // Encoder bugfix
            mod_block = 1152;                    
        }
        FilterDecay = (mod_block + MPC_DECODER_SYNTH_DELAY) % MPC_FRAME_LENGTH;

        // additional FilterDecay samples are needed for decay of synthesis filter
        if (MPC_DECODER_SYNTH_DELAY + mod_block >= MPC_FRAME_LENGTH) {
            if (!d->TrueGaplessPresent) {
                mpc_decoder_reset_y(d);
            } else {
                mpc_decoder_bitstream_read(d, 20);
                mpc_decoder_read_bitstream_sv7(d, FALSE);
                mpc_decoder_requantisierung(d, d->Max_Band);
            }

            mpc_decoder_synthese_filter_float(d, buffer + 2304);

            output_frame_length = MPC_FRAME_LENGTH + FilterDecay;
        }
        else {                              // there are only FilterDecay samples needed for this frame
            output_frame_length = FilterDecay;
        }
    }

    if (d->samples_to_skip) {
        if (output_frame_length < d->samples_to_skip) {
            d->samples_to_skip -= output_frame_length;
            output_frame_length = 0;
        }
        else {
            output_frame_length -= d->samples_to_skip;
            memmove(
                buffer, 
                buffer + d->samples_to_skip * 2, 
                output_frame_length * 2 * sizeof (MPC_SAMPLE_FORMAT));
            d->samples_to_skip = 0;
        }
    }

    return output_frame_length;
}

mpc_uint32_t mpc_decoder_decode(
    mpc_decoder *d,
    MPC_SAMPLE_FORMAT *buffer, 
    mpc_uint32_t *vbr_update_acc, 
    mpc_uint32_t *vbr_update_bits)
{
    for(;;)
    {
        //const mpc_int32_t MaxBrokenFrames = 0; // PluginSettings.MaxBrokenFrames

        mpc_uint32_t RING = d->Zaehler;
        mpc_int32_t vbr_ring = (RING << 5) + d->pos;

        mpc_uint32_t valid_samples = mpc_decoder_decode_internal(d, buffer);

        if (valid_samples == (mpc_uint32_t)(-1) ) {
            return 0;
        }

        /**************** ERROR CONCEALMENT *****************/
        if (d->FrameWasValid == 0 ) {
            // error occurred in bitstream
            return (mpc_uint32_t)(-1);
        } 
        else {
            if (vbr_update_acc && vbr_update_bits) {
                (*vbr_update_acc) ++;
                vbr_ring = (d->Zaehler << 5) + d->pos - vbr_ring;
                if (vbr_ring < 0) {
                    vbr_ring += 524288;
                }
                (*vbr_update_bits) += vbr_ring;
            }

        }
        mpc_decoder_update_buffer(d, RING);

        if (valid_samples > 0) {
            return valid_samples;
        }
    }
}

void
mpc_decoder_requantisierung(mpc_decoder *d, const mpc_int32_t Last_Band) 
{
    mpc_int32_t     Band;
    mpc_int32_t     n;
    MPC_SAMPLE_FORMAT facL;
    MPC_SAMPLE_FORMAT facR;
    MPC_SAMPLE_FORMAT templ;
    MPC_SAMPLE_FORMAT tempr;
    MPC_SAMPLE_FORMAT* YL;
    MPC_SAMPLE_FORMAT* YR;
    mpc_int32_t*    L;
    mpc_int32_t*    R;

#ifdef MPC_FIXED_POINT
#if MPC_FIXED_POINT_FRACTPART == 14
#define MPC_MULTIPLY_SCF(CcVal, SCF_idx) \
    MPC_MULTIPLY_EX(CcVal, d->SCF[SCF_idx], d->SCF_shift[SCF_idx])
#else

#error FIXME, Cc table is in 18.14 format

#endif
#else
#define MPC_MULTIPLY_SCF(CcVal, SCF_idx) \
    MPC_MULTIPLY(CcVal, d->SCF[SCF_idx])
#endif
    // requantization and scaling of subband-samples
    for ( Band = 0; Band <= Last_Band; Band++ ) {   // setting pointers
        YL = d->Y_L[0] + Band;
        YR = d->Y_R[0] + Band;
        L  = d->Q[Band].L;
        R  = d->Q[Band].R;
        /************************** MS-coded **************************/
        if ( d->MS_Flag [Band] ) {
            if ( d->Res_L [Band] ) {
                if ( d->Res_R [Band] ) {    // M!=0, S!=0
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][0]);
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = MPC_MULTIPLY_FLOAT_INT(facL,*L++))+(tempr = MPC_MULTIPLY_FLOAT_INT(facR,*R++));
                        *YR   = templ - tempr;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][1]);
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = MPC_MULTIPLY_FLOAT_INT(facL,*L++))+(tempr = MPC_MULTIPLY_FLOAT_INT(facR,*R++));
                        *YR   = templ - tempr;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][2]);
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = MPC_MULTIPLY_FLOAT_INT(facL,*L++))+(tempr = MPC_MULTIPLY_FLOAT_INT(facR,*R++));
                        *YR   = templ - tempr;
                    }
                } else {    // M!=0, S==0
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                    }
                }
            } else {
                if (d->Res_R[Band])    // M==0, S!=0
                {
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = MPC_MULTIPLY_FLOAT_INT(facR,*(R++)));
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = MPC_MULTIPLY_FLOAT_INT(facR,*(R++)));
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = MPC_MULTIPLY_FLOAT_INT(facR,*(R++)));
                    }
                } else {    // M==0, S==0
                    for ( n = 0; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = 0;
                    }
                }
            }
        }
        /************************** LR-coded **************************/
        else {
            if ( d->Res_L [Band] ) {
                if ( d->Res_R [Band] ) {    // L!=0, R!=0
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][0]);
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][0]);
                    for (n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][1]);
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][1]);
                    for (; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][2]);
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][2]);
                    for (; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                } else {     // L!=0, R==0
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = 0;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = 0;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[d->Res_L[Band]] , (unsigned char)d->SCF_Index_L[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = 0;
                    }
                }
            }
            else {
                if ( d->Res_R [Band] ) {    // L==0, R!=0
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = 0;
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = 0;
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[d->Res_R[Band]] , (unsigned char)d->SCF_Index_R[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = 0;
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                } else {    // L==0, R==0
                    for ( n = 0; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = 0;
                    }
                }
            }
        }
    }
}

#ifdef MPC_SUPPORT_SV456
static const unsigned char Q_res[32][16] = {
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,17},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,17,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,17,0,0,0,0,0,0,0,0,0,0,0,0},
};

/****************************************** SV 6 ******************************************/
void
mpc_decoder_read_bitstream_sv6(mpc_decoder *d, mpc_bool_t seeking)
{
    mpc_int32_t n,k;
    mpc_int32_t Max_used_Band=0;
    const HuffmanTyp *Table;
    const HuffmanTyp *x1;
    const HuffmanTyp *x2;
    mpc_int32_t *L;
    mpc_int32_t *R;
    mpc_int32_t *ResL = d->Res_L;
    mpc_int32_t *ResR = d->Res_R;

    /************************ HEADER **************************/
    ResL = d->Res_L;
    ResR = d->Res_R;
    for (n=0; n <= d->Max_Band; ++n, ++ResL, ++ResR)
    {
        if      (n<11)           Table = mpc_table_Region_A;
        else if (n>=11 && n<=22) Table = mpc_table_Region_B;
        else /*if (n>=23)*/      Table = mpc_table_Region_C;

        *ResL = Q_res[n][mpc_decoder_huffman_decode(d, Table, 14)];
        if (d->MS_used) {
            d->MS_Flag[n] = mpc_decoder_bitstream_read(d,  1);
        }
        *ResR = Q_res[n][mpc_decoder_huffman_decode(d, Table, 14)];

        // only perform the following procedure up to the maximum non-zero subband
        if (*ResL || *ResR) Max_used_Band = n;
    }

    /************************* SCFI-Bundle *****************************/
    ResL = d->Res_L;
    ResR = d->Res_R;
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR) {
        if (*ResL) mpc_decoder_scfi_bundle_read(d, mpc_table_SCFI_Bundle, &(d->SCFI_L[n]), &(d->DSCF_Flag_L[n]));
        if (*ResR) mpc_decoder_scfi_bundle_read(d, mpc_table_SCFI_Bundle, &(d->SCFI_R[n]), &(d->DSCF_Flag_R[n]));
    }

    /***************************** SCFI ********************************/
    ResL = d->Res_L;
    ResR = d->Res_R;
    L    = d->SCF_Index_L[0];
    R    = d->SCF_Index_R[0];
    for (n=0; n <= Max_used_Band; ++n, ++ResL, ++ResR, L+=3, R+=3)
    {
        if (*ResL)
        {
            /*********** DSCF ************/
            if (d->DSCF_Flag_L[n]==1)
            {
                switch (d->SCFI_L[n])
                {
                case 3:
                    L[0] = L[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 1:
                    L[0] = L[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    L[1] = L[0] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    L[2] = L[1];
                    break;
                case 2:
                    L[0] = L[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    L[1] = L[0];
                    L[2] = L[1] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    break;
                case 0:
                    L[0] = L[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    L[1] = L[0] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    L[2] = L[1] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    break;
                default:
                    return;
                }
                if (L[0] > 1024)
                    L[0] = 0x8080;
                if (L[1] > 1024)
                    L[1] = 0x8080;
                if (L[2] > 1024)
                    L[2] = 0x8080;
            }
            /************ SCF ************/
            else
            {
                switch (d->SCFI_L[n])
                {
                case 3:
                    L[0] = mpc_decoder_bitstream_read(d,  6);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 1:
                    L[0] = mpc_decoder_bitstream_read(d,  6);
                    L[1] = mpc_decoder_bitstream_read(d,  6);
                    L[2] = L[1];
                    break;
                case 2:
                    L[0] = mpc_decoder_bitstream_read(d,  6);
                    L[1] = L[0];
                    L[2] = mpc_decoder_bitstream_read(d,  6);
                    break;
                case 0:
                    L[0] = mpc_decoder_bitstream_read(d,  6);
                    L[1] = mpc_decoder_bitstream_read(d,  6);
                    L[2] = mpc_decoder_bitstream_read(d,  6);
                    break;
                default:
                    return;
                }
            }
        }
        if (*ResR)
        {
            /*********** DSCF ************/
            if (d->DSCF_Flag_R[n]==1)
            {
                switch (d->SCFI_R[n])
                {
                case 3:
                    R[0] = R[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 1:
                    R[0] = R[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    R[1] = R[0] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    R[2] = R[1];
                    break;
                case 2:
                    R[0] = R[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    R[1] = R[0];
                    R[2] = R[1] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    break;
                case 0:
                    R[0] = R[2] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    R[1] = R[0] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    R[2] = R[1] + mpc_decoder_huffman_decode(d,  mpc_table_DSCF_Entropie, 6);
                    break;
                default:
                    return;
                }
                if (R[0] > 1024)
                    R[0] = 0x8080;
                if (R[1] > 1024)
                    R[1] = 0x8080;
                if (R[2] > 1024)
                    R[2] = 0x8080;
            }
            /************ SCF ************/
            else
            {
                switch (d->SCFI_R[n])
                {
                case 3:
                    R[0] = mpc_decoder_bitstream_read(d, 6);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 1:
                    R[0] = mpc_decoder_bitstream_read(d, 6);
                    R[1] = mpc_decoder_bitstream_read(d, 6);
                    R[2] = R[1];
                    break;
                case 2:
                    R[0] = mpc_decoder_bitstream_read(d, 6);
                    R[1] = R[0];
                    R[2] = mpc_decoder_bitstream_read(d, 6);
                    break;
                case 0:
                    R[0] = mpc_decoder_bitstream_read(d, 6);
                    R[1] = mpc_decoder_bitstream_read(d, 6);
                    R[2] = mpc_decoder_bitstream_read(d, 6);
                    break;
                default:
                    return;
                    break;
                }
            }
        }
    }

    if (seeking == TRUE)
        return;

    /**************************** Samples ****************************/
    ResL = d->Res_L;
    ResR = d->Res_R;
    for (n=0; n <= Max_used_Band; ++n, ++ResL, ++ResR)
    {
        // setting pointers
        x1 = mpc_table_SampleHuff[*ResL];
        x2 = mpc_table_SampleHuff[*ResR];
        L = d->Q[n].L;
        R = d->Q[n].R;

        if (x1!=NULL || x2!=NULL)
            for (k=0; k<36; ++k)
            {
                if (x1 != NULL) *L++ = mpc_decoder_huffman_decode(d,  x1, 8);
                if (x2 != NULL) *R++ = mpc_decoder_huffman_decode(d,  x2, 8);
            }

        if (*ResL>7 || *ResR>7)
            for (k=0; k<36; ++k)
            {
                if (*ResL>7) *L++ = (mpc_int32_t)mpc_decoder_bitstream_read(d,  Res_bit[*ResL]) - Dc[*ResL];
                if (*ResR>7) *R++ = (mpc_int32_t)mpc_decoder_bitstream_read(d,  Res_bit[*ResR]) - Dc[*ResR];
            }
    }
}
#endif //MPC_SUPPORT_SV456
/****************************************** SV 7 ******************************************/
void
mpc_decoder_read_bitstream_sv7(mpc_decoder *d, mpc_bool_t seeking)
{
    // these arrays hold decoding results for bundled quantizers (3- and 5-step)
    static const mpc_int32_t idx30[] = { -1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1};
    static const mpc_int32_t idx31[] = { -1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1};
    static const mpc_int32_t idx32[] = { -1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    static const mpc_int32_t idx50[] = { -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
    static const mpc_int32_t idx51[] = { -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2};

    mpc_int32_t n,k;
    mpc_int32_t Max_used_Band=0;
    const HuffmanTyp *Table;
    mpc_int32_t idx;
    mpc_int32_t *L   ,*R;
    mpc_int32_t *ResL,*ResR;
    mpc_uint32_t tmp;

    /***************************** Header *****************************/
    ResL  = d->Res_L;
    ResR  = d->Res_R;

    // first subband
    *ResL = mpc_decoder_bitstream_read(d, 4);
    *ResR = mpc_decoder_bitstream_read(d, 4);
    if (d->MS_used && !(*ResL==0 && *ResR==0)) {
        d->MS_Flag[0] = mpc_decoder_bitstream_read(d, 1);
    }

    // consecutive subbands
    ++ResL; ++ResR; // increase pointers
    for (n=1; n <= d->Max_Band; ++n, ++ResL, ++ResR)
    {
        idx   = mpc_decoder_huffman_decode(d, mpc_table_HuffHdr, 9);
        *ResL = (idx!=4) ? *(ResL-1) + idx : (int) mpc_decoder_bitstream_read(d, 4);

        idx   = mpc_decoder_huffman_decode(d, mpc_table_HuffHdr, 9);
        *ResR = (idx!=4) ? *(ResR-1) + idx : (int) mpc_decoder_bitstream_read(d, 4);

        if (d->MS_used && !(*ResL==0 && *ResR==0)) {
            d->MS_Flag[n] = mpc_decoder_bitstream_read(d, 1);
        }

        // only perform following procedures up to the maximum non-zero subband
        if (*ResL!=0 || *ResR!=0) {
            Max_used_Band = n;
        }
    }
    /****************************** SCFI ******************************/
    L     = d->SCFI_L;
    R     = d->SCFI_R;
    ResL  = d->Res_L;
    ResR  = d->Res_R;
    for (n=0; n <= Max_used_Band; ++n, ++L, ++R, ++ResL, ++ResR) {
        if (*ResL) *L = mpc_decoder_huffman_decode(d, mpc_table_HuffSCFI, 3);
        if (*ResR) *R = mpc_decoder_huffman_decode(d, mpc_table_HuffSCFI, 3);
    }

    /**************************** SCF/DSCF ****************************/
    ResL  = d->Res_L;
    ResR  = d->Res_R;
    L     = d->SCF_Index_L[0];
    R     = d->SCF_Index_R[0];
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR, L+=3, R+=3) {
        if (*ResL)
        {
            switch (d->SCFI_L[n])
            {
            case 1:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[0] = (idx!=8) ? L[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[1] = (idx!=8) ? L[0] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                L[2] = L[1];
                break;
            case 3:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[0] = (idx!=8) ? L[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                L[1] = L[0];
                L[2] = L[1];
                break;
            case 2:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[0] = (idx!=8) ? L[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                L[1] = L[0];
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[2] = (idx!=8) ? L[1] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                break;
            case 0:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[0] = (idx!=8) ? L[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[1] = (idx!=8) ? L[0] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                L[2] = (idx!=8) ? L[1] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                break;
            default:
                return;
            }
            if (L[0] > 1024)
                L[0] = 0x8080;
            if (L[1] > 1024)
                L[1] = 0x8080;
            if (L[2] > 1024)
                L[2] = 0x8080;
        }
        if (*ResR)
        {
            switch (d->SCFI_R[n])
            {
            case 1:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[0] = (idx!=8) ? R[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[1] = (idx!=8) ? R[0] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                R[2] = R[1];
                break;
            case 3:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[0] = (idx!=8) ? R[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                R[1] = R[0];
                R[2] = R[1];
                break;
            case 2:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[0] = (idx!=8) ? R[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                R[1] = R[0];
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[2] = (idx!=8) ? R[1] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                break;
            case 0:
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[0] = (idx!=8) ? R[2] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[1] = (idx!=8) ? R[0] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                idx  = mpc_decoder_huffman_decode(d, mpc_table_HuffDSCF, 6);
                R[2] = (idx!=8) ? R[1] + idx : (int) mpc_decoder_bitstream_read(d, 6);
                break;
            default:
                return;
            }
            if (R[0] > 1024)
                R[0] = 0x8080;
            if (R[1] > 1024)
                R[1] = 0x8080;
            if (R[2] > 1024)
                R[2] = 0x8080;
        }
    }

    if (seeking == TRUE)
        return;

    /***************************** Samples ****************************/
    ResL = d->Res_L;
    ResR = d->Res_R;
    L    = d->Q[0].L;
    R    = d->Q[0].R;
    for (n=0; n <= Max_used_Band; ++n, ++ResL, ++ResR, L+=36, R+=36)
    {
        /************** links **************/
        switch (*ResL)
        {
        case  -2: case  -3: case  -4: case  -5: case  -6: case  -7: case  -8: case  -9:
        case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17:
            L += 36;
            break;
        case -1:
            for (k=0; k<36; k++ ) {
                tmp  = mpc_random_int(d);
                *L++ = ((tmp >> 24) & 0xFF) + ((tmp >> 16) & 0xFF) + ((tmp >>  8) & 0xFF) + ((tmp >>  0) & 0xFF) - 510;
            }
            break;
        case 0:
            L += 36;// increase pointer
            break;
        case 1:
            Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][1];
            for (k=0; k<12; ++k)
            {
                idx = mpc_decoder_huffman_decode(d, Table, 9);
                *L++ = idx30[idx];
                *L++ = idx31[idx];
                *L++ = idx32[idx];
            }
            break;
        case 2:
            Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][2];
            for (k=0; k<18; ++k)
            {
                idx = mpc_decoder_huffman_decode(d, Table, 10);
                *L++ = idx50[idx];
                *L++ = idx51[idx];
            }
            break;
        case 3:
        case 4:
            Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][*ResL];
            for (k=0; k<36; ++k)
                *L++ = mpc_decoder_huffman_decode(d, Table, 5);
            break;
        case 5:
            Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][*ResL];
            for (k=0; k<36; ++k)
                *L++ = mpc_decoder_huffman_decode(d, Table, 8);
            break;
        case 6:
        case 7:
            Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][*ResL];
            for (k=0; k<36; ++k)
                *L++ = mpc_decoder_huffman_decode(d, Table, 14);
            break;
        case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
            tmp = Dc[*ResL];
            for (k=0; k<36; ++k)
                *L++ = (mpc_int32_t)mpc_decoder_bitstream_read(d, Res_bit[*ResL]) - tmp;
            break;
        default:
            return;
        }
        /************** rechts **************/
        switch (*ResR)
        {
        case  -2: case  -3: case  -4: case  -5: case  -6: case  -7: case  -8: case  -9:
        case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17:
            R += 36;
            break;
        case -1:
                for (k=0; k<36; k++ ) {
                    tmp  = mpc_random_int(d);
                    *R++ = ((tmp >> 24) & 0xFF) + ((tmp >> 16) & 0xFF) + ((tmp >>  8) & 0xFF) + ((tmp >>  0) & 0xFF) - 510;
                }
                break;
            case 0:
                R += 36;// increase pointer
                break;
            case 1:
                Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][1];
                for (k=0; k<12; ++k)
                {
                    idx = mpc_decoder_huffman_decode(d, Table, 9);
                    *R++ = idx30[idx];
                    *R++ = idx31[idx];
                    *R++ = idx32[idx];
                }
                break;
            case 2:
                Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][2];
                for (k=0; k<18; ++k)
                {
                    idx = mpc_decoder_huffman_decode(d, Table, 10);
                    *R++ = idx50[idx];
                    *R++ = idx51[idx];
                }
                break;
            case 3:
            case 4:
                Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = mpc_decoder_huffman_decode(d, Table, 5);
                break;
            case 5:
                Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = mpc_decoder_huffman_decode(d, Table, 8);
                break;
            case 6:
            case 7:
                Table = mpc_table_HuffQ[mpc_decoder_bitstream_read(d, 1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = mpc_decoder_huffman_decode(d, Table, 14);
                break;
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
                tmp = Dc[*ResR];
                for (k=0; k<36; ++k)
                    *R++ = (mpc_int32_t)mpc_decoder_bitstream_read(d, Res_bit[*ResR]) - tmp;
                break;
            default:
                return;
        }
    }
}

void mpc_decoder_setup(mpc_decoder *d, mpc_reader *r)
{
  d->r = r;

  d->MPCHeaderPos = 0;
  d->StreamVersion = 0;
  d->MS_used = 0;
  d->FrameWasValid = 0;
  d->OverallFrames = 0;
  d->DecodedFrames = 0;
  d->TrueGaplessPresent = 0;
  d->WordsRead = 0;
  d->Max_Band = 0;
  d->SampleRate = 0;
  d->__r1 = 1;
  d->__r2 = 1;

  d->Max_Band = 0;
  d->seeking_window = FAST_SEEKING_WINDOW;

  mpc_decoder_reset_bitstream_decode(d);
  mpc_decoder_initialisiere_quantisierungstabellen(d, 1.0f);
#if 0
  mpc_decoder_init_huffman_sv6(d);
  mpc_decoder_init_huffman_sv7(d);
#endif
}

static mpc_uint32_t get_initial_fpos(mpc_decoder *d)
{
    mpc_uint32_t fpos = 0;
    switch ( d->StreamVersion ) {   // setting position to the beginning of the data-bitstream
    case  0x04: fpos =  48; break;
    case  0x05:
    case  0x06: fpos =  64; break;
    case  0x07:
    case  0x17: fpos = 200; break;
    }
    return fpos;
}

void mpc_decoder_set_streaminfo(mpc_decoder *d, mpc_streaminfo *si)
{
    mpc_decoder_reset_synthesis(d);
    mpc_decoder_reset_globals(d);

    d->StreamVersion      = si->stream_version;
    d->MS_used            = si->ms;
    d->Max_Band           = si->max_band;
    d->OverallFrames      = si->frames;
    d->MPCHeaderPos       = si->header_position;
    d->TrueGaplessPresent = si->is_true_gapless;
    d->SampleRate         = (mpc_int32_t)si->sample_freq;

    d->samples_to_skip = MPC_DECODER_SYNTH_DELAY;
}

mpc_bool_t mpc_decoder_initialize(mpc_decoder *d, mpc_streaminfo *si) 
{
    mpc_decoder_set_streaminfo(d, si);

    // AB: setting position to the beginning of the data-bitstream
    mpc_decoder_seek(d, get_initial_fpos(d));

    d->seeking_pwr = 0;
    while( d->OverallFrames > ((mpc_int64_t) SEEKING_TABLE_SIZE << d->seeking_pwr) )
        d->seeking_pwr++;
    d->seeking_table_frames = 0;
    d->seeking_table[0] = get_initial_fpos(d);

    return TRUE;
}

void mpc_decoder_set_seeking(mpc_decoder *d, mpc_streaminfo *si, mpc_bool_t fast_seeking)
{
    d->seeking_window = FAST_SEEKING_WINDOW;
    if (si->fast_seek == 0 && fast_seeking == 0)
        d->seeking_window = SLOW_SEEKING_WINDOW;
}

mpc_bool_t mpc_decoder_seek_seconds(mpc_decoder *d, double seconds) 
{
    return mpc_decoder_seek_sample(d, (mpc_int64_t)(seconds * (double)d->SampleRate + 0.5));
}

mpc_bool_t mpc_decoder_seek_sample(mpc_decoder *d, mpc_int64_t destsample) 
{
    mpc_uint32_t fpos;
    mpc_uint32_t fwd;

    fwd = (mpc_uint32_t) (destsample / MPC_FRAME_LENGTH);
    d->samples_to_skip = MPC_DECODER_SYNTH_DELAY + (mpc_uint32_t)(destsample % MPC_FRAME_LENGTH);

    // resetting synthesis filter to avoid "clicks"
    mpc_decoder_reset_synthesis(d);

    // prevent from desired position out of allowed range
    fwd = fwd < d->OverallFrames  ?  fwd  :  d->OverallFrames;

    if (fwd > d->DecodedFrames + d->seeking_window || fwd < d->DecodedFrames) {
        memset(d->SCF_Index_L, 1, sizeof d->SCF_Index_L );
        memset(d->SCF_Index_R, 1, sizeof d->SCF_Index_R );
    }

    if (d->seeking_table_frames > d->DecodedFrames || fwd < d->DecodedFrames) {
        d->DecodedFrames = 0;
        if (fwd > d->seeking_window)
            d->DecodedFrames = (fwd - d->seeking_window) & (-1 << d->seeking_pwr);
        if (d->DecodedFrames > d->seeking_table_frames)
            d->DecodedFrames = d->seeking_table_frames;
        fpos = d->seeking_table[d->DecodedFrames >> d->seeking_pwr];
        mpc_decoder_seek(d, fpos);
    }

    // read the last 32 frames before the desired position to scan the scalefactors (artifactless jumping)
    for ( ; d->DecodedFrames < fwd; d->DecodedFrames++ ) {
        mpc_uint32_t RING = d->Zaehler;
        mpc_uint32_t FwdJumpInfo;

        // add seeking info
        if (d->seeking_table_frames < d->DecodedFrames &&
           (d->DecodedFrames & ((1 << d->seeking_pwr) - 1)) == 0) {
            d->seeking_table[d->DecodedFrames >> d->seeking_pwr] = mpc_decoder_bits_read(d);
            d->seeking_table_frames = d->DecodedFrames;
        }

        // read jump-info
        FwdJumpInfo  = mpc_decoder_bitstream_read(d, 20);
        FwdJumpInfo += mpc_decoder_bits_read(d);

        if (fwd <= d->DecodedFrames + d->seeking_window) {
            if (d->StreamVersion >= 7) {
                mpc_decoder_read_bitstream_sv7(d, TRUE);
            } else {
#ifdef MPC_SUPPORT_SV456
                mpc_decoder_read_bitstream_sv6(d, TRUE);
#else
                return FALSE;
#endif
            }
        }
        mpc_decoder_bitstream_jump(d, FwdJumpInfo - mpc_decoder_bits_read(d));

        // update buffer
        mpc_decoder_update_buffer(d, RING);
    }

    return TRUE;
}
