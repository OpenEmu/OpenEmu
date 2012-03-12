#include "cps.h"

// CPS Tiles
UINT32 *CpstPal=NULL;

// Arguments for the tile draw function
UINT32 nCpstType = 0;
INT32 nCpstX = 0, nCpstY = 0;
UINT32 nCpstTile = 0;
INT32 nCpstFlip = 0;
INT16 *CpstRowShift = NULL;
UINT32 CpstPmsk = 0;		// Pixel mask

INT32 nBgHi = 0;
UINT16  ZValue = 1;
UINT16* ZBuf = NULL;
UINT16* pZVal = NULL;

static INT32 CpstOne();
static INT32 Cps2tOne();
static INT32 CpstOneBgHi();
static INT32 CpstOneObjZ();
CpstOneDoFn CpstOneDoX[3]    = { CpstOne, CpstOneBgHi, Cps2tOne};
CpstOneDoFn CpstOneObjDoX[2] = { CpstOne, CpstOneObjZ};

static INT32 CpstOne()
{
  INT32 nFun; INT32 nSize;
  nSize=(nCpstType&24)+8;

  if (nCpstType&CTT_CARE)
  {
    if ((nCpstType&CTT_ROWS)==0)
    {
      // Return if not visible at all
      if (nCpstX <= -nSize) return 0;
      if (nCpstX >= 384) return 0;
      if (nCpstY <= -nSize) return 0;
      if (nCpstY >= 224) return 0;
    }
    nCtvRollX=0x4000017f + nCpstX * 0x7fff;
    nCtvRollY=0x400000df + nCpstY * 0x7fff;
  }

  // Clip to loaded graphics data (we have a gap of 0x200 at the end)
  nCpstTile&=nCpsGfxMask; if (nCpstTile>=nCpsGfxLen) return 1;
  pCtvTile=CpsGfx+nCpstTile;

  // Find pLine (pointer to first pixel)
  pCtvLine=pBurnDraw + nCpstY*nBurnPitch + nCpstX*nBurnBpp;

  if (nSize==32) nCtvTileAdd=16; else nCtvTileAdd=8;

  if (nCpstFlip&2)
  {
    // Flip vertically
         if (nSize==16) { nCtvTileAdd= -8; pCtvTile+=15* 8; }
    else if (nSize==32) { nCtvTileAdd=-16; pCtvTile+=31*16; }
    else                { nCtvTileAdd= -8; pCtvTile+= 7* 8; }
  }

  nFun =nCpstType&0x1e;
  nFun|=nCpstFlip&1;
  return CtvDoX[nFun]();
}

static INT32 CpstOneBgHi()
{
  INT32 nFun; INT32 nSize;
  nSize=(nCpstType&24)+8;

  if (nCpstType&CTT_CARE)
  {
    if ((nCpstType&CTT_ROWS)==0)
    {
      // Return if not visible at all
      if (nCpstX<=-nSize) return 0;
      if (nCpstX>=384)   return 0;
      if (nCpstY<=-nSize) return 0;
      if (nCpstY>=224)   return 0;
    }
    nCtvRollX=0x4000017f + nCpstX * 0x7fff;
    nCtvRollY=0x400000df + nCpstY * 0x7fff;
  }

  // Clip to loaded graphics data (we have a gap of 0x200 at the end)
  nCpstTile&=nCpsGfxMask;
  if (nCpstTile>=nCpsGfxLen) return 1;
  pCtvTile=CpsGfx+nCpstTile;

  // Find pLine (pointer to first pixel)
  pCtvLine=pBurnDraw + nCpstY*nBurnPitch + nCpstX*nBurnBpp;

  if (nSize==32) nCtvTileAdd=16; else nCtvTileAdd=8;

  if (nCpstFlip&2)
  {
    // Flip vertically
         if (nSize==16) { nCtvTileAdd= -8; pCtvTile+=15* 8; }
    else if (nSize==32) { nCtvTileAdd=-16; pCtvTile+=31*16; }
    else                { nCtvTileAdd= -8; pCtvTile+= 7* 8; }
  }

  nFun =nCpstType&0x1e;
  nFun|=nCpstFlip&1;
  return CtvDoXB[nFun]();
}

static INT32 Cps2tOne()
{
  INT32 nFun; INT32 nSize;
  nSize=(nCpstType&24)+8;

  if (nCpstType&CTT_CARE)
  {
    if ((nCpstType&CTT_ROWS)==0)
    {
     // Return if not visible at all
     if (nCpstX <= -nSize) return 0;
     if (nCpstX >= 384)    return 0;
     if (nCpstY <= -nStartline - nSize) return 0;
     if (nCpstY >= nEndline)    return 0;
    }
    nCtvRollX=0x4000017f + nCpstX * 0x7fff;
    nCtvRollY=0x40000000 + nEndline - nStartline - 1 + (nCpstY - nStartline) * 0x7fff;
  }

  // Clip to loaded graphics data (we have a gap of 0x200 at the end)
  nCpstTile&=nCpsGfxMask; if (nCpstTile>=nCpsGfxLen) return 0;
  pCtvTile=CpsGfx+nCpstTile;

  // Find pLine (pointer to first pixel)
  pCtvLine=pBurnDraw + nCpstY*nBurnPitch + nCpstX*nBurnBpp;

  if (nSize==32) nCtvTileAdd=16; else nCtvTileAdd=8;

  if (nCpstFlip&2)
  {
    // Flip vertically
         if (nSize==16) { nCtvTileAdd= -8; pCtvTile+=15* 8; }
    else if (nSize==32) { nCtvTileAdd=-16; pCtvTile+=31*16; }
    else                { nCtvTileAdd= -8; pCtvTile+= 7* 8; }
  }

  nFun =nCpstType&0x1e;
  nFun|=nCpstFlip&1;
  return CtvDoX[nFun]();
}

static INT32 CpstOneObjZ()
{
  INT32 nFun; INT32 nSize;
  nSize=(nCpstType&24)+8;


  if (nCpstType&CTT_CARE)
  {
    if ((nCpstType&CTT_ROWS)==0)
    {
      // Return if not visible at all
		if (nCpstX <= -nSize) return 0;
		if (nCpstX >= 384)    return 0;
		if (nCpstY <= -nSize) return 0;
		if (nCpstY >= 224)    return 0;
    }
    nCtvRollX=0x4000017f + nCpstX * 0x7fff;
    nCtvRollY=0x400000df + nCpstY * 0x7fff;
  }


  // Clip to loaded graphics data (we have a gap of 0x200 at the end)
  nCpstTile&=nCpsGfxMask; if (nCpstTile>=nCpsGfxLen) return 1;
  pCtvTile=CpsGfx+nCpstTile;

  // Find pLine (pointer to first pixel)
  pCtvLine=pBurnDraw + nCpstY*nBurnPitch + nCpstX*nBurnBpp;
  pZVal=ZBuf + nCpstY*384 + nCpstX;

  if (nSize==32) nCtvTileAdd=16; else nCtvTileAdd=8;

  if (nCpstFlip&2)
  {
    // Flip vertically
         if (nSize==16) { nCtvTileAdd= -8; pCtvTile+=15* 8; }
    else if (nSize==32) { nCtvTileAdd=-16; pCtvTile+=31*16; }
    else                { nCtvTileAdd= -8; pCtvTile+= 7* 8; }
  }

  nFun =nCpstType&0x1e;
  nFun|=nCpstFlip&1;
  return CtvDoXM[nFun]();
}
