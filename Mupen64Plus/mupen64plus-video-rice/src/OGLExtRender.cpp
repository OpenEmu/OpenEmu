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
#include "OGLExtRender.h"
#include "OGLTexture.h"

void COGLExtRender::Initialize(void)
{
    OGLRender::Initialize();

    // Initialize multitexture
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&m_maxTexUnits);
    OPENGL_CHECK_ERRORS;

    for( int i=0; i<8; i++ )
        m_textureUnitMap[i] = -1;
    m_textureUnitMap[0] = 0;    // T0 is usually using texture unit 0
    m_textureUnitMap[1] = 1;    // T1 is usually using texture unit 1
}


void COGLExtRender::BindTexture(GLuint texture, int unitno)
{
    if( m_bEnableMultiTexture )
    {
        if( unitno < m_maxTexUnits )
        {
            if( m_curBoundTex[unitno] != texture )
            {
                pglActiveTexture(GL_TEXTURE0_ARB+unitno);
                OPENGL_CHECK_ERRORS;
                glBindTexture(GL_TEXTURE_2D,texture);
                OPENGL_CHECK_ERRORS;
                m_curBoundTex[unitno] = texture;
            }
        }
    }
    else
    {
        OGLRender::BindTexture(texture, unitno);
    }
}

void COGLExtRender::DisBindTexture(GLuint texture, int unitno)
{
    if( m_bEnableMultiTexture )
    {
        pglActiveTexture(GL_TEXTURE0_ARB+unitno);
        OPENGL_CHECK_ERRORS;
        glBindTexture(GL_TEXTURE_2D, 0);    //Not to bind any texture
        OPENGL_CHECK_ERRORS;
    }
    else
        OGLRender::DisBindTexture(texture, unitno);
}

void COGLExtRender::TexCoord2f(float u, float v)
{
    if( m_bEnableMultiTexture )
    {
        for( int i=0; i<8; i++ )
        {
            if( m_textureUnitMap[i] >= 0 )
            {
                pglMultiTexCoord2f(GL_TEXTURE0_ARB+i, u, v);
            }
        }
    }
    else
        OGLRender::TexCoord2f(u,v);
}

void COGLExtRender::TexCoord(TLITVERTEX &vtxInfo)
{
    if( m_bEnableMultiTexture )
    {
        for( int i=0; i<8; i++ )
        {
            if( m_textureUnitMap[i] >= 0 )
            {
                pglMultiTexCoord2fv(GL_TEXTURE0_ARB+i, &(vtxInfo.tcord[m_textureUnitMap[i]].u));
            }
        }
    }
    else
        OGLRender::TexCoord(vtxInfo);
}


void COGLExtRender::SetTexWrapS(int unitno,GLuint flag)
{
    static GLuint mflag[8];
    static GLuint mtex[8];
    if( m_curBoundTex[unitno] != mtex[unitno] || mflag[unitno] != flag )
    {
        mtex[unitno] = m_curBoundTex[0];
        mflag[unitno] = flag;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, flag);
        OPENGL_CHECK_ERRORS;
    }
}
void COGLExtRender::SetTexWrapT(int unitno,GLuint flag)
{
    static GLuint mflag[8];
    static GLuint mtex[8];
    if( m_curBoundTex[unitno] != mtex[unitno] || mflag[unitno] != flag )
    {
        mtex[unitno] = m_curBoundTex[0];
        mflag[unitno] = flag;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, flag);
        OPENGL_CHECK_ERRORS;
    }
}

extern UVFlagMap OGLXUVFlagMaps[];
void COGLExtRender::SetTextureUFlag(TextureUVFlag dwFlag, uint32 dwTile)
{
    TileUFlags[dwTile] = dwFlag;
    if( !m_bEnableMultiTexture )
    {
        OGLRender::SetTextureUFlag(dwFlag, dwTile);
        return;
    }

    int tex;
    if( dwTile == gRSP.curTile )
        tex=0;
    else if( dwTile == ((gRSP.curTile+1)&7) )
        tex=1;
    else
    {
        if( dwTile == ((gRSP.curTile+2)&7) )
            tex=2;
        else if( dwTile == ((gRSP.curTile+3)&7) )
            tex=3;
        else
        {
            TRACE2("Incorrect tile number for OGL SetTextureUFlag: cur=%d, tile=%d", gRSP.curTile, dwTile);
            return;
        }
    }

    for( int textureNo=0; textureNo<8; textureNo++)
    {
        if( m_textureUnitMap[textureNo] == tex )
        {
            pglActiveTexture(GL_TEXTURE0_ARB+textureNo);
            OPENGL_CHECK_ERRORS;
            COGLTexture* pTexture = g_textures[(gRSP.curTile+tex)&7].m_pCOGLTexture;
            if( pTexture ) 
            {
                EnableTexUnit(textureNo,TRUE);
                BindTexture(pTexture->m_dwTextureName, textureNo);
            }
            SetTexWrapS(textureNo, OGLXUVFlagMaps[dwFlag].realFlag);
        }
    }
}
void COGLExtRender::SetTextureVFlag(TextureUVFlag dwFlag, uint32 dwTile)
{
    TileVFlags[dwTile] = dwFlag;
    if( !m_bEnableMultiTexture )
    {
        OGLRender::SetTextureVFlag(dwFlag, dwTile);
        return;
    }

    int tex;
    if( dwTile == gRSP.curTile )
        tex=0;
    else if( dwTile == ((gRSP.curTile+1)&7) )
        tex=1;
    else
    {
        if( dwTile == ((gRSP.curTile+2)&7) )
            tex=2;
        else if( dwTile == ((gRSP.curTile+3)&7) )
            tex=3;
        else
        {
            TRACE2("Incorrect tile number for OGL SetTextureVFlag: cur=%d, tile=%d", gRSP.curTile, dwTile);
            return;
        }
    }
    
    for( int textureNo=0; textureNo<8; textureNo++)
    {
        if( m_textureUnitMap[textureNo] == tex )
        {
            COGLTexture* pTexture = g_textures[(gRSP.curTile+tex)&7].m_pCOGLTexture;
            if( pTexture )
            {
                EnableTexUnit(textureNo,TRUE);
                BindTexture(pTexture->m_dwTextureName, textureNo);
            }
            SetTexWrapT(textureNo, OGLXUVFlagMaps[dwFlag].realFlag);
        }
    }
}

void COGLExtRender::EnableTexUnit(int unitno, BOOL flag)
{
    if( m_texUnitEnabled[unitno] != flag )
    {
        m_texUnitEnabled[unitno] = flag;
        pglActiveTexture(GL_TEXTURE0_ARB+unitno);
        OPENGL_CHECK_ERRORS;
        if( flag == TRUE )
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);
        OPENGL_CHECK_ERRORS;
    }
}

void COGLExtRender::ApplyTextureFilter()
{
    static uint32 minflag[8], magflag[8];
    static uint32 mtex[8];

    int iMinFilter, iMagFilter;

    for( int i=0; i<m_maxTexUnits; i++ )
    {
        //Compute iMinFilter and iMagFilter
        if(m_dwMinFilter == FILTER_LINEAR) //Texture will use filtering
        {
            iMagFilter = GL_LINEAR;

            //Texture filtering method user want
            switch(options.mipmapping)
            {
            case TEXTURE_BILINEAR_FILTER:
                iMinFilter = GL_LINEAR_MIPMAP_NEAREST;
                break;
            case TEXTURE_TRILINEAR_FILTER:
                iMinFilter = GL_LINEAR_MIPMAP_LINEAR;
                break;
            case TEXTURE_NO_FILTER:
                iMinFilter = GL_NEAREST_MIPMAP_NEAREST;
                break;
	    case TEXTURE_NO_MIPMAP:
            default:
                //Bilinear without mipmap
                iMinFilter = GL_LINEAR;
            }
        }
        else    //dont use filtering, all is nearest
        {
            iMagFilter = GL_NEAREST;

            if(options.mipmapping)
            {
                iMinFilter = GL_NEAREST_MIPMAP_NEAREST;
            }
            else
            {
                iMinFilter = GL_NEAREST;
            }
        }

        if( m_texUnitEnabled[i] )
        {
            if( mtex[i] != m_curBoundTex[i] )
            {
                mtex[i] = m_curBoundTex[i];
                pglActiveTexture(GL_TEXTURE0_ARB+i);
                OPENGL_CHECK_ERRORS;
                minflag[i] = m_dwMinFilter;
                magflag[i] = m_dwMagFilter;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iMinFilter);
                OPENGL_CHECK_ERRORS;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iMagFilter);
                OPENGL_CHECK_ERRORS;
            }
            else
            {
                if( minflag[i] != (unsigned int)m_dwMinFilter )
                {
                    minflag[i] = m_dwMinFilter;
                    pglActiveTexture(GL_TEXTURE0_ARB+i);
                    OPENGL_CHECK_ERRORS;
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iMinFilter);
                    OPENGL_CHECK_ERRORS;
                }
                if( magflag[i] != (unsigned int)m_dwMagFilter )
                {
                    magflag[i] = m_dwMagFilter;
                    pglActiveTexture(GL_TEXTURE0_ARB+i);
                    OPENGL_CHECK_ERRORS;
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iMagFilter);
                    OPENGL_CHECK_ERRORS;
                }
            }
        }
    }
}

void COGLExtRender::SetTextureToTextureUnitMap(int tex, int unit)
{
    if( unit < 8 && (tex >= -1 || tex <= 1))
        m_textureUnitMap[unit] = tex;
}

