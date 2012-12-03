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

#include <cmath>
#include "Render.h"
#include "Timing.h"

void RSP_GBI1_SpNoop(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_SpNoop);

    if( (gfx+1)->words.cmd == 0x00 && gRSP.ucode >= 17 )
    {
        RSP_RDP_NOIMPL("Double SPNOOP, Skip remain ucodes, PC=%08X, Cmd1=%08X", gDlistStack[gDlistStackPointer].pc, gfx->words.w1);
        RDP_GFX_PopDL();
        //if( gRSP.ucode < 17 ) TriggerDPInterrupt();
    }
}

void RSP_GBI0_Mtx(Gfx *gfx)
{   
    SP_Timing(RSP_GBI0_Mtx);

    uint32 addr = RSPSegmentAddr((gfx->gbi0matrix.addr));

    LOG_UCODE("    Command: %s %s %s Length %d Address 0x%08x",
        gfx->gbi0matrix.projection == 1 ? "Projection" : "ModelView",
        gfx->gbi0matrix.load == 1 ? "Load" : "Mul", 
        gfx->gbi0matrix.push == 1 ? "Push" : "NoPush",
        gfx->gbi0matrix.len, addr);

    if (addr + 64 > g_dwRamSize)
    {
        TRACE1("Mtx: Address invalid (0x%08x)", addr);
        return;
    }

    LoadMatrix(addr);
    
    if (gfx->gbi0matrix.projection)
    {
        CRender::g_pRender->SetProjection(matToLoad, gfx->gbi0matrix.push, gfx->gbi0matrix.load);
    }
    else
    {
        CRender::g_pRender->SetWorldView(matToLoad, gfx->gbi0matrix.push, gfx->gbi0matrix.load);
    }

#ifdef DEBUGGER
    const char *loadstr = gfx->gbi0matrix.load == 1 ? "Load" : "Mul";
    const char *pushstr = gfx->gbi0matrix.push == 1 ? "Push" : "Nopush";
    int projlevel = CRender::g_pRender->GetProjectMatrixLevel();
    int worldlevel = CRender::g_pRender->GetWorldViewMatrixLevel();
    if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
    {
        pauseAtNext = false;
        debuggerPause = true;
        if (gfx->gbi0matrix.projection)
        {
            TRACE3("Pause after %s and %s Matrix: Projection, level=%d\n", loadstr, pushstr, projlevel );
        }
        else
        {
            TRACE3("Pause after %s and %s Matrix: WorldView level=%d\n", loadstr, pushstr, worldlevel);
        }
    }
    else
    {
        if( pauseAtNext && logMatrix ) 
        {
            if (gfx->gbi0matrix.projection)
            {
                TRACE3("Matrix: %s and %s Projection level=%d\n", loadstr, pushstr, projlevel);
            }
            else
            {
                TRACE3("Matrix: %s and %s WorldView\n level=%d", loadstr, pushstr, worldlevel);
            }
        }
    }
#endif
}




void RSP_GBI1_Reserved(Gfx *gfx)
{       
    SP_Timing(RSP_GBI1_Reserved);
    RSP_RDP_NOIMPL("RDP: Reserved (0x%08x 0x%08x)", (gfx->words.w0), (gfx->words.w1));
}



void RSP_GBI1_MoveMem(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_MoveMem);

    uint32 type    = ((gfx->words.w0)>>16)&0xFF;
    uint32 dwLength  = ((gfx->words.w0))&0xFFFF;
    uint32 addr = RSPSegmentAddr((gfx->words.w1));

    switch (type) 
    {
        case RSP_GBI1_MV_MEM_VIEWPORT:
            {
                LOG_UCODE("    RSP_GBI1_MV_MEM_VIEWPORT. Address: 0x%08x, Length: 0x%04x", addr, dwLength);
                RSP_MoveMemViewport(addr);
            }
            break;
        case RSP_GBI1_MV_MEM_LOOKATY:
            LOG_UCODE("    RSP_GBI1_MV_MEM_LOOKATY");
            break;
        case RSP_GBI1_MV_MEM_LOOKATX:
            LOG_UCODE("    RSP_GBI1_MV_MEM_LOOKATX");
            break;
        case RSP_GBI1_MV_MEM_L0:
        case RSP_GBI1_MV_MEM_L1:
        case RSP_GBI1_MV_MEM_L2:
        case RSP_GBI1_MV_MEM_L3:
        case RSP_GBI1_MV_MEM_L4:
        case RSP_GBI1_MV_MEM_L5:
        case RSP_GBI1_MV_MEM_L6:
        case RSP_GBI1_MV_MEM_L7:
            {
                uint32 dwLight = (type-RSP_GBI1_MV_MEM_L0)/2;
                LOG_UCODE("    RSP_GBI1_MV_MEM_L%d", dwLight);
                LOG_UCODE("    Light%d: Length:0x%04x, Address: 0x%08x", dwLight, dwLength, addr);

                RSP_MoveMemLight(dwLight, addr);
            }
            break;
        case RSP_GBI1_MV_MEM_TXTATT:
            LOG_UCODE("    RSP_GBI1_MV_MEM_TXTATT");
            break;
        case RSP_GBI1_MV_MEM_MATRIX_1:
            RSP_GFX_Force_Matrix(addr);
            break;
        case RSP_GBI1_MV_MEM_MATRIX_2:
            break;
        case RSP_GBI1_MV_MEM_MATRIX_3:
            break;
        case RSP_GBI1_MV_MEM_MATRIX_4:
            break;
        default:
            RSP_RDP_NOIMPL("MoveMem: Unknown Move Type, cmd=%08X, %08X", gfx->words.w0, gfx->words.w1);
            break;
    }
}


void RSP_GBI0_Vtx(Gfx *gfx)
{
    SP_Timing(RSP_GBI0_Vtx);

    int n = gfx->gbi0vtx.n + 1;
    int v0 = gfx->gbi0vtx.v0;
    uint32 addr = RSPSegmentAddr((gfx->gbi0vtx.addr));

    LOG_UCODE("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", addr, v0, n, gfx->gbi0vtx.len);

    if ((v0 + n) > 80)
    {
        TRACE3("Warning, invalid vertex positions, N=%d, v0=%d, Addr=0x%08X", n, v0, addr);
        n = 32 - v0;
    }

    // Check that address is valid...
    if ((addr + n*16) > g_dwRamSize)
    {
        TRACE1("Vertex Data: Address out of range (0x%08x)", addr);
    }
    else
    {
        ProcessVertexData(addr, v0, n);
        status.dwNumVertices += n;
        DisplayVertexInfo(addr, v0, n);
    }
}


void RSP_GBI0_DL(Gfx *gfx)
{   
    SP_Timing(RSP_GBI0_DL);

    uint32 addr = RSPSegmentAddr((gfx->gbi0dlist.addr)) & (g_dwRamSize-1);

    LOG_UCODE("    Address=0x%08x Push: 0x%02x", addr, gfx->gbi0dlist.param);
    if( addr > g_dwRamSize )
    {
        RSP_RDP_NOIMPL("Error: DL addr = %08X out of range, PC=%08X", addr, gDlistStack[gDlistStackPointer].pc );
        addr &= (g_dwRamSize-1);
        DebuggerPauseCountN( NEXT_DLIST );
    }

    if( gfx->gbi0dlist.param == RSP_DLIST_PUSH )
        gDlistStackPointer++;

    gDlistStack[gDlistStackPointer].pc = addr;
    gDlistStack[gDlistStackPointer].countdown = MAX_DL_COUNT;

        LOG_UCODE("Level=%d", gDlistStackPointer+1);
        LOG_UCODE("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}


void RSP_GBI1_RDPHalf_Cont(Gfx *gfx)    
{
    SP_Timing(RSP_GBI1_RDPHalf_Cont);

    LOG_UCODE("RDPHalf_Cont: (Ignored)"); 
}
void RSP_GBI1_RDPHalf_2(Gfx *gfx)       
{ 
    SP_Timing(RSP_GBI1_RDPHalf_2);

    LOG_UCODE("RDPHalf_2: (Ignored)"); 
}

void RSP_GBI1_RDPHalf_1(Gfx *gfx)       
{
    SP_Timing(RSP_GBI1_RDPHalf_1);

    LOG_UCODE("RDPHalf_1: (Ignored)"); 
}

void RSP_GBI1_Line3D(Gfx *gfx)
{
    status.primitiveType = PRIM_LINE3D;

    uint32 dwPC = gDlistStack[gDlistStackPointer].pc;

    BOOL bTrisAdded = FALSE;

    if( gfx->ln3dtri2.v3 == 0 )
    {
        // Flying Dragon
        uint32 dwV0     = gfx->ln3dtri2.v0/gRSP.vertexMult;
        uint32 dwV1     = gfx->ln3dtri2.v1/gRSP.vertexMult;
        uint32 dwWidth  = gfx->ln3dtri2.v2;
        //uint32 dwFlag = gfx->ln3dtri2.v3/gRSP.vertexMult; 
        
        CRender::g_pRender->SetCombinerAndBlender();

        status.dwNumTrisRendered++;

        CRender::g_pRender->Line3D(dwV0, dwV1, dwWidth);
        SP_Timing(RSP_GBI1_Line3D);
        DP_Timing(RSP_GBI1_Line3D);
    }
    else
    {
        do {
            uint32 dwV3  = gfx->ln3dtri2.v3/gRSP.vertexMult;        
            uint32 dwV0  = gfx->ln3dtri2.v0/gRSP.vertexMult;
            uint32 dwV1  = gfx->ln3dtri2.v1/gRSP.vertexMult;
            uint32 dwV2  = gfx->ln3dtri2.v2/gRSP.vertexMult;

            LOG_UCODE("    Line3D: V0: %d, V1: %d, V2: %d, V3: %d", dwV0, dwV1, dwV2, dwV3);

            // Do first tri
            if (IsTriangleVisible(dwV0, dwV1, dwV2))
            {
                DEBUG_DUMP_VERTEXES("Line3D 1/2", dwV0, dwV1, dwV2);
                if (!bTrisAdded && CRender::g_pRender->IsTextureEnabled())
                {
                    PrepareTextures();
                    InitVertexTextureConstants();
                }

                if( !bTrisAdded )
                {
                    CRender::g_pRender->SetCombinerAndBlender();
                }

                bTrisAdded = true;
                PrepareTriangle(dwV0, dwV1, dwV2);
            }

            // Do second tri
            if (IsTriangleVisible(dwV2, dwV3, dwV0))
            {
                DEBUG_DUMP_VERTEXES("Line3D 2/2", dwV0, dwV1, dwV2);
                if (!bTrisAdded && CRender::g_pRender->IsTextureEnabled())
                {
                    PrepareTextures();
                    InitVertexTextureConstants();
                }

                if( !bTrisAdded )
                {
                    CRender::g_pRender->SetCombinerAndBlender();
                }

                bTrisAdded = true;
                PrepareTriangle(dwV2, dwV3, dwV0);
            }

            gfx++;
            dwPC += 8;
#ifdef DEBUGGER
        } while (gfx->words.cmd == (uint8)RSP_LINE3D && !(pauseAtNext && eventToPause==NEXT_FLUSH_TRI));
#else
        } while (gfx->words.cmd == (uint8)RSP_LINE3D);
#endif

        gDlistStack[gDlistStackPointer].pc = dwPC-8;

        if (bTrisAdded) 
        {
            CRender::g_pRender->DrawTriangles();
        }
    }
}


void RSP_GBI1_ClearGeometryMode(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_ClearGeometryMode);
    uint32 dwMask = ((gfx->words.w1));

#ifdef DEBUGGER
    LOG_UCODE("    Mask=0x%08x", dwMask);
    if (dwMask & G_ZBUFFER)                     LOG_UCODE("  Disabling ZBuffer");
    if (dwMask & G_TEXTURE_ENABLE)              LOG_UCODE("  Disabling Texture");
    if (dwMask & G_SHADE)                       LOG_UCODE("  Disabling Shade");
    if (dwMask & G_SHADING_SMOOTH)              LOG_UCODE("  Disabling Smooth Shading");
    if (dwMask & G_CULL_FRONT)                  LOG_UCODE("  Disabling Front Culling");
    if (dwMask & G_CULL_BACK)                   LOG_UCODE("  Disabling Back Culling");
    if (dwMask & G_FOG)                         LOG_UCODE("  Disabling Fog");
    if (dwMask & G_LIGHTING)                    LOG_UCODE("  Disabling Lighting");
    if (dwMask & G_TEXTURE_GEN)                 LOG_UCODE("  Disabling Texture Gen");
    if (dwMask & G_TEXTURE_GEN_LINEAR)          LOG_UCODE("  Disabling Texture Gen Linear");
    if (dwMask & G_LOD)                         LOG_UCODE("  Disabling LOD (no impl)");
#endif

    gRDP.geometryMode &= ~dwMask;
    RSP_GFX_InitGeometryMode();
}



void RSP_GBI1_SetGeometryMode(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_SetGeometryMode);
    uint32 dwMask = ((gfx->words.w1));

#ifdef DEBUGGER
    LOG_UCODE("    Mask=0x%08x", dwMask);
    if (dwMask & G_ZBUFFER)                     LOG_UCODE("  Enabling ZBuffer");
    if (dwMask & G_TEXTURE_ENABLE)              LOG_UCODE("  Enabling Texture");
    if (dwMask & G_SHADE)                       LOG_UCODE("  Enabling Shade");
    if (dwMask & G_SHADING_SMOOTH)              LOG_UCODE("  Enabling Smooth Shading");
    if (dwMask & G_CULL_FRONT)                  LOG_UCODE("  Enabling Front Culling");
    if (dwMask & G_CULL_BACK)                   LOG_UCODE("  Enabling Back Culling");
    if (dwMask & G_FOG)                         LOG_UCODE("  Enabling Fog");
    if (dwMask & G_LIGHTING)                    LOG_UCODE("  Enabling Lighting");
    if (dwMask & G_TEXTURE_GEN)                 LOG_UCODE("  Enabling Texture Gen");
    if (dwMask & G_TEXTURE_GEN_LINEAR)          LOG_UCODE("  Enabling Texture Gen Linear");
    if (dwMask & G_LOD)                         LOG_UCODE("  Enabling LOD (no impl)");
#endif // DEBUGGER
    gRDP.geometryMode |= dwMask;
    RSP_GFX_InitGeometryMode();
}




void RSP_GBI1_EndDL(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_EndDL);
    RDP_GFX_PopDL();
}


//static const char * sc_szBlClr[4] = { "In", "Mem", "Bl", "Fog" };
//static const char * sc_szBlA1[4] = { "AIn", "AFog", "AShade", "0" };
//static const char * sc_szBlA2[4] = { "1-A", "AMem", "1", "?" };

void RSP_GBI1_SetOtherModeL(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_SetOtherModeL);

    uint32 dwShift = ((gfx->words.w0)>>8)&0xFF;
    uint32 dwLength= ((gfx->words.w0)   )&0xFF;
    uint32 dwData  = (gfx->words.w1);

    uint32 dwMask = ((1<<dwLength)-1)<<dwShift;

    uint32 modeL = gRDP.otherModeL;
    modeL = (modeL&(~dwMask)) | dwData;

    Gfx tempgfx;
    tempgfx.words.w0 = gRDP.otherModeH;
    tempgfx.words.w1 = modeL;
    DLParser_RDPSetOtherMode(&tempgfx);
}


void RSP_GBI1_SetOtherModeH(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_SetOtherModeH);

    uint32 dwShift = ((gfx->words.w0)>>8)&0xFF;
    uint32 dwLength= ((gfx->words.w0)   )&0xFF;
    uint32 dwData  = (gfx->words.w1);

    uint32 dwMask = ((1<<dwLength)-1)<<dwShift;
    uint32 dwModeH = gRDP.otherModeH;

    dwModeH = (dwModeH&(~dwMask)) | dwData;
    Gfx tempgfx;
    tempgfx.words.w0 = dwModeH;
    tempgfx.words.w1 = gRDP.otherModeL;
    DLParser_RDPSetOtherMode(&tempgfx );
}


void RSP_GBI1_Texture(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_Texture);

    float fTextureScaleS = (float)(gfx->texture.scaleS) / (65536.0f * 32.0f);
    float fTextureScaleT = (float)(gfx->texture.scaleT) / (65536.0f * 32.0f);

    if( (((gfx->words.w1)>>16)&0xFFFF) == 0xFFFF )
    {
        fTextureScaleS = 1/32.0f;
    }
    else if( (((gfx->words.w1)>>16)&0xFFFF) == 0x8000 )
    {
        fTextureScaleS = 1/64.0f;
    }
#ifdef DEBUGGER
    else if( ((gfx->words.w1>>16)&0xFFFF) != 0 )
    {
        //DebuggerAppendMsg("Warning, texture scale = %08X is not integer", (word1>>16)&0xFFFF);
    }
#endif

    if( (((gfx->words.w1)    )&0xFFFF) == 0xFFFF )
    {
        fTextureScaleT = 1/32.0f;
    }
    else if( (((gfx->words.w1)    )&0xFFFF) == 0x8000 )
    {
        fTextureScaleT = 1/64.0f;
    }
#ifdef DEBUGGER
    else if( (gfx->words.w1&0xFFFF) != 0 )
    {
        //DebuggerAppendMsg("Warning, texture scale = %08X is not integer", (word1)&0xFFFF);
    }
#endif

    if( gRSP.ucode == 6 )
    {
        if( fTextureScaleS == 0 )   fTextureScaleS = 1.0f/32.0f;
        if( fTextureScaleT == 0 )   fTextureScaleT = 1.0f/32.0f;
    }

    CRender::g_pRender->SetTextureEnableAndScale(gfx->texture.tile, gfx->texture.enable_gbi0, fTextureScaleS, fTextureScaleT);

    // What happens if these are 0? Interpret as 1.0f?

    LOG_TEXTURE(
    {
        DebuggerAppendMsg("SetTexture: Level: %d Tile: %d %s\n", gfx->texture.level, gfx->texture.tile, gfx->texture.enable_gbi0 ? "enabled":"disabled");
        DebuggerAppendMsg("            ScaleS: %f, ScaleT: %f\n", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
    });

    DEBUGGER_PAUSE_COUNT_N(NEXT_SET_TEXTURE);
    LOG_UCODE("    Level: %d Tile: %d %s", gfx->texture.level, gfx->texture.tile, gfx->texture.enable_gbi0 ? "enabled":"disabled");
    LOG_UCODE("    ScaleS: %f, ScaleT: %f", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
}

extern void RSP_RDP_InsertMatrix(uint32 word0, uint32 word1);
void RSP_GBI1_MoveWord(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_MoveWord);

    switch (gfx->gbi0moveword.type)
    {
    case RSP_MOVE_WORD_MATRIX:
        RSP_RDP_InsertMatrix(gfx);
        break;
    case RSP_MOVE_WORD_NUMLIGHT:
        {
            uint32 dwNumLights = (((gfx->gbi0moveword.value)-0x80000000)/32)-1;
            LOG_UCODE("    RSP_MOVE_WORD_NUMLIGHT: Val:%d", dwNumLights);

            gRSP.ambientLightIndex = dwNumLights;
            SetNumLights(dwNumLights);
        }
        break;
    case RSP_MOVE_WORD_CLIP:
        {
            switch (gfx->gbi0moveword.offset)
            {
            case RSP_MV_WORD_OFFSET_CLIP_RNX:
            case RSP_MV_WORD_OFFSET_CLIP_RNY:
            case RSP_MV_WORD_OFFSET_CLIP_RPX:
            case RSP_MV_WORD_OFFSET_CLIP_RPY:
                CRender::g_pRender->SetClipRatio(gfx->gbi0moveword.offset, gfx->gbi0moveword.value);
                break;
            default:
                LOG_UCODE("    RSP_MOVE_WORD_CLIP  ?   : 0x%08x", gfx->words.w1);
                break;
            }
        }
        break;
    case RSP_MOVE_WORD_SEGMENT:
        {
            uint32 dwSegment = (gfx->gbi0moveword.offset >> 2) & 0xF;
            uint32 dwBase = (gfx->gbi0moveword.value)&0x00FFFFFF;
            LOG_UCODE("    RSP_MOVE_WORD_SEGMENT Seg[%d] = 0x%08x", dwSegment, dwBase);
            if( dwBase > g_dwRamSize )
            {
                gRSP.segments[dwSegment] = dwBase;
#ifdef DEBUGGER
                if( pauseAtNext )
                    DebuggerAppendMsg("warning: Segment %d addr is %8X", dwSegment, dwBase);
#endif
            }
            else
            {
                gRSP.segments[dwSegment] = dwBase;
            }
        }
        break;
    case RSP_MOVE_WORD_FOG:
        {
            uint16 wMult = (uint16)(((gfx->gbi0moveword.value) >> 16) & 0xFFFF);
            uint16 wOff  = (uint16)(((gfx->gbi0moveword.value)      ) & 0xFFFF);

            float fMult = (float)(short)wMult;
            float fOff = (float)(short)wOff;

            float rng = 128000.0f / fMult;
            float fMin = 500.0f - (fOff*rng/256.0f);
            float fMax = rng + fMin;

            FOG_DUMP(TRACE4("Set Fog: Min=%f, Max=%f, Mul=%f, Off=%f", fMin, fMax, fMult, fOff));
            //if( fMult <= 0 || fMin > fMax || fMax < 0 || fMin > 1000 )
            if( fMult <= 0 || fMax < 0 )
            {
                // Hack
                fMin = 996;
                fMax = 1000;
                fMult = 0;
                fOff = 1;
            }

            LOG_UCODE("    RSP_MOVE_WORD_FOG/Mul=%d: Off=%d", wMult, wOff);
            FOG_DUMP(TRACE3("Set Fog: Min=%f, Max=%f, Data=%08X", fMin, fMax, gfx->gbi0moveword.value));
            SetFogMinMax(fMin, fMax, fMult, fOff);
        }
        break;
    case RSP_MOVE_WORD_LIGHTCOL:
        {
            uint32 dwLight = gfx->gbi0moveword.offset / 0x20;
            uint32 dwField = (gfx->gbi0moveword.offset & 0x7);

            LOG_UCODE("    RSP_MOVE_WORD_LIGHTCOL/0x%08x: 0x%08x", gfx->gbi0moveword.offset, gfx->words.w1);

            switch (dwField)
            {
            case 0:
                if (dwLight == gRSP.ambientLightIndex)
                {
                    SetAmbientLight( ((gfx->gbi0moveword.value)>>8) );
                }
                else
                {
                    SetLightCol(dwLight, gfx->gbi0moveword.value);
                }
                break;

            case 4:
                break;

            default:
                TRACE1("RSP_MOVE_WORD_LIGHTCOL with unknown offset 0x%08x", dwField);
                break;
            }
        }
        break;
    case RSP_MOVE_WORD_POINTS:
        {
            uint32 vtx = gfx->gbi0moveword.offset/40;
            uint32 where = gfx->gbi0moveword.offset - vtx*40;
            ModifyVertexInfo(where, vtx, gfx->gbi0moveword.value);
        }
        break;
    case RSP_MOVE_WORD_PERSPNORM:
        LOG_UCODE("    RSP_MOVE_WORD_PERSPNORM");
        //if( word1 != 0x1A ) DebuggerAppendMsg("PerspNorm: 0x%04x", (short)word1); 
        break;
    default:
        RSP_RDP_NOIMPL("Unknown MoveWord, %08X, %08X", gfx->words.w0, gfx->words.w1);
        break;
    }

}


void RSP_GBI1_PopMtx(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_PopMtx);

    LOG_UCODE("    Command: (%s)",  gfx->gbi0popmatrix.projection ? "Projection" : "ModelView");

    // Do any of the other bits do anything?
    // So far only Extreme-G seems to Push/Pop projection matrices

    if (gfx->gbi0popmatrix.projection)
    {
        CRender::g_pRender->PopProjection();
    }
    else
    {
        CRender::g_pRender->PopWorldView();
    }
#ifdef DEBUGGER
    if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
    {
        pauseAtNext = false;
        debuggerPause = true;
        DebuggerAppendMsg("Pause after Pop Matrix: %s\n", gfx->gbi0popmatrix.projection ? "Proj":"World");
    }
    else
    {
        if( pauseAtNext && logMatrix ) 
        {
            DebuggerAppendMsg("Pause after Pop Matrix: %s\n", gfx->gbi0popmatrix.projection ? "Proj":"World");
        }
    }
#endif
}



void RSP_GBI1_CullDL(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_CullDL);

#ifdef DEBUGGER
    if( !debuggerEnableCullFace )
    {
        return; //Disable Culling
    }
#endif
    if( g_curRomInfo.bDisableCulling )
    {
        return; //Disable Culling
    }

    uint32 i;
    uint32 dwVFirst = ((gfx->words.w0) & 0xFFF) / gRSP.vertexMult;
    uint32 dwVLast  = (((gfx->words.w1)) & 0xFFF) / gRSP.vertexMult;

    LOG_UCODE("    Culling using verts %d to %d", dwVFirst, dwVLast);

    // Mask into range
    dwVFirst &= 0x1f;
    dwVLast &= 0x1f;

    if( dwVLast < dwVFirst )    return;
    if( !gRSP.bRejectVtx )  return;

    for (i = dwVFirst; i <= dwVLast; i++)
    {
        if (g_clipFlag[i] == 0)
        {
            LOG_UCODE("    Vertex %d is visible, continuing with display list processing", i);
            return;
        }
    }

    status.dwNumDListsCulled++;

    LOG_UCODE("    No vertices were visible, culling rest of display list");

    RDP_GFX_PopDL();
}



void RSP_GBI1_Tri1(Gfx *gfx)
{
    status.primitiveType = PRIM_TRI1;
    bool bTrisAdded = false;
    bool bTexturesAreEnabled = CRender::g_pRender->IsTextureEnabled();

    // While the next command pair is Tri1, add vertices
    uint32 dwPC = gDlistStack[gDlistStackPointer].pc;
    //uint32 * pCmdBase = (uint32 *)(g_pRDRAMu8 + dwPC);
    
    do
    {
        uint32 dwV0 = gfx->tri1.v0/gRSP.vertexMult;
        uint32 dwV1 = gfx->tri1.v1/gRSP.vertexMult;
        uint32 dwV2 = gfx->tri1.v2/gRSP.vertexMult;

        if (IsTriangleVisible(dwV0, dwV1, dwV2))
        {
            DEBUG_DUMP_VERTEXES("Tri1", dwV0, dwV1, dwV2);
            LOG_UCODE("    Tri1: 0x%08x 0x%08x %d,%d,%d", gfx->words.w0, gfx->words.w1, dwV0, dwV1, dwV2);

            if (!bTrisAdded)
            {
                if( bTexturesAreEnabled )
                {
                    PrepareTextures();
                    InitVertexTextureConstants();
                }
                CRender::g_pRender->SetCombinerAndBlender();
                bTrisAdded = true;
            }
            PrepareTriangle(dwV0, dwV1, dwV2);
        }

        gfx++;
        dwPC += 8;

#ifdef DEBUGGER
    } while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && gfx->words.cmd == (uint8)RSP_TRI1);
#else
    } while (gfx->words.cmd == (uint8)RSP_TRI1);
#endif

    gDlistStack[gDlistStackPointer].pc = dwPC-8;

    if (bTrisAdded) 
    {
        CRender::g_pRender->DrawTriangles();
    }

    DEBUG_TRIANGLE(TRACE0("Pause at GBI0 TRI1"));
}


void RSP_GBI0_Tri4(Gfx *gfx)
{
    uint32 w0 = gfx->words.w0;
    uint32 w1 = gfx->words.w1;

    status.primitiveType = PRIM_TRI2;

    // While the next command pair is Tri2, add vertices
    uint32 dwPC = gDlistStack[gDlistStackPointer].pc;

    BOOL bTrisAdded = FALSE;

    do {
        uint32 dwFlag = (w0>>16)&0xFF;
        LOG_UCODE("    PD Tri4: 0x%08x 0x%08x Flag: 0x%02x", gfx->words.w0, gfx->words.w1, dwFlag);

        BOOL bVisible;
        for( int i=0; i<4; i++)
        {
            uint32 v0 = (w1>>(4+(i<<3))) & 0xF;
            uint32 v1 = (w1>>(  (i<<3))) & 0xF;
            uint32 v2 = (w0>>(  (i<<2))) & 0xF;
            bVisible = IsTriangleVisible(v0, v2, v1);
            LOG_UCODE("       (%d, %d, %d) %s", v0, v1, v2, bVisible ? "": "(clipped)");
            if (bVisible)
            {
                DEBUG_DUMP_VERTEXES("Tri4_PerfectDark 1/2", v0, v1, v2);
                if (!bTrisAdded && CRender::g_pRender->IsTextureEnabled())
                {
                    PrepareTextures();
                    InitVertexTextureConstants();
                }

                if( !bTrisAdded )
                {
                    CRender::g_pRender->SetCombinerAndBlender();
                }

                bTrisAdded = true;
                PrepareTriangle(v0, v2, v1);
            }
        }
        
        w0          = *(uint32 *)(g_pRDRAMu8 + dwPC+0);
        w1          = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
        dwPC += 8;

#ifdef DEBUGGER
    } while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (w0>>24) == (uint8)RSP_TRI2);
#else
    } while (((w0)>>24) == (uint8)RSP_TRI2);
#endif


    gDlistStack[gDlistStackPointer].pc = dwPC-8;


    if (bTrisAdded) 
    {
        CRender::g_pRender->DrawTriangles();
    }
    
    DEBUG_TRIANGLE(TRACE0("Pause at GBI0 TRI4"));

    gRSP.DKRVtxCount=0;
}

//Nintro64 uses Sprite2d 


void RSP_RDP_Nothing(Gfx *gfx)
{
    SP_Timing(RSP_RDP_Nothing);

#ifdef DEBUGGER
    if( logWarning )
    {
        TRACE0("Stack Trace");
        for( int i=0; i<gDlistStackPointer; i++ )
        {
            DebuggerAppendMsg("  %08X", gDlistStack[i].pc);
        }

        uint32 dwPC = gDlistStack[gDlistStackPointer].pc-8;
        DebuggerAppendMsg("PC=%08X",dwPC);
        DebuggerAppendMsg("Warning, unknown ucode PC=%08X: 0x%08x 0x%08x\n", dwPC, gfx->words.w0, gfx->words.w1);
    }
    DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_UNKNOWN_OP, {TRACE0("Paused at unknown ucode\n");})
    if( debuggerContinueWithUnknown )
    {
        return;
    }
#endif
        
    if( options.bEnableHacks )
        return;
    
    gDlistStackPointer=-1;
}


void RSP_RDP_InsertMatrix(Gfx *gfx)
{
    float fraction;

    UpdateCombinedMatrix();

    if ((gfx->words.w0) & 0x20)
    {
        int x = ((gfx->words.w0) & 0x1F) >> 1;
        int y = x >> 2;
        x &= 3;

        fraction = ((gfx->words.w1)>>16)/65536.0f;
        gRSPworldProject.m[y][x] = (float)(int)gRSPworldProject.m[y][x];
        gRSPworldProject.m[y][x] += fraction;

        fraction = ((gfx->words.w1)&0xFFFF)/65536.0f;
        gRSPworldProject.m[y][x+1] = (float)(int)gRSPworldProject.m[y][x+1];
        gRSPworldProject.m[y][x+1] += fraction;
    }
    else
    {
        int x = ((gfx->words.w0) & 0x1F) >> 1;
        int y = x >> 2;
        x &= 3;

        fraction = (float)fabs(gRSPworldProject.m[y][x] - (int)gRSPworldProject.m[y][x]);
        gRSPworldProject.m[y][x] = (short)((gfx->words.w1)>>16) + fraction;

        fraction = (float)fabs(gRSPworldProject.m[y][x+1] - (int)gRSPworldProject.m[y][x+1]);
        gRSPworldProject.m[y][x+1] = (short)((gfx->words.w1)&0xFFFF) + fraction;
    }

    gRSP.bMatrixIsUpdated = false;
    gRSP.bCombinedMatrixIsUpdated = true;

#ifdef DEBUGGER
    if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
    {
        pauseAtNext = false;
        debuggerPause = true;
        DebuggerAppendMsg("Pause after insert matrix: %08X, %08X", gfx->words.w0, gfx->words.w1);
    }
    else
    {
        if( pauseAtNext && logMatrix ) 
        {
            DebuggerAppendMsg("insert matrix: %08X, %08X", gfx->words.w0, gfx->words.w1);
        }
    }
#endif
}

