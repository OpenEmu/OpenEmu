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

#include "OGLDecodedMux.h"

//========================================================================
void COGLDecodedMux::Simplify(void)
{
    DecodedMux::Simplify();

}

void COGLDecodedMux::Reformat(void)
{
    DecodedMux::Reformat();
    mType = max(max(max(splitType[0], splitType[1]),splitType[2]),splitType[3]);
}

void COGLExtDecodedMux::Simplify(void)
//========================================================================
{
    COGLDecodedMux::Simplify();
    FurtherFormatForOGL2();
    Reformat();     // Reformat again
}

void COGLExtDecodedMux::FurtherFormatForOGL2(void)
{
    // This function is used by OGL 1.2, no need to call this function
    // for Nvidia register combiner
    
    // And OGL 1.2 extension only supports 1 constant color, we can not use both PRIM and ENV
    // constant color, and we can not use SPECULAR color as the 2nd color.

    // To futher format the mux.
    // - For each stage, allow only 1 texel, change the 2nd texel in the same stage to MUX_SHADE
    // - Only allow 1 constant color. Count PRIM and ENV, left the most used one, and change
    //   the 2nd one to MUX_SHADE

    if( Count(MUX_PRIM) >= Count(MUX_ENV) )
    {
        ReplaceVal(MUX_ENV, MUX_PRIM);
        //ReplaceVal(MUX_ENV, MUX_SHADE);
        //ReplaceVal(MUX_ENV, MUX_1);
        //ReplaceVal(MUX_PRIM, MUX_0);
    }
    else
    {
        //ReplaceVal(MUX_PRIM, MUX_ENV);
        //ReplaceVal(MUX_PRIM, MUX_SHADE);
        ReplaceVal(MUX_PRIM, MUX_0);
    }

    /*
    // Because OGL 1.2, we may use more than 1 texture unit, but for each texture unit,
    // we can not use multitexture to do color combiner. Each combiner stage can only
    // use 1 texture.

    if( isUsed(MUX_TEXEL0) && isUsed(MUX_TEXEL1) )
    {
        if( Count(MUX_TEXEL0,0)+Count(MUX_TEXEL0,1) >= Count(MUX_TEXEL1,0)+Count(MUX_TEXEL1,1) )
        {
            ReplaceVal(MUX_TEXEL1, MUX_TEXEL0, 0);
            ReplaceVal(MUX_TEXEL1, MUX_TEXEL0, 1);
        }
        else
        {
            ReplaceVal(MUX_TEXEL0, MUX_TEXEL1, 0);
            ReplaceVal(MUX_TEXEL0, MUX_TEXEL1, 1);
        }

        if( Count(MUX_TEXEL0,2)+Count(MUX_TEXEL0,3) >= Count(MUX_TEXEL1,2)+Count(MUX_TEXEL1,3) )
        {
            ReplaceVal(MUX_TEXEL1, MUX_TEXEL0, 2);
            ReplaceVal(MUX_TEXEL1, MUX_TEXEL0, 3);
        }
        else
        {
            ReplaceVal(MUX_TEXEL0, MUX_TEXEL1, 2);
            ReplaceVal(MUX_TEXEL0, MUX_TEXEL1, 3);
        }
    }
    */
}


void COGLExtDecodedMuxTNT2::FurtherFormatForOGL2(void)
{
    if( Count(MUX_PRIM) >= Count(MUX_ENV) )
    {
        //ReplaceVal(MUX_ENV, MUX_PRIM);
        //ReplaceVal(MUX_ENV, MUX_SHADE);
        ReplaceVal(MUX_ENV, MUX_1);
        //ReplaceVal(MUX_PRIM, MUX_0);
    }
    else
    {
        //ReplaceVal(MUX_PRIM, MUX_ENV);
        //ReplaceVal(MUX_PRIM, MUX_SHADE);
        ReplaceVal(MUX_PRIM, MUX_0);
    }
}

