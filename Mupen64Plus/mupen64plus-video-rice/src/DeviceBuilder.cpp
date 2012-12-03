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

#include "DeviceBuilder.h"
#include "FrameBuffer.h"
#include "OGLCombinerNV.h"
#include "OGLCombinerTNT2.h"
#include "OGLDebug.h"
#include "OGLFragmentShaders.h"
#include "OGLExtRender.h"
#include "OGLExtensions.h"
#include "OGLGraphicsContext.h"
#include "OGLTexture.h"

//========================================================================
CDeviceBuilder* CDeviceBuilder::m_pInstance=NULL;
SupportedDeviceType CDeviceBuilder::m_deviceType = DIRECTX_DEVICE;
SupportedDeviceType CDeviceBuilder::m_deviceGeneralType = DIRECTX_DEVICE;

CDeviceBuilder* CDeviceBuilder::GetBuilder(void)
{
    if( m_pInstance == NULL )
        CreateBuilder(m_deviceType);
    
    return m_pInstance;
}

void CDeviceBuilder::SelectDeviceType(SupportedDeviceType type)
{
    if( type != m_deviceType && m_pInstance != NULL )
    {
        DeleteBuilder();
    }

    CDeviceBuilder::m_deviceType = type;
    switch(type)
    {
    case OGL_DEVICE:
    case OGL_1_1_DEVICE:
    case OGL_1_2_DEVICE:
    case OGL_1_3_DEVICE:
    case OGL_1_4_DEVICE:
    case OGL_1_4_V2_DEVICE:
    case OGL_TNT2_DEVICE:
    case NVIDIA_OGL_DEVICE:
    case OGL_FRAGMENT_PROGRAM:
        CDeviceBuilder::m_deviceGeneralType = OGL_DEVICE;
        break;
     default:
       break;
    }
}

SupportedDeviceType CDeviceBuilder::GetDeviceType(void)
{
    return CDeviceBuilder::m_deviceType;
}

SupportedDeviceType CDeviceBuilder::GetGeneralDeviceType(void)
{
    return CDeviceBuilder::m_deviceGeneralType;
}

CDeviceBuilder* CDeviceBuilder::CreateBuilder(SupportedDeviceType type)
{
    if( m_pInstance == NULL )
    {
        switch( type )
        {
        case    OGL_DEVICE:
        case    OGL_1_1_DEVICE:
        case    OGL_1_2_DEVICE:
        case    OGL_1_3_DEVICE:
        case    OGL_1_4_DEVICE:
        case    OGL_1_4_V2_DEVICE:
        case    OGL_TNT2_DEVICE:
        case    NVIDIA_OGL_DEVICE:
        case OGL_FRAGMENT_PROGRAM:
            m_pInstance = new OGLDeviceBuilder();
            break;
        default:
            DebugMessage(M64MSG_ERROR, "CreateBuilder: unknown OGL device type");
            exit(1);
        }

        SAFE_CHECK(m_pInstance);
    }

    return m_pInstance;
}

void CDeviceBuilder::DeleteBuilder(void)
{
    delete m_pInstance;
    m_pInstance = NULL;
}

CDeviceBuilder::CDeviceBuilder() :
    m_pRender(NULL),
    m_pGraphicsContext(NULL),
    m_pColorCombiner(NULL),
    m_pAlphaBlender(NULL)
{
}

CDeviceBuilder::~CDeviceBuilder()
{
    DeleteGraphicsContext();
    DeleteRender();
    DeleteColorCombiner();
    DeleteAlphaBlender();
}

void CDeviceBuilder::DeleteGraphicsContext(void)
{
    if( m_pGraphicsContext != NULL )
    {
        delete m_pGraphicsContext;
        CGraphicsContext::g_pGraphicsContext = m_pGraphicsContext = NULL;
    }

    SAFE_DELETE(g_pFrameBufferManager);
}

void CDeviceBuilder::DeleteRender(void)
{
    if( m_pRender != NULL )
    {
        delete m_pRender;
        CRender::g_pRender = m_pRender = NULL;
        CRender::gRenderReferenceCount = 0;
    }
}

void CDeviceBuilder::DeleteColorCombiner(void)
{
    if( m_pColorCombiner != NULL )
    {
        delete m_pColorCombiner;
        m_pColorCombiner = NULL;
    }
}

void CDeviceBuilder::DeleteAlphaBlender(void)
{
    if( m_pAlphaBlender != NULL )
    {
        delete m_pAlphaBlender;
        m_pAlphaBlender = NULL;
    }
}


//========================================================================

CGraphicsContext * OGLDeviceBuilder::CreateGraphicsContext(void)
{
    if( m_pGraphicsContext == NULL )
    {
        m_pGraphicsContext = new COGLGraphicsContext();
        SAFE_CHECK(m_pGraphicsContext);
        CGraphicsContext::g_pGraphicsContext = m_pGraphicsContext;
    }

    g_pFrameBufferManager = new FrameBufferManager;
    return m_pGraphicsContext;
}

CRender * OGLDeviceBuilder::CreateRender(void)
{
    if( m_pRender == NULL )
    {
        if( CGraphicsContext::g_pGraphicsContext == NULL && CGraphicsContext::g_pGraphicsContext->Ready() )
        {
            DebugMessage(M64MSG_ERROR, "Can not create ColorCombiner before creating and initializing GraphicsContext");
            m_pRender = NULL;
            SAFE_CHECK(m_pRender);
        }

        COGLGraphicsContext &context = *((COGLGraphicsContext*)CGraphicsContext::g_pGraphicsContext);

        if( context.m_bSupportMultiTexture )
        {
            // OGL extension render
            m_pRender = new COGLExtRender();
        }
        else
        {
            // Basic OGL Render
            m_pRender = new OGLRender();
        }
        SAFE_CHECK(m_pRender);
        CRender::g_pRender = m_pRender;
    }

    return m_pRender;
}

CTexture * OGLDeviceBuilder::CreateTexture(uint32 dwWidth, uint32 dwHeight, TextureUsage usage)
{
    COGLTexture *txtr = new COGLTexture(dwWidth, dwHeight, usage);
    if( txtr->m_pTexture == NULL )
    {
        delete txtr;
        TRACE0("Cannot create new texture, out of video memory");
        return NULL;
    }
    else
        return txtr;
}

CColorCombiner * OGLDeviceBuilder::CreateColorCombiner(CRender *pRender)
{
    if( m_pColorCombiner == NULL )
    {
        if( CGraphicsContext::g_pGraphicsContext == NULL && CGraphicsContext::g_pGraphicsContext->Ready() )
        {
            DebugMessage(M64MSG_ERROR, "Can not create ColorCombiner before creating and initializing GraphicsContext");
        }
        else
        {
            m_deviceType = (SupportedDeviceType)options.OpenglRenderSetting;
            if (m_deviceType == NVIDIA_OGL_DEVICE && !bNvidiaExtensionsSupported)
            {
                DebugMessage(M64MSG_WARNING, "Your video card does not support Nvidia OpenGL extensions.  Falling back to auto device.");
                m_deviceType = OGL_DEVICE;
            }
            if( m_deviceType == OGL_DEVICE )    // Best fit
            {
                GLint maxUnit = 2;
                COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
                glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&maxUnit);
                OPENGL_CHECK_ERRORS;

                if( pcontext->IsExtensionSupported("GL_ARB_fragment_program") )
                {
                    m_pColorCombiner = new COGL_FragmentProgramCombiner(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: Fragment Program");
                }
                else if( pcontext->IsExtensionSupported("GL_NV_texture_env_combine4") || 
                    pcontext->IsExtensionSupported("GL_NV_register_combiners") )
                {
                    m_pColorCombiner = new COGLColorCombinerNvidia(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: NVidia");
                }
                else if( pcontext->IsExtensionSupported("GL_NV_texture_env_combine4") )
                {
                    m_pColorCombiner = new COGLColorCombinerTNT2(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: TNT2");
                }
                else if( pcontext->IsExtensionSupported("GL_EXT_texture_env_combine") ||
                         pcontext->IsExtensionSupported("GL_ARB_texture_env_combine") )
                {
                    if( pcontext->IsExtensionSupported("GL_ARB_texture_env_crossbar") )
                    {
                        if( maxUnit > 2 )
                        {
                            m_pColorCombiner = new COGLColorCombiner4v2(pRender);
                            DebugMessage(M64MSG_INFO, "OpenGL Combiner: OGL 1.4 version 2");
                        }
                        else
                        {
                            m_pColorCombiner = new COGLColorCombiner4(pRender);
                            DebugMessage(M64MSG_INFO, "OpenGL Combiner: OGL 1.4");
                        }
                    }
                    else
                    {
                        if( maxUnit > 2 )
                        {
                            m_pColorCombiner = new COGLColorCombiner4v2(pRender);
                            DebugMessage(M64MSG_INFO, "OpenGL Combiner: OGL 1.4 version 2 (w/o env crossbar)");
                        }
                        else
                        {
                            m_pColorCombiner = new COGLColorCombiner2(pRender);
                            DebugMessage(M64MSG_INFO, "OpenGL Combiner: OGL 1.2/1.3");
                        }
                    }
                }
                else
                {
                    m_pColorCombiner = new COGLColorCombiner(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: Basic OGL");
                }
            }
            else
            {
                switch(m_deviceType)
                {
                case OGL_1_1_DEVICE:
                    m_pColorCombiner = new COGLColorCombiner(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: Basic OGL");
                    break;
                case OGL_1_2_DEVICE:
                case OGL_1_3_DEVICE:
                    m_pColorCombiner = new COGLColorCombiner2(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: OGL 1.2/1.3");
                    break;
                case OGL_1_4_DEVICE:
                    m_pColorCombiner = new COGLColorCombiner4(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: OGL 1.4");
                    break;
                case OGL_1_4_V2_DEVICE:
                    m_pColorCombiner = new COGLColorCombiner4v2(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: OGL 1.4 Version 2");
                    break;
                case OGL_TNT2_DEVICE:
                    m_pColorCombiner = new COGLColorCombinerTNT2(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: TNT2");
                    break;
                case NVIDIA_OGL_DEVICE:
                    m_pColorCombiner = new COGLColorCombinerNvidia(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: Nvidia");
                    break;
                case OGL_FRAGMENT_PROGRAM:
                    m_pColorCombiner = new COGL_FragmentProgramCombiner(pRender);
                    DebugMessage(M64MSG_INFO, "OpenGL Combiner: Fragment Program");
                    break;
                 default:
                        break;
                }
            }
        }

        SAFE_CHECK(m_pColorCombiner);
    }

    return m_pColorCombiner;
}

CBlender * OGLDeviceBuilder::CreateAlphaBlender(CRender *pRender)
{
    if( m_pAlphaBlender == NULL )
    {
        m_pAlphaBlender = new COGLBlender(pRender);
        SAFE_CHECK(m_pAlphaBlender);
    }

    return m_pAlphaBlender;
}

