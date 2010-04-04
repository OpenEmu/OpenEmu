  if(firstpixel == 256 && lastpixel == 341)
  {
   Fixit2();
   if(MMC5Mode >= 3)
   {
    xs = 0;
    tochange = MMC5HackSPMode&0x1F;
   }
   if(scanline == -1)
    RefreshAddr = TempAddr;

   FetchNT(MMC5Mode);
   FetchAT(MMC5Mode);
   FetchCD1(MMC5Mode);
   FetchCD2(MMC5Mode);
   FetchNT(MMC5Mode);
   FetchAT(MMC5Mode);
   FetchCD1(MMC5Mode);
   FetchCD2(MMC5Mode);

   //printf("Meow: %d %d %d\n", scanline, firstpixel, lastpixel);
  }
  else
  for(x=firstpixel; x<lastpixel;x++)
  {
   if(x < 256 && scanline >= 0)
   {
    uint8 *S=PALRAMCache;
    uint32 pixdata;

    pixdata=ppulut1[(pshift[0]>>(16-XOffset)) &0xFF]|ppulut2[(pshift[1]>>(16-XOffset)) &0xFF];
    pixdata|=ppulut3[XOffset|((atlatch&0xf)<<3)];

    if(!RCBGOn) pixdata = 0;
    if(!(PPU[1]&2) && x < 8) pixdata = 0;

    emphlinebuf[x] = PPU[1] >> 5;
    Pline[x] = S[(pixdata >> (4 * (x&7))) & 0xF];

    if(RCSPROn && ((PPU[1]&4) || x>= 8))
    {
     if(sphitx != 0x100 && scanline >= 0)
     {
      if(x >= sphitx && x < (sphitx + 8) && x < 255) // Don't check on column 255.
       if((sphitdata & (0x80 >> (x - sphitx)))&& !(Pline[x]&64))
       {
//	printf("Slow Hit: %d\n",scanline);
        PPU_status |= 0x40;
        sphitx = 0x100;
       }
     }
     if(!(sprlinebuf[x] & 0x80))
     {
      if((sprlinebuf[x]&0x40) || (Pline[x] & 0x40))
       Pline[x] = sprlinebuf[x];
     }
    }
    if(rendis & 1)
    {
     emphlinebuf[x] = 0;
     Pline[x] = 0x3C | 0x40;
    }
    Pline[x] = (Pline[x] & pix_mask);
    switch(x & 7)
    {
     case 1: FetchNT(MMC5Mode); break;
     case 3: FetchAT(MMC5Mode); break;
     case 5: FetchCD1(MMC5Mode); break;
     case 7: FetchCD2(MMC5Mode); break;
    }
    if(x == 252) { Fixit1(); }
   }
   if(x == (257))
   {
    Fixit2();
    if(MMC5Mode >= 3)
    {
     xs = 0;
     tochange = MMC5HackSPMode&0x1F;
    }
   }
   if(x == 304 && scanline == -1)
    RefreshAddr = TempAddr;

   if(x == 321 || x == 329) FetchNT(MMC5Mode);
   else if(x == 323 || x == 331) FetchAT(MMC5Mode);
   else if(x == 325 || x == 333) FetchCD1(MMC5Mode);
   else if(x == 327 || x == 335) FetchCD2(MMC5Mode);
  }

