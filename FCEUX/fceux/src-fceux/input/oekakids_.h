static uint8 OKValR,LastWR;
static uint32 OKData;
static uint32 OKX,OKY,OKB;

static uint8 OK_Read(int w, uint8 ret)
{
	if(w) 
		ret|=OKValR;
	return(ret);
}

static void OK_Write(uint8 V)
{
	if(!(V&0x1))
	{
		int32 vx,vy;

		//puts("Redo");
		OKValR=OKData=0;

		if(OKB) OKData|=1;

		if(OKY >= 48) 
			OKData|=2;
		else if(OKB) OKData|=3;

		vx=OKX*240/256+8;
		vy=OKY*256/240-12;
		if(vy<0) vy=0;
		if(vy>255) vy=255;
		if(vx<0) vx=0;
		if(vx>255) vx=255;
		OKData |= (vx<<10) | (vy<<2);
	}
	else
	{
		if((~LastWR)&V&0x02)
			OKData<<=1;

		if(!(V&0x2)) OKValR=0x4;
		else 
		{
			if(OKData&0x40000)
				OKValR=0;
			else
				OKValR=0x8;
		}
	} 
	LastWR=V;
}

static void OK_Update(void *data, int arg)
{
	OKX=((uint32*)data)[0];
	OKY=((uint32*)data)[1];
	OKB=((uint32*)data)[2];
}

static INPUTCFC OekaKids={OK_Read,OK_Write,0,OK_Update,0};

INPUTCFC *FCEU_InitOekaKids(void)
{
	OKValR=0;
	return(&OekaKids);
}
