#include "fkb.h"

#define AK2(x,y)        ( (FKB_##x) | (FKB_##y <<8) )
#define AK(x)                 FKB_##x

static uint8 bufit[0x49];
static uint8 ksmode;
static uint8 ksindex;


static uint16 matrix[9][2][4]=
{
	{{AK(F8),AK(RETURN),AK(BRACKETLEFT),AK(BRACKETRIGHT)},   
		{AK(KANA),AK(RIGHTSHIFT),AK(BACKSLASH),AK(STOP)}},
	{{AK(F7),AK(AT),AK(COLON),AK(SEMICOLON)}, 		
		{AK(UNDERSCORE),AK(SLASH),AK(MINUS),AK(CARET)}},
	{{AK(F6),AK(O),AK(L),AK(K)},   
		{AK(PERIOD),AK(COMMA),AK(P),AK(0)}},
	{{AK(F5),AK(I),AK(U),AK(J)},   	
		{AK(M),AK(N),AK(9),AK(8)}},
	{{AK(F4),AK(Y),AK(G),AK(H)},  
		{AK(B),AK(V),AK(7),AK(6)}},
	{{AK(F3),AK(T),AK(R),AK(D)},  
		{AK(F),AK(C),AK(5),AK(4)}},
	{{AK(F2),AK(W),AK(S),AK(A)},  
		{AK(X),AK(Z),AK(E),AK(3)}},
	{{AK(F1),AK(ESCAPE),AK(Q),AK(CONTROL)},    
		{AK(LEFTSHIFT),AK(GRAPH),AK(1),AK(2)}},
	{{AK(CLEAR),AK(UP),AK(RIGHT),AK(LEFT)},    
		{AK(DOWN),AK(SPACE),AK(DELETE),AK(INSERT)}},
};

static void FKB_Write(uint8 v)
{
	v>>=1;
	if(v&2)
	{
		if((ksmode&1) && !(v&1))
			ksindex=(ksindex+1)%9;
	}
	ksmode=v;
}

static uint8 FKB_Read(int w, uint8 ret)
{
	//printf("$%04x, %d, %d\n",w+0x4016,ksindex,ksmode&1);
	if(w)
	{
		int x;

		ret&=~0x1E;
		for(x=0;x<4;x++)
			if(bufit[ matrix[ksindex][ksmode&1][x]&0xFF ] || bufit[ matrix[ksindex][ksmode&1][x]>>8])
			{
				ret|=1<<(x+1);
			}
		ret^=0x1E;
	}
	return(ret);
}

static void FKB_Strobe(void)
{
	ksmode=0;
	ksindex=0;
	//printf("strobe\n");
}

static void FKB_Update(void *data, int arg)
{
	memcpy(bufit+1,data,0x48);
}

static INPUTCFC FKB={FKB_Read,FKB_Write,FKB_Strobe,FKB_Update,0};

INPUTCFC *FCEU_InitFKB(void)
{
	memset(bufit,0,sizeof(bufit));
	ksmode=ksindex=0;
	return(&FKB);
}
