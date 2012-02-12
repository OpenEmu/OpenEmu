static uint32 FTVal,FTValR;
static char side;

static uint8 FT_Read(int w, uint8 ret)
{
	if(w) 
		ret|=FTValR;
	return(ret);
}

static void FT_Write(uint8 V)
{
	FTValR=0;

	//printf("%08x\n",FTVal);
	if(!(V&0x1))
		FTValR=(FTVal>>8);
	else if(!(V&0x2))
		FTValR=(FTVal>>4);
	else if(!(V&0x4))
		FTValR=FTVal;

	FTValR=(~FTValR)&0xF;
	if(side=='B')
		FTValR=((FTValR&0x8)>>3) | ((FTValR&0x4)>>1) | ((FTValR&0x2)<<1) | ((FTValR&0x1)<<3);
	FTValR<<=1;
}

static void FT_Update(void *data, int arg)
{
	FTVal=*(uint32 *)data;
}

static INPUTCFC FamilyTrainer={FT_Read,FT_Write,0,FT_Update,0};

INPUTCFC *FCEU_InitFamilyTrainerA(void)
{
	side='A';
	FTVal=FTValR=0;
	return(&FamilyTrainer);
}

INPUTCFC *FCEU_InitFamilyTrainerB(void)
{
	side='B';
	FTVal=FTValR=0;
	return(&FamilyTrainer);
}
