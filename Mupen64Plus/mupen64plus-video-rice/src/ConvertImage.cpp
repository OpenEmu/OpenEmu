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

#include "Config.h"
#include "ConvertImage.h"
#include "RenderBase.h"

ConvertFunction     gConvertFunctions_FullTMEM[ 8 ][ 4 ] = 
{
    // 4bpp             8bpp            16bpp               32bpp
    {  Convert4b,       Convert8b,      Convert16b,         ConvertRGBA32 },        // RGBA
    {  NULL,            NULL,           ConvertYUV,         NULL },                 // YUV
    {  Convert4b,       Convert8b,      NULL,               NULL },                 // CI
    {  Convert4b,       Convert8b,      Convert16b,         NULL },                 // IA
    {  Convert4b,       Convert8b,      Convert16b,         NULL },                 // I
    {  NULL,            NULL,           NULL,               NULL },                 // ?
    {  NULL,            NULL,           NULL,               NULL },                 // ?
    {  NULL,            NULL,           NULL,               NULL }                  // ?
};
ConvertFunction     gConvertFunctions[ 8 ][ 4 ] = 
{
    // 4bpp             8bpp            16bpp               32bpp
    {  ConvertCI4,      ConvertCI8,     ConvertRGBA16,      ConvertRGBA32 },        // RGBA
    {  NULL,            NULL,           ConvertYUV,         NULL },                 // YUV
    {  ConvertCI4,      ConvertCI8,     NULL,               NULL },                 // CI
    {  ConvertIA4,      ConvertIA8,     ConvertIA16,        NULL },                 // IA
    {  ConvertI4,       ConvertI8,      ConvertIA16,        NULL },                 // I
    {  NULL,            NULL,           NULL,               NULL },                 // ?
    {  NULL,            NULL,           NULL,               NULL },                 // ?
    {  NULL,            NULL,           NULL,               NULL }                  // ?
};

ConvertFunction     gConvertTlutFunctions[ 8 ][ 4 ] = 
{
    // 4bpp             8bpp            16bpp               32bpp
    {  ConvertCI4,      ConvertCI8,     ConvertRGBA16,      ConvertRGBA32 },        // RGBA
    {  NULL,            NULL,           ConvertYUV,         NULL },                 // YUV
    {  ConvertCI4,      ConvertCI8,     NULL,               NULL },                 // CI
    {  ConvertCI4,      ConvertCI8,     ConvertIA16,        NULL },                 // IA
    {  ConvertCI4,      ConvertCI8,     ConvertIA16,        NULL },                 // I
    {  NULL,            NULL,           NULL,               NULL },                 // ?
    {  NULL,            NULL,           NULL,               NULL },                 // ?
    {  NULL,            NULL,           NULL,               NULL }                  // ?
};

extern bool conkerSwapHack;

void ConvertRGBA16(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;

    // Copy of the base pointer
    uint16 * pSrc = (uint16*)(tinfo.pPhysicalAddress);

    uint8 * pByteSrc = (uint8 *)pSrc;
    if (!pTexture->StartUpdate(&dInfo))
        return;

    uint32 nFiddle;

    if (tinfo.bSwapped)
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            if ((y&1) == 0)
                nFiddle = 0x2;
            else
                nFiddle = 0x2 | 0x4;

            // dwDst points to start of destination row
            uint32 * dwDst = (uint32 *)((uint8 *)dInfo.lpSurface + y*dInfo.lPitch);

            // DWordOffset points to the current dword we're looking at
            // (process 2 pixels at a time). May be a problem if we don't start on even pixel
            uint32 dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint16 w = *(uint16 *)&pByteSrc[dwWordOffset ^ nFiddle];

                dwDst[x] = Convert555ToRGBA(w);
                
                // Increment word offset to point to the next two pixels
                dwWordOffset += 2;
            }
        }
    }
    else
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            // dwDst points to start of destination row
            uint32 * dwDst = (uint32 *)((uint8 *)dInfo.lpSurface + y*dInfo.lPitch);

            // DWordOffset points to the current dword we're looking at
            // (process 2 pixels at a time). May be a problem if we don't start on even pixel
            uint32 dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint16 w = *(uint16 *)&pByteSrc[dwWordOffset ^ 0x2];

                dwDst[x] = Convert555ToRGBA(w);
                
                // Increment word offset to point to the next two pixels
                dwWordOffset += 2;
            }
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

void ConvertRGBA32(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    if (!pTexture->StartUpdate(&dInfo))
        return;

    uint32 * pSrc = (uint32*)(tinfo.pPhysicalAddress);

    if( options.bUseFullTMEM )
    {
        Tile &tile = gRDP.tiles[tinfo.tileNo];

        uint32 *pWordSrc;
        if( tinfo.tileNo >= 0 )
        {
            pWordSrc = (uint32*)&g_Tmem.g_Tmem64bit[tile.dwTMem];

            for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
            {
                uint32 * dwDst = (uint32 *)((uint8 *)dInfo.lpSurface + y*dInfo.lPitch);

                uint32 nFiddle = ( y&1 )? 0x2 : 0;
                int idx = tile.dwLine*4*y;

                for (uint32 x = 0; x < tinfo.WidthToLoad; x++, idx++)
                {
                    uint32 w = pWordSrc[idx^nFiddle];
                    uint8* psw = (uint8*)&w;
                    uint8* pdw = (uint8*)&dwDst[x];
                    pdw[0] = psw[2];    // Blue
                    pdw[1] = psw[1];    // Green
                    pdw[2] = psw[0];    // Red
                    pdw[3] = psw[3];    // Alpha
                }
            }
        }
    }
    else
    {

        if (tinfo.bSwapped)
        {
            for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
            {
                if ((y%2) == 0)
                {

                    uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;
                    uint8 *pS = (uint8 *)pSrc + (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);

                    for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
                    {
                        pDst[0] = pS[1];    // Blue
                        pDst[1] = pS[2];    // Green
                        pDst[2] = pS[3];    // Red
                        pDst[3] = pS[0];    // Alpha
                        pS+=4;
                        pDst+=4;
                    }
                }
                else
                {

                    uint32 *pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);
                    uint8 *pS = (uint8 *)pSrc;
                    int n;

                    n = (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);
                    for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
                    {
                        *pDst++ = COLOR_RGBA(pS[(n+3)^0x8],
                            pS[(n+2)^0x8],
                            pS[(n+1)^0x8],
                            pS[(n+0)^0x8]);

                        n += 4;
                    }
                }
            }
        }
        else
        {
            for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
            {
                uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;
                uint8 *pS = (uint8 *)pSrc + (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);

                for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
                {
                    pDst[0] = pS[1];    // Blue
                    pDst[1] = pS[2];    // Green
                    pDst[2] = pS[3];    // Red
                    pDst[3] = pS[0];    // Alpha
                    pS+=4;
                    pDst+=4;
                }
            }

        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

// E.g. Dear Mario text
// Copy, Score etc
void ConvertIA4(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint8 * pSrc = (uint8*)(tinfo.pPhysicalAddress);

#ifdef DEBUGGER
    if (((long)pSrc) % 4) TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

    if (!pTexture->StartUpdate(&dInfo))
        return;

    if (tinfo.bSwapped)
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;

            // For odd lines, swap words too
            if ((y%2) == 0)
                nFiddle = 0x3;
            else
                nFiddle = 0x7;


            // This may not work if X is not even?
            uint32 dwByteOffset = (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad/2);

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ nFiddle];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = OneToEight[(b & 0x10) >> 4];  
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // Do two pixels at a time
                uint8 b = pSrc[dwByteOffset ^ nFiddle];

                // Even
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = OneToEight[(b & 0x10) >> 4];  
                // Odd
                *pDst++ = ThreeToEight[(b & 0x0E) >> 1];
                *pDst++ = ThreeToEight[(b & 0x0E) >> 1];
                *pDst++ = ThreeToEight[(b & 0x0E) >> 1];
                *pDst++ = OneToEight[(b & 0x01)     ];

                dwByteOffset++;
            }

        }
    }
    else
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + (y * dInfo.lPitch);

            // This may not work if X is not even?
            uint32 dwByteOffset = (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad/2);

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ 0x3];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = OneToEight[(b & 0x10) >> 4];  
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // Do two pixels at a time
                uint8 b = pSrc[dwByteOffset ^ 0x3];

                // Even
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = ThreeToEight[(b & 0xE0) >> 5];
                *pDst++ = OneToEight[(b & 0x10) >> 4];  
                // Odd
                *pDst++ = ThreeToEight[(b & 0x0E) >> 1];
                *pDst++ = ThreeToEight[(b & 0x0E) >> 1];
                *pDst++ = ThreeToEight[(b & 0x0E) >> 1];
                *pDst++ = OneToEight[(b & 0x01)     ];

                dwByteOffset++;

            }
        }   
    }
    
    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();

}

// E.g Mario's head textures
void ConvertIA8(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint8 * pSrc = (uint8*)(tinfo.pPhysicalAddress);

#ifdef DEBUGGER
    if (((long)pSrc) % 4) TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

    if (!pTexture->StartUpdate(&dInfo))
        return;


    if (tinfo.bSwapped)
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            // For odd lines, swap words too
            if ((y%2) == 0)
                nFiddle = 0x3;
            else
                nFiddle = 0x7;


            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;
            // Points to current byte
            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint8 b = pSrc[dwByteOffset ^ nFiddle];
                uint8 I = FourToEight[(b & 0xf0)>>4];

                *pDst++ = I;
                *pDst++ = I;
                *pDst++ = I;
                *pDst++ = FourToEight[(b & 0x0f)   ];

                dwByteOffset++;
            }
        }       
    }
    else
    {
        register const uint8* FourToEightArray = &FourToEight[0];
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;


            // Points to current byte
            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                register uint8 b = pSrc[(dwByteOffset++) ^ 0x3];
                uint8 I = *(FourToEightArray+(b>>4));

                *pDst++ = I;
                *pDst++ = I;
                *pDst++ = I;
                *pDst++ = *(FourToEightArray+(b&0xF));
            }
        }
    }   
    
    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();

}

// E.g. camera's clouds, shadows
void ConvertIA16(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint16 * pSrc = (uint16*)(tinfo.pPhysicalAddress);
    uint8 * pByteSrc = (uint8 *)pSrc;

    if (!pTexture->StartUpdate(&dInfo))
        return;


    if (tinfo.bSwapped)
    {
        
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;

            if ((y%2) == 0)
                nFiddle = 0x2;
            else
                nFiddle = 0x4 | 0x2;

            // Points to current word
            uint32 dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint16 w = *(uint16 *)&pByteSrc[dwWordOffset^nFiddle];

                *pDst++ = (uint8)(w >> 8);
                *pDst++ = (uint8)(w >> 8);
                *pDst++ = (uint8)(w >> 8);
                *pDst++ = (uint8)(w & 0xFF);

                dwWordOffset += 2;
            }
        }       
    }
    else
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;

            // Points to current word
            uint32 dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint16 w = *(uint16 *)&pByteSrc[dwWordOffset^0x2];

                *pDst++ = (uint8)(w >> 8);
                *pDst++ = (uint8)(w >> 8);
                *pDst++ = (uint8)(w >> 8);
                *pDst++ = (uint8)(w & 0xFF);

                dwWordOffset += 2;
            }
        }       
    }


    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}



// Used by MarioKart
void ConvertI4(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint8 * pSrc = (uint8*)(tinfo.pPhysicalAddress);

#ifdef DEBUGGER
    if (((long) pSrc) % 4) TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

    if (!pTexture->StartUpdate(&dInfo))
        return;

    if (tinfo.bSwapped)
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;

            // Might not work with non-even starting X
            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

            // For odd lines, swap words too
            if( !conkerSwapHack || (y&4) == 0 )
            {
                if ((y%2) == 0)
                    nFiddle = 0x3;
                else
                    nFiddle = 0x7;
            }
            else
            {
                if ((y%2) == 1)
                    nFiddle = 0x3;
                else
                    nFiddle = 0x7;
            }

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ nFiddle];
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];   
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // two pixels at a time
                uint8 b = pSrc[dwByteOffset ^ nFiddle];

                // Even
                *pDst++ = FourToEight[(b & 0xF0)>>4];   // Other implementations seem to or in (b&0xF0)>>4
                *pDst++ = FourToEight[(b & 0xF0)>>4]; // why?
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];   
                // Odd
                *pDst++ = FourToEight[(b & 0x0F)];
                *pDst++ = FourToEight[(b & 0x0F)];
                *pDst++ = FourToEight[(b & 0x0F)];
                *pDst++ = FourToEight[(b & 0x0F)];

                dwByteOffset++;
            }

        }   

        conkerSwapHack = false;
    }
    else
    {

        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;

            // Might not work with non-even starting X
            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ 0x3];
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];   
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // two pixels at a time
                uint8 b = pSrc[dwByteOffset ^ 0x3];

                // Even
                *pDst++ = FourToEight[(b & 0xF0)>>4];   // Other implementations seem to or in (b&0xF0)>>4
                *pDst++ = FourToEight[(b & 0xF0)>>4]; // why?
                *pDst++ = FourToEight[(b & 0xF0)>>4];
                *pDst++ = FourToEight[(b & 0xF0)>>4];   
                // Odd
                *pDst++ = FourToEight[(b & 0x0F)];
                *pDst++ = FourToEight[(b & 0x0F)];
                *pDst++ = FourToEight[(b & 0x0F)];
                *pDst++ = FourToEight[(b & 0x0F)];

                dwByteOffset++;
            }
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

// Used by MarioKart
void ConvertI8(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    long pSrc = (long) tinfo.pPhysicalAddress;
    if (!pTexture->StartUpdate(&dInfo))
        return;


    if (tinfo.bSwapped)
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            if ((y%2) == 0)
                nFiddle = 0x3;
            else
                nFiddle = 0x7;

            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint8 b = *(uint8*)((pSrc+dwByteOffset)^nFiddle);

                *pDst++ = b;
                *pDst++ = b;
                *pDst++ = b;
                *pDst++ = b;        // Alpha not 255?

                dwByteOffset++;
            }
        }   
    }
    else
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint8 *pDst = (uint8 *)dInfo.lpSurface + y * dInfo.lPitch;

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint8 b = *(uint8*)((pSrc+dwByteOffset)^0x3);

                *pDst++ = b;
                *pDst++ = b;
                *pDst++ = b;
                *pDst++ = b;        // Alpha not 255?

                dwByteOffset++;
            }
        }   
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();

}

//*****************************************************************************
// Convert CI4 images. We need to switch on the palette type
//*****************************************************************************
void    ConvertCI4( CTexture * p_texture, const TxtrInfo & tinfo )
{
    if ( tinfo.TLutFmt == TLUT_FMT_RGBA16 )
    {
        ConvertCI4_RGBA16( p_texture, tinfo );  
    }
    else if ( tinfo.TLutFmt == TLUT_FMT_IA16 )
    {
        ConvertCI4_IA16( p_texture, tinfo );                    
    }
}

//*****************************************************************************
// Convert CI8 images. We need to switch on the palette type
//*****************************************************************************
void    ConvertCI8( CTexture * p_texture, const TxtrInfo & tinfo )
{
    if ( tinfo.TLutFmt == TLUT_FMT_RGBA16 )
    {
        ConvertCI8_RGBA16( p_texture, tinfo );  
    }
    else if ( tinfo.TLutFmt == TLUT_FMT_IA16 )
    {
        ConvertCI8_IA16( p_texture, tinfo );                    
    }
}

// Used by Starfox intro
void ConvertCI4_RGBA16(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint8 * pSrc = (uint8*)(tinfo.pPhysicalAddress);
    uint16 * pPal = (uint16 *)tinfo.PalAddress;
    bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);
    
    if (!pTexture->StartUpdate(&dInfo))
        return;

    if (tinfo.bSwapped)
    {

        for (uint32 y = 0; y <  tinfo.HeightToLoad; y++)
        {
            if ((y%2) == 0)
                nFiddle = 0x3;
            else
                nFiddle = 0x7;


            uint32 * pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch);

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ nFiddle];
                uint8 bhi = (b&0xf0)>>4;
                *pDst = Convert555ToRGBA(pPal[bhi^1]);    // Remember palette is in different endian order!
                if( bIgnoreAlpha )
                {
                    *pDst |= 0xFF000000;
                }
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // two at a time
                uint8 b = pSrc[dwByteOffset ^ nFiddle];

                uint8 bhi = (b&0xf0)>>4;
                uint8 blo = (b&0x0f);

                pDst[0] = Convert555ToRGBA(pPal[bhi^1]);    // Remember palette is in different endian order!
                pDst[1] = Convert555ToRGBA(pPal[blo^1]);    // Remember palette is in different endian order!

                if( bIgnoreAlpha )
                {
                    pDst[0] |= 0xFF000000;
                    pDst[1] |= 0xFF000000;
                }

                pDst+=2;

                dwByteOffset++;
            }
        }   
        
    }
    else
    {

        for (uint32 y = 0; y <  tinfo.HeightToLoad; y++)
        {
            uint32 * pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ 0x3];
                uint8 bhi = (b&0xf0)>>4;
                *pDst = Convert555ToRGBA(pPal[bhi^1]);    // Remember palette is in different endian order!
                if( bIgnoreAlpha )
                {
                    *pDst |= 0xFF000000;
                }
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // two at a time
                uint8 b = pSrc[dwByteOffset ^ 0x3];

                uint8 bhi = (b&0xf0)>>4;
                uint8 blo = (b&0x0f);

                pDst[0] = Convert555ToRGBA(pPal[bhi^1]);    // Remember palette is in different endian order!
                pDst[1] = Convert555ToRGBA(pPal[blo^1]);    // Remember palette is in different endian order!
                
                if( bIgnoreAlpha )
                {
                    pDst[0] |= 0xFF000000;
                    pDst[1] |= 0xFF000000;
                }

                pDst+=2;

                dwByteOffset++;
            }
        }   
    }
    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

// Used by Starfox intro
void ConvertCI4_IA16(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint8 * pSrc = (uint8*)(tinfo.pPhysicalAddress);

#ifdef DEBUGGER
    if (((long) pSrc) % 4) TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif


    uint16 * pPal = (uint16 *)tinfo.PalAddress;
    bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);

    if (!pTexture->StartUpdate(&dInfo))
        return;

    if (tinfo.bSwapped)
    {

        for (uint32 y = 0; y <  tinfo.HeightToLoad; y++)
        {
            if ((y%2) == 0)
                nFiddle = 0x3;
            else
                nFiddle = 0x7;


            uint32 * pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ nFiddle];
                uint8 bhi = (b&0xf0)>>4;
                *pDst = ConvertIA16ToRGBA(pPal[bhi^1]);   // Remember palette is in different endian order!
                if( bIgnoreAlpha )
                    *pDst |= 0xFF000000;
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // two at a time
                uint8 b = pSrc[dwByteOffset ^ nFiddle];

                uint8 bhi = (b&0xf0)>>4;
                uint8 blo = (b&0x0f);

                pDst[0] = ConvertIA16ToRGBA(pPal[bhi^1]);   // Remember palette is in different endian order!
                pDst[1] = ConvertIA16ToRGBA(pPal[blo^1]);   // Remember palette is in different endian order!
                
                if( bIgnoreAlpha )
                {
                    pDst[0] |= 0xFF000000;
                    pDst[1] |= 0xFF000000;
                }

                pDst+=2;

                dwByteOffset++;
            }
        }   
        
    }
    else
    {

        for (uint32 y = 0; y <  tinfo.HeightToLoad; y++)
        {
            uint32 * pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

            if (tinfo.WidthToLoad == 1)
            {
                // corner case
                uint8 b = pSrc[dwByteOffset ^ 0x3];
                uint8 bhi = (b&0xf0)>>4;
                *pDst = ConvertIA16ToRGBA(pPal[bhi^1]);   // Remember palette is in different endian order!
                if( bIgnoreAlpha )
                    *pDst |= 0xFF000000;
            }
            else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2)
            {
                // two pixels at a time
                uint8 b = pSrc[dwByteOffset ^ 0x3];

                uint8 bhi = (b&0xf0)>>4;
                uint8 blo = (b&0x0f);

                pDst[0] = ConvertIA16ToRGBA(pPal[bhi^1]);   // Remember palette is in different endian order!
                pDst[1] = ConvertIA16ToRGBA(pPal[blo^1]);   // Remember palette is in different endian order!
                
                if( bIgnoreAlpha )
                {
                    pDst[0] |= 0xFF000000;
                    pDst[1] |= 0xFF000000;
                }

                pDst+=2;

                dwByteOffset++;
            }
        }   
    }
    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}




// Used by MarioKart for Cars etc
void ConvertCI8_RGBA16(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint8 * pSrc = (uint8*)(tinfo.pPhysicalAddress);

#ifdef DEBUGGER
    if (((long) pSrc) % 4) TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

    uint16 * pPal = (uint16 *)tinfo.PalAddress;
    bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);

    if (!pTexture->StartUpdate(&dInfo))
        return;
    
    if (tinfo.bSwapped)
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            if ((y%2) == 0)
                nFiddle = 0x3;
            else
                nFiddle = 0x7;

            uint32 *pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
            
            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint8 b = pSrc[dwByteOffset ^ nFiddle];

                *pDst++ = Convert555ToRGBA(pPal[b^1]);  // Remember palette is in different endian order!
                
                if( bIgnoreAlpha )
                {
                    *(pDst-1) |= 0xFF000000;
                }


                dwByteOffset++;
            }
        }   
    

    }
    else
    {

        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint32 *pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            int dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
            
            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint8 b = pSrc[dwByteOffset ^ 0x3];

                *pDst++ = Convert555ToRGBA(pPal[b^1]);  // Remember palette is in different endian order!
                if( bIgnoreAlpha )
                {
                    *(pDst-1) |= 0xFF000000;
                }

                dwByteOffset++;
            }
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();

}



// Used by MarioKart for Cars etc
void ConvertCI8_IA16(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    uint32 nFiddle;

    uint8 * pSrc = (uint8*)(tinfo.pPhysicalAddress);

#ifdef DEBUGGER
    if (((long) pSrc) % 4) TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

    uint16 * pPal = (uint16 *)tinfo.PalAddress;
    bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);

    if (!pTexture->StartUpdate(&dInfo))
        return;

    if (tinfo.bSwapped)
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            if ((y%2) == 0)
                nFiddle = 0x3;
            else
                nFiddle = 0x7;

            uint32 *pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
            
            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint8 b = pSrc[dwByteOffset ^ nFiddle];

                *pDst++ = ConvertIA16ToRGBA(pPal[b^1]); // Remember palette is in different endian order!
                if( bIgnoreAlpha )
                {
                    *(pDst-1) |= 0xFF000000;
                }

                dwByteOffset++;
            }
        }   
    }
    else
    {
        for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
        {
            uint32 *pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

            uint32 dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
            
            for (uint32 x = 0; x < tinfo.WidthToLoad; x++)
            {
                uint8 b = pSrc[dwByteOffset ^ 0x3];

                *pDst++ = ConvertIA16ToRGBA(pPal[b^1]); // Remember palette is in different endian order!
                if( bIgnoreAlpha )
                {
                    *(pDst-1) |= 0xFF000000;
                }

                dwByteOffset++;
            }
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

void ConvertYUV(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    if (!pTexture->StartUpdate(&dInfo))
        return;


    uint32 x, y;
    uint32 nFiddle;

    if( options.bUseFullTMEM )
    {
        Tile &tile = gRDP.tiles[tinfo.tileNo];

        uint16 * pSrc;
        if( tinfo.tileNo >= 0 )
            pSrc = (uint16*)&g_Tmem.g_Tmem64bit[tile.dwTMem];
        else
            pSrc = (uint16*)(tinfo.pPhysicalAddress);

        uint8 * pByteSrc = (uint8 *)pSrc;

        for (y = 0; y < tinfo.HeightToLoad; y++)
        {
            nFiddle = ( y&1 )? 0x4 : 0;
            int dwWordOffset = tinfo.tileNo>=0? tile.dwLine*8*y : ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);
            uint32 * dwDst = (uint32 *)((uint8 *)dInfo.lpSurface + y*dInfo.lPitch);

            for (x = 0; x < tinfo.WidthToLoad/2; x++)
            {
                int y0 = *(uint8*)&pByteSrc[(dwWordOffset+1)^nFiddle];
                int y1 = *(uint8*)&pByteSrc[(dwWordOffset+3)^nFiddle];
                int u0 = *(uint8*)&pByteSrc[(dwWordOffset  )^nFiddle];
                int v0 = *(uint8*)&pByteSrc[(dwWordOffset+2)^nFiddle];

                dwDst[x*2+0] = ConvertYUV16ToR8G8B8(y0,u0,v0);
                dwDst[x*2+1] = ConvertYUV16ToR8G8B8(y1,u0,v0);

                dwWordOffset += 4;
            }
        }
    }
    else
    {
        uint16 * pSrc = (uint16*)(tinfo.pPhysicalAddress);
        uint8 * pByteSrc = (uint8 *)pSrc;

        if (tinfo.bSwapped)
        {
            for (y = 0; y < tinfo.HeightToLoad; y++)
            {
                if ((y&1) == 0)
                    nFiddle = 0x3;
                else
                    nFiddle = 0x7;

                // dwDst points to start of destination row
                uint32 * dwDst = (uint32 *)((uint8 *)dInfo.lpSurface + y*dInfo.lPitch);

                // DWordOffset points to the current dword we're looking at
                // (process 2 pixels at a time). May be a problem if we don't start on even pixel
                uint32 dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

                for (x = 0; x < tinfo.WidthToLoad/2; x++)
                {
                    int y0 = *(uint8*)&pByteSrc[(dwWordOffset  )^nFiddle];
                    int v0 = *(uint8*)&pByteSrc[(dwWordOffset+1)^nFiddle];
                    int y1 = *(uint8*)&pByteSrc[(dwWordOffset+2)^nFiddle];
                    int u0 = *(uint8*)&pByteSrc[(dwWordOffset+3)^nFiddle];

                    dwDst[x*2+0] = ConvertYUV16ToR8G8B8(y0,u0,v0);
                    dwDst[x*2+1] = ConvertYUV16ToR8G8B8(y1,u0,v0);

                    dwWordOffset += 4;
                }
            }
        }
        else
        {
            for (y = 0; y < tinfo.HeightToLoad; y++)
            {
                // dwDst points to start of destination row
                uint32 * dwDst = (uint32 *)((uint8 *)dInfo.lpSurface + y*dInfo.lPitch);
                uint32 dwByteOffset = y * 32;

                for (x = 0; x < tinfo.WidthToLoad/2; x++)
                {
                    int y0 = *(uint8*)&pByteSrc[dwByteOffset];
                    int v0 = *(uint8*)&pByteSrc[dwByteOffset+1];
                    int y1 = *(uint8*)&pByteSrc[dwByteOffset+2];
                    int u0 = *(uint8*)&pByteSrc[dwByteOffset+3];

                    dwDst[x*2+0] = ConvertYUV16ToR8G8B8(y0,u0,v0);
                    dwDst[x*2+1] = ConvertYUV16ToR8G8B8(y1,u0,v0);

                    // Increment word offset to point to the next two pixels
                    dwByteOffset += 4;
                }
            }
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

uint32 ConvertYUV16ToR8G8B8(int Y, int U, int V)
{
    uint32 A= 1;

    /*
    int R = int(g_convc0 *(Y-16) + g_convc1 * V);
    int G = int(g_convc0 *(Y-16) + g_convc2 * U - g_convc3 * V);
    int B = int(g_convc0 *(Y-16) + g_convc4 * U);
    */

    int R = int(Y + (1.370705f * (V-128)));
    int G = int(Y - (0.698001f * (V-128)) - (0.337633f * (U-128)));
    int B = int(Y + (1.732446f * (U-128)));

    R = R<0 ? 0 : R;
    G = G<0 ? 0 : G;
    B = B<0 ? 0 : B;

    uint32 R2 = R>255 ? 255 : R;
    uint32 G2 = G>255 ? 255 : G;
    uint32 B2 = B>255 ? 255 : B;

    return COLOR_RGBA(R2, G2, B2, 0xFF*A);
}




// Used by Starfox intro
void Convert4b(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;

    if (!pTexture->StartUpdate(&dInfo)) 
        return;

    uint16 * pPal = (uint16 *)tinfo.PalAddress;
    bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);
    if( tinfo.Format <= TXT_FMT_CI ) bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);

    Tile &tile = gRDP.tiles[tinfo.tileNo];

    uint8 *pByteSrc = tinfo.tileNo >= 0 ? (uint8*)&g_Tmem.g_Tmem64bit[tile.dwTMem] : (uint8*)(tinfo.pPhysicalAddress);

    for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
    {
        uint32 nFiddle;
        if( tinfo.tileNo < 0 )  
        {
            if (tinfo.bSwapped)
            {
                if ((y%2) == 0)
                    nFiddle = 0x3;
                else
                    nFiddle = 0x7;
            }
            else
            {
                nFiddle = 3;
            }
        }
        else
        {
            nFiddle = ( y&1 )? 0x4 : 0;
        }

        uint32 * pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);
        int idx = tinfo.tileNo>=0 ? tile.dwLine*8*y : ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

        if (tinfo.WidthToLoad == 1)
        {
            // corner case
            uint8 b = pByteSrc[idx^nFiddle];
            uint8 bhi = (b&0xf0)>>4;
            if( gRDP.otherMode.text_tlut>=2 || ( tinfo.Format != TXT_FMT_IA && tinfo.Format != TXT_FMT_I) )
            {
                if( tinfo.TLutFmt == TLUT_FMT_IA16 )
                {
                    if( tinfo.tileNo>=0 )
                        *pDst = ConvertIA16ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
                    else
                        *pDst = ConvertIA16ToRGBA(pPal[bhi^1]);
                }
                else
                {
                    if( tinfo.tileNo>=0 )
                        *pDst = Convert555ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
                    else
                        *pDst = Convert555ToRGBA(pPal[bhi^1]);
                }
            }
            else if( tinfo.Format == TXT_FMT_IA )
                *pDst = ConvertIA4ToRGBA(b>>4);
            else    // if( tinfo.Format == TXT_FMT_I )
                *pDst = ConvertI4ToRGBA(b>>4);
            if( bIgnoreAlpha )
                *pDst |= 0xFF000000;
        }
        else for (uint32 x = 0; x < tinfo.WidthToLoad; x+=2, idx++)
        {
            // two pixels at a time
            uint8 b = pByteSrc[idx^nFiddle];
            uint8 bhi = (b&0xf0)>>4;
            uint8 blo = (b&0x0f);

            if( gRDP.otherMode.text_tlut>=2 || ( tinfo.Format != TXT_FMT_IA && tinfo.Format != TXT_FMT_I) )
            {
                if( tinfo.TLutFmt == TLUT_FMT_IA16 )
                {
                    if( tinfo.tileNo>=0 )
                    {
                        pDst[0] = ConvertIA16ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
                        pDst[1] = ConvertIA16ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(blo<<2)]);
                    }
                    else
                    {
                        pDst[0] = ConvertIA16ToRGBA(pPal[bhi^1]);
                        pDst[1] = ConvertIA16ToRGBA(pPal[blo^1]);
                    }
                }
                else
                {
                    if( tinfo.tileNo>=0 )
                    {
                        pDst[0] = Convert555ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
                        pDst[1] = Convert555ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(blo<<2)]);
                    }
                    else
                    {
                        pDst[0] = Convert555ToRGBA(pPal[bhi^1]);
                        pDst[1] = Convert555ToRGBA(pPal[blo^1]);
                    }
                }
            }
            else if( tinfo.Format == TXT_FMT_IA )
            {
                pDst[0] = ConvertIA4ToRGBA(b>>4);
                pDst[1] = ConvertIA4ToRGBA(b&0xF);
            }
            else    // if( tinfo.Format == TXT_FMT_I )
            {
                pDst[0] = ConvertI4ToRGBA(b>>4);
                pDst[1] = ConvertI4ToRGBA(b&0xF);
            }

            if( bIgnoreAlpha )
            {
                pDst[0] |= 0xFF000000;
                pDst[1] |= 0xFF000000;
            }
            pDst+=2;
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

void Convert8b(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    if (!pTexture->StartUpdate(&dInfo)) 
        return;

    uint16 * pPal = (uint16 *)tinfo.PalAddress;
    bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);
    if( tinfo.Format <= TXT_FMT_CI ) bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);

    Tile &tile = gRDP.tiles[tinfo.tileNo];

    uint8 *pByteSrc;
    if( tinfo.tileNo >= 0 )
    {
        pByteSrc = (uint8*)&g_Tmem.g_Tmem64bit[tile.dwTMem];
    }
    else
    {
        pByteSrc = (uint8*)(tinfo.pPhysicalAddress);
    }


    for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
    {
        uint32 * pDst = (uint32 *)((uint8 *)dInfo.lpSurface + y * dInfo.lPitch);

        uint32 nFiddle;
        if( tinfo.tileNo < 0 )  
        {
            if (tinfo.bSwapped)
            {
                if ((y%2) == 0)
                    nFiddle = 0x3;
                else
                    nFiddle = 0x7;
            }
            else
            {
                nFiddle = 3;
            }
        }
        else
        {
            nFiddle = ( y&1 )? 0x4 : 0;
        }


        int idx = tinfo.tileNo>=0? tile.dwLine*8*y : ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

        for (uint32 x = 0; x < tinfo.WidthToLoad; x++, idx++)
        {
            uint8 b = pByteSrc[idx^nFiddle];

            if( gRDP.otherMode.text_tlut>=2 || ( tinfo.Format != TXT_FMT_IA && tinfo.Format != TXT_FMT_I) )
            {
                if( tinfo.TLutFmt == TLUT_FMT_IA16 )
                {
                    if( tinfo.tileNo>=0 )
                        *pDst = ConvertIA16ToRGBA(g_Tmem.g_Tmem16bit[0x400+(b<<2)]);
                    else
                        *pDst = ConvertIA16ToRGBA(pPal[b^1]);
                }
                else
                {
                    if( tinfo.tileNo>=0 )
                        *pDst = Convert555ToRGBA(g_Tmem.g_Tmem16bit[0x400+(b<<2)]);
                    else
                        *pDst = Convert555ToRGBA(pPal[b^1]);
                }
            }
            else if( tinfo.Format == TXT_FMT_IA )
            {
                uint8 I = FourToEight[(b & 0xf0)>>4];
                uint8 * pByteDst = (uint8*)pDst;
                pByteDst[0] = I;
                pByteDst[1] = I;
                pByteDst[2] = I;
                pByteDst[3] = FourToEight[(b & 0x0f)   ];
            }
            else    // if( tinfo.Format == TXT_FMT_I )
            {
                uint8 * pByteDst = (uint8*)pDst;
                pByteDst[0] = b;
                pByteDst[1] = b;
                pByteDst[2] = b;
                pByteDst[3] = b;
            }

            if( bIgnoreAlpha )
            {
                *pDst |= 0xFF000000;
            }
            pDst++;
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}


void Convert16b(CTexture *pTexture, const TxtrInfo &tinfo)
{
    DrawInfo dInfo;
    if (!pTexture->StartUpdate(&dInfo)) 
        return;

    Tile &tile = gRDP.tiles[tinfo.tileNo];

    uint16 *pWordSrc;
    if( tinfo.tileNo >= 0 )
        pWordSrc = (uint16*)&g_Tmem.g_Tmem64bit[tile.dwTMem];
    else
        pWordSrc = (uint16*)(tinfo.pPhysicalAddress);


    for (uint32 y = 0; y < tinfo.HeightToLoad; y++)
    {
        uint32 * dwDst = (uint32 *)((uint8 *)dInfo.lpSurface + y*dInfo.lPitch);

        uint32 nFiddle;
        if( tinfo.tileNo < 0 )  
        {
            if (tinfo.bSwapped)
            {
                if ((y&1) == 0)
                    nFiddle = 0x1;
                else
                    nFiddle = 0x3;
            }
            else
            {
                nFiddle = 0x1;
            }
        }
        else
        {
            nFiddle = ( y&1 )? 0x2 : 0;
        }


        int idx = tinfo.tileNo>=0? tile.dwLine*4*y : (((y+tinfo.TopToLoad) * tinfo.Pitch)>>1) + tinfo.LeftToLoad;

        for (uint32 x = 0; x < tinfo.WidthToLoad; x++, idx++)
        {
            uint16 w = pWordSrc[idx^nFiddle];
            uint16 w2 = tinfo.tileNo>=0? ((w>>8)|(w<<8)) : w;

            if( tinfo.Format == TXT_FMT_RGBA )
            {
                dwDst[x] = Convert555ToRGBA(w2);
            }
            else if( tinfo.Format == TXT_FMT_YUV )
            {
            }
            else if( tinfo.Format >= TXT_FMT_IA )
            {
                uint8 * pByteDst = (uint8*)&dwDst[x];
                *pByteDst++ = (uint8)(w2 >> 8);
                *pByteDst++ = (uint8)(w2 >> 8);
                *pByteDst++ = (uint8)(w2 >> 8);
                *pByteDst++ = (uint8)(w2 & 0xFF);
            }
        }
    }

    pTexture->EndUpdate(&dInfo);
    pTexture->SetOthersVariables();
}

