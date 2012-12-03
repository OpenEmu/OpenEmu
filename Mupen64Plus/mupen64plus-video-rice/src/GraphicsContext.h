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

#ifndef GFXCONTEXT_H
#define GFXCONTEXT_H

#include "typedefs.h"
#include "CritSect.h"

enum ClearFlag
{
    CLEAR_COLOR_BUFFER=0x01,
    CLEAR_DEPTH_BUFFER=0x02,
    CLEAR_COLOR_AND_DEPTH_BUFFER=0x03,
};


typedef struct
{
    uint32  addr;   //N64 RDRAM address
    uint32  size;   //N64 buffer size
    uint32  format; //N64 format
    uint32  width;
    uint32  height;
} TextureBufferShortInfo;


// This class basically provides an extra level of security for our
// multithreaded code. Threads can Grab the CGraphicsContext to prevent
// other threads from changing/releasing any of the pointers while it is
// running.

// It is based on CCritSect for Lock() and Unlock()

class CGraphicsContext : public CCritSect
{
    friend class CDeviceBuilder;
    
public:
    bool Ready() { return m_bReady; }
    bool IsWindowed() {return m_bWindowed;}

    virtual bool Initialize(uint32 dwWidth, uint32 dwHeight, BOOL bWindowed );
    virtual void CleanUp();

    virtual void Clear(ClearFlag flags, uint32 color=0xFF000000, float depth=1.0f) = 0;
    virtual void UpdateFrame(bool swaponly=false) = 0;
    virtual int ToggleFullscreen()=0;       // return 0 as the result is windowed

    static void InitWindowInfo();
    static void InitDeviceParameters();

    bool m_supportTextureMirror;

public:
    static  int          m_maxFSAA;
    static  int          m_maxAnisotropy;

protected:
    static  uint32      m_dwWindowStyle;     // Saved window style for mode switches
    static  uint32      m_dwWindowExStyle;   // Saved window style for mode switches
    static  uint32      m_dwStatusWindowStyle;     // Saved window style for mode switches

    static  bool        m_deviceCapsIsInitialized;

    bool                m_bReady;
    bool                m_bActive;
    
    bool                m_bWindowed;
    RECT                m_rcWindowBounds;

    char                m_strDeviceStats[256];

    virtual ~CGraphicsContext();
    CGraphicsContext();
    
public:
    static CGraphicsContext *g_pGraphicsContext;
    static CGraphicsContext * Get(void);
    inline const char* GetDeviceStr() {return m_strDeviceStats;}
    static bool needCleanScene;
};

#endif

