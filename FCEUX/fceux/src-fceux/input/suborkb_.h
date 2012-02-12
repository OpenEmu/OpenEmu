
#include "suborkb.h"
#undef AK
#undef AK2
#define AK2(x,y)        ( (SUBORB_FKB_##x) | (SUBORB_FKB_##y <<8) )
#define AK(x)                 SUBORB_FKB_##x

static uint8 suborb_bufit[0x61];
static uint8 suborb_ksmode;
static uint8 suborb_ksindex;

static uint16 suborb_matrix[13][2][4]=
{
	{{AK(4),AK(G),AK(F),AK(C)},
		{AK(F2),AK(E),AK(5),AK(V)}},
	{{AK(2),AK(D),AK(S),AK(END)},
		{AK(F1),AK(W),AK(3),AK(X)}},
	{{AK(INSERT),AK(BACK),AK(NEXT),AK(RIGHT)},
		{AK(F8),AK(PRIOR),AK(DELETE),AK(HOME)}},
	{{AK(9),AK(I),AK(L),AK(COMMA)},
		{AK(F5),AK(O),AK(0),AK(PERIOD)}},
	{{AK(RBRACKET),AK(RETURN),AK(UP),AK(LEFT)},
		{AK(F7),AK(LBRACKET),AK(BACKSLASH),AK(DOWN)}},
	{{AK(Q),AK(CAPITAL),AK(Z),AK(TAB)},
		{AK(ESCAPE),AK(A),AK(1),AK(LCONTROL)}},
	{{AK(7),AK(Y),AK(K),AK(M)},
		{AK(F4),AK(U),AK(8),AK(J)}},
	{{AK(MINUS),AK(SEMICOLON),AK(APOSTROPHE),AK(SLASH)},
		{AK(F6),AK(P),AK(EQUALS),AK(LSHIFT)}},
	{{AK(T),AK(H),AK(N),AK(SPACE)},
		{AK(F3),AK(R),AK(6),AK(B)}},
	{{0,0,0,0},
		{0,0,0,0}},
	{{AK(LMENU),AK(NUMPAD4),AK(NUMPAD7),AK(F11)},
		{AK(F12),AK(NUMPAD1),AK(NUMPAD2),AK(NUMPAD8)}},
	{{AK(SUBTRACT),AK(ADD),AK(MULTIPLY),AK(NUMPAD9)},
		{AK(F10),AK(NUMPAD5),AK(DIVIDE),AK(NUMLOCK)}},
	{{AK(GRAVE),AK(NUMPAD6),AK(PAUSE),AK(SPACE)},
		{AK(F9),AK(NUMPAD3),AK(DECIMAL),AK(NUMPAD0)}},
};

static void SuborKB_Write(uint8 v)
{
	v>>=1;
	if(v&2)
	{
		if((suborb_ksmode&1) && !(v&1))
			suborb_ksindex=(suborb_ksindex+1)%13;
	}
	suborb_ksmode=v;
}

static uint8 SuborKB_Read(int w, uint8 ret)
{
	if(w)
	{
		int x;

		ret&=~0x1E;
		for(x=0;x<4;x++)
			if(suborb_bufit[suborb_matrix[suborb_ksindex][suborb_ksmode&1][x]&0xFF]||suborb_bufit[suborb_matrix[suborb_ksindex][suborb_ksmode&1][x]>>8])
				ret|=1<<(x+1);
		ret^=0x1E;
	}
	return(ret);
}

static void SuborKB_Strobe(void)
{
	suborb_ksmode=0;
	suborb_ksindex=0;
}

static void SuborKB_Update(void *data, int arg)
{
	memcpy(suborb_bufit+1,data,0x60);
}

static INPUTCFC SuborKB={SuborKB_Read,SuborKB_Write,SuborKB_Strobe,SuborKB_Update,0};

INPUTCFC *FCEU_InitSuborKB(void)
{
	memset(suborb_bufit,0,sizeof(suborb_bufit));
	suborb_ksmode=suborb_ksindex=0;
	return(&SuborKB);
}
