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

#ifndef _NVIDIA_TNT_COMBINER_H_
#define _NVIDIA_TNT_COMBINER_H_

#include <vector>

#include "Combiner.h"

typedef struct
{
    uint8   arg0;
    uint8   arg1;
    uint8   arg2;
    uint8   arg3;
} TNT2CombType;

typedef struct {
    union {
        struct {
            unsigned int    rgbOp;
            unsigned int    alphaOp;
        };
        unsigned int ops[2];
    };

    union {
        struct {
            uint8   rgbArg0;
            uint8   rgbArg1;
            uint8   rgbArg2;
            uint8   rgbArg3;
            uint8   alphaArg0;
            uint8   alphaArg1;
            uint8   alphaArg2;
            uint8   alphaArg3;
        };
        TNT2CombType Combs[2];
        uint8 args[2][4];
    };

    int constant;
} TNT2CombinerType;

typedef struct {
    uint32  dwMux0;
    uint32  dwMux1;
    union {
        struct {
            TNT2CombinerType    unit1;
            TNT2CombinerType    unit2;
        };
        TNT2CombinerType units[2];
    };
    int     numOfUnits;
} TNT2CombinerSaveType;

class CNvTNTCombiner
{
protected:
    CNvTNTCombiner();
        virtual ~CNvTNTCombiner();

    int FindCompiledMux();
    int ParseDecodedMux();              // Compile the decodedMux into NV register combiner setting
    virtual void ParseDecodedMuxForConstants(TNT2CombinerSaveType &res);
    int SaveParserResult(TNT2CombinerSaveType &result);
    
#ifdef DEBUGGER
    void DisplaySimpleMuxString();
#endif
    std::vector<TNT2CombinerSaveType>   m_vCompiledTNTSettings;
    int m_lastIndexTNT;
    DecodedMux **m_ppDecodedMux;
};

#endif

