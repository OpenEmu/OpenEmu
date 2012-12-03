/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <SDL_opengl.h>

#include "CNvTNTCombiner.h"

CNvTNTCombiner::CNvTNTCombiner()
{
    m_lastIndexTNT = 0;
}

CNvTNTCombiner::~CNvTNTCombiner()
{
}


int CNvTNTCombiner::FindCompiledMux( )
{
    for( uint32 i=0; i<m_vCompiledTNTSettings.size(); i++ )
    {
        if( m_vCompiledTNTSettings[i].dwMux0 == (*m_ppDecodedMux)->m_dwMux0 && m_vCompiledTNTSettings[i].dwMux1 == (*m_ppDecodedMux)->m_dwMux1 )
        {
            m_lastIndexTNT = i;
            return i;
        }
    }

    return -1;
}

bool isTex(uint32 val);
bool isComb(uint32 val);

int CNvTNTCombiner::ParseDecodedMux()
{
    TNT2CombinerSaveType res;
    res.numOfUnits = 2;

    (*m_ppDecodedMux)->To_AB_Add_CD_Format();

    for( int i=0; i<res.numOfUnits*2; i++ ) // Set combiner for each texture unit
    {
        // For each texture unit, set both RGB and Alpha channel
        // Keep in mind that the m_pDecodeMux has been reformated and simplified very well

        TNT2CombinerType &unit = res.units[i/2];
        TNT2CombType &comb = unit.Combs[i%2];

        CombinerFormatType type = (*m_ppDecodedMux)->splitType[i];
        N64CombinerType &m = (*m_ppDecodedMux)->m_n64Combiners[i];

        comb.arg0 = comb.arg1 = comb.arg2 = comb.arg3 = MUX_0;
        unit.ops[i%2] = 0x0104; //Add;
        //Subtract

        switch( type )
        {
        case CM_FMT_TYPE_NOT_USED:
            comb.arg0 = MUX_COMBINED;
            comb.arg1 = MUX_1;
            comb.arg2 = MUX_0;
            comb.arg3 = MUX_1;
        case CM_FMT_TYPE_D:             // = A
            comb.arg0 = m.d;
            comb.arg1 = MUX_1;
            comb.arg2 = MUX_0;
            comb.arg3 = MUX_0;
            break;
        case CM_FMT_TYPE_A_ADD_D:           // = A+D
            comb.arg0 = m.a;
            comb.arg1 = MUX_1;
            comb.arg2 = m.d;
            comb.arg3 = MUX_1;
            if( isComb(m.d) )
            {
                swap(comb.arg0, comb.arg2);
                swap(comb.arg1, comb.arg3);
            }
            break;
        case CM_FMT_TYPE_A_SUB_B:           // = A-B
            comb.arg0 = m.a^MUX_COMPLEMENT;
            comb.arg1 = MUX_1;
            unit.ops[i%2] = GL_SUBTRACT_ARB;
            comb.arg2 = m.b;
            comb.arg3 = MUX_1;
            break;
        case CM_FMT_TYPE_A_MOD_C:           // = A*C
            comb.arg0 = m.a;
            comb.arg1 = m.c;
            comb.arg2 = MUX_0;
            comb.arg3 = MUX_0;
            break;
        case CM_FMT_TYPE_A_MOD_C_ADD_D: // = A*C+D
            comb.arg0 = m.a;
            comb.arg1 = m.c;
            comb.arg2 = m.d;
            comb.arg3 = MUX_1;
            if( isComb(m.d) )
            {
                swap(comb.arg0, comb.arg2);
                swap(comb.arg1, comb.arg3);
            }
            break;
        case CM_FMT_TYPE_A_LERP_B_C:        // = (A-B)*C+B
            comb.arg0 = m.a;
            comb.arg1 = m.c;
            comb.arg2 = m.c^MUX_COMPLEMENT;
            comb.arg3 = m.b;
            if( isComb(m.b) )
            {
                swap(comb.arg0, comb.arg2);
                swap(comb.arg1, comb.arg3);
            }
            break;
        case CM_FMT_TYPE_A_SUB_B_ADD_D: // = A-B+D
            // fix me, to use 2 texture units
            if( isTex(m.b) && isTex(m.d) )
            {
                comb.arg0 = m.a;
                comb.arg1 = m.b;
                comb.arg2 = m.d;
                comb.arg3 = MUX_1;
                if( isComb(m.d) )
                {
                    swap(comb.arg0, comb.arg2);
                    swap(comb.arg1, comb.arg3);
                }
            }
            else if( isTex(m.b) && !isComb(m.d) )
            {
                comb.arg0 = m.a^MUX_COMPLEMENT;
                comb.arg1 = MUX_1;
                comb.arg2 = m.b;
                comb.arg3 = MUX_1;
                unit.ops[i%2] = GL_SUBTRACT_ARB;
            }
            else if( !isTex(m.b) && isTex(m.d) )
            {
                comb.arg0 = m.a;
                comb.arg1 = MUX_1;
                comb.arg2 = m.d;
                comb.arg3 = MUX_1;
                if( isComb(m.d) )
                {
                    swap(comb.arg0, comb.arg2);
                    swap(comb.arg1, comb.arg3);
                }
            }
            else
            {
                comb.arg0 = m.a;
                comb.arg1 = m.b;
                comb.arg2 = m.d;
                comb.arg3 = MUX_1;
                if( isComb(m.d) )
                {
                    swap(comb.arg0, comb.arg2);
                    swap(comb.arg1, comb.arg3);
                }
            }
            break;
        case CM_FMT_TYPE_A_SUB_B_MOD_C: // = (A-B)*C
            comb.arg0 = m.a^MUX_COMPLEMENT;
            comb.arg1 = m.c;
            comb.arg2 = m.c;
            comb.arg3 = m.b;
            unit.ops[i%2] = GL_SUBTRACT_ARB;
            break;
        case CM_FMT_TYPE_AB_ADD_CD:         // = AB+CD
            comb.arg0 = m.a;
            comb.arg1 = m.b;
            comb.arg2 = m.c;
            comb.arg3 = m.d;
            if( isComb(m.d) || isComb(m.c) )
            {
                swap(comb.arg0, comb.arg2);
                swap(comb.arg1, comb.arg3);
            }

            break;
        case CM_FMT_TYPE_AB_SUB_CD:         // = AB-CD
            comb.arg0 = m.a^MUX_COMPLEMENT;
            comb.arg1 = m.b;
            unit.ops[i%2] = GL_SUBTRACT_ARB;
            comb.arg2 = m.c;
            comb.arg3 = m.d;
            break;
        case CM_FMT_TYPE_A_B_C_D:           // = (A-B)*C+D
        default:
            if( !isComb(m.d) && !isTex(m.d) )
            {
                comb.arg0 = m.a^MUX_COMPLEMENT;
                comb.arg1 = m.c;
                unit.ops[i%2] = GL_SUBTRACT_ARB;
                comb.arg2 = m.c;
                comb.arg3 = m.b;
            }
            else if( !isComb(m.b) && !isTex(m.b) )
            {
                comb.arg0 = m.a;
                comb.arg1 = m.c;
                comb.arg2 = m.d;
                comb.arg3 = MUX_1;
                if( isComb(m.d) )
                {
                    swap(comb.arg0, comb.arg2);
                    swap(comb.arg1, comb.arg3);
                }
            }
            else if( !isComb(m.c) && !isTex(m.c) )
            {
                comb.arg0 = m.a;
                comb.arg1 = m.b;
                comb.arg2 = m.d;
                comb.arg3 = MUX_1;
                if( isComb(m.d) )
                {
                    swap(comb.arg0, comb.arg2);
                    swap(comb.arg1, comb.arg3);
                }
            }
            else
            {
                comb.arg0 = m.a;
                comb.arg1 = m.c;
                comb.arg2 = m.d;
                comb.arg3 = MUX_1;
                if( isComb(m.d) )
                {
                    swap(comb.arg0, comb.arg2);
                    swap(comb.arg1, comb.arg3);
                }
            }
            break;
        }
    }

    ParseDecodedMuxForConstants(res);
    return SaveParserResult(res);
}

int CNvTNTCombiner::SaveParserResult(TNT2CombinerSaveType &result)
{
    result.dwMux0 = (*m_ppDecodedMux)->m_dwMux0;
    result.dwMux1 = (*m_ppDecodedMux)->m_dwMux1;

    m_vCompiledTNTSettings.push_back(result);
    m_lastIndexTNT = m_vCompiledTNTSettings.size()-1;

#ifdef DEBUGGER
    if( logCombiners )
    {
        DisplaySimpleMuxString();
    }
#endif

    return m_lastIndexTNT;
}

void CNvTNTCombiner::ParseDecodedMuxForConstants(TNT2CombinerSaveType &res)
{
    res.unit1.constant = MUX_0;
    res.unit2.constant = MUX_0;

    for( int i=0; i<2; i++ )
    {
        if( (*m_ppDecodedMux)->isUsedInCycle(MUX_PRIM, i,COLOR_CHANNEL) || (*m_ppDecodedMux)->isUsedInCycle(MUX_PRIM, i,ALPHA_CHANNEL) )
        {
            res.units[i].constant = MUX_PRIM;
        }
        else if( (*m_ppDecodedMux)->isUsedInCycle(MUX_ENV, i,COLOR_CHANNEL) || (*m_ppDecodedMux)->isUsedInCycle(MUX_ENV, i,ALPHA_CHANNEL) )
        {
            res.units[i].constant = MUX_ENV;
        }
        else if( (*m_ppDecodedMux)->isUsedInCycle(MUX_LODFRAC, i,COLOR_CHANNEL) || (*m_ppDecodedMux)->isUsedInCycle(MUX_LODFRAC, i,ALPHA_CHANNEL) )
        {
            res.units[i].constant = MUX_LODFRAC;
        }
        else if( (*m_ppDecodedMux)->isUsedInCycle(MUX_PRIMLODFRAC, i,COLOR_CHANNEL) || (*m_ppDecodedMux)->isUsedInCycle(MUX_PRIMLODFRAC, i,ALPHA_CHANNEL) )
        {
            res.units[i].constant = MUX_PRIMLODFRAC;
        }
    }
}

#ifdef DEBUGGER
extern const char *translatedCombTypes[];
void CNvTNTCombiner::DisplaySimpleMuxString()
{
    char buf0[30];
    char buf1[30];
    char buf2[30];
    char buf3[30];

    TNT2CombinerSaveType &result = m_vCompiledTNTSettings[m_lastIndexTNT];

    TRACE0("\nNVidia TNT2+ Combiner\n");        
    DebuggerAppendMsg("//aRGB0:\t(%s * %s) %s (%s * %s)\n", DecodedMux::FormatStr(result.unit1.rgbArg0,buf0), DecodedMux::FormatStr(result.unit1.rgbArg1,buf1), result.unit1.rgbOp==0x0104?"+":"-", DecodedMux::FormatStr(result.unit1.rgbArg2,buf2), DecodedMux::FormatStr(result.unit1.rgbArg3,buf3));        
    DebuggerAppendMsg("//aRGB1:\t(%s * %s) %s (%s * %s)\n", DecodedMux::FormatStr(result.unit2.rgbArg0,buf0), DecodedMux::FormatStr(result.unit2.rgbArg1,buf1), result.unit2.rgbOp==0x0104?"+":"-", DecodedMux::FormatStr(result.unit2.rgbArg2,buf2), DecodedMux::FormatStr(result.unit2.rgbArg3,buf3));        
    DebuggerAppendMsg("//aAlpha0:\t(%s * %s) %s (%s * %s)\n", DecodedMux::FormatStr(result.unit1.alphaArg0,buf0), DecodedMux::FormatStr(result.unit1.alphaArg1,buf1), result.unit1.alphaOp==0x0104?"+":"-", DecodedMux::FormatStr(result.unit1.alphaArg2,buf2), DecodedMux::FormatStr(result.unit1.alphaArg3,buf3));        
    DebuggerAppendMsg("//aAlpha1:\t(%s * %s) %s (%s * %s)\n", DecodedMux::FormatStr(result.unit2.alphaArg0,buf0), DecodedMux::FormatStr(result.unit2.alphaArg1,buf1), result.unit2.alphaOp==0x0104?"+":"-", DecodedMux::FormatStr(result.unit2.alphaArg2,buf2), DecodedMux::FormatStr(result.unit2.alphaArg3,buf3));
    if( result.unit1.constant != MUX_0 )
        DebuggerAppendMsg("//Constant for unit 1:\t%s\n", DecodedMux::FormatStr(result.unit1.constant,buf0));
    if( result.unit2.constant != MUX_0 )
        DebuggerAppendMsg("//Constant for unit 2:\t%s\n", DecodedMux::FormatStr(result.unit2.constant,buf0));
    TRACE0("\n\n");
}
#endif

