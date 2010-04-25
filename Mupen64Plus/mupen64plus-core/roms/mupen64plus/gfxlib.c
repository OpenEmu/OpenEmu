/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - hardware.h                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Marshallh                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <ultra64.h>

#include "include/config.h"

Lights1  light1 = gdSPDefLights1(
                                 0xFF, 0x10, 0x10, /* Ambient color. */
                                 0xaf, 0xaf, 0xaf, /* Diffuse color. */
                                 -1, 0, 1          /* Normal. */
                                );

/* Initialize RSP state DL. */
Gfx N64DLinitRSP[] =
{
    gsSPViewport(&vp),
    gsSPClearGeometryMode(G_SHADE | G_SHADING_SMOOTH | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD),
    gsSPTexture(0, 0, 0, 0, G_OFF),
    gsSPEndDisplayList(),
};



/* Initialize RDP state DL. */
Gfx N64DLinitRDP[] =
{
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
    gsDPSetColorDither(G_CD_ENABLE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};

/* Clear ZBuffer DL. */
Gfx N64DLclearZbuffer[] =
{
    gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, OS_K0_TO_PHYSICAL(_zbuf)),
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_FILL),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF),
    gsDPSetFillColor(GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0)),
    gsDPFillRectangle(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1),
    gsDPNoOp(),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};

/* Setup 3D rendering mode. */
Gfx N64DLset3dRenderMode[] =
{
    gsSPClearGeometryMode(G_SHADE | G_SHADING_SMOOTH | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD ),
    gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_ZBUFFER),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPPipelineMode(G_PM_1PRIMITIVE),
    gsDPSetRenderMode(G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_MODULATERGBA, G_CC_MODULATERGBA),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp (G_TP_PERSP),
    gsDPSetTextureLOD (G_TL_TILE),
    gsDPSetTextureFilter (G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT (G_TT_NONE),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPTexture(0x8000, 0x8000, 0, 0, G_ON),
    gsDPPipeSync(),
    gsSPEndDisplayList()
};

void N64setViewPort(int Xs, int Ys, int Xt, int Yt)
{
    vp.vp.vscale[0]= Xs<<1;
    vp.vp.vscale[1]= Ys<<1;
    vp.vp.vtrans[0]= Xt<<1;
    vp.vp.vtrans[1]= Yt<<1;
}

void N64clearScreen(Gfx** l)
{
    Gfx* dl;

    dl = *l;

    gDPPipeSync(dl++);
    gDPSetCycleType(dl++, G_CYC_FILL);
    gDPSetRenderMode(dl++, G_RM_OPA_SURF, G_RM_OPA_SURF);
    gDPSetFillColor(dl++, ScreenClearColor);
    gDPFillRectangle(dl++, 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);
    gDPNoOp(dl++);
    gDPPipeSync(dl++);

    *l = dl ;
}

/* Initialize main displaylist. */
void N64initDL(Gfx** DL_Pos, Gfx* DL_Start, char* rsp_static_addr, Dynamic* generate, u16* CFB, u16 cfbflag, u16 zbufflag)
{
    Gfx* glistp;
    int i;

    glistp = *DL_Pos;

    gSPSegment(glistp++, 0, 0x0);
    gSPSegment(glistp++, STATIC_SEG,  OS_K0_TO_PHYSICAL(rsp_static_addr));
    gSPSegment(glistp++, DYNAMIC_SEG, OS_K0_TO_PHYSICAL(generate));

    gSPDisplayList(glistp++, N64DLinitRSP);
    gSPDisplayList(glistp++, N64DLinitRDP);

    if(cfbflag == YES)
        {
        gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, OS_K0_TO_PHYSICAL(CFB));
        N64clearScreen(&glistp);
        }

    if(zbufflag == YES)
        gSPDisplayList(glistp++, N64DLclearZbuffer);

    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, OS_K0_TO_PHYSICAL(CFB));

    gSPDisplayList(glistp++, N64DLset3dRenderMode);

    gDPSetDepthImage(glistp++, OS_K0_TO_PHYSICAL(_zbuf));

    *DL_Pos = glistp;
}

/* End main displaylist. */
void N64terminateDL(Gfx** DL_Pos)
{
    Gfx* glistp;

    glistp = *DL_Pos;

    gDPFullSync(glistp++); /* Only need this if you want 'accurate' SP done? */
    gSPEndDisplayList(glistp++);

    *DL_Pos = glistp;
}

/* Swap color frame buffers. */
void N64swapCFB()
{
    static int first=1;

    if(first)
        {
        osViBlack(FALSE);
        first = 0;
        }

    osViSwapBuffer(Logical_CFB);

    CFB_Count++;
    if (CFB_Count >= NUM_CFB)
        CFB_Count = 0;

    Logical_CFB = _cfb[CFB_Count];

    CFB_NextCount = CFB_Count+1;
    if(CFB_NextCount >= NUM_CFB)
        CFB_NextCount = 0;

    Next_CFB = _cfb[CFB_NextCount];
}

/* Wait for Vertical Retrace. */
void N64waitVRT()
{
    if(MQ_IS_FULL(&RetraceMessageQ))
        osRecvMesg(&RetraceMessageQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&RetraceMessageQ, NULL, OS_MESG_BLOCK);
}

/* Wait on RDP. */
void N64waitRDP(int first)
{
    if(!first)
        osRecvMesg(&RDPDoneMessageQ, NULL, OS_MESG_BLOCK);
}

/* Wait for RSP. */
void N64waitRSP(int first)
{
    if(!first)
        osRecvMesg(&RSPDoneMessageQ, NULL, OS_MESG_BLOCK);
}

/* Start rendering task and load microcode. */
void N64initTask(OSTask* task, Gfx* gdl_end, Gfx* gdl_start)
{
    task->t.type = M_GFXTASK;
    task->t.flags = 0;
    task->t.ucode_boot = rspbootTextStart;
    task->t.ucode_boot_size=((u32)rspbootTextEnd -                                (u32)rspbootTextStart);
    task->t.ucode = gspF3DEX_fifoTextStart;
    task->t.ucode_size = SP_UCODE_SIZE;
    task->t.ucode_data = gspF3DEX_fifoDataStart;
    task->t.ucode_data_size = SP_UCODE_DATA_SIZE;
    task->t.dram_stack = (u64*)_rspdrambuf;
    task->t.dram_stack_size = SP_DRAM_STACK_SIZE8;
    task->t.output_buff = fifobuffer;
    task->t.output_buff_size = (u64*)fifosize;
    task->t.data_ptr = (u64*)gdl_start;
    task->t.data_size = ((int) gdl_end - (int) gdl_start);
    task->t.yield_data_ptr = 0;
    task->t.yield_data_size = 0;
}


