static uint8 QZVal,QZValR;
static uint8 FunkyMode;

static uint8 QZ_Read(int w, uint8 ret)
{
	if(w) 
	{
		ret|=(QZValR&0x7)<<2;
		QZValR=QZValR>>3;

		if(FunkyMode)
			QZValR|=0x28;
		else
			QZValR|=0x38;
	}
	return(ret);
}

static void QZ_Strobe(void)
{
	QZValR=QZVal;
}

static void QZ_Write(uint8 V)
{
	//printf("Wr: %02x\n",V);
	FunkyMode=V&4;
}

static void QZ_Update(void *data, int arg)
{
	QZVal=*(uint8 *)data;
}

static INPUTCFC QuizKing={QZ_Read,QZ_Write,QZ_Strobe,QZ_Update,0};

INPUTCFC *FCEU_InitQuizKing(void)
{
	QZVal=QZValR=0;
	return(&QuizKing);
}
