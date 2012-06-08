#include "video-common.h"
#include "Deinterlacer.h"

Deinterlacer::Deinterlacer()
{
 FieldBuffer = NULL;

 StateValid = false;
 PrevHeight = 0;
}

Deinterlacer::~Deinterlacer()
{
 if(FieldBuffer)
 {
  delete FieldBuffer;
  FieldBuffer = NULL;
 }
}

void Deinterlacer::Process(MDFN_Surface *surface, const MDFN_Rect &DisplayRect, MDFN_Rect *LineWidths, const bool field)
{
 if(!FieldBuffer || FieldBuffer->w < surface->w || FieldBuffer->h < (surface->h / 2))
 {
  if(FieldBuffer)
   delete FieldBuffer;

  FieldBuffer = new MDFN_Surface(NULL, surface->w, surface->h / 2, surface->w, surface->format);
  LWBuffer.resize(FieldBuffer->h);
 }

 //
 // We need to output with LineWidths as always being valid to handle the case of horizontal resolution change between fields
 // while in interlace mode, so clear the first LineWidths entry if it's == ~0, and
 // [...]
 const bool LineWidths_In_Valid = (LineWidths[0].w != ~0);
 if(surface->h && !LineWidths_In_Valid)
 {
  LineWidths[0].x = 0;
  LineWidths[0].w = 0;
 }

 for(int y = 0; y < DisplayRect.h / 2; y++)
 {
  // [...]
  // set all relevant source line widths to the contents of DisplayRect(also simplifies the src_lw and related pointer calculation code
  // farther below.
  if(!LineWidths_In_Valid)
   LineWidths[(y * 2) + field + DisplayRect.y] = DisplayRect;

  if(StateValid && PrevHeight == DisplayRect.h)
  {
   const uint32 *src = FieldBuffer->pixels + y * FieldBuffer->pitch32;
   uint32 *dest = surface->pixels + ((y * 2) + (field ^ 1) + DisplayRect.y) * surface->pitch32;
   MDFN_Rect *dest_lw = &LineWidths[(y * 2) + (field ^ 1) + DisplayRect.y];

   dest_lw->x = 0;
   dest_lw->w = LWBuffer[y];

   memcpy(dest, src, LWBuffer[y] * sizeof(uint32));
  }
  else
  {
   const MDFN_Rect *src_lw = &LineWidths[(y * 2) + field + DisplayRect.y];
   const uint32 *src = surface->pixels + ((y * 2) + field + DisplayRect.y) * surface->pitch32 + src_lw->x;
   const int32 dly = ((y * 2) + (field + 1) + DisplayRect.y);
   uint32 *dest = surface->pixels + dly * surface->pitch32;

   if(y == 0 && field)
   {
    uint32 black = surface->MakeColor(0, 0, 0);

    LineWidths[dly - 2] = *src_lw;
    memset(&surface->pixels[(dly - 2) * surface->pitch32], black, src_lw->w * sizeof(uint32));
   }

   if(dly < (DisplayRect.y + DisplayRect.h))
   {
    LineWidths[dly] = *src_lw;
    memcpy(dest, src, src_lw->w * sizeof(uint32));
   }
  }

  //
  //
  //
  //
  //
  //
  {
   const MDFN_Rect *src_lw = &LineWidths[(y * 2) + field + DisplayRect.y];
   const uint32 *src = surface->pixels + ((y * 2) + field + DisplayRect.y) * surface->pitch32 + src_lw->x;
   uint32 *dest = FieldBuffer->pixels + y * FieldBuffer->pitch32;

   memcpy(dest, src, src_lw->w * sizeof(uint32));
   LWBuffer[y] = src_lw->w;
  }
 }

 PrevHeight = DisplayRect.h;
 StateValid = true;
}


void Deinterlacer::ClearState(void)
{
 StateValid = false;
 PrevHeight = 0;
}
