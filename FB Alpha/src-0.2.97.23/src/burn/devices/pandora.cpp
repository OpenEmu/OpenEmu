// Kaneko Pandora module

#include "tiles_generic.h"

static UINT16 *pandora_temp	= NULL;
static UINT8 *pandora_ram	= NULL;
static UINT8 *pandora_gfx	= NULL;
static INT32 pandora_clear;
static INT32 pandora_xoffset;
static INT32 pandora_yoffset;
static INT32 pandora_color_offset;
INT32 pandora_flipscreen;

void pandora_set_clear(INT32 clear)
{
#if defined FBA_DEBUG
	if (!DebugDev_PandoraInitted) bprintf(PRINT_ERROR, _T("pandora_set_clear called without init\n"));
#endif

	pandora_clear = clear;
}

void pandora_update(UINT16 *dest)
{
#if defined FBA_DEBUG
	if (!DebugDev_PandoraInitted) bprintf(PRINT_ERROR, _T("pandora_update called without init\n"));
#endif

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		if (pandora_temp[i]) {
			dest[i] = pandora_temp[i] & 0x3ff;
		}
	}
}

void pandora_buffer_sprites()
{
#if defined FBA_DEBUG
	if (!DebugDev_PandoraInitted) bprintf(PRINT_ERROR, _T("pandora_buffer_sprites called without init\n"));
#endif

	INT32 sx=0, sy=0, x=0, y=0;

	if (pandora_clear) memset (pandora_temp, 0, nScreenWidth * nScreenHeight * sizeof(UINT16));

	for (INT32 offs = 0; offs < 0x1000; offs += 8)
	{
		INT32 attr	= pandora_ram[offs+7];
		INT32 code	= pandora_ram[offs+6] + ((attr & 0x3f) << 8);
		INT32 dy		= pandora_ram[offs+5];
		INT32 dx		= pandora_ram[offs+4];
		INT32 color	= pandora_ram[offs+3];
		INT32 flipy	= attr & 0x40;
		INT32 flipx	= attr & 0x80;

		if (color & 1) dx |= 0x100;
		if (color & 2) dy |= 0x100;

		if (color & 4)
		{
			x += dx;
			y += dy;
		}
		else
		{
			x = dx;
			y = dy;
		}

		if (pandora_flipscreen)
		{
			sx = 240 - x;
			sy = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sx = x;
			sy = y;
		}

		sx = (sx + pandora_xoffset) & 0x1ff;
		sy = (sy + pandora_yoffset) & 0x1ff;
		if (sx & 0x100) sx -= 0x200;
		if (sy & 0x100) sy -= 0x200;

		if (sx >= nScreenWidth  || sx < -15) continue;
		if (sy >= nScreenHeight || sy < -15) continue;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pandora_temp, code, sx, sy, color >> 4, 4, 0, pandora_color_offset, pandora_gfx);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pandora_temp, code, sx, sy, color >> 4, 4, 0, pandora_color_offset, pandora_gfx);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pandora_temp, code, sx, sy, color >> 4, 4, 0, pandora_color_offset, pandora_gfx);
			} else {
				Render16x16Tile_Mask_Clip(pandora_temp, code, sx, sy, color >> 4, 4, 0, pandora_color_offset, pandora_gfx);
			}
		}
	}
}

// must be called after GenericTilesInit()
void pandora_init(UINT8 *ram, UINT8 *gfx, INT32 color_offset, INT32 x, INT32 y)
{
	DebugDev_PandoraInitted = 1;
	
	pandora_ram	= ram;
	pandora_xoffset	= x;
	pandora_yoffset	= y;
	pandora_gfx	= gfx;
	pandora_color_offset	= color_offset;

	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nScreenHeight, &nScreenWidth);
	} else {
		BurnDrvGetVisibleSize(&nScreenWidth, &nScreenHeight);
	}

	pandora_temp = (UINT16*)malloc(nScreenWidth * nScreenHeight * sizeof(UINT16));
	pandora_clear = 1;
}

void pandora_exit()
{
#if defined FBA_DEBUG
	if (!DebugDev_PandoraInitted) bprintf(PRINT_ERROR, _T("pandora_exit called without init\n"));
#endif

	if (pandora_temp) {
		free (pandora_temp);
		pandora_temp = NULL;
	}

	pandora_ram = pandora_gfx = NULL;
	
	DebugDev_PandoraInitted = 0;
}
