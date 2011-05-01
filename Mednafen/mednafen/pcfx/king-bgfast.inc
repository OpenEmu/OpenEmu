// Loop prefix non-endless
#define DRAWBG8x1_LPRE() if(bat_x < bat_width) {

// Loop postfix non-endless
#define DRAWBG8x1_LPOST() } bat_x = (bat_x + 1) & bat_width_mask;

#define CDBG_REASON(format, ...) 
//printf("BG%d Reason: " format "\n", n, ## __VA_ARGS__);
static bool CanDrawBG_Fast(int n)
{
 static const int cg_per_mode[0x8] =
 {
  0, // Invalid mode
  1, // 2-bit mode
  2, // 4-bit mode
  4, // 8-bit mode
  8, // 16-bit mode
  8, // 16-bit mode
  8, // 16-bit mode
  8, // 16-bit mode
 };

 const unsigned int bgmode = (king->bgmode >> (n * 4)) & 0xF;
 const uint32 bat_offset = king->BGBATAddr[n] * 1024;
 const uint32 cg_offset = king->BGCGAddr[n] * 1024;
 const uint32 bg_width = (king->BGSize[n] & 0xF0) >> 4;
 const uint32 bg_height = king->BGSize[n] & 0x0F;
 const uint32 bg_maxsize = n ? 0x9 : 0xA;

 // If the bgmode is 0, or 6/7(EXT DOT modes), abort.
 if(!(bgmode & 0x7) || ((bgmode & 0x7) >= 6))
 {
  CDBG_REASON("Mode %02x out of range", bgmode);
  return(FALSE);
 }

 // BG width out of range?
 if(bg_width < 0x3 || bg_width > bg_maxsize)
 {
  CDBG_REASON("Width %02x out of range", bg_width);
  return(FALSE);
 }

 // BG height out of range?
 if(bg_height < 0x3 || bg_height > bg_maxsize)
 {
  CDBG_REASON("Height %02x out of range", bg_height);
  return(FALSE);
 }

 // Time for very stringent checks for BG0!  Don't draw if the sub-screen is a different size than the main screen,
 // or the subscreen CG base != the main screen CG base, or the subscreen BAT base != main screen BAT base(when bgmode & 0x8 is TRUE)
 if(!n)
 {
  if(king->priority & 0x1000)
  {
   CDBG_REASON("Affine transform enabled");
   return(FALSE);
  }

  if((king->BGSize[0] & 0xFF) != (king->BGSize[0] >> 8))
  {
   CDBG_REASON("Main Screen/Sub Screen Size Mismatch");
   return(FALSE);
  }

  // Since we already made sure the main screen/sub screen sizes match, we only
  // care if BG base or CG base don't match if endless/repeat scrolling is enabled.
  // Otherwise, the subscreen won't show at all.
  if(king->BGScrollMode & 0x1)
  {
   if(king->BGCGAddr[0] != king->BG0SubCGAddr)
   {
    CDBG_REASON("Main Screen/Sub Screen CG base mismatch");
    return(FALSE);
   }

   if(bgmode & 0x8)
   {
    if(king->BGBATAddr[0] != king->BG0SubBATAddr)
    {
     CDBG_REASON("Main Screen/Sub Screen BAT base mismatch");
     return(FALSE);
    }
   }
  }
 }


 // If microprogram fetching is disabled, abort.
 if(!(king->MPROGControl & 0x1))
 {
  CDBG_REASON("Microprogram disabled");
  return(FALSE);
 }
 else
 {
  int remap_thing = 0;
  bool bat_fetch = FALSE;

  for(int x = 0; x < 16; x++)
  {
   uint16 mpd;

   // Forcing CG and BAT to 0 if the affine bit != rotate_mode is not technically correct.
   // If there is a mismatch, it's more likely the effective CG and BAT address for the pixel/segment
   // being drawn won't be calculated correctly, likely being just the initial offsets.

   mpd = king->MPROGData[x];

   // Fetching for this BG number?
   if(((mpd >> 6) & 0x3) != n)
    continue;

   // NOP
   if(mpd & 0x100)
    continue;

   // Affine bit.
   if(mpd & 0x20)
   {
    CDBG_REASON("Affine bit set");
    return(FALSE);
   }

   // BAT fetch
   if(mpd & 0x10)
   {
    if(((bat_offset & 0x20000) >> 14) != (x & 8))
    {
     CDBG_REASON("BAT MPROG/base bank mismatch");
     return(FALSE);
    }

    bat_fetch = TRUE;
   }
   else // CG fetch:
   {
    // CG offset/bank mismatch...
    if(((cg_offset & 0x20000) >> 14) != (x & 8))
    {
     CDBG_REASON("CG MPROG/base bank mismatch");
     return(FALSE);
    }

    // Mismatch between CG fetch type and BG mode!
    if((mpd & 0x8) != (bgmode & 0x8))
    {
     CDBG_REASON("Mismatch between CG fetch type and bg mode");
     return(FALSE);
    }

    // Skewed CG fetch order
    if((mpd & 0x7) != remap_thing)
    {
     CDBG_REASON("Skewed CG fetch order");
     return(FALSE);
    }
    remap_thing++;
   }
  }
 
  // CG fetch count mismatch
  if(remap_thing != cg_per_mode[bgmode & 0x7])
  {
   CDBG_REASON("CG fetch count mismatch");
   return(FALSE);
  }

  // BG mode demands a BAT fetch, but we don't have one!
  if((bgmode & 0x8) && !bat_fetch)
  {
   CDBG_REASON("Missing BAT fetch");
   return(FALSE);
  }
 }
 

 return(TRUE);
}

static void DrawBG_Fast(uint32 *target, int n)
{
 const uint16 bgmode = (king->bgmode >> (n * 4)) & 0xF;
 const bool endless = (king->BGScrollMode >> n) & 0x1;
 const uint32 XScroll = king->BGXScroll[n];
 const uint32 YScroll = king->BGYScroll[n];
 const uint32 bat_offset = king->BGBATAddr[n] * 1024;
 const uint32 cg_offset = king->BGCGAddr[n] * 1024;
 const uint32 bat_and_cg_page = (king->PageSetting & 0x0010) ? 1 : 0;

 const int max_size_setting = n ? 0x9 : 0xA;

 const uint32 YOffset = (YScroll + (fx_vce.raster_counter - 22)) & 0xFFFF;
 const uint32 layer_or = (LAYER_BG0 + n) << 28;
 const int ysmall = YOffset & 0x7;

 const unsigned int bat_width_shift = (king->BGSize[n] & 0xF0) >> 4;
 const unsigned int bat_width = (1 << bat_width_shift) >> 3;
 const unsigned int bat_width_mask = endless ? (bat_width - 1) : ((((1 << max_size_setting) * 2) / 8) - 1);

 const int bat_height_shift = king->BGSize[n] & 0x0F;
 const int bat_height = (1 << bat_height_shift) >> 3;
 const int bat_height_mask = endless ? (bat_height - 1) : ((((1 << max_size_setting) * 2) / 8) - 1);

 // We don't need to &= cg_offset and bat_offset with 0x1ffff after here, as the effective addresses
 // calculated with them are anded with 0x1ffff in the rendering code already.
 const uint16 * const king_cg_base = &king->KRAM[bat_and_cg_page][cg_offset & 0x20000];
 const uint16 * const king_bat_base = &king->KRAM[bat_and_cg_page][bat_offset & 0x20000];

 int bat_y = (YOffset >> 3) & bat_height_mask;
 uint32 bat_x = (XScroll >> 3) & bat_width_mask;

 target += 8 - (XScroll & 0x7);

 // If we're in non-endless scroll mode, and we've scrolled past our visible area in the vertical direction, so return.
 if(!endless && bat_y >= bat_height) // Draw this line as transparency and return?
 {
  return;
 }

 // Adjust/corrupt bat_y to be faster in our blitting code
 bat_y = (bat_y << bat_width_shift) >> 3;

 const uint32 palette_offset = ((vce_rendercache.palette_offset[1 + (n >> 1)] >> ((n & 1) ? 8 : 0)) << 1) & 0x1FF;
 const uint32 * const palette_ptr = &vce_rendercache.palette_table_cache[palette_offset];

 {
  int wmul = (1 << bat_width_shift), wmask = (1 << bat_height_shift) - 1;
  int sexy_y_pos = (YOffset & wmask) * wmul;
  
  #if 0
  #define BGFAST_BATMODE (bgmode & 0x8)
  #include "king-bgfast-blit.inc"

  #else
  if(bgmode & 0x8)
  {
   #define BGFAST_BATMODE 1
   #include "king-bgfast-blit.inc"
   #undef BGFAST_BATMODE
  }
  else
  {
   #define BGFAST_BATMODE 0
   #include "king-bgfast-blit.inc"
   #undef BGFAST_BATMODE
  }
  #endif
 }
}

