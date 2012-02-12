static char powerpad_side;
static uint32 pprsb[2];
static uint32 pprdata[2];

static uint8 ReadPP(int w)
{
	uint8 ret=0;
	ret|=((pprdata[w]>>pprsb[w])&1)<<3;
	ret|=((pprdata[w]>>(pprsb[w]+8))&1)<<4;
	if(pprsb[w]>=4) 
	{
		ret|=0x10;
		if(pprsb[w]>=8) 
			ret|=0x08;
	}
	pprsb[w]++;
	return ret;
}

static void StrobePP(int w)
{
		pprsb[w]=0;
}

void UpdatePP(int w, void *data, int arg)
{
	static const char shifttableA[12]={8,9,0,1,11,7,4,2,10,6,5,3};
	static const char shifttableB[12]={1,0,9,8,2,4,7,11,3,5,6,10};
	int x;

	pprdata[w]=0;

	if(powerpad_side=='A')
		for(x=0;x<12;x++)
			pprdata[w]|=(((*(uint32 *)data)>>x)&1)<<shifttableA[x];
	else
		for(x=0;x<12;x++)
			pprdata[w]|=(((*(uint32 *)data)>>x)&1)<<shifttableB[x];
}

static INPUTC PwrPadCtrl={ReadPP,0,StrobePP,UpdatePP,0};

static INPUTC *FCEU_InitPowerpad(int w)
{
	pprsb[w]=pprdata[w]=0;
	return(&PwrPadCtrl);
}

INPUTC *FCEU_InitPowerpadA(int w)
{
	powerpad_side='A';
	return(FCEU_InitPowerpad(w));
}

INPUTC *FCEU_InitPowerpadB(int w)
{
	powerpad_side='B';
	return(FCEU_InitPowerpad(w));
}
