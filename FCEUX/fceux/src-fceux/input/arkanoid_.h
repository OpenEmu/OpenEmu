typedef struct {
	uint32 mzx,mzb;
	uint32 readbit;
} ARK;

static ARK NESArk[2];
static ARK FCArk;

static void StrobeARKFC(void)
{
	FCArk.readbit=0;
}

static uint8 ReadARKFC(int w,uint8 ret)
{
	ret&=~2;

	if(w)  
	{
		if(FCArk.readbit>=8) 
			ret|=2;
		else
		{
			ret|=((FCArk.mzx>>(7-FCArk.readbit))&1)<<1;
			FCArk.readbit++;
		}
	}
	else
		ret|=FCArk.mzb<<1;
	return(ret);
}

static uint32 FixX(uint32 x)
{
 x=98+x*144/240;
 if(x>242) x=242;
 x=~x;
 return(x);
}

static void UpdateARKFC(void *data, int arg)
{
 uint32 *ptr=(uint32 *)data;
 FCArk.mzx=FixX(ptr[0]);
 FCArk.mzb=ptr[2]?1:0;
}

static INPUTCFC ARKCFC={ReadARKFC,0,StrobeARKFC,UpdateARKFC,0};

INPUTCFC *FCEU_InitArkanoidFC(void)
{
 FCArk.mzx=98;
 FCArk.mzb=0;
 return(&ARKCFC);
}

static uint8 ReadARK(int w)
{
	uint8 ret=0;

	if(NESArk[w].readbit>=8)
		ret|=1<<4;
	else
	{
		ret|=((NESArk[w].mzx>>(7-NESArk[w].readbit))&1)<<4;
		NESArk[w].readbit++;
	}
	ret|=(NESArk[w].mzb&1)<<3;
	return(ret);
}


static void StrobeARK(int w)
{
	NESArk[w].readbit=0;
}

static void UpdateARK(int w, void *data, int arg)
{
 uint32 *ptr=(uint32*)data;
 NESArk[w].mzx=FixX(ptr[0]);
 NESArk[w].mzb=ptr[2]?1:0;
}

static INPUTC ARKC={ReadARK, 0, StrobeARK, UpdateARK, 0};

INPUTC *FCEU_InitArkanoid(int w)
{
 NESArk[w].mzx=98;
 NESArk[w].mzb=0;
 return(&ARKC);
}
