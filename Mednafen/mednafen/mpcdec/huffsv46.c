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

/// \file huffsv46.c
/// Implementations of huffman decoding for streamversions < 7.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mpcdec/mpcdec.h>
#include <mpcdec/requant.h>
#include <mpcdec/huffman.h>

#ifdef MPC_SUPPORT_SV456


const HuffmanTyp    mpc_table_SCFI_Bundle   [ 8] = 
{{2147483648u,1,7},{1073741824u,2,3},{939524096u,5,1},{805306368u,5,2},{738197504u,6,0},{671088640u,6,6},{536870912u,5,4},{0u,3,5},};
const HuffmanTyp    mpc_table_DSCF_Entropie [13] = 
{{3758096384u,3,1},{3489660928u,4,3},{3355443200u,5,5},{3221225472u,5,-3},{2952790016u,4,-2},{2684354560u,4,4},{2147483648u,3,-1},{1610612736u,3,2},{1476395008u,5,-5},{1409286144u,6,6},{1342177280u,6,-6},{1073741824u,4,-4},{0u,2,0},};
const HuffmanTyp    mpc_table_Region_A [16] =
{{2147483648u,1,1},{2013265920u,5,3},{1946157056u,6,4},{1912602624u,7,7},{1895825408u,8,8},{1887436800u,9,9},{1883242496u,10,10},{1881145344u,11,11},{1880096768u,12,12},{1879572480u,13,13},{1879310336u,14,14},{1879048192u,14,15},{1744830464u,5,5},{1610612736u,5,6},{1073741824u,3,0},{0u,2,2},};
const HuffmanTyp    mpc_table_Region_B [ 8] = 
{{2147483648u,1,1},{1073741824u,2,0},{536870912u,3,2},{268435456u,4,3},{134217728u,5,4},{67108864u,6,5},{33554432u,7,6},{0u,7,7},};
const HuffmanTyp    mpc_table_Region_C [ 4] = 
{{2147483648u,1,0},{1073741824u,2,1},{536870912u,3,2},{0u,3,3},};

static const HuffmanTyp    mpc_table_Entropie_1 [ 3] =
{{2147483648u,1,0},{1073741824u,2,-1},{0u,2,1},};
static const HuffmanTyp    mpc_table_Entropie_2 [ 5] = 
{{3221225472u,2,0},{2684354560u,3,2},{2147483648u,3,-2},{1073741824u,2,1},{0u,2,-1},};
static const HuffmanTyp    mpc_table_Entropie_3 [ 7] =
{{3221225472u,2,0},{2684354560u,3,-2},{2415919104u,4,2},{2281701376u,5,-3},{2147483648u,5,3},{1073741824u,2,-1},{0u,2,1},};
static const HuffmanTyp    mpc_table_Entropie_4 [ 9] = 
{{4026531840u,4,3},{3758096384u,4,-3},{3221225472u,3,1},{2684354560u,3,-1},{2147483648u,3,2},{1610612736u,3,-2},{1342177280u,4,-4},{1073741824u,4,4},{0u,2,0},};
static const HuffmanTyp    mpc_table_Entropie_5 [15] =
{{4026531840u,4,-2},{3892314112u,5,-5},{3825205248u,6,-7},{3758096384u,6,7},{3489660928u,4,-3},{3221225472u,4,3},{3087007744u,5,-6},{2952790016u,5,6},{2684354560u,4,4},{2147483648u,3,0},{1610612736u,3,1},{1073741824u,3,-1},{805306368u,4,-4},{536870912u,4,5},{0u,3,2},};
static const HuffmanTyp    mpc_table_Entropie_6 [31] =
{{4160749568u,5,-4},{4026531840u,5,5},{3892314112u,5,-5},{3825205248u,6,10},{3758096384u,6,-10},{3623878656u,5,-6},{3489660928u,5,6},{3355443200u,5,7},{3221225472u,5,-7},{3087007744u,5,-8},{3019898880u,6,-11},{2986344448u,7,14},{2952790016u,7,-14},{2818572288u,5,8},{2751463424u,6,11},{2684354560u,6,-13},{2415919104u,4,0},{2147483648u,4,1},{1879048192u,4,-1},{1610612736u,4,3},{1342177280u,4,2},{1207959552u,5,-9},{1140850688u,6,12},{1073741824u,6,13},{805306368u,4,-3},{536870912u,4,-2},{402653184u,5,9},{335544320u,6,-12},{301989888u,7,15},{268435456u,7,-15},{0u,4,4},};
static const HuffmanTyp    mpc_table_Entropie_7 [63] =
{{4278190080u,8,28},{4261412864u,8,26},{4227858432u,7,-20},{4160749568u,6,8},{4093640704u,6,-8},{4026531840u,6,-9},{3959422976u,6,9},{3925868544u,7,20},{3892314112u,7,21},{3825205248u,6,-10},{3758096384u,6,-11},{3690987520u,6,10},{3623878656u,6,11},{3590324224u,7,-21},{3573547008u,8,29},{3556769792u,8,-29},{3489660928u,6,13},{3422552064u,6,-13},{3355443200u,6,-12},{3288334336u,6,12},{3254779904u,7,-22},{3221225472u,7,22},{3154116608u,6,14},{3087007744u,6,15},{3019898880u,6,-14},{2986344448u,7,-23},{2952790016u,7,23},{2885681152u,6,-15},{2818572288u,6,-16},{2751463424u,6,16},{2717908992u,7,27},{2684354560u,7,-27},{2617245696u,6,17},{2550136832u,6,-17},{2533359616u,8,-30},{2516582400u,8,30},{2483027968u,7,24},{2415919104u,6,-18},{2281701376u,5,-1},{2147483648u,5,1},{2113929216u,7,-24},{2080374784u,7,25},{2013265920u,6,18},{1879048192u,5,-3},{1744830464u,5,3},{1610612736u,5,5},{1476395008u,5,0},{1342177280u,5,-2},{1275068416u,6,19},{1207959552u,6,-19},{1073741824u,5,-5},{939524096u,5,-4},{805306368u,5,-7},{671088640u,5,2},{536870912u,5,4},{402653184u,5,7},{369098752u,7,-25},{335544320u,7,-26},{301989888u,7,-28},{285212672u,8,-31},{268435456u,8,31},{134217728u,5,6},{0u,5,-6},};

const HuffmanTyp*   mpc_table_SampleHuff [18] = {
	NULL,mpc_table_Entropie_1,mpc_table_Entropie_2,mpc_table_Entropie_3,mpc_table_Entropie_4,mpc_table_Entropie_5,mpc_table_Entropie_6,mpc_table_Entropie_7,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};

#endif //#ifdef MPC_SUPPORT_SV456
