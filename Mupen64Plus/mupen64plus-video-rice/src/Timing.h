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

#ifndef _RSP_RDP_TIMING_H_
#define _RSP_RDP_TIMING_H_

enum
{
    Timing_SP_Minimal = 10,
    Timing_SP_Minimal2 = 20,
    Timing_SP_Minimal4 = 40,
    Timing_SP_Minimal8 = 80,
    Timing_SP_Each_Triangle = 80,
    Timing_SP_Each_2_Triangle2 = 160,

    Timing_RSP_GBI1_SpNoop = Timing_SP_Minimal,
    Timing_RSP_GBI0_Mtx = Timing_SP_Minimal8,
    Timing_RSP_GBI1_Reserved = Timing_SP_Minimal2,
    Timing_RSP_GBI1_MoveMem = Timing_SP_Minimal2,   // Fix me
    Timing_RSP_GBI1_Vtx = Timing_SP_Minimal4,       // for each vertex
    Timing_RSP_GBI0_Vtx = Timing_SP_Minimal4,       // for each vertex
    Timing_RSP_GBI0_DL = Timing_SP_Minimal*2,
    Timing_RSP_GBI1_Sprite2DBase = Timing_SP_Minimal8,
    Timing_RSP_GBI1_LoadUCode = 800,

    Timing_RSP_GBI1_BranchZ = Timing_SP_Minimal2,
    Timing_RSP_GBI1_Tri2 = Timing_SP_Each_2_Triangle2,
    Timing_RSP_GBI1_ModifyVtx = Timing_SP_Minimal4,
    Timing_RSP_GBI1_RDPHalf_2 = Timing_SP_Minimal,
    Timing_RSP_GBI1_RDPHalf_1 = Timing_SP_Minimal,
    Timing_RSP_GBI1_RDPHalf_Cont = Timing_SP_Minimal,
    Timing_RSP_GBI1_Line3D = Timing_SP_Each_Triangle,
    Timing_RSP_GBI1_ClearGeometryMode = Timing_SP_Minimal,
    Timing_RSP_GBI1_SetGeometryMode = Timing_SP_Minimal,
    Timing_RSP_GBI2_GeometryMode = Timing_SP_Minimal,
    Timing_RSP_GBI1_EndDL = Timing_SP_Minimal,
    Timing_RSP_GBI1_SetOtherModeL = Timing_SP_Minimal,
    Timing_RSP_GBI1_SetOtherModeH = Timing_SP_Minimal,
    Timing_RSP_GBI1_Texture = Timing_SP_Minimal2,
    Timing_RSP_GBI1_MoveWord = Timing_SP_Minimal2,
    Timing_RSP_GBI2_SubModule = Timing_SP_Minimal2,

    Timing_RSP_GBI1_PopMtx = Timing_SP_Minimal8,
    Timing_RSP_GBI1_CullDL = Timing_SP_Minimal2,
    Timing_RSP_GBI1_Tri1 = Timing_SP_Each_Triangle,

    Timing_RSP_GBI1_Noop = Timing_SP_Minimal,
    Timing_RSP_S2DEX_SPObjLoadTxtr_Ucode1 = Timing_SP_Minimal8,

    Timing_DP_Minimal = 10,
    Timing_DP_Minimal2 = 20,
    Timing_DP_Minimal4 = 40,
    Timing_DP_Minimal8 = 80,
    Timing_DP_Minimal16 = 160,
    Timing_DP_Each_Point = 1,

    Timing_RDP_TriFill = Timing_DP_Minimal8,
    Timing_RDP_TriFillZ = Timing_DP_Minimal8,
    Timing_RDP_TriTxtr = Timing_DP_Minimal8,
    Timing_RDP_TriTxtrZ = Timing_DP_Minimal8,
    Timing_RDP_TriShade = Timing_DP_Minimal8,
    Timing_RDP_TriShadeZ = Timing_DP_Minimal8,
    Timing_RDP_TriShadeTxtr = Timing_DP_Minimal8,
    Timing_RDP_TriShadeTxtrZ = Timing_DP_Minimal8,

    Timing_DLParser_TexRect = Timing_DP_Minimal8,
    Timing_DLParser_TexRectFlip = Timing_DP_Minimal8,
    Timing_DLParser_RDPLoadSync = Timing_DP_Minimal,
    Timing_DLParser_RDPPipeSync = Timing_DP_Minimal,
    Timing_DLParser_RDPTileSync = Timing_DP_Minimal,
    Timing_DLParser_RDPFullSync = Timing_DP_Minimal8,
    Timing_DLParser_SetKeyGB = Timing_DP_Minimal,
    Timing_DLParser_SetKeyR = Timing_DP_Minimal,
    Timing_DLParser_SetConvert = Timing_DP_Minimal2,
    Timing_DLParser_SetScissor = Timing_DP_Minimal2,
    Timing_DLParser_SetPrimDepth = Timing_DP_Minimal2,
    Timing_DLParser_RDPSetOtherMode = Timing_DP_Minimal,

    Timing_DLParser_LoadTLut = Timing_DP_Minimal16,
    Timing_RSP_RDP_Nothing = Timing_DP_Minimal,
    Timing_DLParser_SetTileSize = Timing_DP_Minimal4,
    Timing_DLParser_LoadBlock = Timing_DP_Minimal16, 
    Timing_DLParser_LoadTile = Timing_DP_Minimal16,
    Timing_DLParser_SetTile = Timing_DP_Minimal8,
    Timing_DLParser_FillRect = Timing_DP_Minimal16,
    Timing_DLParser_SetFillColor = Timing_DP_Minimal,
    Timing_DLParser_SetFogColor = Timing_DP_Minimal,
    Timing_DLParser_SetBlendColor = Timing_DP_Minimal,
    Timing_DLParser_SetPrimColor = Timing_DP_Minimal,
    Timing_DLParser_SetEnvColor = Timing_DP_Minimal,
    Timing_DLParser_SetCombine = Timing_DP_Minimal,
    Timing_DLParser_SetTImg = Timing_DP_Minimal,
    Timing_DLParser_SetZImg = Timing_DP_Minimal,
    Timing_DLParser_SetCImg = Timing_DP_Minimal,
};

#define DP_Timing(t) {status.DPCycleCount+=Timing_##t;}
#define SP_Timing(t) {status.SPCycleCount+=Timing_##t;}

#endif

