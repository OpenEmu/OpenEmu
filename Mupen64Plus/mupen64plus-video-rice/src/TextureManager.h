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

#ifndef __TEXTUREHANDLER_H__
#define __TEXTUREHANDLER_H__

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_CHECK
# define SAFE_CHECK(a)  if( (a) == NULL ) {DebugMessage(M64MSG_ERROR, "Creater out of memory"); throw new std::exception();}
#endif

#include <string.h>

#include "typedefs.h"
#include "Texture.h"
#define absi(x)     ((x)>=0?(x):(-x))
#define S_FLAG  0
#define T_FLAG  1

class TxtrInfo
{
public:
    uint32 WidthToCreate;
    uint32 HeightToCreate;

    uint32 Address;
    void  *pPhysicalAddress;

    uint32 Format;
    uint32 Size;

    int  LeftToLoad;
    int  TopToLoad;
    uint32 WidthToLoad;
    uint32 HeightToLoad;
    uint32 Pitch;

    uchar *PalAddress;
    uint32 TLutFmt;
    uint32 Palette;
    
    BOOL  bSwapped;
    
    uint32 maskS;
    uint32 maskT;

    BOOL  clampS;
    BOOL  clampT;
    BOOL  mirrorS;
    BOOL  mirrorT;

    int   tileNo;

    inline TxtrInfo& operator = (const TxtrInfo& src)
    {
        memcpy(this, &src, sizeof( TxtrInfo ));
        return *this;
    }

    inline TxtrInfo& operator = (const Tile& tile)
    {
        Format = tile.dwFormat;
        Size = tile.dwSize;
        Palette = tile.dwPalette;
        
        maskS = tile.dwMaskS;
        maskT = tile.dwMaskT;
        mirrorS = tile.bMirrorS;
        mirrorT = tile.bMirrorT;
        clampS = tile.bClampS;
        clampT = tile.bClampT;

        return *this;
    }

    inline bool operator == ( const TxtrInfo& sec)
    {
        return (
            Address == sec.Address &&
            WidthToLoad == sec.WidthToLoad &&
            HeightToLoad == sec.HeightToLoad &&
            WidthToCreate == sec.WidthToCreate &&
            HeightToCreate == sec.HeightToCreate &&
            maskS == sec.maskS &&
            maskT == sec.maskT &&
            TLutFmt == sec.TLutFmt &&
            PalAddress == sec.PalAddress &&
            Palette == sec.Palette &&
            LeftToLoad == sec.LeftToLoad &&
            TopToLoad == sec.TopToLoad &&           
            Format == sec.Format &&         
            Size == sec.Size &&
            Pitch == sec.Pitch &&
            bSwapped == sec.bSwapped &&
            TLutFmt == sec.TLutFmt &&
            mirrorS == sec.mirrorS &&
            mirrorT == sec.mirrorT &&
            clampS == sec.clampS &&
            clampT == sec.clampT
            );
    }

    inline bool isEqual(const TxtrInfo& sec)
    {
        return (*this == sec);
    }
    
} ;



typedef struct TxtrCacheEntry
{
    TxtrCacheEntry():
        pTexture(NULL),pEnhancedTexture(NULL),txtrBufIdx(0) {}

    ~TxtrCacheEntry()
    {
        SAFE_DELETE(pTexture);
        SAFE_DELETE(pEnhancedTexture);
    }
    
    struct TxtrCacheEntry *pNext;       // Must be first element!

    struct TxtrCacheEntry *pNextYoungest;
    struct TxtrCacheEntry *pLastYoungest;

    TxtrInfo ti;
    uint32      dwCRC;
    uint32      dwPalCRC;
    int         maxCI;

    uint32  dwUses;         // Total times used (for stats)
    uint32  dwTimeLastUsed; // timeGetTime of time of last usage
    uint32  FrameLastUsed;  // Frame # that this was last used
    uint32  FrameLastUpdated;

    CTexture    *pTexture;
    CTexture    *pEnhancedTexture;

    uint32      dwEnhancementFlag;
    int         txtrBufIdx;
    bool        bExternalTxtrChecked;

    TxtrCacheEntry *lastEntry;
} TxtrCacheEntry;


//*****************************************************************************
// Texture cache implementation
//*****************************************************************************
class CTextureManager
{
protected:
    TxtrCacheEntry * CreateNewCacheEntry(uint32 dwAddr, uint32 dwWidth, uint32 dwHeight);
    void AddTexture(TxtrCacheEntry *pEntry);
    void RemoveTexture(TxtrCacheEntry * pEntry);
    void RecycleTexture(TxtrCacheEntry *pEntry);
    TxtrCacheEntry * ReviveTexture( uint32 width, uint32 height );
    TxtrCacheEntry * GetTxtrCacheEntry(TxtrInfo * pti);
    
    void ConvertTexture(TxtrCacheEntry * pEntry, bool fromTMEM);
    void ConvertTexture_16(TxtrCacheEntry * pEntry, bool fromTMEM);

    void ClampS32(uint32 *array, uint32 width, uint32 towidth, uint32 arrayWidth, uint32 rows);
    void ClampS16(uint16 *array, uint32 width, uint32 towidth, uint32 arrayWidth, uint32 rows);
    void ClampT32(uint32 *array, uint32 height, uint32 toheight, uint32 arrayWidth, uint32 cols);
    void ClampT16(uint16 *array, uint32 height, uint32 toheight, uint32 arrayWidth, uint32 cols);

    void MirrorS32(uint32 *array, uint32 width, uint32 mask, uint32 towidth, uint32 arrayWidth, uint32 rows);
    void MirrorS16(uint16 *array, uint32 width, uint32 mask, uint32 towidth, uint32 arrayWidth, uint32 rows);
    void MirrorT32(uint32 *array, uint32 height, uint32 mask, uint32 toheight, uint32 arrayWidth, uint32 cols);
    void MirrorT16(uint16 *array, uint32 height, uint32 mask, uint32 toheight, uint32 arrayWidth, uint32 cols);

    void WrapS32(uint32 *array, uint32 width, uint32 mask, uint32 towidth, uint32 arrayWidth, uint32 rows);
    void WrapS16(uint16 *array, uint32 width, uint32 mask, uint32 towidth, uint32 arrayWidth, uint32 rows);
    void WrapT32(uint32 *array, uint32 height, uint32 mask, uint32 toheight, uint32 arrayWidth, uint32 cols);
    void WrapT16(uint16 *array, uint32 height, uint32 mask, uint32 toheight, uint32 arrayWidth, uint32 cols);

    void ExpandTextureS(TxtrCacheEntry * pEntry);
    void ExpandTextureT(TxtrCacheEntry * pEntry);
    void ExpandTexture(TxtrCacheEntry * pEntry, uint32 sizeOfLoad, uint32 sizeToCreate, uint32 sizeCreated,
        int arrayWidth, int flag, int mask, int mirror, int clamp, uint32 otherSize);

    uint32 Hash(uint32 dwValue);
    bool TCacheEntryIsLoaded(TxtrCacheEntry *pEntry);

    void updateColorTexture(CTexture *ptexture, uint32 color);
    
public:
    void Wrap(void *array, uint32 width, uint32 mask, uint32 towidth, uint32 arrayWidth, uint32 rows, int flag, int size );
    void Clamp(void *array, uint32 width, uint32 towidth, uint32 arrayWidth, uint32 rows, int flag, int size );
    void Mirror(void *array, uint32 width, uint32 mask, uint32 towidth, uint32 arrayWidth, uint32 rows, int flag, int size );
    
protected:
    TxtrCacheEntry * m_pHead;
    TxtrCacheEntry ** m_pCacheTxtrList;
    uint32 m_numOfCachedTxtrList;

    TxtrCacheEntry m_blackTextureEntry;
    TxtrCacheEntry m_PrimColorTextureEntry;
    TxtrCacheEntry m_EnvColorTextureEntry;
    TxtrCacheEntry m_LODFracTextureEntry;
    TxtrCacheEntry m_PrimLODFracTextureEntry;
    TxtrCacheEntry * GetPrimColorTexture(uint32 color);
    TxtrCacheEntry * GetEnvColorTexture(uint32 color);
    TxtrCacheEntry * GetLODFracTexture(uint8 fac);
    TxtrCacheEntry * GetPrimLODFracTexture(uint8 fac);

    void MakeTextureYoungest(TxtrCacheEntry *pEntry);
    unsigned int m_currentTextureMemUsage;
    TxtrCacheEntry *m_pYoungestTexture;
    TxtrCacheEntry *m_pOldestTexture;

public:
    CTextureManager();
    ~CTextureManager();

    TxtrCacheEntry * GetBlackTexture(void);
    TxtrCacheEntry * GetConstantColorTexture(uint32 constant);
    TxtrCacheEntry * GetTexture(TxtrInfo * pgti, bool fromTMEM, bool doCRCCheck=true, bool AutoExtendTexture = false);
    
    void PurgeOldTextures();
    void RecycleAllTextures();
    void RecheckHiresForAllTextures();
    bool CleanUp();
    
#ifdef DEBUGGER
    TxtrCacheEntry * GetCachedTexture(uint32 tex);
    uint32 GetNumOfCachedTexture();
#endif
};

extern CTextureManager gTextureManager;     // The global instance of CTextureManager class
extern void DumpCachedTexture(TxtrCacheEntry &entry);

#endif

