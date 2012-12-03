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

#ifndef _DEVICE_BUILDER_H
#define _DEVICE_BUILDER_H

#include "Blender.h"
#include "Combiner.h"
#include "Config.h"
#include "GraphicsContext.h"
#include "TextureManager.h"

//========================================================================

class CDeviceBuilder
{
public:
    virtual CGraphicsContext * CreateGraphicsContext(void)=0;
    virtual CRender * CreateRender(void)=0;
    virtual CTexture * CreateTexture(uint32 dwWidth, uint32 dwHeight, TextureUsage usage = AS_NORMAL)=0;
    virtual CColorCombiner * CreateColorCombiner(CRender *pRender)=0;
    virtual CBlender * CreateAlphaBlender(CRender *pRender)=0;

    void DeleteGraphicsContext(void);
    void DeleteRender(void);
    void DeleteColorCombiner(void);
    void DeleteAlphaBlender(void);

    static void DeleteBuilder(void);
    static CDeviceBuilder* GetBuilder(void);
    static void SelectDeviceType(SupportedDeviceType type);
    static SupportedDeviceType GetDeviceType(void);
    static SupportedDeviceType GetGeneralDeviceType(void);
    static SupportedDeviceType m_deviceGeneralType;
protected:
    CDeviceBuilder();
    virtual ~CDeviceBuilder();

    static CDeviceBuilder* CreateBuilder(SupportedDeviceType type);
    static SupportedDeviceType m_deviceType;
    static CDeviceBuilder* m_pInstance;

    CRender* m_pRender;
    CGraphicsContext* m_pGraphicsContext;
    CColorCombiner* m_pColorCombiner;
    CBlender* m_pAlphaBlender;
};

class OGLDeviceBuilder : public CDeviceBuilder
{
    friend class CDeviceBuilder;
public:
    CGraphicsContext * CreateGraphicsContext(void);
    CRender * CreateRender(void);
    CTexture * CreateTexture(uint32 dwWidth, uint32 dwHeight, TextureUsage usage = AS_NORMAL);
    CColorCombiner * CreateColorCombiner(CRender *pRender);
    CBlender * CreateAlphaBlender(CRender *pRender);

protected:
    OGLDeviceBuilder() {};
    virtual  ~OGLDeviceBuilder() {};

};

#endif


