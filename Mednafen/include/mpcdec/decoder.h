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

/// \file decoder.h

#ifndef _mpcdec_decoder_h_
#define _mpcdec_decoder_h_

#include "huffman.h"
#include "math.h"
#include "mpcdec.h"
#include "reader.h"
#include "streaminfo.h"

#define MPC_SUPPORT_SV456

#define SEEKING_TABLE_SIZE  256u
// set it to SLOW_SEEKING_WINDOW to not use fast seeking
#define FAST_SEEKING_WINDOW 32
// set it to FAST_SEEKING_WINDOW to only use fast seeking
#define SLOW_SEEKING_WINDOW 0x80000000

enum {
    MPC_V_MEM = 2304,
    MPC_DECODER_MEMSIZE = 16384,  // overall buffer size
};

typedef struct {
    mpc_int32_t  L [36];
    mpc_int32_t  R [36];
} QuantTyp;

typedef struct mpc_decoder_t {
    mpc_reader *r;

    /// @name internal state variables
    //@{

    mpc_uint32_t  dword; /// currently decoded 32bit-word
    mpc_uint32_t  pos;   /// bit-position within dword
    mpc_uint32_t  Speicher[MPC_DECODER_MEMSIZE]; /// read-buffer
    mpc_uint32_t  Zaehler; /// actual index within read-buffer

    mpc_uint32_t  samples_to_skip;

    mpc_uint32_t  DecodedFrames;
    mpc_uint32_t  OverallFrames;
    mpc_int32_t   SampleRate;                 // Sample frequency

    mpc_uint32_t  StreamVersion;              // version of bitstream
    mpc_int32_t   Max_Band;
    mpc_uint32_t  MPCHeaderPos;               // AB: needed to support ID3v2

    mpc_uint32_t  FrameWasValid;
    mpc_uint32_t  MS_used;                    // MS-coding used ?
    mpc_uint32_t  TrueGaplessPresent;

    mpc_uint32_t  WordsRead;                  // counts amount of decoded dwords

    // randomizer state variables
    mpc_uint32_t  __r1; 
    mpc_uint32_t  __r2; 

    // seeking
    mpc_uint32_t  seeking_table[SEEKING_TABLE_SIZE];
    mpc_uint32_t  seeking_pwr;                // distance between 2 frames in seeking_table = 2^seeking_pwr
    mpc_uint32_t  seeking_table_frames;       // last frame in seaking table
    mpc_uint32_t  seeking_window;             // number of frames to look for scalefactors

    mpc_int32_t   SCF_Index_L [32] [3];
    mpc_int32_t   SCF_Index_R [32] [3];       // holds scalefactor-indices
    QuantTyp      Q [32];                     // holds quantized samples
    mpc_int32_t   Res_L [32];
    mpc_int32_t   Res_R [32];                 // holds the chosen quantizer for each subband
    mpc_bool_t    DSCF_Flag_L [32];
    mpc_bool_t    DSCF_Flag_R [32];           // differential SCF used?
    mpc_int32_t   SCFI_L [32];
    mpc_int32_t   SCFI_R [32];                // describes order of transmitted SCF
    mpc_bool_t    MS_Flag[32];                // MS used?
#ifdef MPC_FIXED_POINT
    unsigned char SCF_shift[256];
#endif

    MPC_SAMPLE_FORMAT V_L[MPC_V_MEM + 960];
    MPC_SAMPLE_FORMAT V_R[MPC_V_MEM + 960];
    MPC_SAMPLE_FORMAT Y_L[36][32];
    MPC_SAMPLE_FORMAT Y_R[36][32];
    MPC_SAMPLE_FORMAT SCF[256]; ///< holds adapted scalefactors (for clipping prevention)
    //@}

} mpc_decoder;

#endif // _mpc_decoder_h
