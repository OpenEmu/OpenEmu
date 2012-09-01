/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * Render2D.cpp
 * 
 * Implementation of the CRender2D class: OpenGL tile generator graphics. 
 *
 * To-Do List
 * ----------
 * - Add dirty rectangles? There is already some inactive code in here for
 *   this purpose and it needs to be updated or deleted once and for all.
 * - Are v-scroll values 9 or 10 bits? 
 * - Add fast paths for no scrolling (including unclipped tile rendering).
 * - Inline the loops in the tile renderers.
 * - Update description of tile generator before you forget :)
 * - A proper shut-down function is needed! OpenGL might not be available when
 *   the destructor for this class is called.
 */

#include <string.h>
#include "Pkgs/glew.h"
#include "Supermodel.h"
#include "Graphics/Shaders2D.h"	// fragment and vertex shaders


/******************************************************************************
 Definitions and Constants
******************************************************************************/

// Shader program files (for use in development builds only)
#define VERTEX_2D_SHADER_FILE	"Src/Graphics/Vertex2D.glsl"
#define FRAGMENT_2D_SHADER_FILE	"Src/Graphics/Fragment2D.glsl"


/******************************************************************************
 Tile Drawing Functions
******************************************************************************/

// Draw 4-bit tile line, no clipping performed
void CRender2D::DrawTileLine4BitNoClip(UINT32 *buf, UINT16 tile, int tileLine)
{
    unsigned	tileOffset;	// offset of tile pattern within VRAM
    unsigned	palette;   	// color palette bits obtained from tile
    UINT32  	pattern;    // 8 pattern pixels fetched at once

	// Tile pattern offset: each tile occupies 32 bytes when using 4-bit pixels
    tileOffset = ((tile&0x3FFF)<<1) | ((tile>>15)&1);
    tileOffset *= 32;
    tileOffset /= 4;	// VRAM is a UINT32 array

	// Upper color bits; the lower 4 bits come from the tile pattern
	palette = tile&0x7FF0;
    
    // Draw 8 pixels
    pattern = vram[tileOffset+tileLine];
    *buf++ = pal[((pattern>>28)&0xF) | palette];
    *buf++ = pal[((pattern>>24)&0xF) | palette];
    *buf++ = pal[((pattern>>20)&0xF) | palette];
    *buf++ = pal[((pattern>>16)&0xF) | palette];
    *buf++ = pal[((pattern>>12)&0xF) | palette];
    *buf++ = pal[((pattern>>8)&0xF) | palette];
    *buf++ = pal[((pattern>>4)&0xF) | palette];
    *buf++ = pal[((pattern>>0)&0xF) | palette];
}

// Draw 8-bit tile line, clipped at left edge
void CRender2D::DrawTileLine8BitNoClip(UINT32 *buf, UINT16 tile, int tileLine)
{
    unsigned	tileOffset;	// offset of tile pattern within VRAM
    unsigned	palette;   	// color palette bits obtained from tile
    UINT32  	pattern;    // 4 pattern pixels fetched at once

	tileLine *= 2;	// 8-bit pixels, each line is two words
	
	// Tile pattern offset: each tile occupies 64 bytes when using 8-bit pixels
    tileOffset = tile&0x3FFF;
    tileOffset *= 64;
    tileOffset /= 4;

	// Upper color bits
	palette = tile&0x7F00;
    
    // Draw 4 pixels at a time
    pattern = vram[tileOffset+tileLine];
    *buf++ = pal[((pattern>>24)&0xFF) | palette];
    *buf++ = pal[((pattern>>16)&0xFF) | palette];
    *buf++ = pal[((pattern>>8)&0xFF) | palette];
    *buf++ = pal[((pattern>>0)&0xFF) | palette];
    pattern = vram[tileOffset+tileLine+1];
    *buf++ = pal[((pattern>>24)&0xFF) | palette];
    *buf++ = pal[((pattern>>16)&0xFF) | palette];
    *buf++ = pal[((pattern>>8)&0xFF) | palette];
    *buf++ = pal[((pattern>>0)&0xFF) | palette];
}


// Draw 4-bit tile line, clipped at left edge
void CRender2D::DrawTileLine4Bit(UINT32 *buf, int offset, UINT16 tile, int tileLine)
{
    unsigned	tileOffset;	// offset of tile pattern within VRAM
    unsigned	palette;   	// color palette bits obtained from tile
    UINT32  	pattern;    // 8 pattern pixels fetched at once

	// Tile pattern offset: each tile occupies 32 bytes when using 4-bit pixels
    tileOffset = ((tile&0x3FFF)<<1) | ((tile>>15)&1);
    tileOffset *= 32;
    tileOffset /= 4;	// VRAM is a UINT32 array

	// Upper color bits; the lower 4 bits come from the tile pattern
	palette = tile&0x7FF0;
    
    // Draw 8 pixels
    pattern = vram[tileOffset+tileLine];
    for (int bitPos = 28; bitPos >= 0; bitPos -= 4)
   	{
    	if (offset >= 0)
    		buf[offset] = pal[((pattern>>bitPos)&0xF) | palette];
    	++offset;
    }
}

// Draw 4-bit tile line, clipped at right edge
void CRender2D::DrawTileLine4BitRightClip(UINT32 *buf, int offset, UINT16 tile, int tileLine, int numPixels)
{
    unsigned	tileOffset;	// offset of tile pattern within VRAM
    unsigned	palette;   	// color palette bits obtained from tile
    UINT32  	pattern;    // 8 pattern pixels fetched at once
    int			bitPos;

	// Tile pattern offset: each tile occupies 32 bytes when using 4-bit pixels
    tileOffset = ((tile&0x3FFF)<<1) | ((tile>>15)&1);
    tileOffset *= 32;
    tileOffset /= 4;	// VRAM is a UINT32 array

	// Upper color bits; the lower 4 bits come from the tile pattern
	palette = tile&0x7FF0;
    
    // Draw 8 pixels
    pattern = vram[tileOffset+tileLine];
    bitPos = 28;
    for (int i = 0; i < numPixels; i++)
   	{
    	buf[offset] = pal[((pattern>>bitPos)&0xF) | palette];
    	++offset;
    	bitPos -= 4;
    }
}

// Draw 8-bit tile line, clipped at left edge
void CRender2D::DrawTileLine8Bit(UINT32 *buf, int offset, UINT16 tile, int tileLine)
{
    unsigned	tileOffset;	// offset of tile pattern within VRAM
    unsigned	palette;   	// color palette bits obtained from tile
    UINT32  	pattern;    // 4 pattern pixels fetched at once

	tileLine *= 2;	// 8-bit pixels, each line is two words
	
	// Tile pattern offset: each tile occupies 64 bytes when using 8-bit pixels
    tileOffset = tile&0x3FFF;
    tileOffset *= 64;
    tileOffset /= 4;

	// Upper color bits
	palette = tile&0x7F00;
    
    // Draw 4 pixels at a time
    pattern = vram[tileOffset+tileLine];
    for (int bitPos = 24; bitPos >= 0; bitPos -= 8)
   	{
    	if (offset >= 0)
    		buf[offset] = pal[((pattern>>bitPos)&0xFF) | palette];
    	++offset;
    }
    
    pattern = vram[tileOffset+tileLine+1];
    for (int bitPos = 24; bitPos >= 0; bitPos -= 8)
   	{
    	if (offset >= 0)
    		buf[offset] = pal[((pattern>>bitPos)&0xFF) | palette];
    	++offset;
    }
}

// Draw 8-bit tile line, clipped at right edge
void CRender2D::DrawTileLine8BitRightClip(UINT32 *buf, int offset, UINT16 tile, int tileLine, int numPixels)
{
    unsigned	tileOffset;	// offset of tile pattern within VRAM
    unsigned	palette;   	// color palette bits obtained from tile
    UINT32  	pattern;    // 4 pattern pixels fetched at once
    int			bitPos;

	tileLine *= 2;	// 8-bit pixels, each line is two words
	
	// Tile pattern offset: each tile occupies 64 bytes when using 8-bit pixels
    tileOffset = tile&0x3FFF;
    tileOffset *= 64;
    tileOffset /= 4;
    
	// Upper color bits
	palette = tile&0x7F00;
    
    // Draw 4 pixels at a time
    pattern = vram[tileOffset+tileLine];
    bitPos = 24;
    for (int i = 0; (i < 4) && (i < numPixels); i++)
   	{
    	buf[offset] = pal[((pattern>>bitPos)&0xFF) | palette];
    	++offset;
    	bitPos -= 8;
    }
    
    pattern = vram[tileOffset+tileLine+1];
    bitPos = 24;
    for (int i = 0; (i < 4) && (i < numPixels); i++)
   	{
    	buf[offset] = pal[((pattern>>bitPos)&0xFF) | palette];
    	++offset;
    	bitPos -= 8;
    }
}


/******************************************************************************
 Layer Rendering
******************************************************************************/

/*
 * DrawCompleteLayer():
 *
 * Updates the complete layer.
 */
void CRender2D::DrawCompleteLayer(int layerNum, const UINT16 *nameTableBase)
{
	UINT32			*dest = surf;							// destination surface to write to
	UINT32			*lineBufferPri = &surf[512*496];		// line buffer for primary and alternate layer
	UINT32			*lineBufferAlt = &surf[512*497];
	UINT32			*buf;
	const UINT16	*maskTable;								// pointer to start of mask table
	const UINT16	*hScrollTablePri, *hScrollTableAlt;		// pointers to line scroll tables
	const UINT16	*nameTablePri = nameTableBase;			// primary (this layer) name table
	const UINT16	*nameTableAlt = &nameTableBase[64*64];	// alternate layer's name table	
	const UINT16	*nameTable;
	int				colorDepthPri, colorDepthAlt;			// primary and alternate layer color depths
	int				hScrollPri, hScrollAlt;					// primary and alternate layer scroll offsets
	int				vScrollPri, vScrollAlt;
	int				hFullScrollPri, hFullScrollAlt;			// full-screen horizontal scroll values (from registers)
	int				vOffset;								// vertical pixel offset within tile
	int				tx, i, j;
	bool			lineScrollPri, lineScrollAlt;			// line scrolling enable/disable
	UINT16			mask;
	
	// Determine layer color depths (1 if 4-bit, 0 if 8-bit)
	colorDepthPri = regs[0x20/4] & (1<<(12+layerNum*2));
	colorDepthAlt = regs[0x20/4] & (1<<(12+layerNum*2+1));
	
	// Line scroll tables
	hScrollTablePri = (UINT16 *) &vram[(0xF6000+layerNum*2*0x400)/4];
	hScrollTableAlt = (UINT16 *) &vram[(0xF6000+layerNum*2*0x400+0x400)/4];
	
	// Get correct offset into mask table
	maskTable = (UINT16 *) &vram[0xF7000/4];
	if (layerNum == 0)
		++maskTable;	// little endian, layer 0 is second word in each pair
		
	// Load horizontal full-screen scroll values and scroll mode
	hFullScrollPri = regs[0x60/4+layerNum*2]&0x3FF;
	hFullScrollAlt = regs[0x60/4+layerNum*2+1]&0x3FF;
	lineScrollPri = regs[0x60/4+layerNum*2]&0x8000;
	lineScrollAlt = regs[0x60/4+layerNum*2+1]&0x8000;
	
	// Load vertical scroll values
	vScrollPri = (regs[0x60/4+layerNum*2]>>16)&0x1FF;
	vScrollAlt = (regs[0x60/4+layerNum*2+1]>>16)&0x1FF;
	
	// Iterate over all displayed lines
	for (int y = 0; y < 384; y++)
	{
		/*
		 * Draw all tiles from primary layer first. Horizontal scrolling is not
		 * applied yet, but vertical scrolling is taken into account. An entire
		 * 512-pixel line is rendered so that it can be scrolled during mixing.
		 */		
		nameTable = &nameTablePri[(64*((y+vScrollPri)/8)) & 0xFFF];	// clamp to 64x64=0x1000
		vOffset = (y+vScrollPri)&7;
		buf = lineBufferPri;	// output to primary line buffer
		for (tx = 0; tx < 64; tx += 4)	// 4 tiles at a time (for masking)
		{
			if (colorDepthPri)	//TODO: move this test outside of loop
			{
				DrawTileLine4BitNoClip(buf, nameTable[1], vOffset);
				buf += 8;
				DrawTileLine4BitNoClip(buf, nameTable[0], vOffset);
				buf += 8;
				DrawTileLine4BitNoClip(buf, nameTable[3], vOffset);
				buf += 8;
				DrawTileLine4BitNoClip(buf, nameTable[2], vOffset);
				buf += 8;
			}
			else
			{
				DrawTileLine8BitNoClip(buf, nameTable[1], vOffset);
				buf += 8;
				DrawTileLine8BitNoClip(buf, nameTable[0], vOffset);
				buf += 8;
				DrawTileLine8BitNoClip(buf, nameTable[3], vOffset);
				buf += 8;
				DrawTileLine8BitNoClip(buf, nameTable[2], vOffset);
				buf += 8;
			}
				
			// Next set of 4 tiles
			nameTable += 4;
		}
		
		/*
		 * Draw the alternate layer wherever the primary layer was masked
		 */		
		nameTable = &nameTableAlt[(64*((y+vScrollAlt)/8))&0xFFF];
		vOffset = (y+vScrollAlt)&7;
		buf = lineBufferAlt;	// output to alternate line buffer
		for (tx = 0; tx < 64; tx += 4)	// 4 tiles at a time (for masking)
		{
			if (colorDepthAlt)	//TODO: move this test outside of loop
			{
				DrawTileLine4BitNoClip(buf, nameTable[1], vOffset);
				buf += 8;
				DrawTileLine4BitNoClip(buf, nameTable[0], vOffset);
				buf += 8;
				DrawTileLine4BitNoClip(buf, nameTable[3], vOffset);
				buf += 8;
				DrawTileLine4BitNoClip(buf, nameTable[2], vOffset);
				buf += 8;
			}
			else
			{
				DrawTileLine8BitNoClip(buf, nameTable[1], vOffset);
				buf += 8;
				DrawTileLine8BitNoClip(buf, nameTable[0], vOffset);
				buf += 8;
				DrawTileLine8BitNoClip(buf, nameTable[3], vOffset);
				buf += 8;
				DrawTileLine8BitNoClip(buf, nameTable[2], vOffset);
				buf += 8;
			}
				
			// Next set of 4 tiles
			nameTable += 4;
		}
		
		/*
		 * Mix the two layers into the current line under control of the 
		 * stencil mask, applying scrolling in the process.
		 */
		 
		// Load horizontal scroll values
		if (lineScrollPri)
			hScrollPri = hScrollTablePri[y];
		else
			hScrollPri = hFullScrollPri;
		if (lineScrollAlt)
			hScrollAlt = hScrollTableAlt[y];
		else
			hScrollAlt = hFullScrollAlt;
			
		// Mix first 60 tiles (4 at a time)
		mask = *maskTable;
		i = hScrollPri&511;	// primary line index
		j = hScrollAlt&511;	// alternate line index
		for (tx = 0; tx < 60; tx += 4)
		{
			if ((mask&0x8000))	// copy tiles from primary layer
			{
				if (i <= (512-32))	// safe to use memcpy for fast blit?
				{
					memcpy(dest, &lineBufferPri[i], 32*sizeof(UINT32));
					i += 32;
					dest += 32;
				}
				else				// slow copy, wrap line boundary
				{
					for (int k = 0; k < 32; k++)
					{
						i &= 511;
						*dest++ = lineBufferPri[i++];
					}
				}
				j += 32;	// update alternate pointer as well
			}		
			else				// copy tiles from alternate layer
			{
				if (j <= (512-32))
				{
					memcpy(dest, &lineBufferAlt[j], 32*sizeof(UINT32));
					j += 32;
					dest += 32;
				}
				else
				{
					for (int k = 0; k < 32; k++)
					{
						j &= 511;
						*dest++ = lineBufferAlt[j++];
					}
				}
				
				i += 32;	// update primary
			}
			
			mask <<= 1;
		}
		
		// Mix last two tiles
		if ((mask&0x8000))	// copy tiles from primary layer
		{
			for (int k = 0; k < 16; k++)
			{
				i &= 511;
				*dest++ = lineBufferPri[i++];
			}
		}
		else				// copy from alternate
		{
			for (int k = 0; k < 16; k++)
			{
				j &= 511;
				*dest++ = lineBufferAlt[j++];
			}
		}

		// Next line
		maskTable += 2;	// next mask line
	}
	
	// Upload
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 496, 384, GL_RGBA, GL_UNSIGNED_BYTE, surf);	
}


/*
 * DrawRect():
 *
 * Draws a rectangular portion of the given layer and uploads it. Scrolling is
 * applied but the result must be clipped against the rectangular window
 * defined here by tileX, tileY, tileW, and tileH.
 *
 * Clipping for the right side is not checked, which will always work as long
 * as the X and Y tile positions never exceed 61 and 47, respectively (the
 * dimensions of the physical display; there is border space because the
 * layers are 64x64).
 * 
 * Parameters:
 *		layerNum		Layer number (0 or 1).
 *		nameTableBase	Pointer to the layer's name table.
 *		tileX			Position of upper-left corner of rectangle on the
 *						screen, in units of 8-pixel tiles (0-61).
 *		tileY			"" (0-47)
 *		tileW			Width of rectangular region in tiles.
 *		tileH			"" (height)
 */
 void CRender2D::DrawRect(int layerNum, const UINT16 *nameTableBase, int tileX, int tileY, int tileW, int tileH)
{
	UINT32			*dest = surf;							// destination surface to write to
	const UINT16	*maskTable;								// pointer to start of mask table
	const UINT16	*hScrollTablePri, *hScrollTableAlt;		// pointers to line scroll tables
	const UINT16	*nameTablePri = nameTableBase;			// primary (this layer) name table
	const UINT16	*nameTableAlt = &nameTableBase[64*64];	// alternate layer's name table	
	const UINT16	*nameTable;
	int				colorDepthPri, colorDepthAlt;			// primary and alternate layer color depths
	int				hScrollPri, hScrollAlt;					// primary and alternate layer scroll offsets
	int				vScrollPri, vScrollAlt;
	int				hFullScrollPri, hFullScrollAlt;			// full-screen horizontal scroll values (from registers)
	int				hOffset, vOffset;						// pixel offsets
	int				ntOffset;								// offset in name table
	int				tx;
	bool			lineScrollPri, lineScrollAlt;			// line scrolling enable/disable
	UINT16			mask;
	
	// Determine layer color depths (1 if 4-bit, 0 if 8-bit)
	colorDepthPri = regs[0x20/4] & (1<<(12+layerNum*2));
	colorDepthAlt = regs[0x20/4] & (1<<(12+layerNum*2+1));
	
	// Line scroll tables
	hScrollTablePri = (UINT16 *) &vram[(0xF6000+layerNum*2*0x400)/4];
	hScrollTableAlt = (UINT16 *) &vram[(0xF6000+layerNum*2*0x400+0x400)/4];
	
	// Get correct offset into mask table
	maskTable = (UINT16 *) &vram[0xF7000/4];
	if (layerNum == 0)
		++maskTable;	// little endian, layer 0 is second word in each pair
		
	// Load horizontal full-screen scroll values and scroll mode
	hFullScrollPri = regs[0x60/4+layerNum*2]&0x3FF;
	hFullScrollAlt = regs[0x60/4+layerNum*2+1]&0x3FF;
	lineScrollPri = regs[0x60/4+layerNum*2]&0x8000;
	lineScrollAlt = regs[0x60/4+layerNum*2+1]&0x8000;
	
	// Load vertical scroll values
	vScrollPri = (regs[0x60/4+layerNum*2]>>16)&0x3FF;
	vScrollAlt = (regs[0x60/4+layerNum*2+1]>>16)&0x3FF;
	
	// Iterate over actual line on screen
	for (int y = tileY*8; y < (tileY+tileH)*8; y++)
	{
		
		// Load horizontal scroll values
		if (lineScrollPri)
			hScrollPri = hScrollTablePri[y];
		else
			hScrollPri = hFullScrollPri;
		if (lineScrollAlt)
			hScrollAlt = hScrollTableAlt[y];
		else
			hScrollAlt = hFullScrollAlt;

		/*
		 * Draw all tiles from primary layer first
		 */		
		
		// Compute scroll offsets into name table and destination
		hOffset = -(hScrollPri&7);
		vOffset = (y+vScrollPri)&7;
		ntOffset = tileX+hScrollPri/8;
		
		// Advance name table to our line (prior to h-scrolling)
		nameTable = &nameTablePri[64*((y+vScrollPri)/8) & 0xFFF];	// clamp to 64x64=0x1000
		
		// Each bit in the mask table corresponds to 4 tiles
		mask = maskTable[y*2];

		// Render a line!		
		//TODO: add one if scrolling
		for (tx = 0; tx < tileW; tx++)
		{
			if ( ((mask<<((tileX+tx)/4)) & 0x8000) )
			{
				if (colorDepthPri)
					DrawTileLine4Bit(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset); // make sure ^1 belongs inside parenthesis...
				else
					DrawTileLine8Bit(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset);
			}
			hOffset += 8;
			ntOffset++;
		}
		
		// When scrolling, extra tile must be rendered at right edge of region
		if ( ((mask<<((tileX+tx)/4)) & 0x8000) )	// re-use the last mask bit (mask doesn't scroll)
		{
			if (colorDepthPri)
				DrawTileLine4BitRightClip(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset, hScrollPri&7);
			else
				DrawTileLine8BitRightClip(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset, hScrollPri&7);
		}
		
		/*
		 * Draw the alternate layer wherever the primary layer was masked
		 */		
		
		hOffset = -(hScrollAlt&7);
		vOffset = (y+vScrollAlt)&7;
		ntOffset = tileX+hScrollAlt/8;
		nameTable = &nameTableAlt[64*((y+vScrollAlt)/8)];
		mask = maskTable[y*2];
		for (tx = 0; tx < tileW; tx++)
		{
			if (0 == ((mask<<((tileX+tx)/4)) & 0x8000))
			{
				if (colorDepthAlt)
					DrawTileLine4Bit(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset);
				else
					DrawTileLine8Bit(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset);
			}
			hOffset += 8;
			ntOffset++;
		}
		
		// When scrolling, extra tile must be rendered at right edge of region
		if (0 == ((mask<<((tileX+tx)/4)) & 0x8000))	// re-use the last mask bit (mask doesn't scroll)
		{
			if (colorDepthAlt)
				DrawTileLine4BitRightClip(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset, hScrollAlt&7);
			else
				DrawTileLine8BitRightClip(dest, hOffset, nameTable[(ntOffset^1)&63], vOffset, hScrollAlt&7);
		}
		
		// Next line
		dest += tileW*8;	// image surface is only as wide as the rectangle we're updating
	}
	
	// Upload
	glTexSubImage2D(GL_TEXTURE_2D, 0, tileX*8, tileY*8, tileW*8, tileH*8, GL_RGBA, GL_UNSIGNED_BYTE, surf);
}
	
// Updates any changed portions of a layer
void CRender2D::UpdateLayer(int layerNum)
{
	glBindTexture(GL_TEXTURE_2D, texID[layerNum]);

	allDirty = true;
	if (allDirty)
	{
		// If everything is dirty, update the whole thing at once
		DrawCompleteLayer(layerNum, (UINT16 *) &vram[(0xF8000+layerNum*2*0x2000)/4]);
		//DrawRect(layerNum, (UINT16 *) &vram[(0xF8000+layerNum*2*0x2000)/4], 0, 0, 62, 48);
		memset(dirty, 0, sizeof(dirty));
	}
	else
	{
		// Otherwise, for now, use a dumb approach that updates each rectangle individually
		for (int y = 0; y < 64/DIRTY_RECT_HEIGHT; y++)
		{
			for (int x = 0; x < 48/DIRTY_RECT_WIDTH; x++)
			{
				if (dirty[layerNum][y][x])
				{
					DrawRect(layerNum, (UINT16 *) &vram[(0xF8000+layerNum*2*0x2000)/4], x*DIRTY_RECT_WIDTH, y*DIRTY_RECT_HEIGHT, DIRTY_RECT_WIDTH, DIRTY_RECT_HEIGHT);
					dirty[layerNum][y][x] = 0;	// not dirty anymore
				}
			}
		}
	}
}


/******************************************************************************
 Frame Display Functions
******************************************************************************/

// Draws a layer to the screen
void CRender2D::DisplayLayer(int layerNum, GLfloat z)
{	
	glBindTexture(GL_TEXTURE_2D, texID[layerNum]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f/512.0f, 0.0f);          	glVertex3f(0.0f, 0.0f, z);
    glTexCoord2f(496.0f/512.0f, 0.0f);         	glVertex3f(1.0f, 0.0f, z);
    glTexCoord2f(496.0f/512.0f, 384.0f/512.0f);	glVertex3f(1.0f, 1.0f, z);
    glTexCoord2f(0.0f/512.0f, 384.0f/512.0f);   glVertex3f(0.0f, 1.0f, z);
    glEnd();
}

// Set up viewport and OpenGL state for 2D rendering (sets up blending function but disables blending)
void CRender2D::Setup2D(void)
{
	// Set up the viewport and orthogonal projection
	glViewport(xOffs, yOffs, xPixels, yPixels);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Enable texture mapping and blending
	glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// alpha of 1.0 is opaque, 0 is transparent
	glDisable(GL_BLEND);

	// Disable Z-buffering
	glDisable(GL_DEPTH_TEST);
	
	// Shader program
	glUseProgram(shaderProgram);
}

// Convert color offset register data to RGB
void CRender2D::ColorOffset(GLfloat colorOffset[3], UINT32 reg)
{
	INT8	ir, ig, ib;
	
	ib = (reg>>16)&0xFF;
	ig = (reg>>8)&0xFF;
	ir = (reg>>0)&0xFF;
	
	/*
	 * Uncertain how these should be interpreted. It appears to be signed, 
	 * which means the values range from -128 to +127. The division by 128
	 * normalizes this to roughly -1,+1.
	 */
	colorOffset[0] = (GLfloat) ir * (1.0f/128.0f);
	colorOffset[1] = (GLfloat) ig * (1.0f/128.0f);
	colorOffset[2] = (GLfloat) ib * (1.0f/128.0f);		
	//printf("%08X -> %g,%g,%g\n", reg, colorOffset[2], colorOffset[1], colorOffset[0]);
}

// Bottom layers
void CRender2D::BeginFrame(void)
{
	GLfloat	colorOffset[3];	
	
	// Update all layers
	for (int i = 0; i < 2; i++)
	{
		UpdateLayer(i);
	}
	allDirty = false;
	
	// Draw bottom layer
	Setup2D();
	ColorOffset(colorOffset, regs[0x44/4]);
	glUniform3fv(colorOffsetLoc, 1, colorOffset);
	DisplayLayer(1, 0.0);
}

// Top layers
void CRender2D::EndFrame(void)
{	
	GLfloat	colorOffset[3];
	
	// Draw top layer
	Setup2D();
	glEnable(GL_BLEND);
	ColorOffset(colorOffset, regs[0x40/4]);
	glUniform3fv(colorOffsetLoc, 1, colorOffset);
	DisplayLayer(0, -0.5);
}


/******************************************************************************
 Emulation Callbacks
******************************************************************************/

void CRender2D::WritePalette(unsigned color, UINT32 data)
{
	UINT8		r, g, b, a;
	
	a = 0xFF * ((data>>15)&1); 	// decode the RGBA (make alpha 0xFF or 0x00)
    a = ~a;                  	// invert it (set on Model 3 means clear pixel)
	
	if ((data&0x8000))
    	r = g = b = 0;
	else
    {
    	b = (data>>7)&0xF8;
        g = (data>>2)&0xF8;
        r = (data<<3)&0xF8;
	}
	
	pal[color] = (a<<24)|(b<<16)|(g<<8)|r;
}

void CRender2D::WriteVRAM(unsigned addr, UINT32 data)
{
	if (vram[addr/4] == data)	// do nothing if no changes
		return;
		
	// For now, mark everything as dirty
	allDirty = true;
		
	// Palette
	if (addr >= 0x100000)
    {
		unsigned color = (addr-0x100000)/4;	// color index
        WritePalette(color, data);
    }
}

/*
 * InitPalette():
 *
 * This must be called from AttachVRAM() to initialize the palette. The reason 
 * is that because WriteVRAM() always compares incoming data to what is already
 * in the VRAM, there is no actual way to initialize the palette by calling
 * WriteVRAM() and passing it the initial VRAM contents. It will always fail to
 * update because nothing is being changed.
 *
 * This function fixes the transparent pixel bug that frequently occurred when
 * loading save states in Supermodel 0.1a.
 */
void CRender2D::InitPalette(void)
{
	for (int i = 0; i < 0x20000/4; i++)
		WritePalette(i, vram[0x100000/4 + i]);
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

void CRender2D::AttachRegisters(const UINT32 *regPtr)
{
	regs = regPtr;
	DebugLog("Render2D attached registers\n");
}

void CRender2D::AttachVRAM(const UINT8 *vramPtr)
{
	vram = (UINT32 *) vramPtr;
	InitPalette();
	DebugLog("Render2D attached VRAM\n");
}

#define MEMORY_POOL_SIZE	(512*512*4+0x20000)

bool CRender2D::Init(unsigned xOffset, unsigned yOffset, unsigned xRes, unsigned yRes)
{
	float	memSizeMB = (float)MEMORY_POOL_SIZE/(float)0x100000;
	
	// Load shaders
	if (OKAY != LoadShaderProgram(&shaderProgram,&vertexShader,&fragmentShader,NULL,NULL,vertexShaderSource,fragmentShaderSource))
		return FAIL;
	
	// Get locations of the uniforms
	glUseProgram(shaderProgram);	// bind program
	textureMapLoc = glGetUniformLocation(shaderProgram, "textureMap");
	glUniform1i(textureMapLoc,0);	// attach it to texture unit 0
	colorOffsetLoc = glGetUniformLocation(shaderProgram, "colorOffset");
	
	// Allocate memory for layer surfaces and palette
	memoryPool = new(std::nothrow) UINT8[MEMORY_POOL_SIZE];
	if (NULL == memoryPool)
		return ErrorLog("Insufficient memory for tile layer surfaces (need %1.1f MB).", memSizeMB);	
	memset(memoryPool,0,MEMORY_POOL_SIZE);
	
	// Set up pointers to memory regions
	surf = (UINT32 *) memoryPool;
	pal = (UINT32 *) &memoryPool[512*512*4];
	
	// Resolution
	xPixels = xRes;
	yPixels = yRes;
	xOffs = xOffset;
	yOffs = yOffset;
	
	// Clear textures and dirty rectangles (all memory)
	memset(memoryPool, 0, MEMORY_POOL_SIZE);
	memset(dirty, 0, sizeof(dirty));
	allDirty = true;
	
	// Create textures
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(2, texID);
    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, texID[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf);
        if (glGetError() != GL_NO_ERROR)
        	return ErrorLog("OpenGL was unable to provide 512x512-texel texture maps for tile map layers.");
    }

	DebugLog("Render2D initialized (allocated %1.1f MB)\n", memSizeMB);
	return OKAY;
}

CRender2D::CRender2D(void)
{
	xPixels = 496;
	yPixels = 384;
	xOffs = 0;
	yOffs = 0;
	
	memoryPool = NULL;
	vram = NULL;
	surf = NULL;
	
	DebugLog("Built Render2D\n");
}

CRender2D::~CRender2D(void)
{
	DestroyShaderProgram(shaderProgram,vertexShader,fragmentShader);
	glDeleteTextures(2, texID);
	
	if (memoryPool != NULL)
	{
		delete [] memoryPool;
		memoryPool = NULL;
	}
	
	surf = NULL;
	vram = NULL;

	DebugLog("Destroyed Render2D\n");
}
