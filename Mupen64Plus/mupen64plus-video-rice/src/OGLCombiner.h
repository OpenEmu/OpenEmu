/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - OGLCombiner.h                                           *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Rice1964                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _OGL_COMBINER_H_
#define _OGL_COMBINER_H_

#include "Blender.h"
#include "Combiner.h"

class OGLRender;

class COGLColorCombiner : public CColorCombiner
{
public:
    bool Initialize(void);
    void InitCombinerBlenderForSimpleTextureDraw(uint32 tile=0);
protected:
    friend class OGLDeviceBuilder;

    void DisableCombiner(void);
    void InitCombinerCycleCopy(void);
    void InitCombinerCycleFill(void);
    void InitCombinerCycle12(void);

    COGLColorCombiner(CRender *pRender);
    ~COGLColorCombiner();
    OGLRender *m_pOGLRender;
    
    bool    m_bSupportAdd;
    bool    m_bSupportSubtract;

#ifdef DEBUGGER
    void DisplaySimpleMuxString(void);
#endif

};

class COGLBlender : public CBlender
{
public:
    void NormalAlphaBlender(void);
    void DisableAlphaBlender(void);
    void BlendFunc(uint32 srcFunc, uint32 desFunc);
    void Enable();
    void Disable();

protected:
    friend class OGLDeviceBuilder;
    COGLBlender(CRender *pRender) : CBlender(pRender), m_pOGLRender((OGLRender*)pRender) {};
    ~COGLBlender() {};

    OGLRender *m_pOGLRender;
};


#endif



