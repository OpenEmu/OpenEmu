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

#include "Combiner.h"
#include "DirectXDecodedMux.h"
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

//This function is called after Reformat to handel two texels in 1 cycle, D3D can not handle
//two texels in a single stage, the texels must be splited into multiple stages
void CDirectXDecodedMux::ReformatAgainWithTwoTexels(void)
{
    if( CountTexels() < 2 )
        return;

    for( int i=0; i<2; i++ )
    {
        N64CombinerType &m = m_n64Combiners[i];
        if( CountTexel1Cycle(m) < 2 )
        {
            continue;   //1st cycle does not have two texels, do nothing here
        }
        else
        {
            N64CombinerType &m2 = m_n64Combiners[i+2];
            
            if( splitType[i] == CM_FMT_TYPE_A_MOD_C )   //Texel0*Texel1
            {
                if( splitType[i+2] == CM_FMT_TYPE_NOT_USED )
                {
                    //Change Texel1*Texel0 to (SEL(tex1), MOD(tex0))
                    m.d = m.a;
                    m.a = MUX_0;
                    m2.a = m.c;
                    m2.c = MUX_COMBINED;
                    m2.d = m2.b = MUX_0;
                    m.c = MUX_0;
                    splitType[i+2] = CM_FMT_TYPE_A_MOD_C;
                    splitType[i] = CM_FMT_TYPE_D;
                }
                else if( splitType[i+2] == CM_FMT_TYPE_A_MOD_C )
                {
                    if( m2.a == MUX_COMBINED )
                    {
                        swap(m2.a, m2.c);
                    }

                    if( m2.a != MUX_TEXEL0 && m2.a != MUX_TEXEL1 )
                    {
                        //cool, we can swap m2.a to cycle1 and swap texel from cycle 1 to cycle 2
                        swap(m.a, m2.a);
                    }
                    else
                    {
                        if( m.a == m2.a )
                        {
                            swap(m.c, m2.a);
                        }
                        else
                        {
                            swap(m.a, m2.a);
                        }
                    }
                }
                else if( splitType[i+2] == CM_FMT_TYPE_A_MOD_C_ADD_D )
                {
                    if( m2.a == MUX_COMBINED )
                    {
                        swap(m2.a, m2.c);
                    }

                    if( m2.c == MUX_COMBINED && m2.d != MUX_COMBINED )
                    {
                        //Cycle1: texel0*texel1
                        //Cycle2: a*cmd+d
                        if( m2.a != MUX_TEXEL0 && m2.a != MUX_TEXEL1 )
                        {
                            //cool, we can swap m2.a to cycle1 and swap texel from cycle 1 to cycle 2
                            swap(m.a, m2.a);
                        }
                        else
                        {
                            if( m.a == m2.a )
                            {
                                swap(m.c, m2.a);
                            }
                            else
                            {
                                swap(m.a, m2.a);
                            }
                        }
                    }
                }
                else if( splitType[i] == CM_FMT_TYPE_A_ADD_D )  //Texel0+Texel1
                {
                    if( splitType[i+2] == CM_FMT_TYPE_NOT_USED )
                    {
                        //Change Texel1*Texel0 to (SEL(tex1), MOD(tex0))
                        m2.a = m.d;
                        m2.d = MUX_COMBINED;
                        m2.b = m2.c = MUX_0;
                        m.d = m.a;
                        m.a = MUX_0;
                        splitType[i+2] = CM_FMT_TYPE_A_ADD_D;
                        splitType[i] = CM_FMT_TYPE_D;
                    }
                    else if( splitType[i+2] == CM_FMT_TYPE_A_ADD_D )
                    {
                        if( m2.a == MUX_COMBINED )
                        {
                            swap(m2.a, m2.d);
                        }

                        if( m2.a != MUX_TEXEL0 && m2.a != MUX_TEXEL1 )
                        {
                            swap(m2.a, m.a);
                        }
                        else
                        {
                            if( m.a == m2.a )
                            {
                                swap(m.d, m2.a);
                            }
                            else
                            {
                                swap(m.a, m2.a);
                            }
                        }
                    }
                }

            }

            if( CountTexel1Cycle(m2) < 2 )
            {
                continue;   //2nd cycle does not have two texels
            }
        }


    }
}

void CDirectXDecodedMux::Reformat(bool do_complement)
{
    DecodedMux::Reformat(do_complement);
    ReformatAgainWithTwoTexels();
    mType = std::max(std::max(std::max(splitType[0], splitType[1]),splitType[2]),splitType[3]);
}


