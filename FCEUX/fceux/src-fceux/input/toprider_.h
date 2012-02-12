static uint32 bs,bss;
static uint32 boop;

static uint8 TopRiderRead(int w, uint8 ret)
{
	if(w) 
	{
		ret|=(bs&1)<<3;
		ret|=(boop&1)<<4;
		bs>>=1;  
		boop>>=1;
	}
	return(ret);
}

static void TopRiderWrite(uint8 V)
{
	// if(V&0x2) 
	bs=bss;
	//printf("Write: %02x\n",V);
	// boop=0xC0;
}

static void TopRiderUpdate(void *data, int arg)
{
	bss=*(uint8*)data;
	bss|=bss<<8;
	bss|=bss<<8;
}

static INPUTCFC TopRider={TopRiderRead,TopRiderWrite,0,TopRiderUpdate,0};

INPUTCFC *FCEU_InitTopRider(void)
{
	return(&TopRider);
}

