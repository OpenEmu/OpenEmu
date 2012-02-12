uint8 *C; 
register uint8 cc;
uint32 vadr;
       
#ifndef PPUT_MMC5SP
	uint8 zz;
#else
    uint8 xs,ys;  
    xs=X1;
    ys=((scanline>>3)+MMC5HackSPScroll)&0x1F;
    if(ys>=0x1E) ys-=0x1E;
#endif
	
if(X1>=2) {
	uint8 *S=PALRAM;
	uint32 pixdata;

	pixdata=ppulut1[(pshift[0]>>(8-XOffset))&0xFF]|ppulut2[(pshift[1]>>(8-XOffset))&0xFF];

	pixdata|=ppulut3[XOffset|(atlatch<<3)];
	//printf("%02x ",ppulut3[XOffset|(atlatch<<3)]);

	P[0]=S[pixdata&0xF];
	pixdata>>=4;
	P[1]=S[pixdata&0xF];
	pixdata>>=4;
	P[2]=S[pixdata&0xF];
	pixdata>>=4;
	P[3]=S[pixdata&0xF];
	pixdata>>=4;
	P[4]=S[pixdata&0xF];
	pixdata>>=4;
	P[5]=S[pixdata&0xF];
	pixdata>>=4;
	P[6]=S[pixdata&0xF];
	pixdata>>=4;
	P[7]=S[pixdata&0xF];
	P+=8;
}

#ifdef PPUT_MMC5SP
	vadr=(MMC5HackExNTARAMPtr[xs|(ys<<5)]<<4)+(vofs&7);
#else
	zz=RefreshAddr&0x1F;
	C=vnapage[(RefreshAddr>>10)&3];
	vadr=(C[RefreshAddr&0x3ff]<<4)+vofs;   /* Fetch name table byte. */
#endif

#ifdef PPUT_HOOK
	PPU_hook(0x2000|(RefreshAddr&0xfff));
#endif

#ifdef PPUT_MMC5SP
	cc=MMC5HackExNTARAMPtr[0x3c0+(xs>>2)+((ys&0x1C)<<1)];
	cc=((cc >> ((xs&2) + ((ys&0x2)<<1))) &3);
#else
	#ifdef PPUT_MMC5CHR1
		cc=(MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff] & 0xC0)>>6;
	#else
		cc=C[0x3c0+(zz>>2)+((RefreshAddr&0x380)>>4)];  /* Fetch attribute table byte. */
		cc=((cc >> ((zz&2) + ((RefreshAddr&0x40)>>4))) &3);
	#endif
#endif

atlatch>>=2;
atlatch|=cc<<2;  
       
pshift[0]<<=8;
pshift[1]<<=8;

#ifdef PPUT_MMC5SP
	C = MMC5HackVROMPTR+vadr;
	C += ((MMC5HackSPPage & 0x3f & MMC5HackVROMMask) << 12);
#else
	#ifdef PPUT_MMC5CHR1
		C = MMC5HackVROMPTR;
		C += (((MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff]) & 0x3f & 
			MMC5HackVROMMask) << 12) + (vadr & 0xfff);
		C += (MMC50x5130&0x3)<<18; //11-jun-2009 for kuja_killer
	#elif defined(PPUT_MMC5)
		C=MMC5BGVRAMADR(vadr);
	#else
		C = VRAMADR(vadr);
	#endif
#endif

#ifdef PPUT_HOOK
	PPU_hook(vadr);
#endif

pshift[0]|=C[0];
pshift[1]|=C[8];

if((RefreshAddr&0x1f)==0x1f)
	RefreshAddr^=0x41F;
else
	RefreshAddr++;

#ifdef PPUT_HOOK
	PPU_hook(0x2000|(RefreshAddr&0xfff));
#endif

