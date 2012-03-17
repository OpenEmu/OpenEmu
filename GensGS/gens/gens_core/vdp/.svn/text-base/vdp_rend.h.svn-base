#ifndef GENS_VDP_REND_H
#define GENS_VDP_REND_H

#ifdef __cplusplus
extern "C" {
#endif

// 15/16-bit color
extern unsigned short MD_Screen[336 * 240];
extern unsigned short Palette[0x1000];
extern unsigned short MD_Palette[256];

// 32-bit color
extern unsigned int MD_Screen32[336 * 240];
extern unsigned int Palette32[0x1000];
extern unsigned int MD_Palette32[256];

// _32X_Rend_Mode is used for the 32X 32-bit color C functions.
// See g_32x_32bit.h
extern unsigned char _32X_Rend_Mode;

extern unsigned long TAB336[336];

extern struct
{
	int Pos_X;
	int Pos_Y;
	unsigned int Size_X;
	unsigned int Size_Y;
	int Pos_X_Max;
	int Pos_Y_Max;
	unsigned int Num_Tile;
	int dirt;
} Sprite_Struct[256];

extern int Sprite_Over;

void Render_Line();
void Render_Line_32X();

// VDP layer control
extern unsigned int VDP_Layers;

// VDP layer flags
#define VDP_LAYER_SCROLLA_LOW		(1 << 0)
#define VDP_LAYER_SCROLLA_HIGH		(1 << 1)
#define VDP_LAYER_SCROLLA_SWAP		(1 << 2)
#define VDP_LAYER_SCROLLB_LOW		(1 << 3)
#define VDP_LAYER_SCROLLB_HIGH		(1 << 4)
#define VDP_LAYER_SCROLLB_SWAP		(1 << 5)
#define VDP_LAYER_SPRITE_LOW		(1 << 6)
#define VDP_LAYER_SPRITE_HIGH		(1 << 7)
#define VDP_LAYER_SPRITE_SWAP		(1 << 8)
#define VDP_LAYER_SPRITE_ALWAYSONTOP	(1 << 9)
#define	VDP_LAYER_PALETTE_LOCK		(1 << 10)

#define VDP_LAYER_DEFAULT	  \
	(VDP_LAYER_SCROLLA_LOW	| \
	 VDP_LAYER_SCROLLA_HIGH	| \
	 VDP_LAYER_SCROLLB_LOW	| \
	 VDP_LAYER_SCROLLB_HIGH	| \
	 VDP_LAYER_SPRITE_LOW	| \
	 VDP_LAYER_SPRITE_HIGH)

#ifdef __cplusplus
}
#endif

#endif
