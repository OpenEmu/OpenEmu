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


#ifndef __SURFACEHANDLER_H__
#define __SURFACEHANDLER_H__

#include "typedefs.h"

/////////////// Define a struct to use as
///////////////  storage for all the surfaces
///////////////  created so far.
class CTexture;

typedef struct {
    unsigned short int  dwWidth;            // Describes the width of the real texture area. Use lPitch to move between successive lines
    unsigned short int  dwHeight;           // Describes the height of the real texture area
    unsigned short int  dwCreatedWidth;     // Describes the width of the created texture area. Use lPitch to move between successive lines
    unsigned short int  dwCreatedHeight;    // Describes the height of the created texture area
    int        lPitch;             // Specifies the number of bytes on each row (not necessarily bitdepth*width/8)
    void        *lpSurface;         // Pointer to the top left pixel of the image
} DrawInfo;


enum TextureFmt {
    TEXTURE_FMT_A8R8G8B8,
    TEXTURE_FMT_A4R4G4B4,
    TEXTURE_FMT_UNKNOWN,
};

enum TextureUsage {
    AS_NORMAL,
    AS_RENDER_TARGET,
    AS_BACK_BUFFER_SAVE,
};

class CTexture
{
public:
    virtual ~CTexture();

    uint32      m_dwWidth;          // The requested Texture w/h
    uint32      m_dwHeight;

    unsigned int        m_dwCreatedTextureWidth;    // What was actually created
    unsigned int        m_dwCreatedTextureHeight;

    float       m_fXScale;      // = m_dwCorrectedWidth/m_dwWidth
    float       m_fYScale;      // = m_dwCorrectedHeight/m_dwWidth

    bool        m_bScaledS;
    bool        m_bScaledT;

    bool        m_bClampedS;
    bool        m_bClampedT;

    bool        m_bIsEnhancedTexture;
    
    TextureUsage    m_Usage;

    virtual void ScaleImageToSurface(bool scaleS=true, bool scaleT=true);
    virtual void ClampImageToSurfaceS();
    virtual void ClampImageToSurfaceT();

    virtual LPRICETEXTURE GetTexture() { return m_pTexture; }

    uint32          GetPixelSize();
    TextureFmt      GetSurfaceFormat(void); // Surface pixel format...
    inline void     SetOthersVariables(void)
    {
        m_bClampedS = m_bScaledS = (m_dwWidth == m_dwCreatedTextureWidth);
        m_bClampedT = m_bScaledT = (m_dwHeight == m_dwCreatedTextureHeight);
    }

    // Provides access to "surface"
    virtual bool StartUpdate(DrawInfo *di)=0;
    virtual void EndUpdate(DrawInfo *di)=0;

    virtual void RestoreAlphaChannel(void); // Restore Alpha channel from RGB channel

protected:
    CTexture(uint32 dwWidth, uint32 dwHeight, TextureUsage usage = AS_NORMAL);
    LPRICETEXTURE   m_pTexture;
    TextureFmt      m_dwTextureFmt;
};

#endif

