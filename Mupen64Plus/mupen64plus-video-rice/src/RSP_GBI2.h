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

#include "Render.h"
#include "Timing.h"

void RSP_GBI2_Vtx(Gfx *gfx)
{
    uint32 addr = RSPSegmentAddr((gfx->gbi2vtx.addr));
    int vend    = gfx->gbi2vtx.vend/2;
    int n       = gfx->gbi2vtx.n;
    int v0      = vend - n;

    LOG_UCODE("    Vtx: Address 0x%08x, vEnd: %d, v0: %d, Num: %d", addr, vend, v0, n);

    if( vend > 64 )
    {
        DebuggerAppendMsg("Warning, attempting to load into invalid vertex positions, v0=%d, n=%d", v0, n);
        return;
    }

    if ((addr + (n*16)) > g_dwRamSize)
    {
        DebuggerAppendMsg("ProcessVertexData: Address out of range (0x%08x)", addr);
    }
    else
    {
        ProcessVertexData(addr, v0, n);
        status.dwNumVertices += n;
        DisplayVertexInfo(addr, v0, n);
    }
}

void RSP_GBI2_EndDL(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_EndDL);

    RDP_GFX_PopDL();
}

void RSP_GBI2_CullDL(Gfx *gfx)
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
    uint32 dwVFirst = (((gfx->words.w0)) & 0xfff) / gRSP.vertexMult;
    uint32 dwVLast  = (((gfx->words.w1)) & 0xfff) / gRSP.vertexMult;

    LOG_UCODE("    Culling using verts %d to %d", dwVFirst, dwVLast);

    // Mask into range
    dwVFirst &= 0x1f;
    dwVLast &= 0x1f;

    if( dwVLast < dwVFirst )    return;
    if( !gRSP.bRejectVtx )  return;

    for (i = dwVFirst; i <= dwVLast; i++)
    {
        //if (g_dwVtxFlags[i] == 0)
        if (g_clipFlag[i] == 0)
        {
            LOG_UCODE("    Vertex %d is visible, returning", i);
            return;
        }
    }

    status.dwNumDListsCulled++;

    LOG_UCODE("    No vertices were visible, culling");

    RDP_GFX_PopDL();
}

void RSP_GBI2_MoveWord(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_MoveWord);

    switch (gfx->gbi2moveword.type)
    {
    case RSP_MOVE_WORD_MATRIX:
        RSP_RDP_InsertMatrix(gfx);
        break;
    case RSP_MOVE_WORD_NUMLIGHT:
        {
            uint32 dwNumLights = gfx->gbi2moveword.value/24;
            gRSP.ambientLightIndex = dwNumLights;
            SetNumLights(dwNumLights);
        }
        break;

    case RSP_MOVE_WORD_CLIP:
        {
            switch (gfx->gbi2moveword.offset)
            {
            case RSP_MV_WORD_OFFSET_CLIP_RNX:
            case RSP_MV_WORD_OFFSET_CLIP_RNY:
            case RSP_MV_WORD_OFFSET_CLIP_RPX:
            case RSP_MV_WORD_OFFSET_CLIP_RPY:
                CRender::g_pRender->SetClipRatio(gfx->gbi2moveword.offset, gfx->gbi2moveword.value);
            default:
                LOG_UCODE("     RSP_MOVE_WORD_CLIP  ?   : 0x%08x", gfx->words.w1);
                break;
            }
        }
        break;

    case RSP_MOVE_WORD_SEGMENT:
        {
            uint32 dwSeg     = gfx->gbi2moveword.offset / 4;
            uint32 dwAddr = gfx->gbi2moveword.value & 0x00FFFFFF;           // Hack - convert to physical

            LOG_UCODE("      RSP_MOVE_WORD_SEGMENT Segment[%d] = 0x%08x",   dwSeg, dwAddr);
            if( dwAddr > g_dwRamSize )
            {
                gRSP.segments[dwSeg] = dwAddr;
#ifdef DEBUGGER
                if( pauseAtNext )
                    DebuggerAppendMsg("warning: Segment %d addr is %8X", dwSeg, dwAddr);
#endif
            }
            else
            {
                gRSP.segments[dwSeg] = dwAddr;
            }
        }
        break;
    case RSP_MOVE_WORD_FOG:
        {
            uint16 wMult = (uint16)((gfx->gbi2moveword.value >> 16) & 0xFFFF);
            uint16 wOff  = (uint16)((gfx->gbi2moveword.value      ) & 0xFFFF);

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

            SetFogMinMax(fMin, fMax, fMult, fOff);
            FOG_DUMP(TRACE3("Set Fog: Min=%f, Max=%f, Data=0x%08X", fMin, fMax, gfx->gbi2moveword.value));
        }
        break;
    case RSP_MOVE_WORD_LIGHTCOL:
        {
            uint32 dwLight = gfx->gbi2moveword.offset / 0x18;
            uint32 dwField = (gfx->gbi2moveword.offset & 0x7);

            switch (dwField)
            {
            case 0:
                if (dwLight == gRSP.ambientLightIndex)
                {
                    SetAmbientLight( (gfx->gbi2moveword.value>>8) );
                }
                else
                {
                    SetLightCol(dwLight, gfx->gbi2moveword.value);
                }
                break;

            case 4:
                break;

            default:
                DebuggerAppendMsg("RSP_MOVE_WORD_LIGHTCOL with unknown offset 0x%08x", dwField);
                break;
            }


        }
        break;

    case RSP_MOVE_WORD_PERSPNORM:
        LOG_UCODE("     RSP_MOVE_WORD_PERSPNORM 0x%04x", (short)gfx->words.w1);
        break;

    case RSP_MOVE_WORD_POINTS:
        LOG_UCODE("     2nd cmd of Force Matrix");
        break;

    default:
        {
            LOG_UCODE("      Ignored!!");

        }
        break;
    }
}

void RSP_GBI2_Tri1(Gfx *gfx)
{
    if( gfx->words.w0 == 0x05000017 && gfx->gbi2tri1.flag == 0x80 )
    {
        // The ObjLoadTxtr / Tlut cmd for Evangelion.v64
        RSP_S2DEX_SPObjLoadTxtr(gfx);
        DebuggerAppendMsg("Fix me, SPObjLoadTxtr as RSP_GBI2_Tri2");
    }
    else
    {
        status.primitiveType = PRIM_TRI1;
        bool bTrisAdded = false;
        bool bTexturesAreEnabled = CRender::g_pRender->IsTextureEnabled();

        // While the next command pair is Tri1, add vertices
        uint32 dwPC = gDlistStack[gDlistStackPointer].pc;
        //uint32 * pCmdBase = (uint32 *)(g_pRDRAMu8 + dwPC);

        do
        {
            uint32 dwV2 = gfx->gbi2tri1.v2/gRSP.vertexMult;
            uint32 dwV1 = gfx->gbi2tri1.v1/gRSP.vertexMult;
            uint32 dwV0 = gfx->gbi2tri1.v0/gRSP.vertexMult;

            if (IsTriangleVisible(dwV0, dwV1, dwV2))
            {
                DEBUG_DUMP_VERTEXES("ZeldaTri1", dwV0, dwV1, dwV2);
                LOG_UCODE("    ZeldaTri1: 0x%08x 0x%08x %d,%d,%d", gfx->words.w0, gfx->words.w1, dwV0, dwV1, dwV2);
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
        } while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && gfx->words.cmd == (uint8)RSP_ZELDATRI1);
#else
        } while( gfx->words.cmd == (uint8)RSP_ZELDATRI1);
#endif

        gDlistStack[gDlistStackPointer].pc = dwPC-8;

        if (bTrisAdded) 
        {
            CRender::g_pRender->DrawTriangles();
        }

        DEBUG_TRIANGLE(TRACE0("Pause at GBI2 TRI1"));
    }
}

void RSP_GBI2_Tri2(Gfx *gfx)
{
    if( gfx->words.w0 == 0x0600002f && gfx->gbi2tri2.flag == 0x80 )
    {
        // The ObjTxSprite cmd for Evangelion.v64
        RSP_S2DEX_SPObjLoadTxSprite(gfx);
        DebuggerAppendMsg("Fix me, SPObjLoadTxSprite as RSP_GBI2_Tri2");
    }
    else
    {
        status.primitiveType = PRIM_TRI2;
        BOOL bTrisAdded = FALSE;

        // While the next command pair is Tri2, add vertices
        uint32 dwPC = gDlistStack[gDlistStackPointer].pc;
        bool bTexturesAreEnabled = CRender::g_pRender->IsTextureEnabled();

        do {
            uint32 dwV2 = gfx->gbi2tri2.v2;
            uint32 dwV1 = gfx->gbi2tri2.v1;
            uint32 dwV0 = gfx->gbi2tri2.v0;

            uint32 dwV5 = gfx->gbi2tri2.v5;
            uint32 dwV4 = gfx->gbi2tri2.v4;
            uint32 dwV3 = gfx->gbi2tri2.v3;

            LOG_UCODE("    ZeldaTri2: 0x%08x 0x%08x", gfx->words.w0, gfx->words.w1);
            LOG_UCODE("           V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
            LOG_UCODE("           V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

            // Do first tri
            if (IsTriangleVisible(dwV0, dwV1, dwV2))
            {
                DEBUG_DUMP_VERTEXES("ZeldaTri2 1/2", dwV0, dwV1, dwV2);
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

            // Do second tri
            if (IsTriangleVisible(dwV3, dwV4, dwV5))
            {
                DEBUG_DUMP_VERTEXES("ZeldaTri2 2/2", dwV3, dwV4, dwV5);
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

                PrepareTriangle(dwV3, dwV4, dwV5);
            }
            
            gfx++;
            dwPC += 8;

#ifdef DEBUGGER
        } while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && gfx->words.cmd == (uint8)RSP_ZELDATRI2);
#else
        } while ( gfx->words.cmd == (uint8)RSP_ZELDATRI2 );//&& status.dwNumTrisRendered < 50);
#endif


        gDlistStack[gDlistStackPointer].pc = dwPC-8;

        if (bTrisAdded) 
        {
            CRender::g_pRender->DrawTriangles();
        }

        DEBUG_TRIANGLE(TRACE0("Pause at GBI2 TRI2"));
    }
}

void RSP_GBI2_Line3D(Gfx *gfx)
{
    if( gfx->words.w0 == 0x0700002f && (gfx->words.w1>>24) == 0x80 )
    {
        // The ObjTxSprite cmd for Evangelion.v64
        RSP_S2DEX_SPObjLoadTxRect(gfx);
    }
    else
    {
        status.primitiveType = PRIM_TRI3;

        uint32 dwPC = gDlistStack[gDlistStackPointer].pc;

        BOOL bTrisAdded = FALSE;

        do {
            uint32 dwV0 = gfx->gbi2line3d.v0/gRSP.vertexMult;
            uint32 dwV1 = gfx->gbi2line3d.v1/gRSP.vertexMult;
            uint32 dwV2 = gfx->gbi2line3d.v2/gRSP.vertexMult;

            uint32 dwV3 = gfx->gbi2line3d.v3/gRSP.vertexMult;
            uint32 dwV4 = gfx->gbi2line3d.v4/gRSP.vertexMult;
            uint32 dwV5 = gfx->gbi2line3d.v5/gRSP.vertexMult;

            LOG_UCODE("    ZeldaTri3: 0x%08x 0x%08x", gfx->words.w0, gfx->words.w1);
            LOG_UCODE("           V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
            LOG_UCODE("           V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

            // Do first tri
            if (IsTriangleVisible(dwV0, dwV1, dwV2))
            {
                DEBUG_DUMP_VERTEXES("ZeldaTri3 1/2", dwV0, dwV1, dwV2);
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
            if (IsTriangleVisible(dwV3, dwV4, dwV5))
            {
                DEBUG_DUMP_VERTEXES("ZeldaTri3 2/2", dwV3, dwV4, dwV5);
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
                PrepareTriangle(dwV3, dwV4, dwV5);
            }
            
            gfx++;
            dwPC += 8;

#ifdef DEBUGGER
        } while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && gfx->words.cmd == (uint8)RSP_LINE3D);
#else
        } while ( gfx->words.cmd == (uint8)RSP_LINE3D);
#endif

        gDlistStack[gDlistStackPointer].pc = dwPC-8;


        if (bTrisAdded) 
        {
            CRender::g_pRender->DrawTriangles();
        }

        DEBUG_TRIANGLE(TRACE0("Pause at GBI2 Line3D"));
    }
}

void RSP_GBI2_Texture(Gfx *gfx)
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
    if( (((gfx->words.w1)    )&0xFFFF) == 0xFFFF )
    {
        fTextureScaleT = 1/32.0f;
    }
    else if( (((gfx->words.w1)    )&0xFFFF) == 0x8000 )
    {
        fTextureScaleT = 1/64.0f;
    }

    CRender::g_pRender->SetTextureEnableAndScale(gfx->texture.tile, gfx->texture.enable_gbi2, fTextureScaleS, fTextureScaleT);

    /*
    if( g_curRomInfo.bTextureScaleHack )
    {
    // Hack, need to verify, refer to N64 programming manual
    // that if scale = 0.5 (1/64), vtx s,t are also doubled

        if( ((word1>>16)&0xFFFF) == 0x8000 )
        {
            fTextureScaleS = 1/128.0f;
            if( ((word1)&0xFFFF) == 0xFFFF )
            {
                fTextureScaleT = 1/64.0f;
            }
        }

        if( ((word1    )&0xFFFF) == 0x8000 )
        {
            fTextureScaleT = 1/128.0f;
            if( ((word1>>16)&0xFFFF) == 0xFFFF )
            {
                fTextureScaleS = 1/64.0f;
            }
        }
    }
    */

    CRender::g_pRender->SetTextureEnableAndScale(gfx->texture.tile, gfx->texture.enable_gbi2, fTextureScaleS, fTextureScaleT);

    LOG_TEXTURE(
    {
        DebuggerAppendMsg("SetTexture: Level: %d Tile: %d %s\n", gfx->texture.level, gfx->texture.tile, gfx->texture.enable_gbi2 ? "enabled":"disabled");
        DebuggerAppendMsg("            ScaleS: %f, ScaleT: %f\n", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
    });

    DEBUGGER_PAUSE_COUNT_N(NEXT_SET_TEXTURE);

    LOG_UCODE("    Level: %d Tile: %d %s", gfx->texture.level, gfx->texture.tile, gfx->texture.enable_gbi2 ? "enabled":"disabled");
    LOG_UCODE("    ScaleS: %f, ScaleT: %f", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
}



void RSP_GBI2_PopMtx(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_PopMtx);

    uint8 nCommand = (uint8)(gfx->words.w0 & 0xFF);

    LOG_UCODE("        PopMtx: 0x%02x (%s)",
        nCommand, 
        (nCommand & RSP_ZELDA_MTX_PROJECTION) ? "Projection" : "ModelView");


/*  if (nCommand & RSP_ZELDA_MTX_PROJECTION)
    {
        CRender::g_pRender->PopProjection();
    }
    else*/
    {
        CRender::g_pRender->PopWorldView();
    }
#ifdef DEBUGGER
    if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
    {
        pauseAtNext = false;
        debuggerPause = true;
        TRACE0("Pause after Pop GBI2_PopMtx:");
    }
    else
    {
        if( pauseAtNext && logMatrix ) 
        {
            TRACE0("Pause after Pop GBI2_PopMtx:");
        }
    }
#endif

}


#define RSP_ZELDA_ZBUFFER               0x00000001      // Guess
#define RSP_ZELDA_CULL_BACK         0x00000200
#define RSP_ZELDA_CULL_FRONT            0x00000400
#define RSP_ZELDA_FOG                   0x00010000
#define RSP_ZELDA_LIGHTING          0x00020000
#define RSP_ZELDA_TEXTURE_GEN           0x00040000
#define RSP_ZELDA_TEXTURE_GEN_LINEAR    0x00080000
#define RSP_ZELDA_SHADING_SMOOTH        0x00200000

void RSP_GBI2_GeometryMode(Gfx *gfx)
{
    SP_Timing(RSP_GBI2_GeometryMode);

    uint32 dwAnd = ((gfx->words.w0)) & 0x00FFFFFF;
    uint32 dwOr  = ((gfx->words.w1)) & 0x00FFFFFF;

#ifdef DEBUGGER
        LOG_UCODE("    0x%08x 0x%08x =(x & 0x%08x) | 0x%08x", gfx->words.w0, gfx->words.w1, dwAnd, dwOr);

        if ((~dwAnd) & RSP_ZELDA_ZBUFFER)                   LOG_UCODE("  Disabling ZBuffer");
        //  if ((~dwAnd) & RSP_ZELDA_TEXTURE_ENABLE)            LOG_UCODE("  Disabling Texture");
        //  if ((~dwAnd) & RSP_ZELDA_SHADE)                 LOG_UCODE("  Disabling Shade");
        if ((~dwAnd) & RSP_ZELDA_SHADING_SMOOTH)            LOG_UCODE("  Disabling Flat Shading");
        if ((~dwAnd) & RSP_ZELDA_CULL_FRONT)                LOG_UCODE("  Disabling Front Culling");
        if ((~dwAnd) & RSP_ZELDA_CULL_BACK)             LOG_UCODE("  Disabling Back Culling");
        if ((~dwAnd) & RSP_ZELDA_FOG)                       LOG_UCODE("  Disabling Fog");
        if ((~dwAnd) & RSP_ZELDA_LIGHTING)              LOG_UCODE("  Disabling Lighting");
        if ((~dwAnd) & RSP_ZELDA_TEXTURE_GEN)               LOG_UCODE("  Disabling Texture Gen");
        if ((~dwAnd) & RSP_ZELDA_TEXTURE_GEN_LINEAR)        LOG_UCODE("  Disabling Texture Gen Linear");
        //  if ((~dwAnd) & RSP_ZELDA_LOD)                       LOG_UCODE("  Disabling LOD (no impl)");

        if (dwOr & RSP_ZELDA_ZBUFFER)                       LOG_UCODE("  Enabling ZBuffer");
        //  if (dwOr & RSP_ZELDA_TEXTURE_ENABLE)                LOG_UCODE("  Enabling Texture");
        //  if (dwOr & RSP_ZELDA_SHADE)                     LOG_UCODE("  Enabling Shade");
        if (dwOr & RSP_ZELDA_SHADING_SMOOTH)                LOG_UCODE("  Enabling Flat Shading");
        if (dwOr & RSP_ZELDA_CULL_FRONT)                    LOG_UCODE("  Enabling Front Culling");
        if (dwOr & RSP_ZELDA_CULL_BACK)                 LOG_UCODE("  Enabling Back Culling");
        if (dwOr & RSP_ZELDA_FOG)                           LOG_UCODE("  Enabling Fog");
        if (dwOr & RSP_ZELDA_LIGHTING)                  LOG_UCODE("  Enabling Lighting");
        if (dwOr & RSP_ZELDA_TEXTURE_GEN)                   LOG_UCODE("  Enabling Texture Gen");
        if (dwOr & RSP_ZELDA_TEXTURE_GEN_LINEAR)            LOG_UCODE("  Enabling Texture Gen Linear");
        //  if (dwOr & RSP_ZELDA_LOD)                           LOG_UCODE("  Enabling LOD (no impl)");
#endif // DEBUGGER

        gRDP.geometryMode &= dwAnd;
    gRDP.geometryMode |= dwOr;


    bool bCullFront     = (gRDP.geometryMode & RSP_ZELDA_CULL_FRONT) ? true : false;
    bool bCullBack      = (gRDP.geometryMode & RSP_ZELDA_CULL_BACK) ? true : false;
    
    //BOOL bShade           = (gRDP.geometryMode & G_SHADE) ? TRUE : FALSE;
    //BOOL bFlatShade       = (gRDP.geometryMode & RSP_ZELDA_SHADING_SMOOTH) ? TRUE : FALSE;
    BOOL bFlatShade     = (gRDP.geometryMode & RSP_ZELDA_TEXTURE_GEN_LINEAR) ? TRUE : FALSE;
    if( options.enableHackForGames == HACK_FOR_TIGER_HONEY_HUNT )
        bFlatShade      = FALSE;
    
    bool bFog           = (gRDP.geometryMode & RSP_ZELDA_FOG) ? true : false;
    bool bTextureGen    = (gRDP.geometryMode & RSP_ZELDA_TEXTURE_GEN) ? true : false;

    bool bLighting      = (gRDP.geometryMode & RSP_ZELDA_LIGHTING) ? true : false;
    BOOL bZBuffer       = (gRDP.geometryMode & RSP_ZELDA_ZBUFFER)   ? TRUE : FALSE; 

    CRender::g_pRender->SetCullMode(bCullFront, bCullBack);
    
    //if (bFlatShade||!bShade)  CRender::g_pRender->SetShadeMode( SHADE_FLAT );
    if (bFlatShade) CRender::g_pRender->SetShadeMode( SHADE_FLAT );
    else            CRender::g_pRender->SetShadeMode( SHADE_SMOOTH );
    
    SetTextureGen(bTextureGen);

    SetLighting( bLighting );
    CRender::g_pRender->ZBufferEnable( bZBuffer );
    CRender::g_pRender->SetFogEnable( bFog );
}


int dlistMtxCount=0;
extern uint32 dwConkerVtxZAddr;

void RSP_GBI2_Mtx(Gfx *gfx)
{   
    SP_Timing(RSP_GBI0_Mtx);
    dwConkerVtxZAddr = 0;   // For Conker BFD

    uint32 addr = RSPSegmentAddr((gfx->gbi2matrix.addr));

    if( gfx->gbi2matrix.param == 0 && gfx->gbi2matrix.len == 0 )
    {
        DLParser_Bomberman2TextRect(gfx);
        return;
    }

    LOG_UCODE("    Mtx: %s %s %s Length %d Address 0x%08x",
        gfx->gbi2matrix.projection ? "Projection" : "ModelView",
        gfx->gbi2matrix.load ? "Load" : "Mul",  
        gfx->gbi2matrix.nopush==0 ? "Push" : "No Push",
        gfx->gbi2matrix.len, addr);

    if (addr + 64 > g_dwRamSize)
    {
        DebuggerAppendMsg("ZeldaMtx: Address invalid (0x%08x)", addr);
        return;
    }

    LoadMatrix(addr);

    if (gfx->gbi2matrix.projection)
    {
        // So far only Extreme-G seems to Push/Pop projection matrices  
        CRender::g_pRender->SetProjection(matToLoad, gfx->gbi2matrix.nopush==0, gfx->gbi2matrix.load);
    }
    else
    {
        CRender::g_pRender->SetWorldView(matToLoad, gfx->gbi2matrix.nopush==0, gfx->gbi2matrix.load);

        if( options.enableHackForGames == HACK_FOR_SOUTH_PARK_RALLY )
        {
            dlistMtxCount++;
            if( dlistMtxCount == 2 )
            {
                CRender::g_pRender->ClearZBuffer(1.0f);
            }
        }
    }

#ifdef DEBUGGER
    const char *loadstr = gfx->gbi2matrix.load?"Load":"Mul";
    const char *pushstr = gfx->gbi2matrix.nopush==0?"Push":"Nopush";
    int projlevel = CRender::g_pRender->GetProjectMatrixLevel();
    int worldlevel = CRender::g_pRender->GetWorldViewMatrixLevel();
    if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
    {
        pauseAtNext = false;
        debuggerPause = true;
        if (gfx->gbi2matrix.projection)
        {
            DebuggerAppendMsg("Pause after %s and %s Matrix: Projection, level=%d\n", loadstr, pushstr, projlevel );
        }
        else
        {
            DebuggerAppendMsg("Pause after %s and %s Matrix: WorldView level=%d\n", loadstr, pushstr, worldlevel);
        }
    }
    else
    {
        if( pauseAtNext && logMatrix ) 
        {
            if (gfx->gbi2matrix.projection)
            {
                DebuggerAppendMsg("Matrix: %s and %s Projection level=%d\n", loadstr, pushstr, projlevel);
            }
            else
            {
                DebuggerAppendMsg("Matrix: %s and %s WorldView\n level=%d", loadstr, pushstr, worldlevel);
            }
        }
    }
#endif
}

void RSP_GBI2_MoveMem(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_MoveMem);

    uint32 addr = RSPSegmentAddr((gfx->words.w1));
    uint32 type    = ((gfx->words.w0)     ) & 0xFE;

    //uint32 dwLen = ((gfx->words.w0) >> 16) & 0xFF;
    //uint32 dwOffset = ((gfx->words.w0) >> 8) & 0xFFFF;

    switch (type)
    {
    case RSP_GBI2_MV_MEM__VIEWPORT:
        {
            RSP_MoveMemViewport(addr);
        }
        break;
    case RSP_GBI2_MV_MEM__LIGHT:
        {
            uint32 dwOffset2 = ((gfx->words.w0) >> 5) & 0x3FFF;
        switch (dwOffset2)
        {
        case 0x00:
            {
                s8 * pcBase = g_pRDRAMs8 + addr;
                LOG_UCODE("    RSP_GBI1_MV_MEM_LOOKATX %f %f %f",
                    (float)pcBase[8 ^ 0x3],
                    (float)pcBase[9 ^ 0x3],
                    (float)pcBase[10 ^ 0x3]);

            }
            break;
        case 0x18:
            {
                s8 * pcBase = g_pRDRAMs8 + addr;
                LOG_UCODE("    RSP_GBI1_MV_MEM_LOOKATY %f %f %f",
                    (float)pcBase[8 ^ 0x3],
                    (float)pcBase[9 ^ 0x3],
                    (float)pcBase[10 ^ 0x3]);
            }
            break;
        default:        //0x30/48/60
            {
                uint32 dwLight = (dwOffset2 - 0x30)/0x18;
                LOG_UCODE("    Light %d:", dwLight);
                    RSP_MoveMemLight(dwLight, addr);
            }
            break;
        }
        break;

        }
    case RSP_GBI2_MV_MEM__MATRIX:
        LOG_UCODE("Force Matrix: addr=%08X", addr);
        RSP_GFX_Force_Matrix(addr);
        break;
    case RSP_GBI2_MV_MEM_O_L0:
    case RSP_GBI2_MV_MEM_O_L1:
    case RSP_GBI2_MV_MEM_O_L2:
    case RSP_GBI2_MV_MEM_O_L3:
    case RSP_GBI2_MV_MEM_O_L4:
    case RSP_GBI2_MV_MEM_O_L5:
    case RSP_GBI2_MV_MEM_O_L6:
    case RSP_GBI2_MV_MEM_O_L7:
        LOG_UCODE("Zelda Move Light");
        RDP_NOIMPL_WARN("Zelda Move Light");
        break;

    case RSP_GBI2_MV_MEM__POINT:
        LOG_UCODE("Zelda Move Point");
        void RDP_NOIMPL_WARN(const char* op);
        RDP_NOIMPL_WARN("Zelda Move Point");
        break;

    case RSP_GBI2_MV_MEM_O_LOOKATX:
        if( (gfx->words.w0) == 0xDC170000 && ((gfx->words.w1)&0xFF000000) == 0x80000000 )
        {
            // Ucode for Evangelion.v64, the ObjMatrix cmd
            RSP_S2DEX_OBJ_MOVEMEM(gfx);
        }
        break;
    case RSP_GBI2_MV_MEM_O_LOOKATY:
        RSP_RDP_NOIMPL("Not implemented ZeldaMoveMem LOOKATY, Cmd0=0x%08X, Cmd1=0x%08X", gfx->words.w0, gfx->words.w1);
        break;
    case 0x02:
        if( (gfx->words.w0) == 0xDC070002 && ((gfx->words.w1)&0xFF000000) == 0x80000000 )
        {
            RSP_S2DEX_OBJ_MOVEMEM(gfx);
            break;
        }
    default:
        LOG_UCODE("ZeldaMoveMem Type: Unknown");
        RSP_RDP_NOIMPL("Unknown ZeldaMoveMem Type, type=0x%X, Addr=%08X", type, addr);
        break;
    }
}



void RSP_GBI2_DL(Gfx *gfx)
{
    SP_Timing(RSP_GBI0_DL);

    uint32 dwPush = ((gfx->words.w0) >> 16) & 0xFF;
    uint32 dwAddr = RSPSegmentAddr((gfx->words.w1));

    if( dwAddr > g_dwRamSize )
    {
        RSP_RDP_NOIMPL("Error: DL addr = %08X out of range, PC=%08X", dwAddr, gDlistStack[gDlistStackPointer].pc );
        dwAddr &= (g_dwRamSize-1);
        DebuggerPauseCountN( NEXT_DLIST );
    }

    LOG_UCODE("    DL: Push:0x%02x Addr: 0x%08x", dwPush, dwAddr);
    
    switch (dwPush)
    {
    case RSP_DLIST_PUSH:
        LOG_UCODE("    Pushing ZeldaDisplayList 0x%08x", dwAddr);
        gDlistStackPointer++;
        gDlistStack[gDlistStackPointer].pc = dwAddr;
        gDlistStack[gDlistStackPointer].countdown = MAX_DL_COUNT;

        break;
    case RSP_DLIST_NOPUSH:
        LOG_UCODE("    Jumping to ZeldaDisplayList 0x%08x", dwAddr);
        if( gDlistStack[gDlistStackPointer].pc == dwAddr+8 )    //Is this a loop
        {
            //Hack for Gauntlet Legends
            gDlistStack[gDlistStackPointer].pc = dwAddr+8;
        }
        else
            gDlistStack[gDlistStackPointer].pc = dwAddr;
        gDlistStack[gDlistStackPointer].countdown = MAX_DL_COUNT;
        break;
    }

    LOG_UCODE("");
    LOG_UCODE("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
    LOG_UCODE("#############################################");


}



void RSP_GBI2_SetOtherModeL(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_SetOtherModeL);

    uint32 dwShift = ((gfx->words.w0)>>8)&0xFF;
    uint32 dwLength= ((gfx->words.w0)   )&0xFF;
    uint32 dwData  = (gfx->words.w1);

    // Mask is constructed slightly differently
    uint32 dwMask = (uint32)((s32)(0x80000000)>>dwLength)>>dwShift;
    dwData &= dwMask;

    uint32 modeL = gRDP.otherModeL;
    modeL = (modeL&(~dwMask)) | dwData;

    Gfx tempgfx;
    tempgfx.words.w0 = gRDP.otherModeH;
    tempgfx.words.w1 = modeL;
    DLParser_RDPSetOtherMode(&tempgfx );
}



void RSP_GBI2_SetOtherModeH(Gfx *gfx)
{
    SP_Timing(RSP_GBI1_SetOtherModeH);

    uint32 dwLength= (((gfx->words.w0))&0xFF)+1;
    uint32 dwShift = 32 - (((gfx->words.w0)>>8)&0xFF) - dwLength;
    uint32 dwData  = (gfx->words.w1);

    uint32 dwMask2 = ((1<<dwLength)-1)<<dwShift;
    uint32 dwModeH = gRDP.otherModeH;
    dwModeH = (dwModeH&(~dwMask2)) | dwData;

    Gfx tempgfx;
    tempgfx.words.w0 = dwModeH;
    tempgfx.words.w1 = gRDP.otherModeL;
    DLParser_RDPSetOtherMode(&tempgfx );
}


void RSP_GBI2_SubModule(Gfx *gfx)
{
    SP_Timing(RSP_GBI2_SubModule);

    RSP_RDP_NOIMPL("RDP: RSP_GBI2_SubModule (0x%08x 0x%08x)", (gfx->words.w0), (gfx->words.w1));
}

