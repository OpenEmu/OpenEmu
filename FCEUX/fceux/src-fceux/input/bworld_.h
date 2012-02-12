static int seq,ptr,bit,cnt,have;
static uint8 bdata[32];

static uint8 BarcodeWorldRead(int w, uint8 ret)
{
	if(w && have) 
	{
		switch(seq)
		{
			case 0: seq++; ptr=0; ret|=0x4; break;
			case 1: seq++; bit=bdata[ptr]; cnt=0; ret|=0x4; break;
			case 2: ret|=((bit&0x01)^0x01)<<2; bit>>=1; if(++cnt > 7) seq++;
				break;
			case 3: if(++ptr > 19)
				{
					seq=-1;
					have=0;
				}
				else
					seq=1;
			default: break;
		}
	}
	return(ret);
}

static void BarcodeWorldWrite(uint8 V)
{
	//printf("%02x\n",V);
}

static void BarcodeWorldUpdate(void *data, int arg)
{
	if(*(uint8 *)data)
	{
		*(uint8 *)data=0;
		seq=ptr=0;
		have=1;
		strcpy((char*) bdata,      (char*) data + 1);    // mbg merge 7/17/06
		strcpy((char*) bdata + 13, "SUNSOFT");           // mbg merge 0/17/06
	}
}

static INPUTCFC BarcodeWorld={BarcodeWorldRead,BarcodeWorldWrite,0,BarcodeWorldUpdate,0};

INPUTCFC *FCEU_InitBarcodeWorld(void)
{
	return(&BarcodeWorld);
}
