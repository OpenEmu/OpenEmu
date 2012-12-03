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

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_plugin.h"
#include "m64p_vidext.h"

#include "FrameBuffer.h"
#include "OGLGraphicsContext.h"
#include "Video.h"

CGraphicsContext* CGraphicsContext::g_pGraphicsContext = NULL;
bool CGraphicsContext::m_deviceCapsIsInitialized = false;
bool CGraphicsContext::needCleanScene = false;
int CGraphicsContext::m_maxFSAA = 16;
int CGraphicsContext::m_maxAnisotropy = 16;

CGraphicsContext * CGraphicsContext::Get(void)
{   
    return CGraphicsContext::g_pGraphicsContext;
}
    
CGraphicsContext::CGraphicsContext() :
    m_supportTextureMirror(false),
    m_bReady(false), 
        m_bActive(false),
        m_bWindowed(true)
{
}
CGraphicsContext::~CGraphicsContext()
{
    g_pFrameBufferManager->CloseUp();
}

uint32      CGraphicsContext::m_dwWindowStyle=0;     // Saved window style for mode switches
uint32      CGraphicsContext::m_dwWindowExStyle=0;   // Saved window style for mode switches
uint32      CGraphicsContext::m_dwStatusWindowStyle=0;     // Saved window style for mode switches

void CGraphicsContext::InitWindowInfo()
{
}

bool CGraphicsContext::Initialize(uint32 dwWidth, uint32 dwHeight, BOOL bWindowed)
{
    m_bWindowed = (bWindowed != 0);

    g_pFrameBufferManager->Initialize();
    return true;
}

void CGraphicsContext::CleanUp()
{
    m_bActive = false;
    m_bReady  = false;
}


int __cdecl SortFrequenciesCallback( const void* arg1, const void* arg2 )
{
    unsigned int* p1 = (unsigned int*)arg1;
    unsigned int* p2 = (unsigned int*)arg2;

    if( *p1 < *p2 )   
        return -1;
    else if( *p1 > *p2 )   
        return 1;
    else 
        return 0;
}
int __cdecl SortResolutionsCallback( const void* arg1, const void* arg2 )
{
    unsigned int* p1 = (unsigned int*)arg1;
    unsigned int* p2 = (unsigned int*)arg2;

    if( *p1 < *p2 )   
        return -1;
    else if( *p1 > *p2 )   
        return 1;
    else 
    {
        if( p1[1] < p2[1] )   
            return -1;
        else if( p1[1] > p2[1] )   
            return 1;
        else
            return 0;
    }
}

// This is a static function, will be called when the plugin DLL is initialized
void CGraphicsContext::InitDeviceParameters(void)
{
    // To initialze device parameters for OpenGL
    COGLGraphicsContext::InitDeviceParameters();
}


