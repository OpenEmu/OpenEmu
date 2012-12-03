/*
Copyright (C) 2002 Rice1964

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

#ifndef _FRAME_BUFFER_H_
#define _FRAME_BUFFER_H_

#include "typedefs.h"
#include "RenderTexture.h"
#include "TextureManager.h"

typedef int SURFFORMAT;

extern void TexRectToN64FrameBuffer_16b(uint32 x0, uint32 y0, uint32 width, uint32 height, uint32 dwTile);
extern void TexRectToFrameBuffer_8b(uint32 dwXL, uint32 dwYL, uint32 dwXH, uint32 dwYH, float t0u0, float t0v0, float t0u1, float t0v1, uint32 dwTile);

class FrameBufferManager
{
    friend class CGraphicsContext;
    friend class CDXGraphicsContext;
public:
    FrameBufferManager();
    virtual ~FrameBufferManager();

    void Initialize();
    void CloseUp();
    void Set_CI_addr(SetImgInfo &newCI);
    void UpdateRecentCIAddr(SetImgInfo &ciinfo);
    void SetAddrBeDisplayed(uint32 addr);
    bool HasAddrBeenDisplayed(uint32 addr, uint32 width);
    int FindRecentCIInfoIndex(uint32 addr);
    bool IsDIaRenderTexture();

    int         CheckAddrInRenderTextures(uint32 addr, bool checkcrc = true);
    uint32      ComputeRenderTextureCRCInRDRAM(int infoIdx);
    void        CheckRenderTextureCRCInRDRAM(void);
    int         CheckRenderTexturesWithNewCI(SetImgInfo &CIinfo, uint32 height, bool byNewTxtrBuf);
    virtual void ClearN64FrameBufferToBlack(uint32 left=0, uint32 top=0, uint32 width=0, uint32 height=0);
    virtual int SetBackBufferAsRenderTexture(SetImgInfo &CIinfo, int ciInfoIdx);
    void        LoadTextureFromRenderTexture(TxtrCacheEntry* pEntry, int infoIdx);
    void        UpdateFrameBufferBeforeUpdateFrame();
    virtual void RestoreNormalBackBuffer();                 // restore the normal back buffer
    virtual void CopyBackToFrameBufferIfReadByCPU(uint32 addr);
    virtual void SetRenderTexture(void);
    virtual void CloseRenderTexture(bool toSave);
    virtual void ActiveTextureBuffer(void);

    int IsAddrInRecentFrameBuffers(uint32 addr);
    int CheckAddrInBackBuffers(uint32 addr, uint32 memsize, bool copyToRDRAM = false);

    uint8 CIFindIndex(uint16 val);
    uint32 ComputeCImgHeight(SetImgInfo &info, uint32 &height);

    int FindASlot(void);

    bool ProcessFrameWriteRecord();
    void FrameBufferWriteByCPU(uint32 addr, uint32 size);
    void FrameBufferReadByCPU( uint32 addr );
    bool FrameBufferInRDRAMCheckCRC();
    void StoreRenderTextureToRDRAM(int infoIdx = -1);

    virtual bool IsRenderingToTexture() {return m_isRenderingToTexture;}

    // Device dependent functions
    virtual void SaveBackBuffer(int ciInfoIdx, RECT* pRect=NULL, bool forceToSaveToRDRAM = false);          // Copy the current back buffer to temp buffer
    virtual void CopyBackBufferToRenderTexture(int idx, RecentCIInfo &ciInfo, RECT* pRect=NULL) {}          // Copy the current back buffer to temp buffer
    virtual void CopyBufferToRDRAM(uint32 addr, uint32 fmt, uint32 siz, uint32 width, 
        uint32 height, uint32 bufWidth, uint32 bufHeight, uint32 startaddr, 
        uint32 memsize, uint32 pitch, TextureFmt bufFmt, void *surf, uint32 bufPitch);
    virtual void StoreBackBufferToRDRAM(uint32 addr, uint32 fmt, uint32 siz, uint32 width, 
        uint32 height, uint32 bufWidth, uint32 bufHeight, uint32 startaddr=0xFFFFFFFF, 
        uint32 memsize=0xFFFFFFFF, uint32 pitch=0, SURFFORMAT surf_fmt=SURFFMT_A8R8G8B8) {}
#ifdef DEBUGGER
    virtual void DisplayRenderTexture(int infoIdx = -1);
#endif

protected:
    bool    m_isRenderingToTexture;
    int     m_curRenderTextureIndex;
    int     m_lastTextureBufferIndex;
};

class DXFrameBufferManager : public FrameBufferManager
{
    virtual ~DXFrameBufferManager() {}

public:
    // Device dependent functions
    virtual void CopyBackBufferToRenderTexture(int idx, RecentCIInfo &ciInfo, RECT* pRect=NULL);            // Copy the current back buffer to temp buffer
    virtual void StoreBackBufferToRDRAM(uint32 addr, uint32 fmt, uint32 siz, uint32 width, 
        uint32 height, uint32 bufWidth, uint32 bufHeight, uint32 startaddr=0xFFFFFFFF, 
        uint32 memsize=0xFFFFFFFF, uint32 pitch=0, SURFFORMAT surf_fmt=SURFFMT_A8R8G8B8);
};

class OGLFrameBufferManager : public FrameBufferManager
{
    virtual ~OGLFrameBufferManager() {}

public:
    // Device dependent functions
    virtual void CopyBackBufferToRenderTexture(int idx, RecentCIInfo &ciInfo, RECT* pRect=NULL);            // Copy the current back buffer to temp buffer
    virtual void StoreBackBufferToRDRAM(uint32 addr, uint32 fmt, uint32 siz, uint32 width, 
        uint32 height, uint32 bufWidth, uint32 bufHeight, uint32 startaddr=0xFFFFFFFF, 
        uint32 memsize=0xFFFFFFFF, uint32 pitch=0, SURFFORMAT surf_fmt=SURFFMT_A8R8G8B8);
};

extern RenderTextureInfo gRenderTextureInfos[];
extern RenderTextureInfo newRenderTextureInfo;

#define NEW_TEXTURE_BUFFER

extern RenderTextureInfo g_ZI_saves[2];
extern RenderTextureInfo *g_pRenderTextureInfo;


extern FrameBufferManager* g_pFrameBufferManager;

extern RecentCIInfo g_RecentCIInfo[];
extern RecentViOriginInfo g_RecentVIOriginInfo[];
extern RenderTextureInfo gRenderTextureInfos[];
extern int numOfTxtBufInfos;
extern RecentCIInfo *g_uRecentCIInfoPtrs[5];
extern uint8 RevTlutTable[0x10000];

extern uint32 CalculateRDRAMCRC(void *pAddr, uint32 left, uint32 top, uint32 width, uint32 height, uint32 size, uint32 pitchInBytes );
extern uint16 ConvertRGBATo555(uint8 r, uint8 g, uint8 b, uint8 a);
extern uint16 ConvertRGBATo555(uint32 color32);
extern void InitTlutReverseLookup(void);

#endif

