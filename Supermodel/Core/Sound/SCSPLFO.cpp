/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * SCSPLFO.cpp
 * 
 * SCSP low frequency oscillator emulation. Included directly into SCSP.cpp. Do
 * not compile this!
 */

#include <cmath>
#include <cstdlib>

#define LFO_SHIFT 	8

struct _LFO
{
    unsigned short phase;
    DWORD phase_step;
    int *table;
    int *scale;
};

#define LFIX(v)	((unsigned int) ((float) (1<<LFO_SHIFT)*(v)))

//Convert DB to multiply amplitude
#define DB(v) 	LFIX(pow(10.0,(float) (v)/20.0))

//Convert cents to step increment
#define CENTS(v) LFIX(pow(2.0,(float) (v)/1200.0))

static int PLFO_TRI[256],PLFO_SQR[256],PLFO_SAW[256],PLFO_NOI[256];
static int ALFO_TRI[256],ALFO_SQR[256],ALFO_SAW[256],ALFO_NOI[256];
static float LFOFreq[32]={0.17f,0.19f,0.23f,0.27f,0.34f,0.39f,0.45f,0.55f,0.68f,0.78f,0.92f,1.10f,1.39f,1.60f,1.87f,2.27f,
			  2.87f,3.31f,3.92f,4.79f,6.15f,7.18f,8.60f,10.8f,14.4f,17.2f,21.5f,28.7f,43.1f,57.4f,86.1f,172.3f};
static float ASCALE[8]={0.0f,0.4f,0.8f,1.5f,3.0f,6.0f,12.0f,24.0f};
static float PSCALE[8]={0.0f,7.0f,13.5f,27.0f,55.0f,112.0f,230.0f,494.0f};
static int PSCALES[8][256];
static int ASCALES[8][256];

void LFO_Init()
{
    int i;
    for(i=0;i<256;++i)
    {
		int a,p;
		float TL;
		//Saw
		a=255-i;
		if(i<128)
			p=i;
		else
			p=255-i;    
		ALFO_SAW[i]=a;
		PLFO_SAW[i]=p;
	
		//Square
		if(i<128)
		{
			a=255;
			p=127;
		}
		else
		{
			a=0;
			p=-128;
		}
		ALFO_SQR[i]=a;
		PLFO_SQR[i]=p;
	
		//Tri
		if(i<128)
			a=255-(i*2);
		else
			a=(i*2)-256;
		if(i<64)
			p=i*2;
		else if(i<128)
			p=255-i*2;
		else if(i<192)
			p=256-i*2;
		else
			p=i*2-511;
		ALFO_TRI[i]=a;
		PLFO_TRI[i]=p;
	
		//noise
		//a=lfo_noise[i];
		a=rand()&0xff;
		p=128-a;
		ALFO_NOI[i]=a;
		PLFO_NOI[i]=p;
    }

	for(int s=0;s<8;++s)
	{
		float limit=PSCALE[s];
		for(i=-128;i<128;++i)
		{
			PSCALES[s][i+128]=CENTS(((limit*((float) i))/128.0));
		}
		limit=-ASCALE[s];
		for(i=0;i<256;++i)
		{
			ASCALES[s][i]=DB(((limit*(float) i)/256.0));
		}
	}
}

signed int inline PLFO_Step(_LFO *LFO)
{
	int p;
    LFO->phase+=LFO->phase_step;    
#if LFO_SHIFT!=8    
    LFO->phase&=(1<<(LFO_SHIFT+8))-1;
#endif    
    p=LFO->table[LFO->phase>>LFO_SHIFT];
	p=LFO->scale[p+128];
	return p<<(SHIFT-LFO_SHIFT);
}

signed int inline ALFO_Step(_LFO *LFO)
{
	int p;
    LFO->phase+=LFO->phase_step;    
#if LFO_SHIFT!=8    
    LFO->phase&=(1<<(LFO_SHIFT+8))-1;
#endif    
    p=LFO->table[LFO->phase>>LFO_SHIFT];
	p=LFO->scale[p];
	return p<<(SHIFT-LFO_SHIFT);
}

void LFO_ComputeStep(_LFO *LFO,DWORD LFOF,DWORD LFOWS,DWORD LFOS,int ALFO)
{
    float step=(float) LFOFreq[LFOF]*256.0f/(float) srate;
    LFO->phase_step=(unsigned int) ((float) (1<<LFO_SHIFT)*step);
    if(ALFO)
    {
		switch(LFOWS)
		{
			case 0: LFO->table=ALFO_SAW; break;
			case 1: LFO->table=ALFO_SQR; break;
			case 2: LFO->table=ALFO_TRI; break;
			case 3: LFO->table=ALFO_NOI; break;
		}
		LFO->scale=ASCALES[LFOS];
	}
	else
	{
		switch(LFOWS)
		{
		    case 0: LFO->table=PLFO_SAW; break;
		    case 1: LFO->table=PLFO_SQR; break;
			case 2: LFO->table=PLFO_TRI; break;
		    case 3: LFO->table=PLFO_NOI; break;
		}
		LFO->scale=PSCALES[LFOS];
	}
}
