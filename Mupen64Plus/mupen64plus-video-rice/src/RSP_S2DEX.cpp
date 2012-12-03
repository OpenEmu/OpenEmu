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


// This file implements the S2DEX ucode, Yoshi story is using this ucodes

#include "UcodeDefs.h"
#include "Render.h"
#include "Timing.h"

uObjTxtr *gObjTxtr = NULL;
uObjTxtrTLUT *gObjTlut = NULL;
uint32 gObjTlutAddr = 0;
uObjMtx *gObjMtx = NULL;
uObjSubMtx *gSubObjMtx = NULL;
uObjMtxReal gObjMtxReal = {1, 0, 0, 1, 0, 0, 0, 0};
Matrix g_MtxReal(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);

uint32 g_TxtLoadBy = CMD_LOAD_OBJ_TXTR;


// YoshiStory uses this - 0x02
void RSP_S2DEX_BG_COPY(Gfx *gfx)
{
    
    

    SP_Timing(DP_Minimal16);
    DP_Timing(DP_Minimal16);

    uint32 dwAddr = RSPSegmentAddr((gfx->words.w1));
    uObjBg *sbgPtr = (uObjBg*)(g_pRDRAMu8+dwAddr);
    CRender::g_pRender->LoadObjBGCopy(*sbgPtr);
    CRender::g_pRender->DrawObjBGCopy(*sbgPtr);
}

// YoshiStory uses this - 0x03
void RSP_S2DEX_OBJ_RECTANGLE(Gfx *gfx)
{
    
    

    uint32 dwAddr = RSPSegmentAddr((gfx->words.w1));
    uObjSprite *ptr = (uObjSprite*)(g_pRDRAMu8+dwAddr);

    uObjTxSprite objtx;
    memcpy(&objtx.sprite,ptr,sizeof(uObjSprite));

    if( g_TxtLoadBy == CMD_LOAD_OBJ_TXTR )
    {
        memcpy(&(objtx.txtr.block),&(gObjTxtr->block),sizeof(uObjTxtr));
        CRender::g_pRender->LoadObjSprite(objtx,true);
    }
    else
    {
        PrepareTextures();
    }
    CRender::g_pRender->DrawSprite(objtx, false);

#ifdef DEBUGGER
    if( (pauseAtNext && (eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI)) || logTextures )
    {   
        if( debuggerPauseCount > 0 ) 
            debuggerPauseCount--; 
        if( debuggerPauseCount == 0 )
        {
            eventToPause = false;
            debuggerPause = true;
            TRACE3("Paused at RSP_S2DEX_OBJ_RECTANGLE\nptr=%08X, img=%08X, Tmem=%08X",
                dwAddr,objtx.txtr.block.image, ptr->imageAdrs);
            CGraphicsContext::g_pGraphicsContext->UpdateFrame();
        }
    }
#endif
}

// YoshiStory uses this - 0x04
void RSP_S2DEX_OBJ_SPRITE(Gfx *gfx)
{
    
    

    uint32 dwAddr = RSPSegmentAddr((gfx->words.w1));
    uObjSprite *info = (uObjSprite*)(g_pRDRAMu8+dwAddr);

    uint32 dwTile   = gRSP.curTile;
    status.bAllowLoadFromTMEM = false;  // Because we need to use TLUT loaded by ObjTlut cmd
    PrepareTextures();
    status.bAllowLoadFromTMEM = true;
    
    //CRender::g_pRender->SetCombinerAndBlender();

    uObjTxSprite drawinfo;
    memcpy( &(drawinfo.sprite), info, sizeof(uObjSprite));
    CRender::g_pRender->DrawSpriteR(drawinfo, false, dwTile, 0, 0, drawinfo.sprite.imageW/32, drawinfo.sprite.imageH/32);


    /*
    static BOOL bWarned = FALSE;
    //if (!bWarned)
    {
        RSP_RDP_NOIMPL("RDP: RSP_S2DEX_OBJ_SPRITE (0x%08x 0x%08x)", (gfx->words.w0), (gfx->words.w1));
        bWarned = TRUE;
    }
    */

#ifdef DEBUGGER
    if( (pauseAtNext && (eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI)) || logTextures )
    {   
        eventToPause = false;
        debuggerPause = true;
        TRACE0("Paused at RSP_S2DEX_OBJ_SPRITE");
        CGraphicsContext::g_pGraphicsContext->UpdateFrame();
    }
#endif
}

// YoshiStory uses this - 0xb0
void RSP_S2DEX_SELECT_DL(Gfx *gfx)
{
    
    

    //static BOOL bWarned = FALSE;
    //if (!bWarned)
    {
        RSP_RDP_NOIMPL("RDP: RSP_S2DEX_SELECT_DL (0x%08x 0x%08x)", (gfx->words.w0), (gfx->words.w1));
        //bWarned = TRUE;
    }

    DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_OBJ_TXT_CMD, {DebuggerAppendMsg("Paused at RSP_S2DEX_SELECT_DL");});
}

void RSP_S2DEX_OBJ_RENDERMODE(Gfx *gfx)
{
    
    

    /*
    static BOOL bWarned = FALSE;
    //if (!bWarned)
    {
    RSP_RDP_NOIMPL("RDP: RSP_S2DEX_OBJ_RENDERMODE (0x%08x 0x%08x)", (gfx->words.w0), (gfx->words.w1));
    bWarned = TRUE;
    }
    */
}

// YoshiStory uses this - 0xb1
void RSP_GBI1_Tri2(Gfx *gfx);
void RSP_S2DEX_OBJ_RENDERMODE_2(Gfx *gfx)
{
    
    

    if( ((gfx->words.w0)&0xFFFFFF) != 0 || ((gfx->words.w1)&0xFFFFFF00) != 0 )
    {
        // This is a TRI2 cmd
        RSP_GBI1_Tri2(gfx);
        return;
    }

    RSP_S2DEX_OBJ_RENDERMODE(gfx);
}

#ifdef DEBUGGER
void DumpBlockParameters(uObjTxtrBlock &ptr)
{
    /*
    typedef struct  {   //Intel format
      uint32    type;       // S2DEX_OBJLT_TXTRBLOCK divided into types.                                
      uint64    *image;     // The texture source address on DRAM.       
  
      uint16    tsize;      // The Texture size.  Specified by the macro  GS_TB_TSIZE().            
      uint16    tmem;       // The  transferred TMEM word address.   (8byteWORD)  
  
      uint16    sid;        // STATE ID Multipled by 4.  Either one of  0,4,8 and 12.               
      uint16    tline;      // The width of the Texture 1-line. Specified by the macro GS_TB_TLINE()

      uint32    flag;       // STATE flag
      uint32    mask;       // STATE mask
    } uObjTxtrBlock;        // 24 bytes
    */

    DebuggerAppendMsg("uObjTxtrBlock Header in RDRAM: 0x%08X", (uint32) ((char *) &ptr - (char *) g_pRDRAMu8));
    DebuggerAppendMsg("ImgAddr=0x%08X(0x%08X), tsize=0x%X, \nTMEM=0x%X, sid=%d, tline=%d, flag=0x%X, mask=0x%X\n\n",
        RSPSegmentAddr(ptr.image), ptr.image, ptr.tsize, ptr.tmem, ptr.sid/4, ptr.tline, ptr.flag, ptr.mask);
}

void DumpSpriteParameters(uObjSprite &ptr)
{
    /*
    typedef struct {    // Intel format
      uint16  scaleW;       // Scaling of the u5.10 width direction.     
      short  objX;      // The x-coordinate of the upper-left end. s10.2 OBJ                
  
      uint16  paddingX; // Unused.  Always 0.        
      uint16  imageW;       // The width of the u10.5 texture. (The length of the S-direction.) 
  
      uint16  scaleH;       // Scaling of the u5.10 height direction. 
      short  objY;      // The y-coordinate of the s10.2 OBJ upper-left end.                
  
      uint16  paddingY; // Unused.  Always 0.              
      uint16  imageH;       // The height of the u10.5 texture. (The length of the T-direction.)
  
      uint16  imageAdrs;    // The texture header position in  TMEM.  (In units of 64bit word.)
      uint16  imageStride;  // The folding width of the texel.        (In units of 64bit word.) 

      uint8   imageFlags;   // The display flag.    S2DEX_OBJ_FLAG_FLIP*  
      uint8   imagePal; //The pallet number.  0-7                        
      uint8   imageSiz; // The size of the texel.         TXT_SIZE_*       
      uint8   imageFmt; // The format of the texel.   TXT_FMT_*       
    } uObjSprite;       // 24 bytes 
    */

    if( logTextures || (pauseAtNext && eventToPause == NEXT_OBJ_TXT_CMD) )
    {
        DebuggerAppendMsg("uObjSprite Header in RDRAM: 0x%08X", (uint32) ((char *) &ptr - (char *) g_pRDRAMu8));
        DebuggerAppendMsg("X=%d, Y=%d, W=%d, H=%d, scaleW=%f, scaleH=%f\n"
            "TAddr=0x%X, Stride=%d, Flag=0x%X, Pal=%d, Fmt=%s-%db\n\n", 
            ptr.objX/4, ptr.objY/4, ptr.imageW/32, ptr.imageH/32, ptr.scaleW/1024.0f, ptr.scaleH/1024.0f,
            ptr.imageAdrs, ptr.imageStride, ptr.imageFlags, ptr.imagePal, pszImgFormat[ptr.imageFmt], pnImgSize[ptr.imageSiz]);
    }
}

void DumpTileParameters(uObjTxtrTile &tile)
{
}

void DumpTlutParameters(uObjTxtrTLUT &tlut)
{
    /*
    typedef struct  {   // Intel Format
      uint32    type;       // S2DEX_OBJLT_TLUT divided into types.                            
      uint32    image;
  
      uint16    pnum;       // The loading pallet number -1.   
      uint16    phead;      // The pallet number of the load header.  Between 256 and 511. 
  
      uint16    sid;        // STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.    
      uint16   zero;        // Assign 0 all the time.                                      
  
      uint32    flag;       // STATE flag  
      uint32    mask;       // STATE mask  
    } uObjTxtrTLUT; // 24 bytes 
    */
    DebuggerAppendMsg("ImgAddr=0x%08X(0x%08X), pnum=%d, phead=%d, sid=%d, flag=0x%X, mask=0x%X\n\n",
        RSPSegmentAddr(tlut.image), tlut.image, tlut.pnum+1, tlut.phead, tlut.sid/4, tlut.flag, tlut.mask);
}


void DumpTxtrInfo(uObjTxtr *ptr)
{
    if( logTextures || (pauseAtNext && eventToPause == NEXT_OBJ_TXT_CMD) )
    {
        DebuggerAppendMsg("uObjTxtr Header in RDRAM: 0x%08X", (uint32) ((char *) ptr - (char *) g_pRDRAMu8));
        switch( ptr->block.type )
        {
        case S2DEX_OBJLT_TXTRBLOCK:
            TRACE0("Loading ObjTxtr: type=BLOCK");
            DumpBlockParameters(ptr->block);
            break;
        case S2DEX_OBJLT_TXTRTILE:
            TRACE0("Loading ObjTxtr: type=TILE");
            DumpTileParameters(ptr->tile);
            break;
        case S2DEX_OBJLT_TLUT:
            TRACE0("Loading ObjTxtr: type=TLUT");
            DumpTlutParameters(ptr->tlut);
            break;
        }
    }
}

void DumpObjMtx(bool fullmtx = true)
{
    if( logTextures || (pauseAtNext && eventToPause == NEXT_OBJ_TXT_CMD) )
    {
        if( fullmtx )
        DebuggerAppendMsg("A=%X, B=%X, C=%X, D=%X, X=%X, Y=%X, BaseX=%X, BaseY=%X",
            gObjMtx->A, gObjMtx->B, gObjMtx->C, gObjMtx->D, gObjMtx->X, gObjMtx->Y, gObjMtx->BaseScaleX, gObjMtx->BaseScaleY);
        else
            DebuggerAppendMsg("SubMatrix: X=%X, Y=%X, BaseX=%X, BaseY=%X", gSubObjMtx->X, gSubObjMtx->Y, gSubObjMtx->BaseScaleX, gSubObjMtx->BaseScaleY);
        
        DebuggerAppendMsg("A=%f, B=%f, C=%f, D=%f, X=%f, Y=%f, BaseX=%f, BaseY=%f",
            gObjMtxReal.A, gObjMtxReal.B, gObjMtxReal.C, gObjMtxReal.D, gObjMtxReal.X, gObjMtxReal.Y, gObjMtxReal.BaseScaleX, gObjMtxReal.BaseScaleY);
    }
}

#endif

void ObjMtxTranslate(float &x, float &y)
{
    float x1 = gObjMtxReal.A*x + gObjMtxReal.B*y + gObjMtxReal.X;
    float y1 = gObjMtxReal.C*x + gObjMtxReal.D*y + gObjMtxReal.Y;

    x = x1;
    y = y1;
}

void RSP_S2DEX_SPObjLoadTxtr(Gfx *gfx)
{
    
    

    gObjTxtr = (uObjTxtr*)(g_pRDRAMu8+(RSPSegmentAddr((gfx->words.w1))&(g_dwRamSize-1)));
    if( gObjTxtr->block.type == S2DEX_OBJLT_TLUT )
    {
        gObjTlut = (uObjTxtrTLUT*)gObjTxtr;
        gObjTlutAddr = (uint32)(RSPSegmentAddr(gObjTlut->image));
        
        // Copy tlut
        int size = gObjTlut->pnum+1;
        int offset = gObjTlut->phead-0x100;

        if( offset+size>0x100)
        {
            size = 0x100 - offset;
        }

        uint32 addr = (gObjTlutAddr);//&0xFFFFFFFC);
        //if( addr & 3 ) addr = (addr&0xFFFFFFF0)+8;;
        //uint16 *srcPal = (uint16*)(g_pRDRAMu8 + (addr& (g_dwRamSize-1)) );

        for( int i=offset; i<offset+size; i++ )
        {
            g_wRDPTlut[i^1] = RDRAM_UHALF(addr);
            addr += 2;
            //g_wRDPTlut[i] = (*(uint16 *)(addr+g_pRDRAMu8));
            //g_wRDPTlut[i] = *(srcPal++);
        }
    }
    else
    {
        // Loading ObjSprite
        g_TxtLoadBy = CMD_LOAD_OBJ_TXTR;
    }

    DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
        {
            DumpTxtrInfo(gObjTxtr);
            TRACE0("Paused at RSP_S2DEX_SPObjLoadTxtr");
        }
    );
}

// YoshiStory uses this - 0xc2
void RSP_S2DEX_SPObjLoadTxSprite(Gfx *gfx)
{
    
    

    uObjTxSprite* ptr = (uObjTxSprite*)(g_pRDRAMu8+(RSPSegmentAddr((gfx->words.w1))&(g_dwRamSize-1)));
    gObjTxtr = (uObjTxtr*)ptr;
    
    //Now draw the sprite
    CRender::g_pRender->LoadObjSprite(*ptr);
    CRender::g_pRender->DrawSpriteR(*ptr);

    DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
        {
            DumpTxtrInfo(gObjTxtr);
            DumpSpriteParameters(ptr->sprite);
            TRACE0("Paused at RSP_S2DEX_SPObjLoadTxSprite");
        }
    );
}


// YoshiStory uses this - 0xc3
void RSP_S2DEX_SPObjLoadTxRect(Gfx *gfx)
{
    
    

    uObjTxSprite* ptr = (uObjTxSprite*)(g_pRDRAMu8+(RSPSegmentAddr((gfx->words.w1))&(g_dwRamSize-1)));
    gObjTxtr = (uObjTxtr*)ptr;
    
    //Now draw the sprite
    CRender::g_pRender->LoadObjSprite(*ptr);
    CRender::g_pRender->DrawSprite(*ptr, false);

    DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
        {
            DumpTxtrInfo(gObjTxtr);
            DumpSpriteParameters(ptr->sprite);
            TRACE0("Paused at RSP_S2DEX_SPObjLoadTxRect");
        }
    );
}

// YoshiStory uses this - 0xc4
void RSP_S2DEX_SPObjLoadTxRectR(Gfx *gfx)
{
    
    

    uObjTxSprite* ptr = (uObjTxSprite*)(g_pRDRAMu8+(RSPSegmentAddr((gfx->words.w1))&(g_dwRamSize-1)));
    gObjTxtr = (uObjTxtr*)ptr;
    
    //Now draw the sprite
    CRender::g_pRender->LoadObjSprite(*ptr);
    CRender::g_pRender->DrawSprite(*ptr, true);

    DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
        {
            DumpTxtrInfo(gObjTxtr);
            DumpSpriteParameters(ptr->sprite);
            TRACE0("Paused at RSP_S2DEX_SPObjLoadTxRect");
        }
    );
}

void DLParser_TexRect(Gfx *gfx);
// YoshiStory uses this - 0xe4
void RSP_S2DEX_RDPHALF_0(Gfx *gfx)
{
    //RDP: RSP_S2DEX_RDPHALF_0 (0xe449c0a8 0x003b40a4)
    //0x001d3c88: e449c0a8 003b40a4 RDP_TEXRECT 
    //0x001d3c90: b4000000 00000000 RSP_RDPHALF_1
    //0x001d3c98: b3000000 04000400 RSP_RDPHALF_2

    uint32 dwPC = gDlistStack[gDlistStackPointer].pc;       // This points to the next instruction
    uint32 dwNextUcode = *(uint32 *)(g_pRDRAMu8 + dwPC);

    if( (dwNextUcode>>24) != S2DEX_SELECT_DL )
    {
        // Pokemom Puzzle League
        if( (dwNextUcode>>24) == 0xB4 )
        {
            DLParser_TexRect(gfx);
        }
        else
        {
            RSP_RDP_NOIMPL("RDP: RSP_S2DEX_RDPHALF_0 (0x%08x 0x%08x)", (gfx->words.w0), (gfx->words.w1));
        }
    }
    else
    {
        RSP_RDP_NOIMPL("RDP: RSP_S2DEX_RDPHALF_0 (0x%08x 0x%08x)", (gfx->words.w0), (gfx->words.w1));
        DEBUGGER_PAUSE_COUNT_N(NEXT_OBJ_TXT_CMD);
    }
}

// YoshiStory uses this - 0x05
void RSP_S2DEX_OBJ_MOVEMEM(Gfx *gfx)
{
    
    

    uint32 dwCommand = ((gfx->words.w0)>>16)&0xFF;
    uint32 dwLength  = ((gfx->words.w0))    &0xFFFF;
    uint32 dwAddr = RSPSegmentAddr((gfx->words.w1));

    if( dwAddr >= g_dwRamSize )
    {
        TRACE0("ObjMtx: memory ptr is invalid");
    }

    if( dwLength == 0 && dwCommand == 23 )
    {
        gObjMtx = (uObjMtx *)(dwAddr+g_pRDRAMu8);
        gObjMtxReal.A = gObjMtx->A/65536.0f;
        gObjMtxReal.B = gObjMtx->B/65536.0f;
        gObjMtxReal.C = gObjMtx->C/65536.0f;
        gObjMtxReal.D = gObjMtx->D/65536.0f;
        gObjMtxReal.X = float(gObjMtx->X>>2);
        gObjMtxReal.Y = float(gObjMtx->Y>>2);
        gObjMtxReal.BaseScaleX = gObjMtx->BaseScaleX/1024.0f;
        gObjMtxReal.BaseScaleY = gObjMtx->BaseScaleY/1024.0f;

#ifdef DEBUGGER
        DumpObjMtx();
#endif
    }
    else if( dwLength == 2 && dwCommand == 7 )
    {
        gSubObjMtx = (uObjSubMtx*)(dwAddr+g_pRDRAMu8);
        gObjMtxReal.X = float(gSubObjMtx->X>>2);
        gObjMtxReal.Y = float(gSubObjMtx->Y>>2);
        gObjMtxReal.BaseScaleX = gSubObjMtx->BaseScaleX/1024.0f;
        gObjMtxReal.BaseScaleY = gSubObjMtx->BaseScaleY/1024.0f;

#ifdef DEBUGGER
        DumpObjMtx(false);
#endif
    }

    g_MtxReal._11 = gObjMtxReal.A;
    g_MtxReal._12 = gObjMtxReal.C;
    g_MtxReal._13 = 0;
    g_MtxReal._14 = 0;//gObjMtxReal.X;

    g_MtxReal._21 = gObjMtxReal.B;
    g_MtxReal._22 = gObjMtxReal.D;
    g_MtxReal._23 = 0;
    g_MtxReal._24 = 0;//gObjMtxReal.Y;

    g_MtxReal._31 = 0;
    g_MtxReal._32 = 0;
    g_MtxReal._33 = 1.0;
    g_MtxReal._34 = 0;

    g_MtxReal._41 = gObjMtxReal.X;
    g_MtxReal._42 = gObjMtxReal.Y;
    g_MtxReal._43 = 0;
    g_MtxReal._44 = 1.0;

    DEBUGGER_PAUSE_COUNT_N(NEXT_OBJ_TXT_CMD);
}

// YoshiStory uses this - 0x01
extern void RSP_GBI0_Mtx(Gfx *gfx);

void RSP_S2DEX_BG_1CYC(Gfx *gfx)
{
    
    

    SP_Timing(DP_Minimal16);
    DP_Timing(DP_Minimal16);

    uint32 dwAddr = RSPSegmentAddr((gfx->words.w1));
    uObjScaleBg *sbgPtr = (uObjScaleBg *)(dwAddr+g_pRDRAMu8);
    CRender::g_pRender->LoadObjBG1CYC(*sbgPtr);
    CRender::g_pRender->DrawObjBG1CYC(*sbgPtr);

    DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI||eventToPause == NEXT_OBJ_BG),
        {
            DebuggerAppendMsg("S2DEX BG 1CYC: %08X-%08X\n", (gfx->words.w0), (gfx->words.w1) );     
            TRACE0("Paused at RSP_S2DEX_BG_1CYC");
        }
    );
}

void RSP_S2DEX_BG_1CYC_2(Gfx *gfx)
{
    
    

    if( ((gfx->words.w0)&0x00FFFFFF) != 0 )
    {
        RSP_GBI0_Mtx(gfx);
        return;
    }

    RSP_S2DEX_BG_1CYC(gfx);
}


// YoshiStory uses this - 0xb2
void RSP_S2DEX_OBJ_RECTANGLE_R(Gfx *gfx)
{
    
    

    uint32 dwAddr = RSPSegmentAddr((gfx->words.w1));
    uObjSprite *ptr = (uObjSprite*)(g_pRDRAMu8+dwAddr);

    uObjTxSprite objtx;
    memcpy(&objtx.sprite,ptr,sizeof(uObjSprite));


    //uObjTxSprite* ptr = (uObjTxSprite*)(g_pRDRAMu8+(RSPSegmentAddr((gfx->words.w1))&(g_dwRamSize-1)));
    //gObjTxtr = (uObjTxtr*)ptr;
    
    //Now draw the sprite
    if( g_TxtLoadBy == CMD_LOAD_OBJ_TXTR )
    {
        memcpy(&(objtx.txtr.block),&(gObjTxtr->block),sizeof(uObjTxtr));
        //CRender::g_pRender->LoadObjSprite(*ptr,true);
        CRender::g_pRender->LoadObjSprite(objtx,true);
    }
    else
    {
        PrepareTextures();
    }
    //CRender::g_pRender->DrawSprite(*ptr, true);
    CRender::g_pRender->DrawSprite(objtx, true);

    DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
        {
            DumpTxtrInfo(gObjTxtr);
            DumpSpriteParameters(*ptr);
            TRACE0("Paused at RSP_S2DEX_OBJ_RECTANGLE_R");
        }
    );
}

