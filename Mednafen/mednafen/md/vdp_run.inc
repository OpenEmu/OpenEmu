
 extern bool MD_OverClock;
 int32 cycles = md_timestamp - vdp_last_ts;
 bool hcell_mode = (reg[0x0C] & 1); // True on H40, FALSE on H32
 bool do_skip = FALSE;

 /* Maximum FIFO wait times(there is no wait during vblank):
	5.98uSec - H32 cell mode
	4.77uSec - H40 cell mode

    Maximum VRAM(8-bit??)/CRAM(16-bit)/VSRAM(16-bit) FIFO writes during a scanline("unlimited" during vblank):
	16	- H32 cell mode
   
    Maximum v-bus(68K) to VRAM writes per line:
	16	- H32 cell mode, active display
       167	- H32 cell mode, vblank
	18	- H40 cell mode, active display
       205	- H40 cell mode, vblank
	
    Maximum VRAM fill writes per line:
	15	- H32 cell mode, active display
       166	- H32 cell mode, vblank
	17	- H40 cell mode, active display
       204	- H40 cell mode, vblank

    Maximum VRAM copy read/write combos per line:
	 8	- H32 cell mode, active display
	83	- H32 cell mode, vblank
	 9	- H40 cell mode, active display
       102	- H40 cell mode, vblank


 H display
 H retrace 

	
	Horizontal blanking(H40 cell mode):
	 Start: H-counter = 0xE4
	 End:   H-counter = 0x08 (of the next line)

	Horizontal blanking(H32 cell mode):
	 Start: H-counter = 0xE9
	 End:	H-counter = ?

	Vertical blanking(H40 cell mode, V28 cell mode):
	 Start:	V-Counter = 0xE0, H-counter = 0xAA
	 End:	V-Counter = 0xFF, H-counter = 0xAA

	Vertical interrupt(H40 cell mode, V28 cell mode):
	 Start: V-Counter = 0xE0, H-counter = 0x08+ish
	 End: ??


	/ 10 in H32 mode
	/ 8 in H40 mode?		 


	28 
 */
 const int divider = hcell_mode ? 8 : 10;
 const int lines_per_frame = is_pal ? 313 : 262;

 //while(cycles > 0)
 {
  vdp_cycle_counter -= cycles; //this_cycles;
  while(vdp_cycle_counter <= 0)
  {
   vdp_line_phase = (vdp_line_phase + 1) % VDPLP_TOTAL;
 
   // Now, we're going INTO the phase that the if statement corresponds to.
   if(vdp_line_phase == VDPLP_HRETRACE_0)
   {
    status |= 0x0004; // Set h-blank flag

    vdp_cycle_counter += divider * (hcell_mode ? (56 + 5) : 46);

    if(hcell_mode)
     vdp_cycle_counter += 4;

    scanline = (scanline + 1) % lines_per_frame;
    v_counter = scanline;

    if(scanline < visible_frame_end)
     render_line(scanline);

    if(scanline < (visible_frame_end - 1))
     parse_satb(0x81 + scanline);
    else if(scanline == (lines_per_frame - 1))
     parse_satb(0x80);

    if(scanline <= visible_frame_end)
    {
     counter--;
     if(counter == 0xFFFF)
     {
      counter = reg[10];
      hint_pending = 1;
     }
    }
    else
    {
     counter = reg[10];
    }
   }
   else if(vdp_line_phase == VDPLP_HRETRACE_1)
   {
    vdp_hcounter_start_ts = md_timestamp + vdp_cycle_counter;
    vdp_cycle_counter += divider * (hcell_mode ? 16 : 13);
   }
   else if(vdp_line_phase == VDPLP_VISIBLE_0)
   {
    status &= ~0x0004; //C lear h-blank flag

    vdp_cycle_counter += divider * (hcell_mode ? 4 : 4);

    /* If a Z80 interrupt is still pending after a scanline, cancel it */
    if(zirq == 1)
    {
     zirq = 0;
     z80_set_interrupt(FALSE);

     MD_OverClock = TRUE;
    }
    if(scanline == visible_frame_end)
    {
     status |= 0x0080;
     vint_pending = 1;

     /* Give enough time to read the interrupt pending flag before
        the interrupt actually occurs. */
     if(!MD_Is68KSuspended())
     {
      C68k_Exec(&Main68K);
     }
     MD_OverClock = FALSE;
     z80_set_interrupt(TRUE);
     zirq = 1;
     MD_ExitCPULoop();
    }   
   }
   else if(vdp_line_phase == VDPLP_VISIBLE_1)
   {
    vdp_cycle_counter += divider * (hcell_mode ? 320 : 256);
   }
   else if(vdp_line_phase == VDPLP_VISIBLE_2)
   {
    CheckDMA();

    if(scanline == visible_frame_end - 1)
    {
     status |= 0x0008;
    }

    if(scanline == lines_per_frame - 1)
    {
     if(im2_flag)
      status ^= 0x0010;
     else
      status &= ~0x0010;

     /* Clear V-Blank flag */
     status &= ~0x0008;
    }
    vdp_cycle_counter += divider * (hcell_mode ? 26 : 23);
   }
  }
 }

 if(!MD_Is68KSuspended())
 {
    if(vint_pending && (reg[1] & 0x20))
    {
     //printf("V-int: %d, %d\n", scanline, md_timestamp);
     C68k_Set_IRQ(&Main68K, 6);
    }
    else if(hint_pending && (reg[0] & 0x10))
    {
     //printf("H-int: %d %d\n", scanline, md_timestamp);
     C68k_Set_IRQ(&Main68K, 4);
    }
    else
    {
     //printf("Int end: %d %d\n", scanline, md_timestamp);
     C68k_Set_IRQ(&Main68K, 0);
    }
 }

 vdp_last_ts = md_timestamp;


