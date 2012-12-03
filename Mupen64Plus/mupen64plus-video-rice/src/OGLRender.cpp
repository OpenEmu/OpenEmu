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

#include "OGLExtensions.h"
#include "OGLDebug.h"
#include "OGLRender.h"
#include "OGLGraphicsContext.h"
#include "OGLTexture.h"
#include "TextureManager.h"

// Fix me, use OGL internal L/T and matrix stack
// Fix me, use OGL lookupAt function
// Fix me, use OGL DisplayList

UVFlagMap OGLXUVFlagMaps[] =
{
{TEXTURE_UV_FLAG_WRAP, GL_REPEAT},
{TEXTURE_UV_FLAG_MIRROR, GL_MIRRORED_REPEAT_ARB},
{TEXTURE_UV_FLAG_CLAMP, GL_CLAMP},
};

//===================================================================
OGLRender::OGLRender()
{
    COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
    m_bSupportFogCoordExt = pcontext->m_bSupportFogCoord;
    m_bMultiTexture = pcontext->m_bSupportMultiTexture;
    m_bSupportClampToEdge = false;
    for( int i=0; i<8; i++ )
    {
        m_curBoundTex[i]=0;
        m_texUnitEnabled[i]=FALSE;
    }
    m_bEnableMultiTexture = false;
}

OGLRender::~OGLRender()
{
    ClearDeviceObjects();
}

bool OGLRender::InitDeviceObjects()
{
    // enable Z-buffer by default
    ZBufferEnable(true);
    return true;
}

bool OGLRender::ClearDeviceObjects()
{
    return true;
}

void OGLRender::Initialize(void)
{
    glMatrixMode(GL_MODELVIEW);
    OPENGL_CHECK_ERRORS;
    glLoadIdentity();
    OPENGL_CHECK_ERRORS;

    glViewportWrapper(0, windowSetting.statusBarHeightToUse, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight);
    OPENGL_CHECK_ERRORS;

    COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
    if( pcontext->IsExtensionSupported("GL_IBM_texture_mirrored_repeat") )
    {
        OGLXUVFlagMaps[TEXTURE_UV_FLAG_MIRROR].realFlag = GL_MIRRORED_REPEAT_IBM;
    }
    else if( pcontext->IsExtensionSupported("ARB_texture_mirrored_repeat") )
    {
        OGLXUVFlagMaps[TEXTURE_UV_FLAG_MIRROR].realFlag = GL_MIRRORED_REPEAT_ARB;
    }
    else
    {
        OGLXUVFlagMaps[TEXTURE_UV_FLAG_MIRROR].realFlag = GL_REPEAT;
    }

    if( pcontext->IsExtensionSupported("GL_ARB_texture_border_clamp") || pcontext->IsExtensionSupported("GL_EXT_texture_edge_clamp") )
    {
        m_bSupportClampToEdge = true;
        OGLXUVFlagMaps[TEXTURE_UV_FLAG_CLAMP].realFlag = GL_CLAMP_TO_EDGE;
    }
    else
    {
        m_bSupportClampToEdge = false;
        OGLXUVFlagMaps[TEXTURE_UV_FLAG_CLAMP].realFlag = GL_CLAMP;
    }

    glVertexPointer( 4, GL_FLOAT, sizeof(float)*5, &(g_vtxProjected5[0][0]) );
    OPENGL_CHECK_ERRORS;
    glEnableClientState( GL_VERTEX_ARRAY );
    OPENGL_CHECK_ERRORS;

    if( m_bMultiTexture )
    {
        pglClientActiveTextureARB( GL_TEXTURE0_ARB );
        OPENGL_CHECK_ERRORS;
        glTexCoordPointer( 2, GL_FLOAT, sizeof( TLITVERTEX ), &(g_vtxBuffer[0].tcord[0].u) );
        OPENGL_CHECK_ERRORS;
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        OPENGL_CHECK_ERRORS;

        pglClientActiveTextureARB( GL_TEXTURE1_ARB );
        OPENGL_CHECK_ERRORS;
        glTexCoordPointer( 2, GL_FLOAT, sizeof( TLITVERTEX ), &(g_vtxBuffer[0].tcord[1].u) );
        OPENGL_CHECK_ERRORS;
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glTexCoordPointer( 2, GL_FLOAT, sizeof( TLITVERTEX ), &(g_vtxBuffer[0].tcord[0].u) );
        OPENGL_CHECK_ERRORS;
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        OPENGL_CHECK_ERRORS;
    }

    if (m_bSupportFogCoordExt)
    {
        pglFogCoordPointerEXT( GL_FLOAT, sizeof(float)*5, &(g_vtxProjected5[0][4]) );
        OPENGL_CHECK_ERRORS;
        glEnableClientState( GL_FOG_COORDINATE_ARRAY_EXT );
        OPENGL_CHECK_ERRORS;
        glFogi( GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT );
        OPENGL_CHECK_ERRORS;
        glFogi(GL_FOG_MODE, GL_LINEAR); // Fog Mode
        OPENGL_CHECK_ERRORS;
        glFogf(GL_FOG_DENSITY, 1.0f); // How Dense Will The Fog Be
        OPENGL_CHECK_ERRORS;
        glHint(GL_FOG_HINT, GL_FASTEST); // Fog Hint Value
        OPENGL_CHECK_ERRORS;
        glFogi( GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT );
        OPENGL_CHECK_ERRORS;
        glFogf( GL_FOG_START, 0.0f );
        OPENGL_CHECK_ERRORS;
        glFogf( GL_FOG_END, 1.0f );
        OPENGL_CHECK_ERRORS;
    }

    //glColorPointer( 1, GL_UNSIGNED_BYTE, sizeof(TLITVERTEX), &g_vtxBuffer[0].r);
    glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(uint8)*4, &(g_oglVtxColors[0][0]) );
    OPENGL_CHECK_ERRORS;
    glEnableClientState( GL_COLOR_ARRAY );
    OPENGL_CHECK_ERRORS;

    if( pcontext->IsExtensionSupported("GL_NV_depth_clamp") )
    {
        glEnable(GL_DEPTH_CLAMP_NV);
        OPENGL_CHECK_ERRORS;
    }

}
//===================================================================
TextureFilterMap OglTexFilterMap[2]=
{
    {FILTER_POINT, GL_NEAREST},
    {FILTER_LINEAR, GL_LINEAR},
};

void OGLRender::ApplyTextureFilter()
{
    static uint32 minflag=0xFFFF, magflag=0xFFFF;
    static uint32 mtex;

    if( m_texUnitEnabled[0] )
    {
        if( mtex != m_curBoundTex[0] )
        {
            mtex = m_curBoundTex[0];
            minflag = m_dwMinFilter;
            magflag = m_dwMagFilter;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OglTexFilterMap[m_dwMinFilter].realFilter);
            OPENGL_CHECK_ERRORS;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OglTexFilterMap[m_dwMagFilter].realFilter);
            OPENGL_CHECK_ERRORS;
        }
        else
        {
            if( minflag != (unsigned int)m_dwMinFilter )
            {
                minflag = m_dwMinFilter;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OglTexFilterMap[m_dwMinFilter].realFilter);
                OPENGL_CHECK_ERRORS;
            }
            if( magflag != (unsigned int)m_dwMagFilter )
            {
                magflag = m_dwMagFilter;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OglTexFilterMap[m_dwMagFilter].realFilter);
                OPENGL_CHECK_ERRORS;
            }   
        }
    }
}

void OGLRender::SetShadeMode(RenderShadeMode mode)
{
    if( mode == SHADE_SMOOTH )
        glShadeModel(GL_SMOOTH);
    else
        glShadeModel(GL_FLAT);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::ZBufferEnable(BOOL bZBuffer)
{
    gRSP.bZBufferEnabled = bZBuffer;
    if( g_curRomInfo.bForceDepthBuffer )
        bZBuffer = TRUE;
    if( bZBuffer )
    {
        glDepthMask(GL_TRUE);
        OPENGL_CHECK_ERRORS;
        //glEnable(GL_DEPTH_TEST);
        glDepthFunc( GL_LEQUAL );
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glDepthMask(GL_FALSE);
        OPENGL_CHECK_ERRORS;
        //glDisable(GL_DEPTH_TEST);
        glDepthFunc( GL_ALWAYS );
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::ClearBuffer(bool cbuffer, bool zbuffer)
{
    uint32 flag=0;
    if( cbuffer )   flag |= GL_COLOR_BUFFER_BIT;
    if( zbuffer )   flag |= GL_DEPTH_BUFFER_BIT;
    float depth = ((gRDP.originalFillColor&0xFFFF)>>2)/(float)0x3FFF;
    glClearDepth(depth);
    OPENGL_CHECK_ERRORS;
    glClear(flag);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::ClearZBuffer(float depth)
{
    uint32 flag=GL_DEPTH_BUFFER_BIT;
    glClearDepth(depth);
    OPENGL_CHECK_ERRORS;
    glClear(flag);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::SetZCompare(BOOL bZCompare)
{
    if( g_curRomInfo.bForceDepthBuffer )
        bZCompare = TRUE;

    gRSP.bZBufferEnabled = bZCompare;
    if( bZCompare == TRUE )
    {
        //glEnable(GL_DEPTH_TEST);
        glDepthFunc( GL_LEQUAL );
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        //glDisable(GL_DEPTH_TEST);
        glDepthFunc( GL_ALWAYS );
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::SetZUpdate(BOOL bZUpdate)
{
    if( g_curRomInfo.bForceDepthBuffer )
        bZUpdate = TRUE;

    if( bZUpdate )
    {
        //glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glDepthMask(GL_FALSE);
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::ApplyZBias(int bias)
{
    float f1 = bias > 0 ? -3.0f : 0.0f;  // z offset = -3.0 * max(abs(dz/dx),abs(dz/dy)) per pixel delta z slope
    float f2 = bias > 0 ? -3.0f : 0.0f;  // z offset += -3.0 * 1 bit
    if (bias > 0)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);  // enable z offsets
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_FILL);  // disable z offsets
        OPENGL_CHECK_ERRORS;
    }
    glPolygonOffset(f1, f2);  // set bias functions
    OPENGL_CHECK_ERRORS;
}

void OGLRender::SetZBias(int bias)
{
#if defined(DEBUGGER)
    if( pauseAtNext == true )
      DebuggerAppendMsg("Set zbias = %d", bias);
#endif
    // set member variable and apply the setting in opengl
    m_dwZBias = bias;
    ApplyZBias(bias);
}

void OGLRender::SetAlphaRef(uint32 dwAlpha)
{
    if (m_dwAlpha != dwAlpha)
    {
        m_dwAlpha = dwAlpha;
        glAlphaFunc(GL_GEQUAL, (float)dwAlpha);
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::ForceAlphaRef(uint32 dwAlpha)
{
    float ref = dwAlpha/255.0f;
    glAlphaFunc(GL_GEQUAL, ref);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::SetFillMode(FillMode mode)
{
    if( mode == RICE_FILLMODE_WINFRAME )
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::SetCullMode(bool bCullFront, bool bCullBack)
{
    CRender::SetCullMode(bCullFront, bCullBack);
    if( bCullFront && bCullBack )
    {
        glCullFace(GL_FRONT_AND_BACK);
        OPENGL_CHECK_ERRORS;
        glEnable(GL_CULL_FACE);
        OPENGL_CHECK_ERRORS;
    }
    else if( bCullFront )
    {
        glCullFace(GL_FRONT);
        OPENGL_CHECK_ERRORS;
        glEnable(GL_CULL_FACE);
        OPENGL_CHECK_ERRORS;
    }
    else if( bCullBack )
    {
        glCullFace(GL_BACK);
        OPENGL_CHECK_ERRORS;
        glEnable(GL_CULL_FACE);
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glDisable(GL_CULL_FACE);
        OPENGL_CHECK_ERRORS;
    }
}

bool OGLRender::SetCurrentTexture(int tile, CTexture *handler,uint32 dwTileWidth, uint32 dwTileHeight, TxtrCacheEntry *pTextureEntry)
{
    RenderTexture &texture = g_textures[tile];
    texture.pTextureEntry = pTextureEntry;

    if( handler!= NULL  && texture.m_lpsTexturePtr != handler->GetTexture() )
    {
        texture.m_pCTexture = handler;
        texture.m_lpsTexturePtr = handler->GetTexture();

        texture.m_dwTileWidth = dwTileWidth;
        texture.m_dwTileHeight = dwTileHeight;

        if( handler->m_bIsEnhancedTexture )
        {
            texture.m_fTexWidth = (float)pTextureEntry->pTexture->m_dwCreatedTextureWidth;
            texture.m_fTexHeight = (float)pTextureEntry->pTexture->m_dwCreatedTextureHeight;
        }
        else
        {
            texture.m_fTexWidth = (float)handler->m_dwCreatedTextureWidth;
            texture.m_fTexHeight = (float)handler->m_dwCreatedTextureHeight;
        }
    }
    
    return true;
}

bool OGLRender::SetCurrentTexture(int tile, TxtrCacheEntry *pEntry)
{
    if (pEntry != NULL && pEntry->pTexture != NULL)
    {   
        SetCurrentTexture( tile, pEntry->pTexture,  pEntry->ti.WidthToCreate, pEntry->ti.HeightToCreate, pEntry);
        return true;
    }
    else
    {
        SetCurrentTexture( tile, NULL, 64, 64, NULL );
        return false;
    }
    return true;
}

void OGLRender::SetAddressUAllStages(uint32 dwTile, TextureUVFlag dwFlag)
{
    SetTextureUFlag(dwFlag, dwTile);
}

void OGLRender::SetAddressVAllStages(uint32 dwTile, TextureUVFlag dwFlag)
{
    SetTextureVFlag(dwFlag, dwTile);
}

void OGLRender::SetTexWrapS(int unitno,GLuint flag)
{
    static GLuint mflag;
    static GLuint mtex;
#ifdef DEBUGGER
    if( unitno != 0 )
    {
        DebuggerAppendMsg("Check me, unitno != 0 in base ogl");
    }
#endif
    if( m_curBoundTex[0] != mtex || mflag != flag )
    {
        mtex = m_curBoundTex[0];
        mflag = flag;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, flag);
        OPENGL_CHECK_ERRORS;
    }
}
void OGLRender::SetTexWrapT(int unitno,GLuint flag)
{
    static GLuint mflag;
    static GLuint mtex;
    if( m_curBoundTex[0] != mtex || mflag != flag )
    {
        mtex = m_curBoundTex[0];
        mflag = flag;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, flag);
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::SetTextureUFlag(TextureUVFlag dwFlag, uint32 dwTile)
{
    TileUFlags[dwTile] = dwFlag;
    if( dwTile == gRSP.curTile )    // For basic OGL, only support the 1st texel
    {
        COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
        if( pTexture )
        {
            EnableTexUnit(0,TRUE);
            BindTexture(pTexture->m_dwTextureName, 0);
        }
        SetTexWrapS(0, OGLXUVFlagMaps[dwFlag].realFlag);
    }
}
void OGLRender::SetTextureVFlag(TextureUVFlag dwFlag, uint32 dwTile)
{
    TileVFlags[dwTile] = dwFlag;
    if( dwTile == gRSP.curTile )    // For basic OGL, only support the 1st texel
    {
        COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
        if( pTexture ) 
        {
            EnableTexUnit(0,TRUE);
            BindTexture(pTexture->m_dwTextureName, 0);
        }
        SetTexWrapT(0, OGLXUVFlagMaps[dwFlag].realFlag);
    }
}

// Basic render drawing functions

bool OGLRender::RenderTexRect()
{
    glViewportWrapper(0, windowSetting.statusBarHeightToUse, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight);
    OPENGL_CHECK_ERRORS;

    GLboolean cullface = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;

    glBegin(GL_TRIANGLE_FAN);

    float depth = -(g_texRectTVtx[3].z*2-1);

    glColor4f(g_texRectTVtx[3].r, g_texRectTVtx[3].g, g_texRectTVtx[3].b, g_texRectTVtx[3].a);
    TexCoord(g_texRectTVtx[3]);
    glVertex3f(g_texRectTVtx[3].x, g_texRectTVtx[3].y, depth);
    
    glColor4f(g_texRectTVtx[2].r, g_texRectTVtx[2].g, g_texRectTVtx[2].b, g_texRectTVtx[2].a);
    TexCoord(g_texRectTVtx[2]);
    glVertex3f(g_texRectTVtx[2].x, g_texRectTVtx[2].y, depth);

    glColor4f(g_texRectTVtx[1].r, g_texRectTVtx[1].g, g_texRectTVtx[1].b, g_texRectTVtx[1].a);
    TexCoord(g_texRectTVtx[1]);
    glVertex3f(g_texRectTVtx[1].x, g_texRectTVtx[1].y, depth);

    glColor4f(g_texRectTVtx[0].r, g_texRectTVtx[0].g, g_texRectTVtx[0].b, g_texRectTVtx[0].a);
    TexCoord(g_texRectTVtx[0]);
    glVertex3f(g_texRectTVtx[0].x, g_texRectTVtx[0].y, depth);

    glEnd();
    OPENGL_CHECK_ERRORS;

    if( cullface ) glEnable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;

    return true;
}

bool OGLRender::RenderFillRect(uint32 dwColor, float depth)
{
    float a = (dwColor>>24)/255.0f;
    float r = ((dwColor>>16)&0xFF)/255.0f;
    float g = ((dwColor>>8)&0xFF)/255.0f;
    float b = (dwColor&0xFF)/255.0f;
    glViewportWrapper(0, windowSetting.statusBarHeightToUse, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight);
    OPENGL_CHECK_ERRORS;

    GLboolean cullface = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(r,g,b,a);
    glVertex4f(m_fillRectVtx[0].x, m_fillRectVtx[1].y, depth, 1);
    glVertex4f(m_fillRectVtx[1].x, m_fillRectVtx[1].y, depth, 1);
    glVertex4f(m_fillRectVtx[1].x, m_fillRectVtx[0].y, depth, 1);
    glVertex4f(m_fillRectVtx[0].x, m_fillRectVtx[0].y, depth, 1);
    glEnd();
    OPENGL_CHECK_ERRORS;

    if( cullface ) glEnable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;

    return true;
}

bool OGLRender::RenderLine3D()
{
    ApplyZBias(0);  // disable z offsets

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(m_line3DVtx[1].r, m_line3DVtx[1].g, m_line3DVtx[1].b, m_line3DVtx[1].a);
    glVertex3f(m_line3DVector[3].x, m_line3DVector[3].y, -m_line3DVtx[1].z);
    glVertex3f(m_line3DVector[2].x, m_line3DVector[2].y, -m_line3DVtx[0].z);
    
    glColor4ub(m_line3DVtx[0].r, m_line3DVtx[0].g, m_line3DVtx[0].b, m_line3DVtx[0].a);
    glVertex3f(m_line3DVector[1].x, m_line3DVector[1].y, -m_line3DVtx[1].z);
    glVertex3f(m_line3DVector[0].x, m_line3DVector[0].y, -m_line3DVtx[0].z);

    glEnd();
    OPENGL_CHECK_ERRORS;

    ApplyZBias(m_dwZBias);          // set Z offset back to previous value

    return true;
}

extern FiddledVtx * g_pVtxBase;

// This is so weired that I can not do vertex transform by myself. I have to use
// OpenGL internal transform
bool OGLRender::RenderFlushTris()
{
    if( !m_bSupportFogCoordExt )    
        SetFogFlagForNegativeW();
    else
    {
        if( !gRDP.bFogEnableInBlender && gRSP.bFogEnabled )
        {
            glDisable(GL_FOG);
            OPENGL_CHECK_ERRORS;
        }
    }

    ApplyZBias(m_dwZBias);                    // set the bias factors

    glViewportWrapper(windowSetting.vpLeftW, windowSetting.uDisplayHeight-windowSetting.vpTopW-windowSetting.vpHeightW+windowSetting.statusBarHeightToUse, windowSetting.vpWidthW, windowSetting.vpHeightW, false);
    OPENGL_CHECK_ERRORS;

    //if options.bOGLVertexClipper == FALSE )
    {
        glDrawElements( GL_TRIANGLES, gRSP.numVertices, GL_UNSIGNED_INT, g_vtxIndex );
        OPENGL_CHECK_ERRORS;
    }
/*  else
    {
        //ClipVertexesOpenGL();
        // Redo the index
        // Set the array
        glVertexPointer( 4, GL_FLOAT, sizeof(float)*5, &(g_vtxProjected5Clipped[0][0]) );
        glEnableClientState( GL_VERTEX_ARRAY );

        pglClientActiveTextureARB( GL_TEXTURE0_ARB );
        glTexCoordPointer( 2, GL_FLOAT, sizeof( TLITVERTEX ), &(g_clippedVtxBuffer[0].tcord[0].u) );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );

        pglClientActiveTextureARB( GL_TEXTURE1_ARB );
        glTexCoordPointer( 2, GL_FLOAT, sizeof( TLITVERTEX ), &(g_clippedVtxBuffer[0].tcord[1].u) );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );

        glDrawElements( GL_TRIANGLES, gRSP.numVertices, GL_UNSIGNED_INT, g_vtxIndex );

        // Reset the array
        pglClientActiveTextureARB( GL_TEXTURE0_ARB );
        glTexCoordPointer( 2, GL_FLOAT, sizeof( TLITVERTEX ), &(g_vtxBuffer[0].tcord[0].u) );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );

        pglClientActiveTextureARB( GL_TEXTURE1_ARB );
        glTexCoordPointer( 2, GL_FLOAT, sizeof( TLITVERTEX ), &(g_vtxBuffer[0].tcord[1].u) );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );

        glVertexPointer( 4, GL_FLOAT, sizeof(float)*5, &(g_vtxProjected5[0][0]) );
        glEnableClientState( GL_VERTEX_ARRAY );
    }
*/

    if( !m_bSupportFogCoordExt )    
        RestoreFogFlag();
    else
    {
        if( !gRDP.bFogEnableInBlender && gRSP.bFogEnabled )
        {
            glEnable(GL_FOG);
            OPENGL_CHECK_ERRORS;
        }
    }
    return true;
}

void OGLRender::DrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, COLOR dif, COLOR spe, float z, float rhw)
{
    if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1ST_PRIMITIVE )
    {
        status.bVIOriginIsUpdated=false;
        CGraphicsContext::Get()->UpdateFrame();
        DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st Simple2DTexture");});
    }

    StartDrawSimple2DTexture(x0, y0, x1, y1, u0, v0, u1, v1, dif, spe, z, rhw);

    GLboolean cullface = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;

    glViewportWrapper(0, windowSetting.statusBarHeightToUse, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight);
    OPENGL_CHECK_ERRORS;

    glBegin(GL_TRIANGLES);
    float a = (g_texRectTVtx[0].dcDiffuse >>24)/255.0f;
    float r = ((g_texRectTVtx[0].dcDiffuse>>16)&0xFF)/255.0f;
    float g = ((g_texRectTVtx[0].dcDiffuse>>8)&0xFF)/255.0f;
    float b = (g_texRectTVtx[0].dcDiffuse&0xFF)/255.0f;
    glColor4f(r,g,b,a);

    OGLRender::TexCoord(g_texRectTVtx[0]);
    glVertex3f(g_texRectTVtx[0].x, g_texRectTVtx[0].y, -g_texRectTVtx[0].z);

    OGLRender::TexCoord(g_texRectTVtx[1]);
    glVertex3f(g_texRectTVtx[1].x, g_texRectTVtx[1].y, -g_texRectTVtx[1].z);

    OGLRender::TexCoord(g_texRectTVtx[2]);
    glVertex3f(g_texRectTVtx[2].x, g_texRectTVtx[2].y, -g_texRectTVtx[2].z);

    OGLRender::TexCoord(g_texRectTVtx[0]);
    glVertex3f(g_texRectTVtx[0].x, g_texRectTVtx[0].y, -g_texRectTVtx[0].z);

    OGLRender::TexCoord(g_texRectTVtx[2]);
    glVertex3f(g_texRectTVtx[2].x, g_texRectTVtx[2].y, -g_texRectTVtx[2].z);

    OGLRender::TexCoord(g_texRectTVtx[3]);
    glVertex3f(g_texRectTVtx[3].x, g_texRectTVtx[3].y, -g_texRectTVtx[3].z);
    
    glEnd();
    OPENGL_CHECK_ERRORS;

    if( cullface ) glEnable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::DrawSimpleRect(int nX0, int nY0, int nX1, int nY1, uint32 dwColor, float depth, float rhw)
{
    StartDrawSimpleRect(nX0, nY0, nX1, nY1, dwColor, depth, rhw);

    GLboolean cullface = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;

    glBegin(GL_TRIANGLE_FAN);

    float a = (dwColor>>24)/255.0f;
    float r = ((dwColor>>16)&0xFF)/255.0f;
    float g = ((dwColor>>8)&0xFF)/255.0f;
    float b = (dwColor&0xFF)/255.0f;
    glColor4f(r,g,b,a);
    glVertex3f(m_simpleRectVtx[1].x, m_simpleRectVtx[0].y, -depth);
    glVertex3f(m_simpleRectVtx[1].x, m_simpleRectVtx[1].y, -depth);
    glVertex3f(m_simpleRectVtx[0].x, m_simpleRectVtx[1].y, -depth);
    glVertex3f(m_simpleRectVtx[0].x, m_simpleRectVtx[0].y, -depth);
    
    glEnd();
    OPENGL_CHECK_ERRORS;

    if( cullface ) glEnable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::InitCombinerBlenderForSimpleRectDraw(uint32 tile)
{
    //glEnable(GL_CULL_FACE);
    EnableTexUnit(0,FALSE);
    OPENGL_CHECK_ERRORS;
    glEnable(GL_BLEND);
    OPENGL_CHECK_ERRORS;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    OPENGL_CHECK_ERRORS;
    //glEnable(GL_ALPHA_TEST);
}

COLOR OGLRender::PostProcessDiffuseColor(COLOR curDiffuseColor)
{
    uint32 color = curDiffuseColor;
    uint32 colorflag = m_pColorCombiner->m_pDecodedMux->m_dwShadeColorChannelFlag;
    uint32 alphaflag = m_pColorCombiner->m_pDecodedMux->m_dwShadeAlphaChannelFlag;
    if( colorflag+alphaflag != MUX_0 )
    {
        if( (colorflag & 0xFFFFFF00) == 0 && (alphaflag & 0xFFFFFF00) == 0 )
        {
            color = (m_pColorCombiner->GetConstFactor(colorflag, alphaflag, curDiffuseColor));
        }
        else
            color = (CalculateConstFactor(colorflag, alphaflag, curDiffuseColor));
    }

    //return (color<<8)|(color>>24);
    return color;
}

COLOR OGLRender::PostProcessSpecularColor()
{
    return 0;
}

void OGLRender::SetViewportRender()
{
    glViewportWrapper(windowSetting.vpLeftW, windowSetting.uDisplayHeight-windowSetting.vpTopW-windowSetting.vpHeightW+windowSetting.statusBarHeightToUse, windowSetting.vpWidthW, windowSetting.vpHeightW);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::RenderReset()
{
    CRender::RenderReset();

    glMatrixMode(GL_PROJECTION);
    OPENGL_CHECK_ERRORS;
    glLoadIdentity();
    OPENGL_CHECK_ERRORS;
    glOrtho(0, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight, 0, -1, 1);
    OPENGL_CHECK_ERRORS;

    // position viewer 
    glMatrixMode(GL_MODELVIEW);
    OPENGL_CHECK_ERRORS;
    glLoadIdentity();
    OPENGL_CHECK_ERRORS;
}

void OGLRender::SetAlphaTestEnable(BOOL bAlphaTestEnable)
{
#ifdef DEBUGGER
    if( bAlphaTestEnable && debuggerEnableAlphaTest )
#else
    if( bAlphaTestEnable )
#endif
        glEnable(GL_ALPHA_TEST);
    else
        glDisable(GL_ALPHA_TEST);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::BindTexture(GLuint texture, int unitno)
{
#ifdef DEBUGGER
    if( unitno != 0 )
    {
        DebuggerAppendMsg("Check me, base ogl bind texture, unit no != 0");
    }
#endif
    if( m_curBoundTex[0] != texture )
    {
        glBindTexture(GL_TEXTURE_2D,texture);
        OPENGL_CHECK_ERRORS;
        m_curBoundTex[0] = texture;
    }
}

void OGLRender::DisBindTexture(GLuint texture, int unitno)
{
    //EnableTexUnit(0,FALSE);
    //glBindTexture(GL_TEXTURE_2D, 0);  //Not to bind any texture
}

void OGLRender::EnableTexUnit(int unitno, BOOL flag)
{
#ifdef DEBUGGER
    if( unitno != 0 )
    {
        DebuggerAppendMsg("Check me, in the base ogl render, unitno!=0");
    }
#endif
    if( m_texUnitEnabled[0] != flag )
    {
        m_texUnitEnabled[0] = flag;
        if( flag == TRUE )
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::TexCoord2f(float u, float v)
{
    glTexCoord2f(u, v);
}

void OGLRender::TexCoord(TLITVERTEX &vtxInfo)
{
    glTexCoord2f(vtxInfo.tcord[0].u, vtxInfo.tcord[0].v);
}

void OGLRender::UpdateScissor()
{
    if( options.bEnableHacks && g_CI.dwWidth == 0x200 && gRDP.scissor.right == 0x200 && g_CI.dwWidth>(*g_GraphicsInfo.VI_WIDTH_REG & 0xFFF) )
    {
        // Hack for RE2
        uint32 width = *g_GraphicsInfo.VI_WIDTH_REG & 0xFFF;
        uint32 height = (gRDP.scissor.right*gRDP.scissor.bottom)/width;
        glEnable(GL_SCISSOR_TEST);
        OPENGL_CHECK_ERRORS;
        glScissor(0, int(height*windowSetting.fMultY+windowSetting.statusBarHeightToUse),
            int(width*windowSetting.fMultX), int(height*windowSetting.fMultY) );
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        UpdateScissorWithClipRatio();
    }
}

void OGLRender::ApplyRDPScissor(bool force)
{
    if( !force && status.curScissor == RDP_SCISSOR )    return;

    if( options.bEnableHacks && g_CI.dwWidth == 0x200 && gRDP.scissor.right == 0x200 && g_CI.dwWidth>(*g_GraphicsInfo.VI_WIDTH_REG & 0xFFF) )
    {
        // Hack for RE2
        uint32 width = *g_GraphicsInfo.VI_WIDTH_REG & 0xFFF;
        uint32 height = (gRDP.scissor.right*gRDP.scissor.bottom)/width;
        glEnable(GL_SCISSOR_TEST);
        OPENGL_CHECK_ERRORS;
        glScissor(0, int(height*windowSetting.fMultY+windowSetting.statusBarHeightToUse),
            int(width*windowSetting.fMultX), int(height*windowSetting.fMultY) );
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glScissor(int(gRDP.scissor.left*windowSetting.fMultX), int((windowSetting.uViHeight-gRDP.scissor.bottom)*windowSetting.fMultY+windowSetting.statusBarHeightToUse),
            int((gRDP.scissor.right-gRDP.scissor.left)*windowSetting.fMultX), int((gRDP.scissor.bottom-gRDP.scissor.top)*windowSetting.fMultY ));
        OPENGL_CHECK_ERRORS;
    }

    status.curScissor = RDP_SCISSOR;
}

void OGLRender::ApplyScissorWithClipRatio(bool force)
{
    if( !force && status.curScissor == RSP_SCISSOR )    return;

    glEnable(GL_SCISSOR_TEST);
    OPENGL_CHECK_ERRORS;
    glScissor(windowSetting.clipping.left, int((windowSetting.uViHeight-gRSP.real_clip_scissor_bottom)*windowSetting.fMultY)+windowSetting.statusBarHeightToUse,
        windowSetting.clipping.width, windowSetting.clipping.height);
    OPENGL_CHECK_ERRORS;

    status.curScissor = RSP_SCISSOR;
}

void OGLRender::SetFogMinMax(float fMin, float fMax)
{
    glFogf(GL_FOG_START, gRSPfFogMin); // Fog Start Depth
    OPENGL_CHECK_ERRORS;
    glFogf(GL_FOG_END, gRSPfFogMax); // Fog End Depth
    OPENGL_CHECK_ERRORS;
}

void OGLRender::TurnFogOnOff(bool flag)
{
    if( flag )
        glEnable(GL_FOG);
    else
        glDisable(GL_FOG);
    OPENGL_CHECK_ERRORS;
}

void OGLRender::SetFogEnable(bool bEnable)
{
    DEBUGGER_IF_DUMP( (gRSP.bFogEnabled != (bEnable==TRUE) && logFog ), TRACE1("Set Fog %s", bEnable? "enable":"disable"));

    gRSP.bFogEnabled = bEnable&&(options.fogMethod == 1);
    
    // If force fog
    if(options.fogMethod == 2)
    {
        gRSP.bFogEnabled = true;
    }

    if( gRSP.bFogEnabled )
    {
        //TRACE2("Enable fog, min=%f, max=%f",gRSPfFogMin,gRSPfFogMax );
        glFogfv(GL_FOG_COLOR, gRDP.fvFogColor); // Set Fog Color
        OPENGL_CHECK_ERRORS;
        glFogf(GL_FOG_START, gRSPfFogMin); // Fog Start Depth
        OPENGL_CHECK_ERRORS;
        glFogf(GL_FOG_END, gRSPfFogMax); // Fog End Depth
        OPENGL_CHECK_ERRORS;
        glEnable(GL_FOG);
        OPENGL_CHECK_ERRORS;
    }
    else
    {
        glDisable(GL_FOG);
        OPENGL_CHECK_ERRORS;
    }
}

void OGLRender::SetFogColor(uint32 r, uint32 g, uint32 b, uint32 a)
{
    gRDP.fogColor = COLOR_RGBA(r, g, b, a); 
    gRDP.fvFogColor[0] = r/255.0f;      //r
    gRDP.fvFogColor[1] = g/255.0f;      //g
    gRDP.fvFogColor[2] = b/255.0f;          //b
    gRDP.fvFogColor[3] = a/255.0f;      //a
    glFogfv(GL_FOG_COLOR, gRDP.fvFogColor); // Set Fog Color
    OPENGL_CHECK_ERRORS;
}

void OGLRender::DisableMultiTexture()
{
    pglActiveTexture(GL_TEXTURE1_ARB);
    OPENGL_CHECK_ERRORS;
    EnableTexUnit(1,FALSE);
    pglActiveTexture(GL_TEXTURE0_ARB);
    OPENGL_CHECK_ERRORS;
    EnableTexUnit(0,FALSE);
    pglActiveTexture(GL_TEXTURE0_ARB);
    OPENGL_CHECK_ERRORS;
    EnableTexUnit(0,TRUE);
}

void OGLRender::EndRendering(void)
{
    glFlush();
    OPENGL_CHECK_ERRORS;
    if( CRender::gRenderReferenceCount > 0 ) 
        CRender::gRenderReferenceCount--;
}

void OGLRender::glViewportWrapper(GLint x, GLint y, GLsizei width, GLsizei height, bool flag)
{
    static GLint mx=0,my=0;
    static GLsizei m_width=0, m_height=0;
    static bool mflag=true;

    if( x!=mx || y!=my || width!=m_width || height!=m_height || mflag!=flag)
    {
        mx=x;
        my=y;
        m_width=width;
        m_height=height;
        mflag=flag;
        glMatrixMode(GL_PROJECTION);
        OPENGL_CHECK_ERRORS;
        glLoadIdentity();
        OPENGL_CHECK_ERRORS;
        if( flag )  glOrtho(0, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight, 0, -1, 1);
        OPENGL_CHECK_ERRORS;
        glViewport(x,y,width,height);
        OPENGL_CHECK_ERRORS;
    }
}

