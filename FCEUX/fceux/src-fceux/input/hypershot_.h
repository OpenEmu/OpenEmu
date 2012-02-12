static uint8 HSVal,HSValR;

static uint8 HS_Read(int w, uint8 ret)
{
	if(w)
		ret|=HSValR;

	return(ret);
}

static void HS_Strobe(void)
{
	HSValR=HSVal<<1;
}

static void HS_Update(void *data, int arg)
{
	HSVal=*(uint8*)data;
}

static INPUTCFC HyperShot={HS_Read,0,HS_Strobe,HS_Update,0};

INPUTCFC *FCEU_InitHS(void)
{
	HSVal=HSValR=0;
	return(&HyperShot);
}
