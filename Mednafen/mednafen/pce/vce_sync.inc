 while(clocks > 0)
 {
  int32 div_clocks;
  int32 chunk_clocks = clocks;

  if(chunk_clocks > hblank_counter)
   chunk_clocks = hblank_counter;

  if(chunk_clocks > vblank_counter)
   chunk_clocks = vblank_counter;

  #ifdef VCE_SGFX_MODE
  for(int chip = 0; chip < 2; chip++)
  {
  #else
  {
   const int chip = 0;
  #endif
   int fwoom = (child_event[chip] * dot_clock_ratio - clock_divider);

   assert(fwoom >= 1);

   if(chunk_clocks > fwoom)
    chunk_clocks = fwoom;
  }
 
  clock_divider += chunk_clocks;
  div_clocks = clock_divider / dot_clock_ratio;
  clock_divider -= div_clocks * dot_clock_ratio;

  child_event[0] -= div_clocks;
  #ifdef VCE_SGFX_MODE
  child_event[1] -= div_clocks;
  #endif

  if(div_clocks > 0)
  {
   #ifdef VCE_SGFX_MODE
   uint16 pixels[2][div_clocks];
   #else
   uint16 pixels[1][div_clocks];
   #endif

   child_event[0] = vdc[0]->Run(div_clocks, pixels[0], skipframe);
   #ifdef VCE_SGFX_MODE
   child_event[1] = vdc[1]->Run(div_clocks, pixels[1], skipframe);
   #endif

   if(!skipframe)
  {
   #ifdef VCE_SGFX_MODE
   {
    for(int32 i = 0; i < div_clocks; i++) // * vce_ratios[dot_clock]; i++)
    {
     static const int prio_select[4] = { 1, 1, 0, 0 };
     static const int prio_shift[4] = { 4, 0, 4, 0 };
     uint32 pix;
     int in_window = 0;

     if(window_counter[0] > 0x40)
     {
      in_window |= 1;
      window_counter[0]--;
     }

     if(window_counter[1] > 0x40)
     {
      in_window |= 2;
      window_counter[1]--;
     }

     uint8 pb = (priority[prio_select[in_window]] >> prio_shift[in_window]) & 0xF;
     uint32 vdc2_pixel, vdc1_pixel;

     vdc2_pixel = vdc1_pixel = 0;

     if(pb & 1)
      vdc1_pixel = pixels[0][i];
     if(pb & 2)
      vdc2_pixel = pixels[1][i];

     /* Dai MakaiMura uses setting 1, and expects VDC #2 sprites in front of VDC #1 background, but
        behind VDC #1's sprites.
     */
     switch(pb >> 2)
     {
      case 1:
                if((vdc2_pixel & 0x100) && !(vdc1_pixel & 0x100) && (vdc2_pixel & 0xF))
                        vdc1_pixel = 0; //amask;
                break;
      case 2:
                if((vdc1_pixel & 0x100) && !(vdc2_pixel & 0x100) && (vdc2_pixel & 0xF))
                        vdc1_pixel = 0; //|= amask;
                break;
     }
     pix = color_table_cache[((vdc1_pixel & 0xF) ? vdc1_pixel : vdc2_pixel) & 0x1FF];

     #if SUPERDUPERMODE
     for(int32 s_i = 0; s_i < dot_clock_ratio; s_i++)
     #endif
     {
      scanline_out_ptr[pixel_offset] = pix;
      pixel_offset++;
     }
    }
   }
   #else
   {
    #if SUPERDUPERMODE
    for(int32 i = 0; i < div_clocks; i++)
    {
     for(int32 si = 0; si < dot_clock_ratio; si++)
     {
      uint32 pix = color_table_cache[pixels[0][i] & 0x3FF];

      scanline_out_ptr[pixel_offset] = pix;
      pixel_offset++;
     }
    }
    #else
    for(int32 i = 0; i < div_clocks; i++) // * vce_ratios[dot_clock]; i++)
    {
     uint32 pix = color_table_cache[pixels[0][i] & 0x3FF];

     scanline_out_ptr[pixel_offset] = pix;
     pixel_offset++;
    }
    #endif

   }
   #endif
  }
   //pixel_offset += div_clocks * dot_clock_ratio;
  }

  clocks -= chunk_clocks;
  hblank_counter -= chunk_clocks;
  if(hblank_counter <= 0)
  {
   hblank ^= 1;
  
   if(hblank)
   {
    // Clock gets stretched and "synchronized" at the beginning of the 237-master-cycle hsync period.
    //clock_divider = 0;
   }
   else
   {
    if(sgfx)
    {
     int add = 0;
     if(dot_clock & 2)
      add = 8 + 96;
     else
      add = 8 + (dot_clock ? 38 : 24);
     window_counter[0] = winwidths[0] + add;
     window_counter[1] = winwidths[1] + add;
    }

    if(NeedSLReset)
     scanline = 0;
    else
     scanline++;

    if(scanline == 14 + 240)
     FrameDone = true;

    if((scanline == 14 + 240) || (scanline == 123))
    {
     HuCPU->Exit();
    }

#if 0	// Testing code
    HuCPU->Exit();
#endif

    //printf("VCE New scanline: %d\n", scanline);

    scanline_out_ptr = &fb[scanline * pitch32];

#ifdef WANT_DEBUGGER
    if(GfxDecode_Buf && GfxDecode_Line == scanline)
     DoGfxDecode();
#endif

    pixel_offset = 0;
    NeedSLReset = FALSE;

    if(!skipframe)
    {
     static const int x_offsets[2][4] = {
                                         { 8 + 24,      8 + 38,      8 + 96,      8 + 96 },
				 	 { 8 + 24 - 12, 8 + 38 - 16, 8 + 96 - 24, 8 + 96 - 24 },
					};
     static const int w_cows[2][4] = {
                                   { 256,      341,      512,      512 },
				   { 256 + 24, 341 + 32, 512 + 48, 512 + 48 },
				  };

     int x, w;

     #if SUPERDUPERMODE
     {
      if(dot_clock >= 2)
       x = 208;
      else if(dot_clock == 1)
       x = 136;
      else
       x = 128;
      w = 1024;

      if(ShowHorizOS)
      {
       x -= 48;
       w += 96;
      }
     }
     #else
     {
      x = x_offsets[ShowHorizOS][dot_clock]; 
      w = w_cows[ShowHorizOS][dot_clock];
     }
     #endif

     LW[scanline].x = x;
     LW[scanline].w = w;
    }
   }
   hblank_counter = hblank ? 237 : 1128;

   for(int chip = 0; chip < chip_count; chip++)
    child_event[chip] = vdc[chip]->HSync(hblank);
  }

  vblank_counter -= chunk_clocks;
  if(vblank_counter <= 0)
  {
   vblank ^= 1;
   vblank_counter = vblank ? 4095 : ((lc263 ? 358995 : 357630) - 4095);

   if(!vblank)
   {
    NeedSLReset = TRUE;
   }

   for(int chip = 0; chip < chip_count; chip++)
    child_event[chip] = vdc[chip]->VSync(vblank);
  }
 }
