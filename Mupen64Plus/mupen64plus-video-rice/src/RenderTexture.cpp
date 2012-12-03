/*
Copyright (C) 2005 Rice1964

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

#include "Debugger.h"
#include "FrameBuffer.h"
#include "OGLTexture.h"

// ===========================================================================
COGLRenderTexture::COGLRenderTexture(int width, int height, RenderTextureInfo* pInfo, TextureUsage usage)
    :   CRenderTexture(width, height, pInfo, usage),
        m_pOGLTexture(NULL)
{
    if( usage == AS_BACK_BUFFER_SAVE )
    {
        m_pTexture = m_pOGLTexture = new COGLTexture(width, height, usage);
        if( !m_pTexture )
        {
            TRACE0("Error to create OGL render_texture");
            SAFE_DELETE(m_pTexture);
        }
    }

    m_width = width;
    m_height = height;
    m_beingRendered = false;
}

COGLRenderTexture::~COGLRenderTexture()
{
    if( m_beingRendered )
    {
        g_pFrameBufferManager->CloseRenderTexture(false);
        SetAsRenderTarget(false);
    }

    ShutdownPBuffer();
    SAFE_DELETE(m_pTexture);
    m_pOGLTexture = NULL;
    m_beingRendered = false;
}

bool COGLRenderTexture::InitPBuffer( void )
{
    return true;
}

void COGLRenderTexture::ShutdownPBuffer(void)
{
}

bool COGLRenderTexture::SetAsRenderTarget(bool enable)
{
   return true;
}

void COGLRenderTexture::LoadTexture(TxtrCacheEntry* pEntry)
{
}

void COGLRenderTexture::StoreToRDRAM(int infoIdx)
{
}

