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

#ifndef _EXTENDED_RENDER_H_
#define _EXTENDED_RENDER_H_

#include "RSP_Parser.h"
#include "RSP_S2DEX.h"

// Define the render extension interface and provide empty implementation of 
// the render extension functions.
// Real render can either implement or not implement these extended render functions

// These extended render functions are in different groups:
// - Group #1:  Related to frame buffer
// - Group #2:  Related to 2D sprite
// - Group #3:  Related BG and ScaledBG
class CExtendedRender
{
public:
    virtual ~CExtendedRender() {}

    virtual void DrawFrameBuffer(bool useVIreg=false, uint32 left=0, uint32 top=0, uint32 width=0, uint32 height=0) {};
    virtual void LoadFrameBuffer(bool useVIreg=false, uint32 left=0, uint32 top=0, uint32 width=0, uint32 height=0) {};
    virtual void LoadTxtrBufFromRDRAM(void) {};
    virtual void LoadTxtrBufIntoTexture(void) {};

    virtual void DrawSprite2D(Sprite2DInfo &info, uint32 ucode) {};
    virtual void LoadSprite2D(Sprite2DInfo &info, uint32 ucode) {};

    
    virtual void DrawSprite(uObjTxSprite &sprite, bool rectR = true) {};
    virtual void DrawObjBG1CYC(uObjScaleBg &bg, bool scaled=true) {};
    virtual void DrawObjBGCopy(uObjBg &info) {};
    virtual void LoadObjBGCopy(uObjBg &info) {};
    virtual void LoadObjBG1CYC(uObjScaleBg &info) {};
    virtual void LoadObjSprite(uObjTxSprite &info, bool useTIAddr=false) {};

    virtual void DrawText(const char* str, RECT *rect) {};
};


#endif

