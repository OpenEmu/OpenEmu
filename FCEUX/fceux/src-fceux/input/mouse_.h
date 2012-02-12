typedef struct {
	int32 mzx, mzy, mzxold, mzyold;
	uint32 readbit;
	uint32 data;
} MOUSE;

static MOUSE Mouse;

static void StrobeMOUSE(int w)
{
	Mouse.readbit=0;
}

static uint8 ReadMOUSE(int w)
{
	uint8 ret=0;
	if(Mouse.readbit>=8)
		ret|=1;
	else
	{
		ret|=(Mouse.data>>Mouse.readbit)&1;
		Mouse.readbit++;
	}
	return(ret);
}

static void UpdateMOUSE(int w, void *data, int arg)
{
	uint32 *ptr=(uint32*)data;
	Mouse.data=0;
	Mouse.mzxold=Mouse.mzx;
	Mouse.mzyold=Mouse.mzy;
	Mouse.mzx=ptr[0];
	Mouse.mzy=ptr[1];
	Mouse.data|=ptr[2];
	if((Mouse.mzxold-Mouse.mzx)>0)
		Mouse.data|=0x0C;
	else if((Mouse.mzxold-Mouse.mzx)<0)
		Mouse.data|=0x04;
	if((Mouse.mzyold-Mouse.mzy)>0)
		Mouse.data|=0x30;
	else if((Mouse.mzyold-Mouse.mzy)<0)
		Mouse.data|=0x10;
}

static INPUTC MOUSEC={ReadMOUSE,0,StrobeMOUSE,UpdateMOUSE,0};

INPUTC *FCEU_InitMouse(int w)
{
	Mouse.mzx=0;
	Mouse.mzy=0;
	Mouse.mzxold=0;
	Mouse.mzyold=0;
	Mouse.data=0;
	return(&MOUSEC);
}
