#ifndef GFX_H
#define GFX_H

#include "GBA.h"
#include "Globals.h"

#include "../common/Port.h"

static INLINE void gfxDrawTextScreen(u16 control, u16 hofs, u16 vofs, u32 *line)
{
	u16 *palette = (u16 *)graphics.paletteRAM;
	u8 *charBase = &vram[((control >> 2) & 0x03) << 14];
	u16 *screenBase = (u16 *)&vram[((control >> 8) & 0x1f) << 11];
	u32 prio = ((control & 3)<<25) + 0x1000000;
	int sizeX = 256;
	int sizeY = 256;

	int tileXOdd = 0;
	switch((control >> 14) & 3)
	{
		case 0:
			break;
		case 1:
			sizeX = 512;
			break;
		case 2:
			sizeY = 512;
			break;
		case 3:
			sizeX = 512;
			sizeY = 512;
			break;
	}

	int maskX = sizeX-1;
	int maskY = sizeY-1;

	int xxx = hofs & maskX;
	int yyy = (vofs + VCOUNT) & maskY;
	int mosaicX = (MOSAIC & 0x000F)+1;
	int mosaicY = ((MOSAIC & 0x00F0)>>4)+1;

	bool mosaicOn = (control & 0x40) ? true : false;

	if(mosaicOn)
	{
		if((VCOUNT % mosaicY) != 0)
		{
			mosaicY = VCOUNT - (VCOUNT % mosaicY);
			yyy = (vofs + mosaicY) & maskY;
		}
	}

	if(yyy > 255 && sizeY > 256)
	{
		yyy &= 255;
		screenBase += 0x400;
		if(sizeX > 256)
			screenBase += 0x400;
	}

	int yshift = ((yyy>>3)<<5);
	u16 *screenSource = screenBase + ((xxx>>8) << 10) + ((xxx & 255)>>3) + yshift;
	if((control) & 0x80)
	{
		for(u32 x = 0; x < 240u; x++)
		{
			u16 data = READ16LE(screenSource);

			int tile = data & 0x3FF;
			int tileX = (xxx & 7);
			int tileY = yyy & 7;

			if(tileX == 7)
				++screenSource;

			if(data & 0x0400)
				tileX = 7 - tileX;
			if(data & 0x0800)
				tileY = 7 - tileY;

			u8 color = charBase[(tile<<6)  + (tileY<<3) + tileX];

			line[x] = color ? (READ16LE(&palette[color]) | prio): 0x80000000;

			if(++xxx == 256)
			{
				screenSource = screenBase + yshift;
				if(sizeX > 256)
					screenSource += 0x400;
				else
					xxx = 0;
			}
			else if(xxx >= sizeX)
			{
				xxx = 0;
				screenSource = screenBase + yshift;
			}
		}
	}
	else
	{ 
		for(u32 x = 0; x < 240u; ++x)
		{
			u16 data = READ16LE(screenSource);

			int tile = data & 0x3FF;
			int tileX = (xxx & 7);
			int tileY = yyy & 7;

			if(tileX == 7)
				++screenSource;

			if(data & 0x0400)
				tileX = 7 - tileX;
			if(data & 0x0800)
				tileY = 7 - tileY;

			u8 color = charBase[(tile<<5) + (tileY<<2) + (tileX>>1)];

			tileXOdd = (tileX & 1) - 1; 

#ifdef BRANCHLESS_GBA_GFX
			color = isel(tileXOdd, color >> 4, color & 0x0F);
#else
			(tileX & 1) ? color >>= 4 : color &= 0x0F;
#endif

			int pal = (data>>8) & 0xF0;
			line[x] = color ? (READ16LE(&palette[pal + color])|prio): 0x80000000;

			if(++xxx == 256)
			{
				screenSource = screenBase + yshift;
				if(sizeX > 256)
					screenSource += 0x400;
				else
					xxx = 0;
			}
			else if(xxx >= sizeX)
			{
				xxx = 0;
				screenSource = screenBase + yshift;
			}
		}
	}
	if(mosaicOn && (mosaicX > 1))
	{
		int m = 1;
		for(u32 i = 0; i < 239u; ++i)
		{
			line[i+1] = line[i];
			++m;
			if(m == mosaicX)
			{
				m = 1;
				++i;
			}
		}
	}
}

static INLINE void gfxDrawRotScreen(u16 control, u16 x_l, u16 x_h, u16 y_l, u16 y_h,
u16 pa,  u16 pb, u16 pc,  u16 pd, int& currentX, int& currentY, int changed, u32 *line)
{
	u16 *palette = (u16 *)graphics.paletteRAM;
	u8 *charBase = &vram[((control >> 2) & 0x03) << 14];
	u8 *screenBase = (u8 *)&vram[((control >> 8) & 0x1f) << 11];
	int prio = ((control & 3) << 25) + 0x1000000;

	u32 sizeX = 128;
	u32 sizeY = 128;
	switch((control >> 14) & 3)
	{
		case 0:
			break;
		case 1:
			sizeX = sizeY = 256;
			break;
		case 2:
			sizeX = sizeY = 512;
			break;
		case 3:
			sizeX = sizeY = 1024;
			break;
	}

	int maskX = sizeX-1;
	int maskY = sizeY-1;

	int yshift = ((control >> 14) & 3)+4;

#ifdef BRANCHLESS_GBA_GFX
	int dx = pa & 0x7FFF;
	int dmx = pb & 0x7FFF;
	int dy = pc & 0x7FFF;
	int dmy = pd & 0x7FFF;

	dx |= isel(-(pa & 0x8000), 0, 0xFFFF8000);

	dmx |= isel(-(pb & 0x8000), 0, 0xFFFF8000);

	dy |= isel(-(pc & 0x8000), 0, 0xFFFF8000);

	dmy |= isel(-(pd & 0x8000), 0, 0xFFFF8000);
#else
	int dx = pa & 0x7FFF;
	if(pa & 0x8000)
		dx |= 0xFFFF8000;
	int dmx = pb & 0x7FFF;
	if(pb & 0x8000)
		dmx |= 0xFFFF8000;
	int dy = pc & 0x7FFF;
	if(pc & 0x8000)
		dy |= 0xFFFF8000;
	int dmy = pd & 0x7FFF;
	if(pd & 0x8000)
		dmy |= 0xFFFF8000;
#endif

	if(VCOUNT == 0)
		changed = 3;

	currentX += dmx;
	currentY += dmy;

	if(changed & 1)
	{
		currentX = (x_l) | ((x_h & 0x07FF)<<16);
		if(x_h & 0x0800)
			currentX |= 0xF8000000;
	}

	if(changed & 2)
	{
		currentY = (y_l) | ((y_h & 0x07FF)<<16);
		if(y_h & 0x0800)
			currentY |= 0xF8000000;
	}

	int realX = currentX;
	int realY = currentY;

	if(control & 0x40)
	{
		int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
		int y = (VCOUNT % mosaicY);
		realX -= y*dmx;
		realY -= y*dmy;
	}

	memset(line, -1, 240 * sizeof(u32));
	if(control & 0x2000)
	{
		for(u32 x = 0; x < 240u; ++x)
		{
			int xxx = (realX >> 8) & maskX;
			int yyy = (realY >> 8) & maskY;

			int tile = screenBase[(xxx>>3) + ((yyy>>3)<<yshift)];

			int tileX = (xxx & 7);
			int tileY = yyy & 7;

			u8 color = charBase[(tile<<6) + (tileY<<3) + tileX];

			if(color)
				line[x] = (READ16LE(&palette[color])|prio);

			realX += dx;
			realY += dy;
		}
	}
	else
	{
		for(u32 x = 0; x < 240u; ++x)
		{
			unsigned xxx = (realX >> 8);
			unsigned yyy = (realY >> 8);

			if(xxx < sizeX && yyy < sizeY)
			{
				int tile = screenBase[(xxx>>3) + ((yyy>>3)<<yshift)];

				int tileX = (xxx & 7);
				int tileY = yyy & 7;

				u8 color = charBase[(tile<<6) + (tileY<<3) + tileX];

				if(color)
					line[x] = (READ16LE(&palette[color])|prio);
			}

			realX += dx;
			realY += dy;
		}
	}

	if(control & 0x40)
	{
		int mosaicX = (MOSAIC & 0xF) + 1;
		if(mosaicX > 1)
		{
			int m = 1;
			for(u32 i = 0; i < 239u; ++i)
			{
				line[i+1] = line[i];
				if(++m == mosaicX)
				{
					m = 1;
					++i;
				}
			}
		}
	}
}

static INLINE void gfxDrawRotScreen16Bit(u16 control, u16 x_l, u16 x_h, 
u16 y_l, u16 y_h, u16 pa,  u16 pb, u16 pc,  u16 pd, int& currentX, 
int& currentY, int changed, u32 *line)
{
	u16 *screenBase = (u16 *)&vram[0];
	int prio = ((control & 3) << 25) + 0x1000000;

	u32 sizeX = 240;
	u32 sizeY = 160;

	int startX = (x_l) | ((x_h & 0x07FF)<<16);
	if(x_h & 0x0800)
		startX |= 0xF8000000;
	int startY = (y_l) | ((y_h & 0x07FF)<<16);
	if(y_h & 0x0800)
		startY |= 0xF8000000;

#ifdef BRANCHLESS_GBA_GFX
	int dx = pa & 0x7FFF;
	dx |= isel(-(pa & 0x8000), 0, 0xFFFF8000);

	int dmx = pb & 0x7FFF;
	dmx |= isel(-(pb & 0x8000), 0, 0xFFFF8000);

	int dy = pc & 0x7FFF;
	dy |= isel(-(pc & 0x8000), 0, 0xFFFF8000);

	int dmy = pd & 0x7FFF;
	dmy |= isel(-(pd & 0x8000), 0, 0xFFFF8000);
#else
	int dx = pa & 0x7FFF;
	if(pa & 0x8000)
		dx |= 0xFFFF8000;
	int dmx = pb & 0x7FFF;
	if(pb & 0x8000)
		dmx |= 0xFFFF8000;
	int dy = pc & 0x7FFF;
	if(pc & 0x8000)
		dy |= 0xFFFF8000;
	int dmy = pd & 0x7FFF;
	if(pd & 0x8000)
		dmy |= 0xFFFF8000;
#endif

	if(VCOUNT == 0)
		changed = 3;

	currentX += dmx;
	currentY += dmy;

	if(changed & 1)
	{
		currentX = (x_l) | ((x_h & 0x07FF)<<16);
		if(x_h & 0x0800)
			currentX |= 0xF8000000;
	}

	if(changed & 2)
	{
		currentY = (y_l) | ((y_h & 0x07FF)<<16);
		if(y_h & 0x0800)
			currentY |= 0xF8000000;
	}

	int realX = currentX;
	int realY = currentY;

	if(control & 0x40) {
		int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
		int y = (VCOUNT % mosaicY);
		realX -= y*dmx;
		realY -= y*dmy;
	}

	unsigned xxx = (realX >> 8);
	unsigned yyy = (realY >> 8);

	memset(line, -1, 240 * sizeof(u32));
	for(u32 x = 0; x < 240u; ++x)
	{
		if(xxx < sizeX && yyy < sizeY)
			line[x] = (READ16LE(&screenBase[yyy * sizeX + xxx]) | prio);

		realX += dx;
		realY += dy;

		xxx = (realX >> 8);
		yyy = (realY >> 8);
	}

	if(control & 0x40) {
		int mosaicX = (MOSAIC & 0xF) + 1;
		if(mosaicX > 1) {
			int m = 1;
			for(u32 i = 0; i < 239u; ++i)
			{
				line[i+1] = line[i];
				if(++m == mosaicX)
				{
					m = 1;
					++i;
				}
			}
		}
	}
}

static INLINE void gfxDrawRotScreen256(u16 control,
				       u16 x_l, u16 x_h,
				       u16 y_l, u16 y_h,
				       u16 pa,  u16 pb,
				       u16 pc,  u16 pd,
				       int &currentX, int& currentY,
				       int changed,
				       u32 *line)
{
  u16 *palette = (u16 *)graphics.paletteRAM;
  u8 *screenBase = (graphics.DISPCNT & 0x0010) ? &vram[0xA000] : &vram[0x0000];
  int prio = ((control & 3) << 25) + 0x1000000;
  u32 sizeX = 240;
  u32 sizeY = 160;

  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800)
    startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800)
    startY |= 0xF8000000;

#ifdef BRANCHLESS_GBA_GFX
  int dx = pa & 0x7FFF;
  dx |= isel(-(pa & 0x8000), 0, 0xFFFF8000);

  int dmx = pb & 0x7FFF;
  dmx |= isel(-(pb & 0x8000), 0, 0xFFFF8000);

  int dy = pc & 0x7FFF;
  dy |= isel(-(pc & 0x8000), 0, 0xFFFF8000);

  int dmy = pd & 0x7FFF;
  dmy |= isel(-(pd & 0x8000), 0, 0xFFFF8000);
#else
  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;
#endif

  if(VCOUNT == 0)
    changed = 3;

  currentX += dmx;
  currentY += dmy;

  if(changed & 1)
  {
    currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      currentX |= 0xF8000000;
  }

  if(changed & 2)
  {
    currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      currentY |= 0xF8000000;
  }

  int realX = currentX;
  int realY = currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = VCOUNT - (VCOUNT % mosaicY);
    realX = startX + y*dmx;
    realY = startY + y*dmy;
  }

  int xxx = (realX >> 8);
  int yyy = (realY >> 8);

  memset(line, -1, 240 * sizeof(u32));
  for(u32 x = 0; x < 240; ++x)
  {
	  u8 color = screenBase[yyy * 240 + xxx];
	  if(unsigned(xxx) < sizeX && unsigned(yyy) < sizeY && color)
		  line[x] = (READ16LE(&palette[color])|prio);
	  realX += dx;
	  realY += dy;

	  xxx = (realX >> 8);
	  yyy = (realY >> 8);
  }

  if(control & 0x40)
  {
	  int mosaicX = (MOSAIC & 0xF) + 1;
	  if(mosaicX > 1)
	  {
		  int m = 1;
		  for(u32 i = 0; i < 239u; ++i)
		  {
			  line[i+1] = line[i];
			  if(++m == mosaicX)
			  {
				  m = 1;
				  ++i;
			  }
		  }
	  }
  }
}

static INLINE void gfxDrawRotScreen16Bit160(u16 control,
					    u16 x_l, u16 x_h,
					    u16 y_l, u16 y_h,
					    u16 pa,  u16 pb,
					    u16 pc,  u16 pd,
					    int& currentX, int& currentY,
					    int changed,
					    u32 *line)
{
  u16 *screenBase = (graphics.DISPCNT & 0x0010) ? (u16 *)&vram[0xa000] :
    (u16 *)&vram[0];
  int prio = ((control & 3) << 25) + 0x1000000;
  u32 sizeX = 160;
  u32 sizeY = 128;

  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800)
    startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800)
    startY |= 0xF8000000;

#ifdef BRANCHLESS_GBA_GFX
  int dx = pa & 0x7FFF;
  dx |= isel(-(pa & 0x8000), 0, 0xFFFF8000);

  int dmx = pb & 0x7FFF;
  dmx |= isel(-(pb & 0x8000), 0, 0xFFFF8000);

  int dy = pc & 0x7FFF;
  dy |= isel(-(pc & 0x8000), 0, 0xFFFF8000);

  int dmy = pd & 0x7FFF;
  dmy |= isel(-(pd & 0x8000), 0, 0xFFFF8000);
#else
  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;
#endif

  if(VCOUNT == 0)
    changed = 3;

  currentX += dmx;
  currentY += dmy;

  if(changed & 1)
  {
	  currentX = (x_l) | ((x_h & 0x07FF)<<16);
	  if(x_h & 0x0800)
		  currentX |= 0xF8000000;
  }

  if(changed & 2)
  {
	  currentY = (y_l) | ((y_h & 0x07FF)<<16);
	  if(y_h & 0x0800)
		  currentY |= 0xF8000000;
  }

  int realX = currentX;
  int realY = currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = VCOUNT - (VCOUNT % mosaicY);
    realX = startX + y*dmx;
    realY = startY + y*dmy;
  }

  int xxx = (realX >> 8);
  int yyy = (realY >> 8);

  memset(line, -1, 240 * sizeof(u32));
  for(u32 x = 0; x < 240u; ++x)
  {
	  if(unsigned(xxx) < sizeX && unsigned(yyy) < sizeY)
		  line[x] = (READ16LE(&screenBase[yyy * sizeX + xxx]) | prio);

	  realX += dx;
	  realY += dy;

	  xxx = (realX >> 8);
	  yyy = (realY >> 8);
  }


  int mosaicX = (MOSAIC & 0xF) + 1;
  if(control & 0x40 && (mosaicX > 1))
  {
	  int m = 1;
	  for(u32 i = 0; i < 239u; ++i)
	  {
		  line[i+1] = line[i];
		  if(++m == mosaicX)
		  {
			  m = 1;
			  ++i;
		  }
	  }
  }
}

/* lineOBJpix is used to keep track of the drawn OBJs
   and to stop drawing them if the 'maximum number of OBJ per line'
   has been reached. */

static INLINE void gfxDrawSprites (void)
{
	unsigned lineOBJpix, m;

	lineOBJpix = (graphics.DISPCNT & 0x20) ? 954 : 1226;
	m = 0;

	memset(line[4], -1, 240 * sizeof(u32));

	if(graphics.layerEnable & 0x1000)
	{
		u16 *sprites = (u16 *)oam;
		u16 *spritePalette = &((u16 *)graphics.paletteRAM)[256];
		int mosaicY = ((MOSAIC & 0xF000)>>12) + 1;
		int mosaicX = ((MOSAIC & 0xF00)>>8) + 1;
		for(u32 x = 0; x < 128; x++)
		{
			u16 a0 = READ16LE(sprites++);
			u16 a1 = READ16LE(sprites++);
			u16 a2 = READ16LE(sprites++);
			++sprites;

			lineOBJpixleft[x]=lineOBJpix;

			lineOBJpix-=2;
			if (lineOBJpix<=0)
				continue;

			if ((a0 & 0x0c00) == 0x0c00)
				a0 &=0xF3FF;

			u16 a0val = a0>>14;

			if (a0val == 3)
			{
				a0 &= 0x3FFF;
				a1 &= 0x3FFF;
			}

			u32 sizeX = 8<<(a1>>14);
			u32 sizeY = sizeX;


			if (a0val & 1)
			{
#ifdef BRANCHLESS_GBA_GFX
				sizeX <<= isel(-(sizeX & (~31u)), 1, 0);
				sizeY >>= isel(-(sizeY>8), 0, 1);
#else
				if (sizeX<32)
					sizeX<<=1;
				if (sizeY>8)
					sizeY>>=1;
#endif
			}
			else if (a0val & 2)
			{
#ifdef BRANCHLESS_GBA_GFX
				sizeX >>= isel(-(sizeX>8), 0, 1);
				sizeY <<= isel(-(sizeY & (~31u)), 1, 0);
#else
				if (sizeX>8)
					sizeX>>=1;
				if (sizeY<32)
					sizeY<<=1;
#endif

			}


			int sy = (a0 & 255);
			int sx = (a1 & 0x1FF);

			// computes ticks used by OBJ-WIN if OBJWIN is enabled
			if (((a0 & 0x0c00) == 0x0800) && (graphics.layerEnable & 0x8000))
			{
				if ((a0 & 0x0300) == 0x0300)
				{
					sizeX<<=1;
					sizeY<<=1;
				}

#ifdef BRANCHLESS_GBA_GFX
				sy -= isel(256 - sy - sizeY, 0, 256);
				sx -= isel(512 - sx - sizeX, 0, 512);
#else
				if((sy+sizeY) > 256)
					sy -= 256;
				if ((sx+sizeX)> 512)
					sx -= 512;
#endif

				if (sx < 0)
				{
					sizeX+=sx;
					sx = 0;
				}
				else if ((sx+sizeX)>240)
					sizeX=240-sx;

				if ((VCOUNT>=sy) && (VCOUNT<sy+sizeY) && (sx<240))
				{
					lineOBJpix -= (sizeX-2);

					if (a0 & 0x0100)
						lineOBJpix -= (10+sizeX); 
				}
				continue;
			}

			// else ignores OBJ-WIN if OBJWIN is disabled, and ignored disabled OBJ
			else if(((a0 & 0x0c00) == 0x0800) || ((a0 & 0x0300) == 0x0200) || lineOBJpix < 0)
				continue;

			if(a0 & 0x0100)
			{
				u32 fieldX = sizeX;
				u32 fieldY = sizeY;
				if(a0 & 0x0200)
				{
					fieldX <<= 1;
					fieldY <<= 1;
				}
				if((sy+fieldY) > 256)
					sy -= 256;
				int t = VCOUNT - sy;
				if(unsigned(t) < fieldY)
				{
					u32 startpix = 0;
					if ((sx+fieldX)> 512)
						startpix=512-sx;

					if (lineOBJpix && ((sx < 240) || startpix))
					{
						lineOBJpix-=8;
						int rot = (((a1 >> 9) & 0x1F) << 4);
						u16 *OAM = (u16 *)oam;
						int dx = READ16LE(&OAM[3 + rot]);
						if(dx & 0x8000)
							dx |= 0xFFFF8000;
						int dmx = READ16LE(&OAM[7 + rot]);
						if(dmx & 0x8000)
							dmx |= 0xFFFF8000;
						int dy = READ16LE(&OAM[11 + rot]);
						if(dy & 0x8000)
							dy |= 0xFFFF8000;
						int dmy = READ16LE(&OAM[15 + rot]);
						if(dmy & 0x8000)
							dmy |= 0xFFFF8000;

						if(a0 & 0x1000)
							t -= (t % mosaicY);

						int realX = ((sizeX) << 7) - (fieldX >> 1)*dx + ((t - (fieldY>>1))* dmx);
						int realY = ((sizeY) << 7) - (fieldX >> 1)*dy + ((t - (fieldY>>1))* dmy);

						u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);

						int c = (a2 & 0x3FF);
						if((graphics.DISPCNT & 7) > 2 && (c < 512))
							continue;

						if(a0 & 0x2000)
						{
							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 2;
							else
								c &= 0x3FE;
							for(u32 x = 0; x < fieldX; x++)
							{
								if (x >= startpix)
									lineOBJpix-=2;
								if (lineOBJpix<0)
									continue;
								unsigned xxx = realX >> 8;
								unsigned yyy = realY >> 8;
								if(xxx < sizeX && yyy < sizeY && sx < 240)
								{

									u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
												+ ((yyy & 7)<<3) + ((xxx >> 3)<<6) +
												(xxx & 7))&0x7FFF)];
									if ((color==0) && (((prio >> 25)&3) <
												((line[4][sx]>>25)&3))) {
										line[4][sx] = (line[4][sx] & 0xF9FFFFFF) | prio;
										if((a0 & 0x1000) && m)
											line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
									} else if((color) && (prio < (line[4][sx]&0xFF000000))) {
										line[4][sx] = READ16LE(&spritePalette[color]) | prio;
										if((a0 & 0x1000) && m)
											line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
									}

									if (a0 & 0x1000)
									{
										if (++m==mosaicX)
											m=0;
									}
								}
								sx = (sx+1)&511;
								realX += dx;
								realY += dy;
							}
						}
						else
						{
							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 3;
							int palette = (a2 >> 8) & 0xF0;
							for(u32 x = 0; x < fieldX; ++x)
							{
								if (x >= startpix)
									lineOBJpix-=2;
								if (lineOBJpix<0)
									continue;
								unsigned xxx = realX >> 8;
								unsigned yyy = realY >> 8;
								if(xxx < sizeX && yyy < sizeY && sx < 240)
								{

									u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
												+ ((yyy & 7)<<2) + ((xxx >> 3)<<5)
												+ ((xxx & 7)>>1))&0x7FFF)];
									if(xxx & 1)
										color >>= 4;
									else
										color &= 0x0F;

									if ((color==0) && (((prio >> 25)&3) <
									((line[4][sx]>>25)&3)))
									{
										line[4][sx] = (line[4][sx] & 0xF9FFFFFF) | prio;
										if((a0 & 0x1000) && m)
											line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
									}
									else if((color) && (prio < (line[4][sx]&0xFF000000)))
									{
										line[4][sx] = READ16LE(&spritePalette[palette+color]) | prio;
										if((a0 & 0x1000) && m)
											line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
									}
								}
								if((a0 & 0x1000) && m)
								{
									if (++m==mosaicX)
										m=0;
								}

								sx = (sx+1)&511;
								realX += dx;
								realY += dy;

							}
						}
					}
				}
			}
			else
			{
				if(sy+sizeY > 256)
					sy -= 256;
				int t = VCOUNT - sy;
				if(unsigned(t) < sizeY)
				{
					u32 startpix = 0;
					if ((sx+sizeX)> 512)
						startpix=512-sx;

					if((sx < 240) || startpix)
					{
						lineOBJpix+=2;

						if(a1 & 0x2000)
							t = sizeY - t - 1;

						int c = (a2 & 0x3FF);
						if((graphics.DISPCNT & 7) > 2 && (c < 512))
							continue;

						if(a0 & 0x2000)
						{
							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 2;
							else
								c &= 0x3FE;

							int xxx = 0;
							if(a1 & 0x1000)
								xxx = sizeX-1;

							if(a0 & 0x1000)
								t -= (t % mosaicY);

							int address = 0x10000 + ((((c+ (t>>3) * inc) << 5)
							+ ((t & 7) << 3) + ((xxx>>3)<<6) + (xxx & 7)) & 0x7FFF);

							if(a1 & 0x1000)
								xxx = 7;
							u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);

							for(u32 xx = 0; xx < sizeX; xx++)
							{
								if (xx >= startpix)
									--lineOBJpix;
								if (lineOBJpix<0)
									continue;
								if(sx < 240)
								{
									u8 color = vram[address];
									if ((color==0) && (((prio >> 25)&3) <
									((line[4][sx]>>25)&3)))
									{
										line[4][sx] = (line[4][sx] & 0xF9FFFFFF) | prio;
										if((a0 & 0x1000) && m)
											line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
									}
									else if((color) && (prio < (line[4][sx]&0xFF000000)))
									{
										line[4][sx] = READ16LE(&spritePalette[color]) | prio;
										if((a0 & 0x1000) && m)
											line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
									}

									if (a0 & 0x1000)
									{
										if (++m==mosaicX)
											m = 0;
									}

								}

								sx = (sx+1) & 511;
								if(a1 & 0x1000)
								{
									--address;
									if(--xxx == -1)
									{
										address -= 56;
										xxx = 7;
									}
									if(address < 0x10000)
										address += 0x8000;
								}
								else
								{
									++address;
									if(++xxx == 8)
									{
										address += 56;
										xxx = 0;
									}
									if(address > 0x17fff)
										address -= 0x8000;
								}
							}
						}
						else
						{
							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 3;

							int xxx = 0;
							if(a1 & 0x1000)
								xxx = sizeX - 1;

							if(a0 & 0x1000)
								t -= (t % mosaicY);

							int address = 0x10000 + ((((c + (t>>3) * inc)<<5)
							+ ((t & 7)<<2) + ((xxx>>3)<<5) + ((xxx & 7) >> 1))&0x7FFF);

							u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
							int palette = (a2 >> 8) & 0xF0;
							if(a1 & 0x1000)
							{
								xxx = 7;
								int xx = sizeX - 1;
								do
								{
									if (xx >= (int)(startpix))
										--lineOBJpix;
									//if (lineOBJpix<0)
									//  continue;
									if(sx < 240)
									{
										u8 color = vram[address];
										if(xx & 1)
											color >>= 4;
										else
											color &= 0x0F;

										if ((color==0) && (((prio >> 25)&3) <
										((line[4][sx]>>25)&3)))
										{
											line[4][sx] = (line[4][sx] & 0xF9FFFFFF) | prio;
											if((a0 & 0x1000) && m)
												line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
										}
										else if((color) && (prio < (line[4][sx]&0xFF000000)))
										{
											line[4][sx] = READ16LE(&spritePalette[palette + color]) | prio;
											if((a0 & 0x1000) && m)
												line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
										}
									}
									if (a0 & 0x1000)
									{
										if (++m==mosaicX)
											m=0;
									}
									sx = (sx+1) & 511;
									if(!(xx & 1))
										--address;
									if(--xxx == -1)
									{
										xxx = 7;
										address -= 28;
									}
									if(address < 0x10000)
										address += 0x8000;
								}while(--xx >= 0);
							}
							else
							{
								for(u32 xx = 0; xx < sizeX; ++xx)
								{
									if (xx >= startpix)
										--lineOBJpix;
									//if (lineOBJpix<0)
									//  continue;
									if(sx < 240)
									{
										u8 color = vram[address];
										if(xx & 1)
											color >>= 4;
										else
											color &= 0x0F;

										if ((color==0) && (((prio >> 25)&3) <
										((line[4][sx]>>25)&3)))
										{
											line[4][sx] = (line[4][sx] & 0xF9FFFFFF) | prio;
											if((a0 & 0x1000) && m)
												line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;
										}
										else if((color) && (prio < (line[4][sx]&0xFF000000)))
										{
											line[4][sx] = READ16LE(&spritePalette[palette + color]) | prio;
											if((a0 & 0x1000) && m)
												line[4][sx]=(line[4][sx-1] & 0xF9FFFFFF) | prio;

										}
									}
									if (a0 & 0x1000)
									{
										if (++m==mosaicX)
											m=0;
									}
									sx = (sx+1) & 511;
									if(xx & 1)
										++address;
									if(++xxx == 8)
									{
										address += 28;
										xxx = 0;
									}
									if(address > 0x17fff)
										address -= 0x8000;
								}
							}
						}
					}
				}
			}
		}
	}
}

static INLINE void gfxDrawOBJWin (void)
{
	memset(line[5], -1, 240 * sizeof(u32));

	if((graphics.layerEnable & 0x9000) == 0x9000)
	{
		u16 *sprites = (u16 *)oam;
		// u16 *spritePalette = &((u16 *)graphics.paletteRAM)[256];
		for(int x = 0; x < 128 ; x++)
		{
			int lineOBJpix = lineOBJpixleft[x];
			u16 a0 = READ16LE(sprites++);
			u16 a1 = READ16LE(sprites++);
			u16 a2 = READ16LE(sprites++);
			sprites++;

			if (lineOBJpix<=0)
				continue;

			// ignores non OBJ-WIN and disabled OBJ-WIN
			if(((a0 & 0x0c00) != 0x0800) || ((a0 & 0x0300) == 0x0200))
				continue;

			if ((a0 & 0x0c00) == 0x0c00)
				a0 &=0xF3FF;

			if ((a0>>14) == 3)
			{
				a0 &= 0x3FFF;
				a1 &= 0x3FFF;
			}

			int sizeX = 8<<(a1>>14);
			int sizeY = sizeX;

			if ((a0>>14) & 1)
			{
				if (sizeX<32)
					sizeX<<=1;
				if (sizeY>8)
					sizeY>>=1;
			}
			else if ((a0>>14) & 2)
			{
				if (sizeX>8)
					sizeX>>=1;
				if (sizeY<32)
					sizeY<<=1;
			}

			int sy = (a0 & 255);

			if(a0 & 0x0100)
			{
				int fieldX = sizeX;
				int fieldY = sizeY;
				if(a0 & 0x0200) {
					fieldX <<= 1;
					fieldY <<= 1;
				}
				if((sy+fieldY) > 256)
					sy -= 256;
				int t = VCOUNT - sy;
				if((t >= 0) && (t < fieldY)) {
					int sx = (a1 & 0x1FF);
					int startpix = 0;
					if ((sx+fieldX)> 512)
					{
						startpix=512-sx;
					}
					if((sx < 240) || startpix) {
						lineOBJpix-=8;
						// int t2 = t - (fieldY >> 1);
						int rot = (a1 >> 9) & 0x1F;
						u16 *OAM = (u16 *)oam;
						int dx = READ16LE(&OAM[3 + (rot << 4)]);
						if(dx & 0x8000)
							dx |= 0xFFFF8000;
						int dmx = READ16LE(&OAM[7 + (rot << 4)]);
						if(dmx & 0x8000)
							dmx |= 0xFFFF8000;
						int dy = READ16LE(&OAM[11 + (rot << 4)]);
						if(dy & 0x8000)
							dy |= 0xFFFF8000;
						int dmy = READ16LE(&OAM[15 + (rot << 4)]);
						if(dmy & 0x8000)
							dmy |= 0xFFFF8000;

						int realX = ((sizeX) << 7) - (fieldX >> 1)*dx - (fieldY>>1)*dmx
							+ t * dmx;
						int realY = ((sizeY) << 7) - (fieldX >> 1)*dy - (fieldY>>1)*dmy
							+ t * dmy;

						// u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);

						int c = (a2 & 0x3FF);
						if((graphics.DISPCNT & 7) > 2 && (c < 512))
							continue;

						if(a0 & 0x2000)
						{
							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 2;
							else
								c &= 0x3FE;
							for(int x = 0; x < fieldX; x++)
							{
								if (x >= startpix)
									lineOBJpix-=2;
								if (lineOBJpix<0)
									continue;
								int xxx = realX >> 8;
								int yyy = realY >> 8;

								if(xxx < 0 || xxx >= sizeX ||
										yyy < 0 || yyy >= sizeY ||
										sx >= 240) {
								}
								else
								{
									u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
												+ ((yyy & 7)<<3) + ((xxx >> 3)<<6) +
												(xxx & 7))&0x7fff)];
									if(color)
										line[5][sx] = 1;
								}
								sx = (sx+1)&511;
								realX += dx;
								realY += dy;
							}
						}
						else
						{

							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 3;
							// int palette = (a2 >> 8) & 0xF0;
							for(int x = 0; x < fieldX; x++) {
								if (x >= startpix)
									lineOBJpix-=2;
								if (lineOBJpix<0)
									continue;
								int xxx = realX >> 8;
								int yyy = realY >> 8;

								if(xxx < 0 || xxx >= sizeX ||
										yyy < 0 || yyy >= sizeY ||
										sx >= 240) {
								}
								else
								{
									u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
												+ ((yyy & 7)<<2) + ((xxx >> 3)<<5) +
												((xxx & 7)>>1))&0x7fff)];
									if(xxx & 1)
										color >>= 4;
									else
										color &= 0x0F;

									if(color)
										line[5][sx] = 1;
								}
								sx = (sx+1)&511;
								realX += dx;
								realY += dy;
							}
						}
					}
				}
			}
			else
			{
				if((sy+sizeY) > 256)
					sy -= 256;
				int t = VCOUNT - sy;
				if((t >= 0) && (t < sizeY))
				{
					int sx = (a1 & 0x1FF);
					int startpix = 0;
					if ((sx+sizeX)> 512)
						startpix=512-sx;

					if((sx < 240) || startpix)
					{
						lineOBJpix+=2;
						if(a1 & 0x2000)
							t = sizeY - t - 1;
						int c = (a2 & 0x3FF);
						if((graphics.DISPCNT & 7) > 2 && (c < 512))
							continue;
						if(a0 & 0x2000)
						{

							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 2;
							else
								c &= 0x3FE;

							int xxx = 0;
							if(a1 & 0x1000)
								xxx = sizeX-1;
							int address = 0x10000 + ((((c+ (t>>3) * inc) << 5)
										+ ((t & 7) << 3) + ((xxx>>3)<<6) + (xxx & 7))&0x7fff);
							if(a1 & 0x1000)
								xxx = 7;
							// u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
							for(int xx = 0; xx < sizeX; xx++) {
								if (xx >= startpix)
									lineOBJpix--;
								if (lineOBJpix<0)
									continue;
								if(sx < 240) {
									u8 color = vram[address];
									if(color) {
										line[5][sx] = 1;
									}
								}

								sx = (sx+1) & 511;
								if(a1 & 0x1000) {
									xxx--;
									address--;
									if(xxx == -1) {
										address -= 56;
										xxx = 7;
									}
									if(address < 0x10000)
										address += 0x8000;
								} else {
									xxx++;
									address++;
									if(xxx == 8) {
										address += 56;
										xxx = 0;
									}
									if(address > 0x17fff)
										address -= 0x8000;
								}
							}
						}
						else
						{
							int inc = 32;
							if(graphics.DISPCNT & 0x40)
								inc = sizeX >> 3;
							int xxx = 0;
							if(a1 & 0x1000)
								xxx = sizeX - 1;
							int address = 0x10000 + ((((c + (t>>3) * inc)<<5)
							+ ((t & 7)<<2) + ((xxx>>3)<<5) + ((xxx & 7) >> 1))&0x7fff);
							// u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
							// int palette = (a2 >> 8) & 0xF0;
							if(a1 & 0x1000)
							{
								xxx = 7;
								for(int xx = sizeX - 1; xx >= 0; xx--)
								{
									if (xx >= startpix)
										lineOBJpix--;
									if (lineOBJpix<0)
										continue;
									if(sx < 240)
									{
										u8 color = vram[address];
										if(xx & 1)
											color = (color >> 4);
										else
											color &= 0x0F;

										if(color)
											line[5][sx] = 1;
									}
									sx = (sx+1) & 511;
									xxx--;
									if(!(xx & 1))
										address--;
									if(xxx == -1) {
										xxx = 7;
										address -= 28;
									}
									if(address < 0x10000)
										address += 0x8000;
								}
							}
							else
							{
								for(int xx = 0; xx < sizeX; xx++)
								{
									if (xx >= startpix)
										lineOBJpix--;
									if (lineOBJpix<0)
										continue;
									if(sx < 240)
									{
										u8 color = vram[address];
										if(xx & 1)
											color = (color >> 4);
										else
											color &= 0x0F;

										if(color)
											line[5][sx] = 1;
									}
									sx = (sx+1) & 511;
									xxx++;
									if(xx & 1)
										address++;
									if(xxx == 8) {
										address += 28;
										xxx = 0;
									}
									if(address > 0x17fff)
										address -= 0x8000;
								}
							}
						}
					}
				}
			}
		}
	}
}

static INLINE u32 gfxIncreaseBrightness(u32 color, int coeff)
{
	color &= 0xffff;
	color = ((color << 16) | color) & 0x3E07C1F;

	color += (((0x3E07C1F - color) * coeff) >> 4);
	color &= 0x3E07C1F;

	return (color >> 16) | color;
}

static INLINE u32 gfxDecreaseBrightness(u32 color, int coeff)
{
	color &= 0xffff;
	color = ((color << 16) | color) & 0x3E07C1F;

	color -= (((color * coeff) >> 4) & 0x3E07C1F);

	return (color >> 16) | color;
}

static INLINE u32 gfxAlphaBlend(u32 color, u32 color2, int ca, int cb)
{
	if(color < 0x80000000) {
		color&=0xffff;
		color2&=0xffff;

		color = ((color << 16) | color) & 0x03E07C1F;
		color2 = ((color2 << 16) | color2) & 0x03E07C1F;
		color = ((color * ca) + (color2 * cb)) >> 4;

		if ((ca + cb)>16)
		{
			if (color & 0x20)
				color |= 0x1f;
			if (color & 0x8000)
				color |= 0x7C00;
			if (color & 0x4000000)
				color |= 0x03E00000;
		}

		color &= 0x03E07C1F;
		color = (color >> 16) | color;
	}
	return color;
}

#endif // GFX_H
