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

#include "osal_files.h"

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_plugin.h"
#include "typedefs.h"
#include "ConvertImage.h"
#include "DeviceBuilder.h"
#include "TextureFilters.h"
#include "Render.h"
#include "Video.h"

#include "liblinux/BMGLibPNG.h"
#include "liblinux/BMGDLL.h"
#include <sys/types.h>
#include <algorithm>

#ifdef min
#undef min
#endif

/************************************************************************/
/* 2X filters                                                           */
/************************************************************************/
// Basic 2x R8G8B8A8 filter with interpolation

void Texture2x_32( DrawInfo &srcInfo, DrawInfo &destInfo)
{
    uint32 *pDst1, *pDst2;
    uint32 *pSrc, *pSrc2;
    uint32 nWidth = srcInfo.dwWidth;
    uint32 nHeight = srcInfo.dwHeight;

    uint32 b1;
    uint32 g1;
    uint32 r1;
    uint32 a1;
    uint32 b2 = 0;
    uint32 g2 = 0;
    uint32 r2 = 0;
    uint32 a2 = 0;
    uint32 b3 = 0;
    uint32 g3 = 0;
    uint32 r3 = 0;
    uint32 a3 = 0;
    uint32 b4 = 0;
    uint32 g4 = 0;
    uint32 r4 = 0;
    uint32 a4 = 0;


    for (uint32 ySrc = 0; ySrc < nHeight; ySrc++)
    {
        pSrc = (uint32*)(((uint8*)srcInfo.lpSurface)+ySrc*srcInfo.lPitch);
        pSrc2 = (uint32*)(((uint8*)srcInfo.lpSurface)+(ySrc+1)*srcInfo.lPitch);
        pDst1 = (uint32*)(((uint8*)destInfo.lpSurface)+(ySrc*2)*destInfo.lPitch);
        pDst2 = (uint32*)(((uint8*)destInfo.lpSurface)+(ySrc*2+1)*destInfo.lPitch);

        for (uint32 xSrc = 0; xSrc < nWidth; xSrc++)
        {
            b1 = (pSrc[xSrc]>>0)&0xFF;
            g1 = (pSrc[xSrc]>>8)&0xFF;
            r1 = (pSrc[xSrc]>>16)&0xFF;
            a1 = (pSrc[xSrc]>>24)&0xFF;

            if( xSrc<nWidth-1 )
            {
                b2 = (pSrc[xSrc+1]>>0)&0xFF;
                g2 = (pSrc[xSrc+1]>>8)&0xFF;
                r2 = (pSrc[xSrc+1]>>16)&0xFF;
                a2 = (pSrc[xSrc+1]>>24)&0xFF;
            }

            if( ySrc<nHeight-1 )
            {
                b3 = (pSrc2[xSrc]>>0)&0xFF;
                g3 = (pSrc2[xSrc]>>8)&0xFF;
                r3 = (pSrc2[xSrc]>>16)&0xFF;
                a3 = (pSrc2[xSrc]>>24)&0xFF;
                if( xSrc<nWidth-1 )
                {
                    b4 = (pSrc2[xSrc+1]>>0)&0xFF;
                    g4 = (pSrc2[xSrc+1]>>8)&0xFF;
                    r4 = (pSrc2[xSrc+1]>>16)&0xFF;
                    a4 = (pSrc2[xSrc+1]>>24)&0xFF;
                }
            }


            // Pixel 1
            pDst1[xSrc*2] = pSrc[xSrc];

            // Pixel 2
            if( xSrc<nWidth-1 )
            {
                pDst1[xSrc*2+1] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
            }
            else
                pDst1[xSrc*2+1] = pSrc[xSrc];


            // Pixel 3
            if( ySrc<nHeight-1 )
            {
                pDst2[xSrc*2] = DWORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
            }
            else
                pDst2[xSrc*2] = pSrc[xSrc];

            // Pixel 4
            if( xSrc<nWidth-1 )
            {
                if( ySrc<nHeight-1 )
                {
                    pDst2[xSrc*2+1] = DWORD_MAKE((r1+r2+r3+r4)/4, (g1+g2+g3+g4)/4, (b1+b2+b3+b4)/4, (a1+a2+a3+a4)/4);
                }
                else
                {
                    pDst2[xSrc*2+1] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
                }
            }
            else
            {
                if( ySrc<nHeight-1 )
                {
                    pDst2[xSrc*2+1] = DWORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
                }
                else
                    pDst2[xSrc*2+1] = pSrc[xSrc];
            }
        }
    }
}

// Basic 2x R4G4B4A4 filter with interpolation
void Texture2x_16( DrawInfo &srcInfo, DrawInfo &destInfo )
{
    uint16 *pDst1, *pDst2;
    uint16 *pSrc, *pSrc2;
    uint32 nWidth = srcInfo.dwWidth;
    uint32 nHeight = srcInfo.dwHeight;

    uint16 b1;
    uint16 g1;
    uint16 r1;
    uint16 a1;
    uint16 b2 = 0;
    uint16 g2 = 0;
    uint16 r2 = 0;
    uint16 a2 = 0;
    uint16 b3 = 0;
    uint16 g3 = 0;
    uint16 r3 = 0;
    uint16 a3 = 0;
    uint16 b4 = 0;
    uint16 g4 = 0;
    uint16 r4 = 0;
    uint16 a4 = 0;

    for (uint16 ySrc = 0; ySrc < nHeight; ySrc++)
    {
        pSrc = (uint16*)(((uint8*)srcInfo.lpSurface)+ySrc*srcInfo.lPitch);
        pSrc2 = (uint16*)(((uint8*)srcInfo.lpSurface)+(ySrc+1)*srcInfo.lPitch);
        pDst1 = (uint16*)(((uint8*)destInfo.lpSurface)+(ySrc*2)*destInfo.lPitch);
        pDst2 = (uint16*)(((uint8*)destInfo.lpSurface)+(ySrc*2+1)*destInfo.lPitch);

        for (uint16 xSrc = 0; xSrc < nWidth; xSrc++)
        {
            b1 = (pSrc[xSrc]>> 0)&0xF;
            g1 = (pSrc[xSrc]>> 4)&0xF;
            r1 = (pSrc[xSrc]>> 8)&0xF;
            a1 = (pSrc[xSrc]>>12)&0xF;

            if( xSrc<nWidth-1 )
            {
                b2 = (pSrc[xSrc+1]>> 0)&0xF;
                g2 = (pSrc[xSrc+1]>> 4)&0xF;
                r2 = (pSrc[xSrc+1]>> 8)&0xF;
                a2 = (pSrc[xSrc+1]>>12)&0xF;
            }

            if( ySrc<nHeight-1 )
            {
                b3 = (pSrc2[xSrc]>> 0)&0xF;
                g3 = (pSrc2[xSrc]>> 4)&0xF;
                r3 = (pSrc2[xSrc]>> 8)&0xF;
                a3 = (pSrc2[xSrc]>>12)&0xF;
                if( xSrc<nWidth-1 )
                {
                    b4 = (pSrc2[xSrc+1]>> 0)&0xF;
                    g4 = (pSrc2[xSrc+1]>> 4)&0xF;
                    r4 = (pSrc2[xSrc+1]>> 8)&0xF;
                    a4 = (pSrc2[xSrc+1]>>12)&0xF;
                }
            }

            // Pixel 1
            pDst1[xSrc*2] = pSrc[xSrc];

            // Pixel 2
            if( xSrc<nWidth-1 )
            {
                pDst1[xSrc*2+1] = WORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
            }
            else
                pDst1[xSrc*2+1] = pSrc[xSrc];


            // Pixel 3
            if( ySrc<nHeight-1 )
            {
                pDst2[xSrc*2] = WORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
            }
            else
                pDst2[xSrc*2] = pSrc[xSrc];

            // Pixel 4
            if( xSrc<nWidth-1 )
            {
                if( ySrc<nHeight-1 )
                {
                    pDst2[xSrc*2+1] = WORD_MAKE((r1+r2+r3+r4)/4, (g1+g2+g3+g4)/4, (b1+b2+b3+b4)/4, (a1+a2+a3+a4)/4);
                }
                else
                {
                    pDst2[xSrc*2+1] = WORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
                }
            }
            else
            {
                if( ySrc<nHeight-1 )
                {
                    pDst2[xSrc*2+1] = WORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
                }
                else
                    pDst2[xSrc*2+1] = pSrc[xSrc];
            }
        }
    }
}

/************************************************************************/
/* Sharpen filters                                                      */
/************************************************************************/
void SharpenFilter_32(uint32 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter)
{
    uint32 len=height*pitch;
    uint32 *pcopy = new uint32[len];

    if( !pcopy )    return;

    memcpy(pcopy, pdata, len<<2);

    uint32 mul1, mul2, mul3, shift4;
    switch( filter )
    {
        case TEXTURE_SHARPEN_MORE_ENHANCEMENT:
            mul1=1;
            mul2=8;
            mul3=12;
            shift4=2;
            break;
        case TEXTURE_SHARPEN_ENHANCEMENT:
        default:
            mul1=1;
            mul2=8;
            mul3=16;
            shift4=3;
            break;
    }

    uint32 x,y,z;
    uint32 *src1, *src2, *src3, *dest;
    uint32 val[4];
    uint32 t1,t2,t3,t4,t5,t6,t7,t8,t9;

    for( y=1; y<height-1; y++)
    {
        dest = pdata+y*pitch;
        src1 = pcopy+(y-1)*pitch;
        src2 = src1 + pitch;
        src3 = src2 + pitch;
        for( x=1; x<width-1; x++)
        {
            for( z=0; z<4; z++ )
            {
                t1 = *((uint8*)(src1+x-1)+z);
                t2 = *((uint8*)(src1+x  )+z);
                t3 = *((uint8*)(src1+x+1)+z);
                t4 = *((uint8*)(src2+x-1)+z);
                t5 = *((uint8*)(src2+x  )+z);
                t6 = *((uint8*)(src2+x+1)+z);
                t7 = *((uint8*)(src3+x-1)+z);
                t8 = *((uint8*)(src3+x  )+z);
                t9 = *((uint8*)(src3+x+1)+z);
                val[z]=t5;
                if( (t5*mul2) > (t1+t3+t7+t9+t2+t4+t6+t8)*mul1 )
                {
                    val[z]= std::min((((t5*mul3) - (t1+t3+t7+t9+t2+t4+t6+t8)*mul1)>>shift4),0xFFU);
                }
            }
            dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
        }
    }
    delete [] pcopy;
}

void SharpenFilter_16(uint16 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter)
{
    //return;   // Sharpen does not make sense for 16 bits

    uint32 len=height*pitch;
    uint16 *pcopy = new uint16[len];

    if( !pcopy )    return;

    memcpy(pcopy, pdata, len<<1);

    uint16 mul1, mul2, mul3, shift4;
    switch( filter )
    {
        case TEXTURE_SHARPEN_MORE_ENHANCEMENT:
            mul1=1;
            mul2=8;
            mul3=12;
            shift4=2;
            break;
        case TEXTURE_SHARPEN_ENHANCEMENT:
        default:
            mul1=1;
            mul2=8;
            mul3=16;
            shift4=3;
            break;
    }

    uint32 x,y,z;
    uint16 *src1, *src2, *src3, *dest;
    uint16 val[4];
    uint16 t1,t2,t3,t4,t5,t6,t7,t8,t9;

    for( y=1; y<height-1; y++)
    {
        dest = pdata+y*pitch;
        src1 = pcopy+(y-1)*pitch;
        src2 = src1 + pitch;
        src3 = src2 + pitch;
        for( x=1; x<width-1; x++)
        {
            for( z=0; z<4; z++ )
            {
                uint32 shift = (z%1)?4:0;
                t1 = (*((uint8*)(src1+x-1)+(z>>1)))>>shift;
                t2 = (*((uint8*)(src1+x  )+(z>>1)))>>shift;
                t3 = (*((uint8*)(src1+x+1)+(z>>1)))>>shift;
                t4 = (*((uint8*)(src2+x-1)+(z>>1)))>>shift;
                t5 = (*((uint8*)(src2+x  )+(z>>1)))>>shift;
                t6 = (*((uint8*)(src2+x+1)+(z>>1)))>>shift;
                t7 = (*((uint8*)(src3+x-1)+(z>>1)))>>shift;
                t8 = (*((uint8*)(src3+x  )+(z>>1)))>>shift;
                t9 = (*((uint8*)(src3+x+1)+(z>>1)))>>shift;
                val[z]=t5;
                if( (t5*mul2) > (t1+t3+t7+t9+t2+t4+t6+t8)*mul1 )
                {
                    val[z] = (((t5*mul3) - (t1+t3+t7+t9+t2+t4+t6+t8)*mul1)>>shift4);
                    val[z]= std::min(val[z],(unsigned short)0xFU);
                }
            }
            dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
        }
    }
    delete [] pcopy;
}

/************************************************************************/
/* Smooth filters                                                       */
/************************************************************************/
void SmoothFilter_32(uint32 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter)
{
    uint32 len=height*pitch;
    uint32 *pcopy = new uint32[len];

    if( !pcopy )    return;

    memcpy(pcopy, pdata, len<<2);

    uint32 mul1, mul2, mul3, shift4;
    switch( filter )
    {
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1:
            mul1=1;
            mul2=2;
            mul3=4;
            shift4=4;
            break;
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2:
            mul1=1;
            mul2=1;
            mul3=8;
            shift4=4;
            break;
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3:
            mul1=1;
            mul2=1;
            mul3=2;
            shift4=2;
            break;
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4:
        default:
            mul1=1;
            mul2=1;
            mul3=6;
            shift4=3;
            break;
    }

    uint32 x,y,z;
    uint32 *src1, *src2, *src3, *dest;
    uint32 val[4];
    uint32 t1,t2,t3,t4,t5,t6,t7,t8,t9;

    if( filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3 || filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4 )
    {
        for( y=1; y<height-1; y+=2)
        {
            dest = pdata+y*pitch;
            src1 = pcopy+(y-1)*pitch;
            src2 = src1 + pitch;
            src3 = src2 + pitch;
            for( x=0; x<width; x++)
            {
                for( z=0; z<4; z++ )
                {
                    t2 = *((uint8*)(src1+x  )+z);
                    t5 = *((uint8*)(src2+x  )+z);
                    t8 = *((uint8*)(src3+x  )+z);
                    val[z] = ((t2+t8)*mul2+(t5*mul3))>>shift4;
                }
                dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
            }
        }
    }
    else
    {
        for( y=0; y<height; y++)
        {
            dest = pdata+y*pitch;
            if( y>0 )
            {
                src1 = pcopy+(y-1)*pitch;
                src2 = src1 + pitch;
            }
            else
            {
                src1 = src2 = pcopy;
            }

            src3 = src2;
            if( y<height-1) src3 += pitch;

            for( x=1; x<width-1; x++)
            {
                for( z=0; z<4; z++ )
                {
                    t1 = *((uint8*)(src1+x-1)+z);
                    t2 = *((uint8*)(src1+x  )+z);
                    t3 = *((uint8*)(src1+x+1)+z);
                    t4 = *((uint8*)(src2+x-1)+z);
                    t5 = *((uint8*)(src2+x  )+z);
                    t6 = *((uint8*)(src2+x+1)+z);
                    t7 = *((uint8*)(src3+x-1)+z);
                    t8 = *((uint8*)(src3+x  )+z);
                    t9 = *((uint8*)(src3+x+1)+z);
                    val[z] = ((t1+t3+t7+t9)*mul1+((t2+t4+t6+t8)*mul2)+(t5*mul3))>>shift4;
                }
                dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
            }
        }
    }
    delete [] pcopy;
}

void SmoothFilter_16(uint16 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter)
{
    uint32 len=height*pitch;
    uint16 *pcopy = new uint16[len];

    if( !pcopy )    return;

    memcpy(pcopy, pdata, len<<1);

    uint16 mul1, mul2, mul3, shift4;
    switch( filter )
    {
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1:
            mul1=1;
            mul2=2;
            mul3=4;
            shift4=4;
            break;
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2:
            mul1=1;
            mul2=1;
            mul3=8;
            shift4=4;
            break;
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3:
            mul1=1;
            mul2=1;
            mul3=2;
            shift4=2;
            break;
        case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4:
        default:
            mul1=1;
            mul2=1;
            mul3=6;
            shift4=3;
            break;
    }

    uint32 x,y,z;
    uint16 *src1, *src2, *src3, *dest;
    uint16 val[4];
    uint16 t1,t2,t3,t4,t5,t6,t7,t8,t9;

    if( filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3 || filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4 )
    {
        for( y=1; y<height-1; y+=2)
        {
            dest = pdata+y*pitch;
            src1 = pcopy+(y-1)*pitch;
            src2 = src1 + pitch;
            src3 = src2 + pitch;
            for( x=0; x<width; x++)
            {
                for( z=0; z<4; z++ )
                {
                    uint32 shift = (z&1)?4:0;
                    t2 = (*((uint8*)(src1+x  )+(z>>1)))>>shift;
                    t5 = (*((uint8*)(src2+x  )+(z>>1)))>>shift;
                    t8 = (*((uint8*)(src3+x  )+(z>>1)))>>shift;
                    val[z] = ((t2+t8)*mul2+(t5*mul3))>>shift4;
                }
                dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
            }
        }
    }
    else
    {
        for( y=0; y<height; y++)
        {
            dest = pdata+y*pitch;
            if( y>0 )
            {
                src1 = pcopy+(y-1)*pitch;
                src2 = src1 + pitch;
            }
            else
            {
                src1 = src2 = pcopy;
            }

            src3 = src2;
            if( y<height-1) src3 += pitch;

            for( x=1; x<width-1; x++)
            {
                for( z=0; z<4; z++ )
                {
                    uint32 shift = (z&1)?4:0;
                    t1 = (*((uint8*)(src1+x-1)+(z>>1)))>>shift;
                    t2 = (*((uint8*)(src1+x  )+(z>>1)))>>shift;
                    t3 = (*((uint8*)(src1+x+1)+(z>>1)))>>shift;
                    t4 = (*((uint8*)(src2+x-1)+(z>>1)))>>shift;
                    t5 = (*((uint8*)(src2+x  )+(z>>1)))>>shift;
                    t6 = (*((uint8*)(src2+x+1)+(z>>1)))>>shift;
                    t7 = (*((uint8*)(src3+x-1)+(z>>1)))>>shift;
                    t8 = (*((uint8*)(src3+x  )+(z>>1)))>>shift;
                    t9 = (*((uint8*)(src3+x+1)+(z>>1)))>>shift;
                    val[z] = ((t1+t3+t7+t9)*mul1+((t2+t4+t6+t8)*mul2)+(t5*mul3))>>shift4;
                }
                dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
            }
        }
    }
    delete [] pcopy;
}


void EnhanceTexture(TxtrCacheEntry *pEntry)
{
    if( pEntry->dwEnhancementFlag == options.textureEnhancement )
    {
        // The texture has already been enhanced
        return;
    }
    else if( options.textureEnhancement == TEXTURE_NO_ENHANCEMENT )
    {
        SAFE_DELETE(pEntry->pEnhancedTexture);
        pEntry->dwEnhancementFlag = TEXTURE_NO_ENHANCEMENT;
        return;
    }

    if( status.primitiveType != PRIM_TEXTRECT && options.bTexRectOnly )
    {
        return;
    }

    DrawInfo srcInfo;   
    if( pEntry->pTexture->StartUpdate(&srcInfo) == false )
    {
        SAFE_DELETE(pEntry->pEnhancedTexture);
        return;
    }

    uint32 realwidth = srcInfo.dwWidth;
    uint32 realheight = srcInfo.dwHeight;
    uint32 nWidth = srcInfo.dwCreatedWidth;
    uint32 nHeight = srcInfo.dwCreatedHeight;

    if( options.textureEnhancement == TEXTURE_SHARPEN_ENHANCEMENT || options.textureEnhancement == TEXTURE_SHARPEN_MORE_ENHANCEMENT )
    {
        if( pEntry->pTexture->GetPixelSize() == 4 )
            SharpenFilter_32((uint32*)srcInfo.lpSurface, nWidth, nHeight, nWidth, options.textureEnhancement);
        else
            SharpenFilter_16((uint16*)srcInfo.lpSurface, nWidth, nHeight, nWidth, options.textureEnhancement);
        pEntry->dwEnhancementFlag = options.textureEnhancement;
        pEntry->pTexture->EndUpdate(&srcInfo);
        SAFE_DELETE(pEntry->pEnhancedTexture);
        return;
    }

    pEntry->dwEnhancementFlag = options.textureEnhancement;
    if( options.bSmallTextureOnly )
    {
        if( nWidth + nHeight > 256 )
        {
            pEntry->pTexture->EndUpdate(&srcInfo);
            SAFE_DELETE(pEntry->pEnhancedTexture);
            pEntry->dwEnhancementFlag = TEXTURE_NO_ENHANCEMENT;
            return;
        }
    }


    CTexture* pSurfaceHandler = NULL;
    if( options.textureEnhancement == TEXTURE_HQ4X_ENHANCEMENT )
    {
        if( nWidth + nHeight > 1024/4 )
        {
            // Don't enhance for large textures
            pEntry->pTexture->EndUpdate(&srcInfo);
            SAFE_DELETE(pEntry->pEnhancedTexture);
            pEntry->dwEnhancementFlag = TEXTURE_NO_ENHANCEMENT;
            return;
        }
        pSurfaceHandler = CDeviceBuilder::GetBuilder()->CreateTexture(nWidth*4, nHeight*4);
    }
    else
    {
        if( nWidth + nHeight > 1024/2 )
        {
            // Don't enhance for large textures
            pEntry->pTexture->EndUpdate(&srcInfo);
            SAFE_DELETE(pEntry->pEnhancedTexture);
            pEntry->dwEnhancementFlag = TEXTURE_NO_ENHANCEMENT;
            return;
        }
        pSurfaceHandler = CDeviceBuilder::GetBuilder()->CreateTexture(nWidth*2, nHeight*2);
    }
    DrawInfo destInfo;
    if(pSurfaceHandler)
    {
        if(pSurfaceHandler->StartUpdate(&destInfo))
        {
            if( options.textureEnhancement == TEXTURE_2XSAI_ENHANCEMENT )
            {
                if( pEntry->pTexture->GetPixelSize() == 4 )
                    Super2xSaI_32((uint32*)(srcInfo.lpSurface),(uint32*)(destInfo.lpSurface), nWidth, realheight, nWidth);
                else
                    Super2xSaI_16((uint16*)(srcInfo.lpSurface),(uint16*)(destInfo.lpSurface), nWidth, realheight, nWidth);
            }
            else if( options.textureEnhancement == TEXTURE_HQ2X_ENHANCEMENT )
            {
                if( pEntry->pTexture->GetPixelSize() == 4 )
                {
                    hq2x_init(32);
                    hq2x_32((uint8*)(srcInfo.lpSurface), srcInfo.lPitch, (uint8*)(destInfo.lpSurface), destInfo.lPitch, nWidth, realheight);
                }
                else
                {
                    hq2x_init(16);
                    hq2x_16((uint8*)(srcInfo.lpSurface), srcInfo.lPitch, (uint8*)(destInfo.lpSurface), destInfo.lPitch, nWidth, realheight);
                }
            }
            else if( options.textureEnhancement == TEXTURE_LQ2X_ENHANCEMENT )
            {
                if( pEntry->pTexture->GetPixelSize() == 4 )
                {
                    hq2x_init(32);
                    lq2x_32((uint8*)(srcInfo.lpSurface), srcInfo.lPitch, (uint8*)(destInfo.lpSurface), destInfo.lPitch, nWidth, realheight);
                }
                else
                {
                    hq2x_init(16);
                    lq2x_16((uint8*)(srcInfo.lpSurface), srcInfo.lPitch, (uint8*)(destInfo.lpSurface), destInfo.lPitch, nWidth, realheight);
                }
            }
            else if( options.textureEnhancement == TEXTURE_HQ4X_ENHANCEMENT )
            {
                if( pEntry->pTexture->GetPixelSize() == 4 )
                {
                    hq4x_InitLUTs();
                    hq4x_32((uint8*)(srcInfo.lpSurface), (uint8*)(destInfo.lpSurface), realwidth, realheight, nWidth, destInfo.lPitch);
                }
                else
                {
                    hq4x_InitLUTs();
                    hq4x_16((uint8*)(srcInfo.lpSurface), (uint8*)(destInfo.lpSurface), realwidth, realheight, nWidth, destInfo.lPitch);
                }
            }
            else 
            {
                if( pEntry->pTexture->GetPixelSize() == 4 )
                    Texture2x_32( srcInfo, destInfo);
                else
                    Texture2x_16( srcInfo, destInfo);
            }

            if( options.textureEnhancementControl >= TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1 )
            {
                if( options.textureEnhancement != TEXTURE_HQ4X_ENHANCEMENT )
                {
                    if( pEntry->pTexture->GetPixelSize() == 4 )
                        SmoothFilter_32((uint32*)destInfo.lpSurface, realwidth<<1, realheight<<1, nWidth<<1, options.textureEnhancementControl);
                    else
                        SmoothFilter_16((uint16*)destInfo.lpSurface, realwidth<<1, realheight<<1, nWidth<<1, options.textureEnhancementControl);
                }
                else
                {
                    if( pEntry->pTexture->GetPixelSize() == 4 )
                        SmoothFilter_32((uint32*)destInfo.lpSurface, realwidth<<2, realheight<<2, nWidth<<2, options.textureEnhancementControl);
                    else
                        SmoothFilter_16((uint16*)destInfo.lpSurface, realwidth<<2, realheight<<2, nWidth<<2, options.textureEnhancementControl);
                }
            }

            pSurfaceHandler->EndUpdate(&destInfo);  
        }
    
        pSurfaceHandler->SetOthersVariables();
        pSurfaceHandler->m_bIsEnhancedTexture = true;
    }

    pEntry->pTexture->EndUpdate(&srcInfo);

    pEntry->pEnhancedTexture = pSurfaceHandler;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
void MirrorEmulator_DrawLine(DrawInfo& destInfo, DrawInfo& srcInfo, uint32 *pSource, uint32 *pDest, uint32 nWidth, BOOL bFlipLeftRight)
{
    if(!bFlipLeftRight)
    {
        memcpy(pDest, pSource, nWidth * 4);
    }
    else
    {
        uint32 *pMaxDest = pDest + nWidth;
        pSource += nWidth - 1;
        for(; pDest < pMaxDest; pDest++, pSource--)
        {
            *pDest = *pSource;
        }
    }
}


void MirrorEmulator_Draw(DrawInfo& destInfo, DrawInfo& srcInfo, uint32 nDestX, uint32 nDestY, BOOL bFlipLeftRight, BOOL bFlipUpDown)
{
    uint8 *pDest = (uint8 *) destInfo.lpSurface + (destInfo.lPitch * nDestY) + (4 * nDestX);
    uint8 *pMaxDest = pDest + (destInfo.lPitch * srcInfo.dwHeight);
    uint8 *pSource = (uint8 *)(srcInfo.lpSurface);
    if(!bFlipUpDown)
    {
        for(; pDest < pMaxDest; pDest += destInfo.lPitch, pSource += srcInfo.lPitch)
        {
            MirrorEmulator_DrawLine(destInfo, srcInfo, (uint32*)pSource, (uint32*)pDest, srcInfo.dwWidth, bFlipLeftRight);
        }
    }
    else
    {
        pSource += (srcInfo.lPitch * (srcInfo.dwHeight - 1));
        for(; pDest < pMaxDest; pDest += destInfo.lPitch, pSource -= srcInfo.lPitch)
        {
            MirrorEmulator_DrawLine(destInfo, srcInfo, (uint32*)pSource, (uint32*)pDest, srcInfo.dwWidth, bFlipLeftRight);
        }
    }
}

void MirrorTexture(uint32 dwTile, TxtrCacheEntry *pEntry)
{
    if( ((gRDP.tiles[dwTile].bMirrorS) || (gRDP.tiles[dwTile].bMirrorT)) && CGraphicsContext::Get()->m_supportTextureMirror == false )
    {
        if(pEntry->pEnhancedTexture)
        {
            return;
        }
        else
        {
            CTexture* pSurfaceHandler = NULL;

            // FIXME: Compute the correct values. 2/2 seems to always work correctly in Mario64
            uint32 nXTimes = gRDP.tiles[dwTile].bMirrorS ? 2 : 1;
            uint32 nYTimes = gRDP.tiles[dwTile].bMirrorT ? 2 : 1;
            
            // For any texture need to use mirror, we should not need to rescale it
            // because texture need to be mirrored must with MaskS and MaskT

            // But again, check me

            //if( pEntry->pTexture->m_bScaledS == false || pEntry->pTexture->m_bScaledT == false)
            //{
            //  pEntry->pTexture->ScaleImageToSurface();
            //}

            DrawInfo srcInfo;   
            if( pEntry->pTexture->StartUpdate(&srcInfo) )
            {
                uint32 nWidth = srcInfo.dwWidth;
                uint32 nHeight = srcInfo.dwHeight;

                pSurfaceHandler = CDeviceBuilder::GetBuilder()->CreateTexture(nWidth * nXTimes, nHeight * nYTimes);
                if( pSurfaceHandler )
                {
                    DrawInfo destInfo;
                    if( pSurfaceHandler->StartUpdate(&destInfo) )
                    {
                        for(uint32 nY = 0; nY < nYTimes; nY++)
                        {
                            for(uint32 nX = 0; nX < nXTimes; nX++)
                            {
                                MirrorEmulator_Draw(destInfo, srcInfo, nWidth * nX, nHeight * nY, nX & 0x1, nY & 0x1);
                            }
                        }

                        pSurfaceHandler->EndUpdate(&destInfo);
                    }
                
                    // fix me, there should be a flag to tell that it is a mirrored texture handler
                    // not the original texture handlers, so all texture coordinate should be divided by 2
                    pSurfaceHandler->SetOthersVariables();
                }

                pEntry->pTexture->EndUpdate(&srcInfo);  
                pEntry->dwEnhancementFlag = TEXTURE_MIRRORED;
            }


            pEntry->pEnhancedTexture = pSurfaceHandler;
        }
    }
}

enum TextureType
{
    NO_TEXTURE,
 RGB_PNG,
 COLOR_INDEXED_BMP,
 RGB_WITH_ALPHA_TOGETHER_PNG,
 RGBA_PNG_FOR_CI,
 RGBA_PNG_FOR_ALL_CI,
};
typedef struct {
    int width;
    int height;
    int fmt;
    int siz;
    int crc32;
    int pal_crc32;
    char *foldername;
    //char name[40];
    char RGBNameTail[23];
    char AlphaNameTail[20];
    TextureType type;
    bool        bSeparatedAlpha;
} ExtTxtrInfo;

CSortedList<uint64,ExtTxtrInfo> gTxtrDumpInfos;
CSortedList<uint64,ExtTxtrInfo> gHiresTxtrInfos;

extern char * right(const char * src, int nchars);

#define SURFFMT_P8 41

int GetImageInfoFromFile(char* pSrcFile, IMAGE_INFO *pSrcInfo)
{
    unsigned char sig[8];
    FILE *f;

    f = fopen(pSrcFile, "rb");
    if (f == NULL)
    {
      DebugMessage(M64MSG_ERROR, "GetImageInfoFromFile() error: couldn't open file '%s'", pSrcFile);
      return 1;
    }
    if (fread(sig, 1, 8, f) != 8)
    {
      DebugMessage(M64MSG_ERROR, "GetImageInfoFromFile() error: couldn't read first 8 bytes of file '%s'", pSrcFile);
      return 1;
    }
    fclose(f);

    if(sig[0] == 'B' && sig[1] == 'M') // BMP
    {
        struct BMGImageStruct img;
        memset(&img, 0, sizeof(BMGImageStruct));
        BMG_Error code = ReadBMP(pSrcFile, &img);
        if( code == BMG_OK )
        {
            pSrcInfo->Width = img.width;
            pSrcInfo->Height = img.height;
            pSrcInfo->Depth = img.bits_per_pixel;
            pSrcInfo->MipLevels = 1;
            if(img.bits_per_pixel == 32)
                pSrcInfo->Format = SURFFMT_A8R8G8B8;
            else if(img.bits_per_pixel == 8)
                pSrcInfo->Format = SURFFMT_P8;
            // Resource and File Format ignored
            FreeBMGImage(&img);
            return 0;
        }
        DebugMessage(M64MSG_ERROR, "Couldn't read BMP file '%s'; error = %i", pSrcFile, code);
        return 1;
    }
    else if(sig[0] == 137 && sig[1] == 'P' && sig[2] == 'N' && sig[3] == 'G' && sig[4] == '\r' && sig[5] == '\n' &&
               sig[6] == 26 && sig[7] == '\n') // PNG
    {
        struct BMGImageStruct img;
        memset(&img, 0, sizeof(BMGImageStruct));
        BMG_Error code = ReadPNG(pSrcFile, &img);
        if( code == BMG_OK )
        {
            pSrcInfo->Width = img.width;
            pSrcInfo->Height = img.height;
            pSrcInfo->Depth = img.bits_per_pixel;
            pSrcInfo->MipLevels = 1;
            if(img.bits_per_pixel == 32)
                pSrcInfo->Format = SURFFMT_A8R8G8B8;
            else if(img.bits_per_pixel == 8)
                pSrcInfo->Format = SURFFMT_P8;
            // Resource and File Format ignored
            FreeBMGImage(&img);
            return 0;
        }
        DebugMessage(M64MSG_ERROR, "Couldn't read PNG file '%s'; error = %i", pSrcFile, code);
        return 1;
    }

    DebugMessage(M64MSG_ERROR, "GetImageInfoFromFile : unknown file format (%s)", pSrcFile);
    return 1;
}

BOOL PathFileExists(char* pszPath)
{
    FILE *f;
    f = fopen(pszPath, "rb");
    if(f != NULL)
    {
        fclose(f);
        return TRUE;
    }
    return FALSE;
}

void FindAllTexturesFromFolder(char *foldername, CSortedList<uint64,ExtTxtrInfo> &infos, bool extraCheck, bool bRecursive)
{
    if (!osal_is_directory(foldername))
        return;

    char texturefilename[PATH_MAX];
    IMAGE_INFO  imgInfo;
    IMAGE_INFO  imgInfo2;

    void *dir;
    dir = osal_search_dir_open(foldername);
    const char *foundfilename;

    int crc, palcrc32;
    unsigned int fmt, siz;
    char crcstr[16], crcstr2[16];

    do
    {
        foundfilename = osal_search_dir_read_next(dir);
        if (foundfilename == NULL) break;
        if (foundfilename[0] == '.' ) continue;

        strcpy(texturefilename, foldername);
        strcat(texturefilename, foundfilename);

        /* handle recursion into sub-directories */
        if (osal_is_directory(texturefilename) && bRecursive )
        {
            strcat(texturefilename, OSAL_DIR_SEPARATOR_STR);
            FindAllTexturesFromFolder(texturefilename, infos, extraCheck, bRecursive);
            continue;
        }
        /* if filename doesn't match the ROM's name, skip it */
        if( strstr(foundfilename,(const char*)g_curRomInfo.szGameName) == 0 )
            continue;

        TextureType type = NO_TEXTURE;
        bool bSeparatedAlpha = false;

        if( strcasecmp(right(foundfilename,7), "_ci.bmp") == 0 )
        {
            if( GetImageInfoFromFile(texturefilename, &imgInfo) != 0)
            {
                DebugMessage(M64MSG_WARNING, "Cannot get image info for file: %s", foundfilename);
                continue;
            }

            if( imgInfo.Format == SURFFMT_P8 )
                type = COLOR_INDEXED_BMP;
            else
                continue;
        }
        else if( strcasecmp(right(foundfilename,13), "_ciByRGBA.png") == 0 )
        {
            if( GetImageInfoFromFile(texturefilename, &imgInfo) != 0 )
            {
                DebugMessage(M64MSG_WARNING, "Cannot get image info for file: %s", foundfilename);
                continue;
            }

            if( imgInfo.Format == SURFFMT_A8R8G8B8 )
                type = RGBA_PNG_FOR_CI;
            else
                continue;
        }
        else if( strcasecmp(right(foundfilename,16), "_allciByRGBA.png") == 0 )
        {
            if( GetImageInfoFromFile(texturefilename, &imgInfo) != 0 )
            {
                DebugMessage(M64MSG_WARNING, "Cannot get image info for file: %s", foundfilename);
                continue;
            }
            if( imgInfo.Format == SURFFMT_A8R8G8B8 )
                type = RGBA_PNG_FOR_ALL_CI;
            else
                continue;
        }
        else if( strcasecmp(right(foundfilename,8), "_rgb.png") == 0 )
        {
            if( GetImageInfoFromFile(texturefilename, &imgInfo) != 0 )
            {
                DebugMessage(M64MSG_WARNING, "Cannot get image info for file: %s", foundfilename);
                continue;
            }

            type = RGB_PNG;

            char filename2[PATH_MAX];
            strcpy(filename2,texturefilename);
            strcpy(filename2+strlen(filename2)-8,"_a.png");
            if( PathFileExists(filename2) )
            {
                if( GetImageInfoFromFile(filename2, &imgInfo2) != 0 )
                {
                    DebugMessage(M64MSG_WARNING, "Cannot get image info for file: %s", filename2);
                    continue;
                }
                
                if( extraCheck && (imgInfo2.Width != imgInfo.Width || imgInfo2.Height != imgInfo.Height) )
                {
                    DebugMessage(M64MSG_WARNING, "RGB and alpha texture size mismatch: %s", filename2);
                    continue;
                }

                bSeparatedAlpha = true;
            }
        }
        else if( strcasecmp(right(foundfilename,8), "_all.png") == 0 )
        {
            if( GetImageInfoFromFile(texturefilename, &imgInfo) != 0 )
            {
                DebugMessage(M64MSG_WARNING, "Cannot get image info for file: %s", foundfilename);
                continue;
            }

            type = RGB_WITH_ALPHA_TOGETHER_PNG;
        }
    
        if( type != NO_TEXTURE )
        {
            /* Try to read image information here.

            (CASTLEVANIA2)(#58E2333F)(#2#0#)(D7A5C6D 9)_ciByRGBA.png
            (------1-----)(----2----)(3)(4)(----5-----)_ciByRGBA.png
 
            1. Internal ROM name
            2. The DRAM CRC
            3. The image pixel size (8b=0, 16b=1, 24b=2, 32b=3)
            4. The texture format (RGBA=0, YUV=1, CI=2, IA=3, I=4)
            5. The PAL CRC
            */
            strcpy(texturefilename, foundfilename);

            char *ptr = strchr(texturefilename,'#');
            *ptr++ = 0;
            if( type == RGBA_PNG_FOR_CI )
            {
                sscanf(ptr,"%8c#%d#%d#%8c", crcstr, &fmt, &siz,crcstr2);
                crcstr2[8] = 0;
                palcrc32 = strtoul(crcstr2,NULL,16);
            }
            else
            {
                sscanf(ptr,"%8c#%d#%d", crcstr, &fmt, &siz);
                palcrc32 = 0xFFFFFFFF;
            }
            crcstr[8]=0;
            crc = strtoul(crcstr,NULL,16);

            int foundIdx = -1;
            for( int k=0; k<infos.size(); k++)
            {
                if( infos[k].crc32 == crc && infos[k].pal_crc32 == palcrc32 )
                {
                    foundIdx = k;
                    break;
                }
            }

            if( foundIdx < 0 || type != infos[foundIdx].type)
            {
                ExtTxtrInfo newinfo;
                newinfo.width = imgInfo.Width;
                newinfo.height = imgInfo.Height;
            //strcpy(newinfo.name,g_curRomInfo.szGameName);
                newinfo.foldername = new char[strlen(foldername)+1];
                strcpy(newinfo.foldername,foldername);
                newinfo.fmt = fmt;
                newinfo.siz = siz;
                newinfo.crc32 = crc;
                newinfo.pal_crc32 = palcrc32;
                newinfo.type = type;
                newinfo.bSeparatedAlpha = bSeparatedAlpha;

                newinfo.RGBNameTail[0] = newinfo.AlphaNameTail[0] = 0;

                switch ( type )
                {
                    case RGB_PNG:
                        strcpy(newinfo.RGBNameTail, "_rgb.png");
                        strcpy(newinfo.AlphaNameTail, "_a.png");
                        break;
                    case COLOR_INDEXED_BMP:
                        strcpy(newinfo.RGBNameTail, "_ci.bmp");
                        break;
                    case RGBA_PNG_FOR_CI:
                        strcpy(newinfo.RGBNameTail, right(ptr,22));
                        break;
                    case RGBA_PNG_FOR_ALL_CI:
                        strcpy(newinfo.RGBNameTail, "_allciByRGBA.png");
                        break;
                    default:
                        strcpy(newinfo.RGBNameTail, "_all.png");
                        break;
                }

                uint64 crc64 = newinfo.crc32;
                crc64 <<= 32;
                crc64 |= (newinfo.pal_crc32&0xFFFFFF00)|(newinfo.fmt<<4)|newinfo.siz;
                infos.add(crc64,newinfo);
            }
        }
    } while(foundfilename != NULL);

    osal_search_dir_close(dir);
}

bool CheckAndCreateFolder(const char* pathname)
{
    if( !PathFileExists((char*)pathname) )
    {
        if (osal_mkdirp(pathname, 0700) != 0)
        {
            DebugMessage(M64MSG_WARNING, "Can not create new folder: %s", pathname);
            return false;
        }
    }

    return true;
}


// Texture dumping filenaming
// GameName_FrameCount_CRC_Fmt_Siz.bmp
// File format:     BMP
// GameName:        N64 game internal name
// CRC:             32 bit, 8 hex digits
// Fmt:             0 - 4
// Siz:             0 - 3

const char *subfolders[] = {
    "png_all",
    "png_by_rgb_a",
    "ci_bmp",
    "ci_bmp_with_pal_crc",
    "ci_by_png",
};

void FindAllDumpedTextures(void)
{
    char    foldername[PATH_MAX + 64];
    strncpy(foldername, ConfigGetUserDataPath(), PATH_MAX);
    foldername[PATH_MAX] = 0;

    if (foldername[strlen(foldername) - 1] != OSAL_DIR_SEPARATOR_CHAR)
        strcat(foldername, OSAL_DIR_SEPARATOR_STR);
    strcat(foldername,"texture_dump" OSAL_DIR_SEPARATOR_STR);

    CheckAndCreateFolder(foldername);

    strcat(foldername,(const char*)g_curRomInfo.szGameName);
    strcat(foldername, OSAL_DIR_SEPARATOR_STR);

    gTxtrDumpInfos.clear();
    if( !PathFileExists(foldername) )
    {
        CheckAndCreateFolder(foldername);
        char    foldername2[PATH_MAX];
        for( int i=0; i<5; i++)
        {
            strcpy(foldername2,foldername);
            strcat(foldername2,subfolders[i]);
            CheckAndCreateFolder(foldername2);
        }
        return;
    }
    else
    {
        gTxtrDumpInfos.clear();
        FindAllTexturesFromFolder(foldername,gTxtrDumpInfos, false, true);

        char    foldername2[PATH_MAX];
        for( int i=0; i<5; i++)
        {
            strcpy(foldername2,foldername);
            strcat(foldername2,subfolders[i]);
            CheckAndCreateFolder(foldername2);
        }
    }
}


void FindAllHiResTextures(void)
{
    char    foldername[PATH_MAX + 64];
    strncpy(foldername, ConfigGetUserDataPath(), PATH_MAX);
    foldername[PATH_MAX] = 0;

    if(foldername[strlen(foldername) - 1] != OSAL_DIR_SEPARATOR_CHAR)
        strcat(foldername, OSAL_DIR_SEPARATOR_STR);
    strcat(foldername,"hires_texture" OSAL_DIR_SEPARATOR_STR);
    CheckAndCreateFolder(foldername);

    strcat(foldername,(const char*)g_curRomInfo.szGameName);
    strcat(foldername, OSAL_DIR_SEPARATOR_STR);
    gHiresTxtrInfos.clear();
    if (!osal_is_directory(foldername))
    {
        DebugMessage(M64MSG_WARNING, "Couldn't open hi-res texture directory: %s", foldername);
        return;
    }
    else
    {
        FindAllTexturesFromFolder(foldername,gHiresTxtrInfos, true, true);
    }
}

void CloseHiresTextures(void)
{
    for( int i=0; i<gHiresTxtrInfos.size(); i++)
    {
        if( gHiresTxtrInfos[i].foldername )
            delete [] gHiresTxtrInfos[i].foldername;
    }

    gHiresTxtrInfos.clear();
}

void CloseTextureDump(void)
{
    for( int i=0; i<gTxtrDumpInfos.size(); i++)
    {
        if( gTxtrDumpInfos[i].foldername )  
            delete [] gTxtrDumpInfos[i].foldername;
    }

    gTxtrDumpInfos.clear();
}

void CloseExternalTextures(void)
{
    CloseHiresTextures();
    CloseTextureDump();
}

void InitHiresTextures(void)
{
if( options.bLoadHiResTextures )
    {
    DebugMessage(M64MSG_INFO, "Texture loading option is enabled. Finding all hires textures");
    FindAllHiResTextures();
    }
}

void InitTextureDump(void)
{
if( options.bDumpTexturesToFiles )
    {
    DebugMessage(M64MSG_INFO, "Texture dump option is enabled. Finding all dumpped textures");
    FindAllDumpedTextures();
    }
}
void InitExternalTextures(void)
{
    DebugMessage(M64MSG_VERBOSE, "InitExternalTextures");
    CloseExternalTextures();
    InitHiresTextures();
    InitTextureDump();
}

/*
ExtTxtrInfo.height ExtTxtrInfo.width are ints.
TxtrInfo.HeightToLoad TxtrInfo.WidthToLoad (ti of TxtrCacheEntry 
is of type TxtrInfo) are uint32.
*/
int FindScaleFactor(const ExtTxtrInfo &info, TxtrCacheEntry &entry)
{
    int scaleShift = 0;
 
    // find the smallest power of 2 which is greater than or equal to the ratio of the hi-res texture's
    // dimensions to the original (N64) texture dimensions
    while (info.height > (int) entry.ti.HeightToLoad * (1 << scaleShift) && info.width > (int) entry.ti.WidthToLoad * (1 << scaleShift))
    {
        scaleShift++;
    }
    // if the ratio of the 2 textures' dimensions is an even power of 2, then the hi-res texture is allowed
    if (info.height == (int) entry.ti.HeightToLoad * (1 << scaleShift) && info.width == (int) entry.ti.WidthToLoad * (1 << scaleShift))
    {
        // found appropriate scale shift, return it
        return scaleShift;
    }

    // the dimensions of the hires replacement are not power of 2 of the original texture
    return -1;
}

int CheckTextureInfos( CSortedList<uint64,ExtTxtrInfo> &infos, TxtrCacheEntry &entry, int &indexa, int &scaleShift, bool bForDump = false)
{
    if ((entry.ti.WidthToLoad  != 0 && entry.ti.WidthToCreate  / entry.ti.WidthToLoad  > 2) ||
        (entry.ti.HeightToLoad != 0 && entry.ti.HeightToCreate / entry.ti.HeightToLoad > 2 ))
    {
        //DebugMessage(M64MSG_WARNING, "Hires texture does not support extreme texture replication");
        return -1;
    }

    bool bCI = (gRDP.otherMode.text_tlut>=2 || entry.ti.Format == TXT_FMT_CI || entry.ti.Format == TXT_FMT_RGBA) && entry.ti.Size <= TXT_SIZE_8b;

    uint64 crc64a = entry.dwCRC;
    crc64a <<= 32;
    uint64 crc64b = crc64a;
    crc64a |= (0xFFFFFF00|(entry.ti.Format<<4)|entry.ti.Size);
    crc64b |= ((entry.dwPalCRC&0xFFFFFF00)|(entry.ti.Format<<4)|entry.ti.Size);

    int infosize = infos.size();
    int indexb=-1;
    indexa = infos.find(crc64a);        // For CI without pal CRC, and for RGBA_PNG_FOR_ALL_CI
    if( bCI )   
        indexb = infos.find(crc64b);    // For CI or PNG with pal CRC

    if( indexa >= infosize )    indexa = -1;
    if( indexb >= infosize )    indexb = -1;

    scaleShift = -1;

    if( indexb >= 0 )
    {
        scaleShift = FindScaleFactor(infos[indexb], entry);
        if( scaleShift >= 0 )
            return indexb;
    }

    if( bForDump && bCI && indexb < 0)
        return -1;

    if( indexa >= 0 )   scaleShift = FindScaleFactor(infos[indexa], entry);

    if( scaleShift >= 0 )
        return indexa;
    else
        return -1;
}

bool SaveCITextureToFile(TxtrCacheEntry &entry, char *filename, bool bShow, bool bWhole);

void DumpCachedTexture( TxtrCacheEntry &entry )
{
    char cSep = '/';

    CTexture *pSrcTexture = entry.pTexture;
    if( pSrcTexture )
    {
        // Check the vector table
        int ciidx, scaleShift;
        if( CheckTextureInfos(gTxtrDumpInfos,entry,ciidx,scaleShift,true) >= 0 )
            return;     // This texture has been dumpped

        char filename1[PATH_MAX + 64];
        char filename2[PATH_MAX + 64];
        char filename3[PATH_MAX + 64];
        char gamefolder[PATH_MAX + 64];
        strncpy(gamefolder, ConfigGetUserDataPath(), PATH_MAX);
        gamefolder[PATH_MAX] = 0;
        
        strcat(gamefolder,"texture_dump" OSAL_DIR_SEPARATOR_STR);
        strcat(gamefolder,(const char*)g_curRomInfo.szGameName);
        strcat(gamefolder, OSAL_DIR_SEPARATOR_STR);

        //sprintf(filename1+strlen(filename1), "%08X#%d#%d", entry.dwCRC, entry.ti.Format, entry.ti.Size);
        sprintf(filename1, "%s%s#%08X#%d#%d", gamefolder, g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size);

        if( (gRDP.otherMode.text_tlut>=2 || entry.ti.Format == TXT_FMT_CI || entry.ti.Format == TXT_FMT_RGBA) && entry.ti.Size <= TXT_SIZE_8b )
        {
            if( ciidx < 0 )
            {
                sprintf(filename1, "%sci_bmp%c%s#%08X#%d#%d_ci", gamefolder, cSep, g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size);
                SaveCITextureToFile(entry, filename1, false, false);
            }

            sprintf(filename1, "%sci_bmp_with_pal_crc%c%s#%08X#%d#%d#%08X_ci", gamefolder, cSep, g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size,entry.dwPalCRC);
            SaveCITextureToFile(entry, filename1, false, false);

            sprintf(filename1, "%sci_by_png%c%s#%08X#%d#%d#%08X_ciByRGBA", gamefolder, cSep, g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size,entry.dwPalCRC);
            CRender::g_pRender->SaveTextureToFile(*pSrcTexture, filename1, TXT_RGBA, false, false, entry.ti.WidthToLoad, entry.ti.HeightToLoad);
        }
        else
        {
            sprintf(filename1, "%spng_by_rgb_a%c%s#%08X#%d#%d_rgb", gamefolder, cSep, g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size);
            sprintf(filename2, "%spng_by_rgb_a%c%s#%08X#%d#%d_a", gamefolder, cSep, g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size);
            sprintf(filename3, "%spng_all%c%s#%08X#%d#%d_all", gamefolder, cSep, g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size);


            CRender::g_pRender->SaveTextureToFile(*pSrcTexture, filename1, TXT_RGB, false, false, entry.ti.WidthToLoad, entry.ti.HeightToLoad);
            CRender::g_pRender->SaveTextureToFile(*pSrcTexture, filename3, TXT_RGBA, false, false, entry.ti.WidthToLoad, entry.ti.HeightToLoad);
            if( entry.ti.Format != TXT_FMT_I )
            {
                DrawInfo srcInfo;   
                uint32 aFF = 0xFF;
                if( pSrcTexture->StartUpdate(&srcInfo) )
                {
                    // Copy RGB to buffer
                    for( int i=entry.ti.HeightToLoad-1; i>=0; i--)
                    {
                        unsigned char *pSrc = (unsigned char*)srcInfo.lpSurface+srcInfo.lPitch * i;
                        for( uint32 j=0; j<entry.ti.WidthToLoad; j++)
                        {
                            aFF &= pSrc[3];
                            pSrc += 4;
                        }
                    }
                    pSrcTexture->EndUpdate(&srcInfo);
                }

                if( aFF != 0xFF)
                    CRender::g_pRender->SaveTextureToFile(*pSrcTexture, filename2, TXT_ALPHA, false, false);
            }       
        }

        ExtTxtrInfo newinfo;
        newinfo.width = entry.ti.WidthToLoad;
        newinfo.height = entry.ti.HeightToLoad;
        //strcpy(newinfo.name,g_curRomInfo.szGameName);
        newinfo.fmt = entry.ti.Format;
        newinfo.siz = entry.ti.Size;
        newinfo.crc32 = entry.dwCRC;

        newinfo.pal_crc32 = entry.dwPalCRC;
        newinfo.foldername = NULL;
        newinfo.RGBNameTail[0] = newinfo.AlphaNameTail[0] = 0;

        uint64 crc64 = newinfo.crc32;
        crc64 <<= 32;
        crc64 |= (newinfo.pal_crc32&0xFFFFFF00)|(newinfo.fmt<<4)|newinfo.siz;
        gTxtrDumpInfos.add(crc64,newinfo);

    }
}

bool LoadRGBBufferFromPNGFile(char *filename, unsigned char **pbuf, int &width, int &height, int bits_per_pixel = 24 )
{
    struct BMGImageStruct img;
    memset(&img, 0, sizeof(BMGImageStruct));
    if (!PathFileExists(filename))
    {
        DebugMessage(M64MSG_ERROR, "File at '%s' doesn't exist in LoadRGBBufferFromPNGFile!", filename);
        return false;
    }

    BMG_Error code = ReadPNG( filename, &img );
    if( code == BMG_OK )
    {
        *pbuf = NULL;

        *pbuf = new unsigned char[img.width*img.height*bits_per_pixel/8];
        if (*pbuf == NULL)
        {
            DebugMessage(M64MSG_ERROR, "new[] returned NULL for image width=%i height=%i bpp=%i", img.width, img.height, bits_per_pixel);
            return false;
        }
        if (img.bits_per_pixel == bits_per_pixel)
        {
            memcpy(*pbuf, img.bits, img.width*img.height*bits_per_pixel/8);
        }
        else if (img.bits_per_pixel == 24 && bits_per_pixel == 32)
        {
            unsigned char *pSrc = img.bits;
            unsigned char *pDst = *pbuf;
            for (int i = 0; i < (int)(img.width*img.height); i++)
            {
                *pDst++ = *pSrc++;
                *pDst++ = *pSrc++;
                *pDst++ = *pSrc++;
                *pDst++ = 0;
            }
        }
        else if (img.bits_per_pixel == 32 && bits_per_pixel == 24)
        {
            unsigned char *pSrc = img.bits;
            unsigned char *pDst = *pbuf;
            for (int i = 0; i < (int)(img.width*img.height); i++)
            {
                *pDst++ = *pSrc++;
                *pDst++ = *pSrc++;
                *pDst++ = *pSrc++;
                pSrc++;
            }
        }
        else if (img.bits_per_pixel == 8 && (bits_per_pixel == 24 || bits_per_pixel == 32))
        {
            // do palette lookup and convert 8bpp to 24/32bpp
            int destBytePP = bits_per_pixel / 8;
            int paletteBytePP = img.bytes_per_palette_entry;
            unsigned char *pSrc = img.bits;
            unsigned char *pDst = *pbuf;
            // clear the destination image data (just to clear alpha if bpp=32)
            memset(*pbuf, 0, img.width*img.height*destBytePP);
            for (int i = 0; i < (int)(img.width*img.height); i++)
            {
                unsigned char clridx = *pSrc++;
                unsigned char *palcolor = img.palette + clridx * paletteBytePP;
                pDst[0] = palcolor[2]; // red
                pDst[1] = palcolor[1]; // green
                pDst[2] = palcolor[0]; // blue
                pDst += destBytePP;
            }
        }
        else
        {
            DebugMessage(M64MSG_ERROR, "PNG file '%s' is %i bpp but texture is %i bpp.", filename, img.bits_per_pixel, bits_per_pixel);
            delete [] *pbuf;
            *pbuf = NULL;
        }

        width = img.width;
        height = img.height;
        FreeBMGImage(&img);

        return true;
    }
    else
    {
        DebugMessage(M64MSG_ERROR, "ReadPNG() returned error for '%s' in LoadRGBBufferFromPNGFile!", filename);
        *pbuf = NULL;
        return false;
    }
}

bool LoadRGBABufferFromColorIndexedFile(char *filename, TxtrCacheEntry &entry, unsigned char **pbuf, int &width, int &height)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    FILE *f;
    f = fopen(filename, "rb");
    if(f != NULL)
    {
        if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, f) != 1 ||
            fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, f) != 1)
        {
            DebugMessage(M64MSG_ERROR, "Couldn't read BMP headers in file '%s'", filename);
            return false;
        }

        if( infoHeader.biBitCount != 4 && infoHeader.biBitCount != 8 )
        {
            fclose(f);
            DebugMessage(M64MSG_ERROR, "Unsupported BMP file format: %s", filename);
            *pbuf = NULL;
            return false;
        }

        int tablesize = infoHeader.biBitCount == 4 ? 16 : 256;
        uint32 *pTable = new uint32[tablesize];
        if (fread(pTable, tablesize*4, 1, f) != 1)
        {
            DebugMessage(M64MSG_ERROR, "Couldn't read BMP palette in file '%s'", filename);
            delete [] pTable;
            return false;
        }

        // Create the pallette table
        uint16 * pPal = (uint16 *)entry.ti.PalAddress;
        if( entry.ti.Size == TXT_SIZE_4b )
        {
            // 4-bit table
            for( int i=0; i<16; i++ )
            {
                pTable[i] = entry.ti.TLutFmt == TLUT_FMT_RGBA16 ? Convert555ToRGBA(pPal[i^1]) : ConvertIA16ToRGBA(pPal[i^1]);
            }
        }
        else
        {
            // 8-bit table
            for( int i=0; i<256; i++ )
            {
                pTable[i] = entry.ti.TLutFmt == TLUT_FMT_RGBA16 ? Convert555ToRGBA(pPal[i^1]) : ConvertIA16ToRGBA(pPal[i^1]);
            }
        }

        *pbuf = new unsigned char[infoHeader.biWidth*infoHeader.biHeight*4];
        if( *pbuf )
        {
            unsigned char *colorIdxBuf = new unsigned char[infoHeader.biSizeImage];
            if( colorIdxBuf )
            {
                if (fread(colorIdxBuf, infoHeader.biSizeImage, 1, f) != 1)
                {
                    DebugMessage(M64MSG_ERROR, "Couldn't read BMP image data in file '%s'", filename);
                }

                width = infoHeader.biWidth;
                height = infoHeader.biHeight;

                // Converting pallette texture to RGBA texture
                int idx = 0;
                uint32 *pbuf2 = (uint32*) *pbuf;

                for( int i=height-1; i>=0; i--)
                {
                    for( int j=0; j<width; j++)
                    {
                        if( entry.ti.Size == TXT_SIZE_4b )
                        {
                            // 4 bits
                            if( idx%2 )
                            {
                                // 1
                                *pbuf2++ = pTable[colorIdxBuf[(idx++)>>1]&0xF];
                            }
                            else
                            {
                                // 0
                                *pbuf2++ = pTable[(colorIdxBuf[(idx++)>>1]>>4)&0xF];
                            }
                        }
                        else
                        {
                            // 8 bits
                            *pbuf2++ = pTable[colorIdxBuf[idx++]];
                        }
                    }
                    if( entry.ti.Size == TXT_SIZE_4b )
                    {
                        if( idx%8 ) idx = (idx/8+1)*8;
                    }
                    else
                    {
                        if( idx%4 ) idx = (idx/4+1)*4;
                    }
                }

                delete [] colorIdxBuf;
            }
            else
            {
                TRACE0("Out of memory");
            }

            delete [] pTable;
            return true;
        }
        else
        {
            fclose(f);
            delete [] pTable;
            return false;
        }
    }
    else
    {
        // Do something
        TRACE1("Fail to open file %s", filename);
        *pbuf = NULL;
        return false;
    }
}

bool LoadRGBBufferFromBMPFile(char *filename, unsigned char **pbuf, int &width, int &height)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    FILE *f;
    f = fopen(filename, "rb");
    if(f != NULL)
    {
        if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, f) != 1 ||
            fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, f) != 1)
        {
            DebugMessage(M64MSG_ERROR, "Couldn't read BMP headers in file '%s'", filename);
            return false;
        }

        if( infoHeader.biBitCount != 24 )
        {
            fclose(f);
            DebugMessage(M64MSG_ERROR, "Unsupported BMP file 16 bits format: %s", filename);
            *pbuf = NULL;
            return false;
        }

        *pbuf = new unsigned char[infoHeader.biWidth*infoHeader.biHeight*3];
        if( *pbuf )
        {
            if (fread(*pbuf, infoHeader.biWidth*infoHeader.biHeight*3, 1, f) != 1)
                DebugMessage(M64MSG_ERROR, "Couldn't read RGB BMP image data in file '%s'", filename);
            fclose(f);
            width = infoHeader.biWidth;
            height = infoHeader.biHeight;
            return true;
        }
        else
        {
            fclose(f);
            return false;
        }
    }
    else
    {
        // Do something
        DebugMessage(M64MSG_WARNING, "Fail to open file %s", filename);
        *pbuf = NULL;
        return false;
    }
}

void LoadHiresTexture( TxtrCacheEntry &entry )
{
    if( entry.bExternalTxtrChecked )
        return;

    if( entry.pEnhancedTexture )
    {
        SAFE_DELETE(entry.pEnhancedTexture);
    }

    int ciidx, scaleShift;
    int idx = CheckTextureInfos(gHiresTxtrInfos,entry,ciidx,scaleShift,false);
    if( idx < 0 )
    {
        entry.bExternalTxtrChecked = true;
        return;
    }

    // Load the bitmap file
    char filename_rgb[PATH_MAX];
    char filename_a[PATH_MAX];

    strcpy(filename_rgb, gHiresTxtrInfos[idx].foldername);

    sprintf(filename_rgb+strlen(filename_rgb), "%s#%08X#%d#%d", g_curRomInfo.szGameName, entry.dwCRC, entry.ti.Format, entry.ti.Size);
    strcpy(filename_a,filename_rgb);
    strcat(filename_rgb,gHiresTxtrInfos[idx].RGBNameTail);
    strcat(filename_a,gHiresTxtrInfos[idx].AlphaNameTail);

    // Load BMP image to buffer_rbg
    unsigned char *buf_rgba = NULL;
    unsigned char *buf_a = NULL;
    int width, height;

    bool bResRGBA=false, bResA=false;
    bool bCI = ((gRDP.otherMode.text_tlut>=2 || entry.ti.Format == TXT_FMT_CI || entry.ti.Format == TXT_FMT_RGBA) && entry.ti.Size <= TXT_SIZE_8b );

    switch( gHiresTxtrInfos[idx].type )
    {
        case RGB_PNG:
            if( bCI )   
                return;
            else
            {
                bResRGBA = LoadRGBBufferFromPNGFile(filename_rgb, &buf_rgba, width, height);
                if( bResRGBA && gHiresTxtrInfos[idx].bSeparatedAlpha )
                    bResA = LoadRGBBufferFromPNGFile(filename_a, &buf_a, width, height);
            }
            break;
        case COLOR_INDEXED_BMP:
            if( bCI )   
                bResRGBA = LoadRGBABufferFromColorIndexedFile(filename_rgb, entry, &buf_rgba, width, height);
            else
                return;
            break;
        case RGBA_PNG_FOR_CI:
        case RGBA_PNG_FOR_ALL_CI:
            if( bCI )   
                bResRGBA = LoadRGBBufferFromPNGFile(filename_rgb, &buf_rgba, width, height, 32);
            else
                return;
            break;
        case RGB_WITH_ALPHA_TOGETHER_PNG:
            if( bCI )   
                return;
            else
                bResRGBA = LoadRGBBufferFromPNGFile(filename_rgb, &buf_rgba, width, height, 32);
            break;
        default:
            return;
    }

    if( !bResRGBA )
    {
        DebugMessage(M64MSG_ERROR, "RGBBuffer creation failed for file '%s'.", filename_rgb);
        return;
    }
    else if( gHiresTxtrInfos[idx].bSeparatedAlpha && !bResA )
    {
        DebugMessage(M64MSG_ERROR, "Alpha buffer creation failed for file '%s'.", filename_a);
        delete [] buf_rgba;
        return;
    }

    // calculate the texture size magnification by comparing the N64 texture size and the hi-res texture size
    int scale = 1 << scaleShift;
    int mirrorx = 1;
    int mirrory = 1;
    if (entry.ti.WidthToCreate/entry.ti.WidthToLoad == 2) mirrorx = 2;
    if (entry.ti.HeightToCreate/entry.ti.HeightToLoad == 2) mirrory = 2;
    entry.pEnhancedTexture = CDeviceBuilder::GetBuilder()->CreateTexture(entry.ti.WidthToCreate*scale, entry.ti.HeightToCreate*scale);
    DrawInfo info;
    
    if( entry.pEnhancedTexture && entry.pEnhancedTexture->StartUpdate(&info) )
    {

        if( gHiresTxtrInfos[idx].type == RGB_PNG )
        {
            unsigned char *pRGB = buf_rgba;
            unsigned char *pA = buf_a;

            if (info.lPitch < width * 4)
                DebugMessage(M64MSG_ERROR, "Texture pitch %i less than width %i times 4", info.lPitch, width);
            if (height > info.dwHeight)
                DebugMessage(M64MSG_ERROR, "Texture source height %i greater than destination height %i", height, info.dwHeight);

            // Update the texture by using the buffer
            for( int i=height-1; i>=0; i--)
            {
                unsigned char* pdst = (unsigned char*)info.lpSurface + i*info.lPitch;
                for( int j=0; j<width; j++)
                {
                    *pdst++ = *pRGB++;      // R
                    *pdst++ = *pRGB++;      // G
                    *pdst++ = *pRGB++;      // B

                    if( gHiresTxtrInfos[idx].bSeparatedAlpha )
                    {
                        *pdst++ = *pA;
                        pA += 3;
                    }
                    else if( entry.ti.Format == TXT_FMT_I )
                    {
                        *pdst++ = pRGB[-1];
                    }
                    else
                    {
                        *pdst++ = 0xFF;
                    }
                }
            }
        }
        else
        {
            // Update the texture by using the buffer
            uint32 *pRGB = (uint32*)buf_rgba;
            for( int i=height-1; i>=0; i--)
            {
                uint32 *pdst = (uint32*)((unsigned char*)info.lpSurface + i*info.lPitch);
                for( int j=0; j<width; j++)
                {
                    *pdst++ = *pRGB++;      // RGBA
                }
            }
        }

        if (mirrorx == 2)
        {
            //printf("Mirror: ToCreate: (%d,%d) ToLoad: (%d,%d) Scale: (%i,%i) Mirror: (%i,%i) Size: (%i,%i) Mask: %i\n", entry.ti.WidthToCreate, entry.ti.HeightToCreate, entry.ti.WidthToLoad, entry.ti.HeightToLoad, scale, scale, mirrorx, mirrory, width, height, entry.ti.maskS+scaleShift);
            gTextureManager.Mirror(info.lpSurface, width, entry.ti.maskS+scaleShift, width*2, width*2, height, S_FLAG, 4 );
        }

        if (mirrory == 2)
        {
            //printf("Mirror: ToCreate: (%d,%d) ToLoad: (%d,%d) Scale: (%i,%i) Mirror: (%i,%i) Size: (%i,%i) Mask: %i\n", entry.ti.WidthToCreate, entry.ti.HeightToCreate, entry.ti.WidthToLoad, entry.ti.HeightToLoad, scale, scale, mirrorx, mirrory, width, height, entry.ti.maskT+scaleShift);
            gTextureManager.Mirror(info.lpSurface, height, entry.ti.maskT+scaleShift, height*2, entry.pEnhancedTexture->m_dwCreatedTextureWidth, height, T_FLAG, 4 );
        }

        if( entry.ti.WidthToCreate*scale < entry.pEnhancedTexture->m_dwCreatedTextureWidth )
        {
            // Clamp
            gTextureManager.Clamp(info.lpSurface, width, entry.pEnhancedTexture->m_dwCreatedTextureWidth, entry.pEnhancedTexture->m_dwCreatedTextureWidth, height, S_FLAG, 4 );
        }
        if( entry.ti.HeightToCreate*scale < entry.pEnhancedTexture->m_dwCreatedTextureHeight )
        {
            // Clamp
            gTextureManager.Clamp(info.lpSurface, height, entry.pEnhancedTexture->m_dwCreatedTextureHeight, entry.pEnhancedTexture->m_dwCreatedTextureWidth, height, T_FLAG, 4 );
        }

        entry.pEnhancedTexture->EndUpdate(&info);
        entry.pEnhancedTexture->SetOthersVariables();
        entry.pEnhancedTexture->m_bIsEnhancedTexture = true;
        entry.dwEnhancementFlag = TEXTURE_EXTERNAL;

        DebugMessage(M64MSG_VERBOSE, "Loaded hi-res texture: %s", filename_rgb);
    }
    else
    {
        DebugMessage(M64MSG_ERROR, "New texture creation failed.");
        TRACE0("Cannot create a new texture");
    }

    if( buf_rgba )
    {
        delete [] buf_rgba;
    }

    if( buf_a )
    {
        delete [] buf_a;
    }
}

