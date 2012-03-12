#include "cps.h"

// CPS Scroll2 with Row scroll support

UINT8 *CpsrBase=NULL; // Tile data base
INT32 nCpsrScrX=0,nCpsrScrY=0; // Basic scroll info
UINT16 *CpsrRows=NULL; // Row scroll table, 0x400 words long
int nCpsrRowStart=0; // Start of row scroll (can wrap?)
static INT32 nShiftY=0;
static INT32 EndLineInfo=0;

struct CpsrLineInfo CpsrLineInfo[15];

static void GetRowsRange(INT32 *pnStart,INT32 *pnWidth,INT32 nRowFrom,INT32 nRowTo)
{
  INT32 i,nStart,nWidth;

  // Get the range of scroll values within nRowCount rows
  // Start with zero range
  nStart = BURN_ENDIAN_SWAP_INT16(CpsrRows[nRowFrom&0x3ff]); nStart&=0x3ff; nWidth=0;
  for (i=nRowFrom;i<nRowTo;i++)
  {
    INT32 nViz; INT32 nDiff;
    nViz = BURN_ENDIAN_SWAP_INT16(CpsrRows[i&0x3ff]); nViz&=0x3ff;
    // Work out if this is on the left or the right of our
    // start point.
    nDiff=nViz-nStart;
    // clip to 10-bit signed
    nDiff=((nDiff+0x200)&0x3ff)-0x200;
    if (nDiff>=0)
    {
      // On the right
      if (nDiff>=nWidth) nWidth=nDiff; // expand width to cover it
    }
    else
    {
      // On the left
      nStart+=nDiff; nStart&=0x3ff;
      nWidth-=nDiff; // expand width to cover it
    }
  }

  if (nWidth>0x400) nWidth=0x400;

  *pnStart=nStart;
  *pnWidth=nWidth;
}


static INT32 PrepareRows()
{
  INT32 y; INT32 r;
  struct CpsrLineInfo *pli;
  // Calculate the amount of pixels to shift each
  // row of the tile lines, assuming we draw tile x at
  // (x-pli->nTileStart)<<4  -  i.e. 0, 16, ...

  r=nShiftY-16;
  for (y = -1, pli = CpsrLineInfo; y < EndLineInfo; y++, pli++)
  {
    // Maximum row scroll left and right on this line
    INT32 nMaxLeft=0,nMaxRight=0;
    INT32 ty; INT16 *pr;

    if (CpsrRows==NULL)
    {
      // No row shift - all the same
      INT32 v;
      v =(pli->nTileStart<<4)-nCpsrScrX;
      nMaxLeft=v; nMaxRight=v;
      for (ty=0,pr=pli->Rows; ty<16; ty++,pr++)
      {
        *pr=(INT16)v;
      }
    }
    else
    {
      for (ty=0,pr=pli->Rows; ty<16; ty++,pr++,r++)
      {
        // Get the row offset, if it's in range
        if (r>=0 && r<nEndline)
        {
          INT32 v;
          v =(pli->nTileStart<<4)-nCpsrScrX;
          v -= BURN_ENDIAN_SWAP_INT16(CpsrRows[(nCpsrRowStart+r)&0x3ff]);
          // clip to 10-bit signed
          v+=0x200; v&=0x3ff; v-=0x200;
          *pr=(INT16)v;
               if (v<nMaxLeft)  nMaxLeft=v;
          else if (v>nMaxRight) nMaxRight=v;
        }
        else
        {
          *pr=0;
        }
      }
    }

    pli->nMaxLeft =nMaxLeft;
    pli->nMaxRight=nMaxRight;
  }

  return 0;
}

// Prepare to draw Scroll 2 with rows, by seeing how much
// row scroll each tile line uses (pli->nStart/nWidth),
// and finding which tiles are visible onscreen (pli->nTileStart/End).

INT32 Cps1rPrepare()
{
  INT32 y; struct CpsrLineInfo *pli;
  if (CpsrBase==NULL) return 1;

  nEndline = 224;
  EndLineInfo = 14;
  nShiftY=16-(nCpsrScrY&15);

  for (y=-1,pli=CpsrLineInfo; y<EndLineInfo; y++,pli++)
  {
    INT32 nStart=0,nWidth=0;

    if (CpsrRows!=NULL)
    {
      INT32 nRowFrom,nRowTo;
      // Find out which rows we need to check
      nRowFrom=(y<<4)+nShiftY;
      nRowTo=nRowFrom+16;
      if (nRowFrom<0) nRowFrom=0;
      if (nRowTo>224) nRowTo=224;

      // Shift by row table start offset
      nRowFrom+=nCpsrRowStart;
      nRowTo  +=nCpsrRowStart;

      // Find out what range of scroll values there are for this line
      GetRowsRange(&nStart,&nWidth,nRowFrom,nRowTo);
    }

    nStart+=nCpsrScrX;
    nStart&=0x3ff;

    // Save info in CpsrLineInfo table
    pli->nStart=nStart;
    pli->nWidth=nWidth;
    // Find range of tiles to draw to see whole width:
    pli->nTileStart=nStart>>4;
    pli->nTileEnd=(nStart+nWidth+0x18f)>>4;
  }

  PrepareRows();
  return 0;
}

INT32 Cps2rPrepare()
{
  INT32 y;
  struct CpsrLineInfo *pli;
  if (CpsrBase==NULL) return 1;

  EndLineInfo = ((nEndline + 15) >> 4);

  nShiftY=16-(nCpsrScrY&15);
  for (y = -1, pli = CpsrLineInfo; y < EndLineInfo; y++, pli++)
  {
    INT32 nStart=0,nWidth=0;

    if (CpsrRows!=NULL)
    {
      INT32 nRowFrom,nRowTo;
      // Find out which rows we need to check
      nRowFrom=(y<<4)+nShiftY;
      nRowTo=nRowFrom+16;
      if (nRowFrom < 0) nRowFrom = 0;
      if (nRowTo > nEndline) nRowTo = nEndline;

      // Shift by row table start offset
      nRowFrom+=nCpsrRowStart;
      nRowTo  +=nCpsrRowStart;

      // Find out what range of scroll values there are for this line
      GetRowsRange(&nStart,&nWidth,nRowFrom,nRowTo);
    }

    nStart+=nCpsrScrX;
    nStart&=0x3ff;

    // Save info in CpsrLineInfo table
    pli->nStart=nStart;
    pli->nWidth=nWidth;
    // Find range of tiles to draw to see whole width:
    pli->nTileStart=nStart>>4;
    pli->nTileEnd=(nStart+nWidth+0x18f)>>4;
  }

  PrepareRows();
  return 0;
}

