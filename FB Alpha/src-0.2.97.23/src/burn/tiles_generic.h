#include "burnint.h"

extern UINT8* pTileData;
extern INT32 nScreenWidth, nScreenHeight;

INT32 GenericTilesInit();
INT32 GenericTilesExit();

// Tile decoding macros
#define RGN_FRAC(length, numerator, denominator) ((((length) * 8) * (numerator)) / (denominator))

#define STEP2(start, step)	((start) + ((step)*0)), ((start) + ((step)*1))
#define STEP4(start, step)	STEP2(start, step),  STEP2((start)+((step)*2), step)
#define STEP8(start, step)	STEP4(start, step),  STEP4((start)+((step)*4), step)
#define STEP16(start, step)	STEP8(start, step),  STEP8((start)+((step)*8), step)
#define STEP32(start, step)	STEP16(start, step), STEP16((start)+((step)*16), step)
#define STEP64(start, step)	STEP32(start, step), STEP32((start)+((step)*32), step)

void GfxDecode(INT32 num, INT32 numPlanes, INT32 xSize, INT32 ySize, INT32 planeoffsets[], INT32 xoffsets[], INT32 yoffsets[], INT32 modulo, UINT8 *pSrc, UINT8 *pDest);
void GfxDecodeSingle(INT32 which, INT32 numPlanes, INT32 xSize, INT32 ySize, INT32 planeoffsets[], INT32 xoffsets[], INT32 yoffsets[], INT32 modulo, UINT8 *pSrc, UINT8 *pDest);

void RenderZoomedTile(UINT16 *pDestDraw, UINT8 *gfx, INT32 code, INT32 color, INT32 trans_color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 width, INT32 height, INT32 zoomx, INT32 zoomy);

void RenderTileTranstab(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 trans_col, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 width, INT32 height, UINT8 *tab);

void Render8x8Tile(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_FlipX_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_FlipY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_FlipXY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);

void Render8x8Tile_Mask(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Mask_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Mask_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Mask_FlipX_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Mask_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Mask_FlipY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Mask_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render8x8Tile_Mask_FlipXY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);

void Render16x16Tile(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_FlipX_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_FlipY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_FlipXY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);

void Render16x16Tile_Mask(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Mask_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Mask_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Mask_FlipX_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Mask_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Mask_FlipY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Mask_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render16x16Tile_Mask_FlipXY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);

void Render32x32Tile(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_FlipX_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_FlipY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_FlipXY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);

void Render32x32Tile_Mask(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Mask_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Mask_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Mask_FlipX_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Mask_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Mask_FlipY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Mask_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void Render32x32Tile_Mask_FlipXY_Clip(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);

void RenderCustomTile(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_FlipX(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_FlipX_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_FlipY(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_FlipY_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_FlipXY(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_FlipXY_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile);

void RenderCustomTile_Mask(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Mask_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Mask_FlipX(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Mask_FlipX_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Mask_FlipY(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Mask_FlipY_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Mask_FlipXY(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
void RenderCustomTile_Mask_FlipXY_Clip(UINT16* pDestDraw, INT32 nWidth, INT32 nHeight, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile);
