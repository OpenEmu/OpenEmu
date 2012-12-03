/*
Copyright (C) 2002 Rice1964

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

#ifndef _COMBINER_H_
#define _COMBINER_H_

#include "typedefs.h"
#include "CombinerDefs.h"
#include "CSortedList.h"
#include "DecodedMux.h"

class CRender;

extern const char* cycleTypeStrs[];

class CColorCombiner
{
    friend class CRender;
public:
    virtual ~CColorCombiner() {};
    COLOR GetConstFactor(uint32 colorFlag, uint32 alphaFlag, uint32 defaultColor = 0);
    virtual void InitCombinerMode(void);

    virtual bool Initialize(void)=0;
    virtual void CleanUp(void) {};
    virtual void UpdateCombiner(uint32 dwMux0, uint32 dwMux1);
    virtual void InitCombinerBlenderForSimpleTextureDraw(uint32 tile=0)=0;
    virtual void DisableCombiner(void)=0;

#ifdef DEBUGGER
    virtual void DisplaySimpleMuxString(void);
    virtual void DisplayMuxString(void);
#endif

    DecodedMux *m_pDecodedMux;
protected:
    CColorCombiner(CRender *pRender) : 
        m_pDecodedMux(NULL), m_bTex0Enabled(false),m_bTex1Enabled(false),m_bTexelsEnable(false),
        m_bCycleChanged(false), m_supportedStages(1),m_bSupportMultiTexture(true),m_pRender(pRender)
    {
    }

    virtual void InitCombinerCycleCopy(void)=0;
    virtual void InitCombinerCycleFill(void)=0;
    virtual void InitCombinerCycle12(void)=0;

    bool    m_bTex0Enabled;
    bool    m_bTex1Enabled;
    bool    m_bTexelsEnable;

    bool    m_bCycleChanged;    // A flag will be set if cycle is changed to FILL or COPY

    int     m_supportedStages;
    bool    m_bSupportMultiTexture;

    CRender *m_pRender;

    CSortedList<uint64, DecodedMux> m_DecodedMuxList;
};

uint32 GetTexelNumber(N64CombinerType &m);
int CountTexel1Cycle(N64CombinerType &m);
bool IsTxtrUsed(N64CombinerType &m);

void swap(uint8 &a, uint8 &b);


inline bool isEqual(uint8 val1, uint8 val2)
{
    if( (val1&MUX_MASK) == (val2&MUX_MASK) )
        return true;
    else
        return false;
}

inline bool isTexel(uint8 val)
{
    if( (val&MUX_MASK) == MUX_TEXEL0 || (val&MUX_MASK) == MUX_TEXEL1 )
        return true;
    else
        return false;
}


extern const char * sc_colcombtypes32[];
extern const char *sc_colcombtypes16[];
extern const char *sc_colcombtypes8[];

COLOR CalculateConstFactor(uint32 colorOp, uint32 alphaOp, uint32 curCol=0);

#endif

