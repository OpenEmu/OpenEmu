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
    uint32 *prev_sop = scanline_out_ptr;

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

     int rect_x, rect_w;

     #if SUPERDUPERMODE
     {
      if(dot_clock >= 2)
       rect_x = 208;
      else if(dot_clock == 1)
       rect_x = 136;
      else
       rect_x = 128;
      rect_w = 1024;

      if(ShowHorizOS)
      {
       rect_x -= 48;
       rect_w += 96;
      }
     }
     #else
     {
      rect_x = x_offsets[ShowHorizOS][dot_clock]; 
      rect_w = w_cows[ShowHorizOS][dot_clock];
     }
     #endif

     LW[scanline].x = rect_x;
     LW[scanline].w = rect_w;

     if(SubTValid && prev_sop)
     {
      uint32 *slp = prev_sop + x_offsets[0][dot_clock];
      uint32 scale_factor = 256 * 65536 / w_cows[0][dot_clock];
      for(int x = 0; x < w_cows[0][dot_clock]; x++)
      {
       uint32 src_x = (x * scale_factor) >> 16;
       uint32 mul_front = (SubTBuffer[src_x] >> 12) & 0xF;
       uint32 mul_back = 16 - mul_front;
       uint32 front = SubTLUT[SubTBuffer[src_x] & 0xFFF];
       uint32 back = slp[x];

       front = (((front & 0xF0F0F0F0) >> 4) * mul_front) + ((((front & 0x0F0F0F0F) * mul_front) >> 4) & 0x0F0F0F0F);
       back = (((back & 0xF0F0F0F0) >> 4) * mul_back) + ((((back & 0x0F0F0F0F) * mul_back) >> 4) & 0x0F0F0F0F);

       slp[x] = front + back;
      }
     }
    }
    SubTValid = SubHW_DisplayLine(SubTBuffer);
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
   else
    SubHW_CVSync();

   for(int chip = 0; chip < chip_count; chip++)
    child_event[chip] = vdc[chip]->VSync(vblank);
  }
 }
