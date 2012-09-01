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
 * SCSP.cpp
 * 
 * WARNING: Here be dragons! Tread carefully. Enabling/disabling things may
 * break save state support.
 *
 * SCSP (Sega Custom Sound Processor) emulation. This code was generously
 * donated by ElSemi. Interfaces directly to the 68K processor through
 * callbacks. Some minor interface changes were made (external global variables
 * were removed). 
 *
 * The MIDI input buffer has been increased from 8 (which I assume is the
 * actual size) in order to accommodate Model 3's PowerPC/68K communication.
 * There is probably tight synchronization between the CPUs, with PowerPC-side
 * interrupts being generated to fill the MIDI buffer as the 68K pulls data
 * out, or there may be a UART with a large FIFO buffer. This can be simulated
 * by increasing the MIDI buffer (MIDI_STACK_SIZE).
 *
 * To-Do List
 * ----------
 * - Wrap up into an object. Remove any unused #ifdef pathways.
 */


/*	
	SEGA Custom Sound Processor (SCSP) Emulation
	by ElSemi.
	Driven by MC68000
*/

#include "Supermodel.h"
#include "Sound/SCSP.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "Sound/SCSPDSP.h"

//#define NEWSCSP
//#define RB_VOLUME

//#define REVERB
#define USEDSP

#define MAX_SCSP	2
/*#define TIMER_LIMITSA  0x101
#define TIMER_LIMITSB  0x100
#define TIMER_LIMITSC  0xff
*/

#define TIMER_LIMITSA  0xff
#define TIMER_LIMITSB  0xff
#define TIMER_LIMITSC  0xff

// These globals control the operation of the SCSP, they are no longer extern and are set through SCSP_SetBuffers(). --Bart
float SysFPS;
//extern "C" A68KContext M68000_regs;
//extern void __cdecl ErrorLogMessage(char *,...);
signed short *bufferl;		
signed short *bufferr;
int length;
int cnts;

signed int *buffertmpl,*buffertmpr;	// these are allocated inside this file

unsigned int srate=44100;



#define REVERB_LEN	0x10000
#define REVERB_DIF	6000
#define REVERB_DIV	4


signed short bufferrevr[REVERB_LEN];
signed short bufferrevl[REVERB_LEN];
unsigned int RevR,RevW;

//#define _DEBUG

#ifndef _DEBUG
#define ErrorLogMessage
#endif

#ifndef BYTE
#define BYTE UINT8
#endif

#ifndef WORD
#define WORD UINT16
#endif

#ifndef DWORD
#define DWORD UINT32
#endif

static CMutex *MIDILock;	// for safe access to the MIDI FIFOs
static int (*Run68kCB)(int cycles);
static void (*Int68kCB)(int irq);
static void (*RetIntCB)();
static DWORD IrqTimA=1;
static DWORD IrqTimBC=2;
static DWORD IrqMidi=3;

#define MIDI_STACK_SIZE			128
#define MIDI_STACK_SIZE_MASK	(MIDI_STACK_SIZE-1)

static BYTE MidiOutStack[8];
static BYTE MidiOutW=0,MidiOutR=0;
static BYTE MidiStack[MIDI_STACK_SIZE];
static BYTE MidiOutFill;
static BYTE MidiInFill;
static BYTE MidiW=0,MidiR=0;
static BYTE HasSlaveSCSP=0;

static DWORD FNS_Table[0x400];
/*static int TLTABLE[256];
static int LPANTABLE[16];
static int RPANTABLE[16];
*/

#ifdef RB_VOLUME
static int volume[256*4];	// precalculated attenuation values with some marging for enveloppe and pan levels
static int pan_left[32], pan_right[32];	// pan volume offsets
#else
static float SDLT[8]={-1000000.0,-36.0,-30.0,-24.0,-18.0,-12.0,-6.0,0.0};
static int LPANTABLE[0x10000];
static int RPANTABLE[0x10000];
#endif

static int TimPris[3];
static int TimCnt[3];

#define SHIFT	12
#define FIX(v)	((DWORD) ((float) (1<<SHIFT)*(v)))


#define EG_SHIFT	8

#include "SCSPLFO.cpp"

/*
	SCSP features 32 programmable slots
	that can generate FM and PCM (from ROM/RAM) sound
*/
//SLOT PARAMETERS
#define KEYONEX(slot)	((slot->data[0x0]>>0x0)&0x1000)
#define KEYONB(slot)	((slot->data[0x0]>>0x0)&0x0800)
#define SBCTL(slot)		((slot->data[0x0]>>0x9)&0x0003)
#define SSCTL(slot)		((slot->data[0x0]>>0x7)&0x0003)
#define LPCTL(slot)		((slot->data[0x0]>>0x5)&0x0003)
#define PCM8B(slot)		((slot->data[0x0]>>0x0)&0x0010)

#define SA(slot)		(((slot->data[0x0]&0xF)<<16)|(slot->data[0x1]))

#define LSA(slot)		(slot->data[0x2])

#define LEA(slot)		(slot->data[0x3])

#define D2R(slot)		((slot->data[0x4]>>0xB)&0x001F)
#define D1R(slot)		((slot->data[0x4]>>0x6)&0x001F)
#define EGHOLD(slot)	((slot->data[0x4]>>0x0)&0x0020)
#define AR(slot)		((slot->data[0x4]>>0x0)&0x001F)

#define LPSLNK(slot)	((slot->data[0x5]>>0x0)&0x4000)
#define KRS(slot)		((slot->data[0x5]>>0xA)&0x000F)
#define DL(slot)		((slot->data[0x5]>>0x5)&0x001F)
#define RR(slot)		((slot->data[0x5]>>0x0)&0x001F)

#define STWINH(slot)	((slot->data[0x6]>>0x0)&0x0200)
#define SDIR(slot)		((slot->data[0x6]>>0x0)&0x0100)
#define TL(slot)		((slot->data[0x6]>>0x0)&0x00FF)

#define MDL(slot)		((slot->data[0x7]>>0xB)&0x0007)
#define MDXSL(slot)		((slot->data[0x7]>>0x6)&0x003F)
#define MDYSL(slot)		((slot->data[0x7]>>0x0)&0x003F)

#define OCT(slot)		((slot->data[0x8]>>0xB)&0x000F)
#define FNS(slot)		((slot->data[0x8]>>0x0)&0x03FF)

#define LFORE(slot)		((slot->data[0x9]>>0x0)&0x8000)
#define LFOF(slot)		((slot->data[0x9]>>0xA)&0x001F)
#define PLFOWS(slot)	((slot->data[0x9]>>0x8)&0x0003)
#define PLFOS(slot)		((slot->data[0x9]>>0x5)&0x0007)
#define ALFOWS(slot)	((slot->data[0x9]>>0x3)&0x0003)
#define ALFOS(slot)		((slot->data[0x9]>>0x0)&0x0007)

#define ISEL(slot)		((slot->data[0xA]>>0x3)&0x000F)
#define IMXL(slot)		((slot->data[0xA]>>0x0)&0x0007)

#define DISDL(slot)		((slot->data[0xB]>>0xD)&0x0007)
#define DIPAN(slot)		((slot->data[0xB]>>0x8)&0x001F)
#define EFSDL(slot)		((slot->data[0xB]>>0x5)&0x0007)
#define EFPAN(slot)		((slot->data[0xB]>>0x0)&0x001F)

//Envelope step (fixed point)
int ARTABLE[64],DRTABLE[64];

//Envelope times in ms
double ARTimes[64]={100000/*infinity*/,100000/*infinity*/,8100.0,6900.0,6000.0,4800.0,4000.0,3400.0,3000.0,2400.0,2000.0,1700.0,1500.0,
					1200.0,1000.0,860.0,760.0,600.0,500.0,430.0,380.0,300.0,250.0,220.0,190.0,150.0,130.0,110.0,95.0,
					76.0,63.0,55.0,47.0,38.0,31.0,27.0,24.0,19.0,15.0,13.0,12.0,9.4,7.9,6.8,6.0,4.7,3.8,3.4,3.0,2.4,
					2.0,1.8,1.6,1.3,1.1,0.93,0.85,0.65,0.53,0.44,0.40,0.35,0.0,0.0};
double DRTimes[64]={100000/*infinity*/,100000/*infinity*/,118200.0,101300.0,88600.0,70900.0,59100.0,50700.0,44300.0,35500.0,29600.0,25300.0,22200.0,17700.0,
					14800.0,12700.0,11100.0,8900.0,7400.0,6300.0,5500.0,4400.0,3700.0,3200.0,2800.0,2200.0,1800.0,1600.0,1400.0,1100.0,
					920.0,790.0,690.0,550.0,460.0,390.0,340.0,270.0,230.0,200.0,170.0,140.0,110.0,98.0,85.0,68.0,57.0,49.0,43.0,34.0,
					28.0,25.0,22.0,18.0,14.0,12.0,11.0,8.5,7.1,6.1,5.4,4.3,3.6,3.1};


typedef enum {ATTACK,DECAY1,DECAY2,RELEASE} _STATE;
struct _EG
{
	int volume;	//
	_STATE state;
	int step;
	//step vals
	int AR;		//Attack
	int D1R;	//Decay1
	int D2R;	//Decay2
	int RR;		//Release

	int DL;		//Decay level
	BYTE EGHOLD;
	BYTE LPLINK;
};

struct _SLOT
{
	union
	{
		WORD data[0x10];	//only 0x1a bytes used
		BYTE datab[0x20];
	};
	BYTE active;	//this slot is currently playing
	BYTE *base;		//samples base address
	DWORD cur_addr;	//current play address (24.8)
	DWORD step;		//pitch step (24.8)
	BYTE Back;
	_EG EG;			//Envelope
	_LFO PLFO;		//Phase LFO
	_LFO ALFO;		//Amplitude LFO
	int slot;
	signed short Prev;	//Previous sample (for interpolation)
};

#define MEM4B(scsp)		((scsp->data[0]>>0x0)&0x0200)
#define DAC18B(scsp)	((scsp->data[0]>>0x0)&0x0100)
#define MVOL(scsp)		((scsp->data[0]>>0x0)&0x000F)
#define RBL(scsp)		((scsp->data[1]>>0x7)&0x0003)
#define RBP(scsp)		((scsp->data[1]>>0x0)&0x007F)
#define MOFULL(scsp)	((scsp->data[2]>>0x0)&0x1000)
#define MOEMPTY(scsp)	((scsp->data[2]>>0x0)&0x0800)
#define MIOVF(scsp)		((scsp->data[2]>>0x0)&0x0400)
#define MIFULL(scsp)	((scsp->data[2]>>0x0)&0x0200)
#define MIEMPTY(scsp)	((scsp->data[2]>>0x0)&0x0100)

#define SCILV0(scsp)    ((scsp->data[0x24/2]>>0x0)&0xff)
#define SCILV1(scsp)    ((scsp->data[0x26/2]>>0x0)&0xff)
#define SCILV2(scsp)    ((scsp->data[0x28/2]>>0x0)&0xff)

#define SCIEX0	0
#define SCIEX1	1
#define SCIEX2	2
#define SCIMID	3
#define SCIDMA	4
#define SCIIRQ	5
#define SCITMA	6
#define SCITMB	7

struct _SCSP
{
	union
	{
		WORD data[0x30/2];
		BYTE datab[0x30];
	};
	_SLOT Slots[32];
	signed short RINGBUF[64];
	unsigned char BUFPTR;
	unsigned char *SCSPRAM;	
	char Master;
#ifdef USEDSP
	_SCSPDSP DSP;
	signed short *MIXBuf;
#endif
} SCSPs[MAX_SCSP],*SCSP=SCSPs;

signed short *RBUFDST;	//this points to where the sample will be stored in the RingBuf

unsigned char DecodeSCI(unsigned char irq)
{
	unsigned char SCI=0;
	unsigned char v;
	v=(SCILV0((SCSP))&(1<<irq))?1:0;
	SCI|=v;
	v=(SCILV1((SCSP))&(1<<irq))?1:0;
	SCI|=v<<1;
	v=(SCILV2((SCSP))&(1<<irq))?1:0;
	SCI|=v<<2;
	return SCI;
}

void CheckPendingIRQ()
{
	DWORD pend=SCSP[0].data[0x20/2];
	DWORD en=SCSP[0].data[0x1e/2];
	/*if(pend&0x8)
		if(en&0x8)
		{
			Int68kCB(IrqMidi);
			return;
		}
	*/
	
	/*
	 * MIDI FIFO critical section
	 *
	 * NOTE: I don't think a mutex is really needed here, so I've disabled
	 * this critical section.
	 */
	//if (g_Config.multiThreaded)
	//	MIDILock->Lock();

	if(MidiW!=MidiR)
	{
		//if (g_Config.multiThreaded)
		//	MIDILock->Unlock();
		
		//SCSP.data[0x20/2]|=0x8;	//Hold midi line while there are commands pending
		Int68kCB(IrqMidi);
		//printf("68K: MIDI IRQ\n");
		//ErrorLogMessage("Midi");
		return;
	}
	
	//if (g_Config.multiThreaded)
	//	MIDILock->Unlock();
	
	if(!pend)
		return;
	if(pend&0x40)
		if(en&0x40)
		{
			Int68kCB(IrqTimA);
			//ErrorLogMessage("TimA");
			return;
		}
	if(pend&0x80)
		if(en&0x80)
		{
			Int68kCB(IrqTimBC);
			//ErrorLogMessage("TimB");
			return;
		}
	if(pend&0x100)
		if(en&0x100)
		{
			Int68kCB(IrqTimBC);
			//ErrorLogMessage("TimC");
			return;
		}
	Int68kCB(0);
}

int Get_AR(int base,int R)
{
	int Rate=base+(R<<1);
//	int Rate=(base+R)<<1;
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return ARTABLE[Rate];
}

int Get_DR(int base,int R)
{
	int Rate=base+(R<<1);
//	int Rate=(base+R)<<1;
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return DRTABLE[Rate];
}

int Get_RR(int base,int R)
{
	int Rate=base+(R<<1);
//	int Rate=(base+R)<<1;
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return DRTABLE[Rate];
}

void Compute_EG(_SLOT *slot)
{
	int octave=OCT(slot);
	int rate;
	if(octave&8) octave=octave-16;
	if(KRS(slot)!=0xf)
		rate=2*(octave+KRS(slot))+((FNS(slot)>>9)&1);
	else
		rate=((FNS(slot)>>9)&1);
	slot->EG.volume=0;
	slot->EG.AR=Get_AR(rate,AR(slot));
	slot->EG.D1R=Get_DR(rate,D1R(slot));
	slot->EG.D2R=Get_DR(rate,D2R(slot));
	slot->EG.RR=Get_RR(rate,RR(slot));
	slot->EG.DL=0x1f-DL(slot);
	slot->EG.EGHOLD=EGHOLD(slot);
}

void SCSP_StopSlot(_SLOT *slot,int keyoff);

int EG_Update(_SLOT *slot)
{
	
	switch(slot->EG.state)
	{
		case ATTACK:
			slot->EG.volume+=slot->EG.AR;
			if(slot->EG.volume>=(0x3ff<<EG_SHIFT))
			{
				slot->EG.state=DECAY1;
				if(slot->EG.D1R>=(1024<<EG_SHIFT))	//Skip DECAY1, go directly to DECAY2
					slot->EG.state=DECAY2;
				slot->EG.volume=0x3ff<<EG_SHIFT;
			}
			if(slot->EG.EGHOLD)
				return 0x3ff<<(SHIFT-10);
			break;
		case DECAY1:
			slot->EG.volume-=slot->EG.D1R;
			if((slot->EG.volume>>(EG_SHIFT+5))<=slot->EG.DL)
				slot->EG.state=DECAY2;
			break;
		case DECAY2:
			if(slot->EG.volume<=0 || slot->EG.DL==0)
			{
				slot->EG.volume=0;
				SCSP_StopSlot(slot,0);
			}
			if(D2R(slot)==0)
				return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
			slot->EG.volume-=slot->EG.D2R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;

			break;
		case RELEASE:
			slot->EG.volume-=slot->EG.RR;
			if(slot->EG.volume<=0)
			{
				SCSP_StopSlot(slot,0);
				slot->EG.volume=0;
				slot->EG.state=ATTACK;
			}
			//slot->EG.volume=0;
			break;
		default:
			return 1<<SHIFT;
	}
	return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
	
}

DWORD SCSP_Step(_SLOT *slot)
{
	int octave=OCT(slot);
	int Fn;
	/*
	int Fo=44100;
	if(octave&8)
		Fo>>=(16-octave);
	else
		Fo<<=octave;
	Fn=Fo*(((FNS(slot))<<(SHIFT-10))|(1<<SHIFT));
	*/
	Fn=(FNS_Table[FNS(slot)]);	//24.8
	if(octave&8)
		Fn>>=(16-octave);
	else
		Fn<<=octave;


	return Fn/srate;
}

void Compute_LFO(_SLOT *slot)
{
	if(PLFOS(slot)!=0)
		LFO_ComputeStep(&(slot->PLFO),LFOF(slot),PLFOWS(slot),PLFOS(slot),0);
	if(ALFOS(slot)!=0)
		LFO_ComputeStep(&(slot->ALFO),LFOF(slot),ALFOWS(slot),ALFOS(slot),1);
}

void SCSP_StartSlot(_SLOT *slot)
{
	slot->active=1;
	slot->Back=0;
	slot->base=SCSP->SCSPRAM+SA(slot);
	slot->cur_addr=0;
	slot->step=SCSP_Step(slot);	
	Compute_EG(slot);
	slot->EG.state=ATTACK;
	slot->EG.volume=0;
	slot->Prev=0;
	Compute_LFO(slot);
	/*{
		char aux[12];
		static n=0;
		sprintf(aux,"smp%d.raw",n);
		FILE *f=fopen(aux,"wb");
		fwrite(slot->base,LEA(slot),1,f);
		fclose(f);
		++n;
	}
	*/

}

void SCSP_StopSlot(_SLOT *slot,int keyoff)
{
	if(keyoff && slot->EG.state!=RELEASE)
	{
		slot->EG.state=RELEASE;
//		return;
	}
	else
		slot->active=0;
	slot->data[0]&=~0x800;
	//DebugLog("KEYOFF2 %d",slot->slot);
}

#define log2(n) (log((float) n)/log((float) 2))

bool SCSP_Init(int n)
{
	if(n==2)
	{
		SCSP=SCSPs+1;
		memset(SCSP,0,sizeof(SCSP));
		SCSP->Master=0;
		HasSlaveSCSP=1;
#ifdef USEDSP
		SCSPDSP_Init(&SCSP->DSP);
#endif

	}
	SCSP=SCSPs+0;
	memset(SCSP,0,sizeof(SCSP));
#ifdef USEDSP
	SCSPDSP_Init(&SCSP->DSP);
#endif
	SCSP->Master=1;
	RevR=0;
	RevW=REVERB_DIF;
	memset(bufferrevl,0,sizeof(bufferrevl));
	memset(bufferrevr,0,sizeof(bufferrevr));
	MidiR=MidiW=0;
	MidiOutR=MidiOutW=0;
	MidiOutFill=0;
	MidiInFill=0;
	for(int i=0;i<0x400;++i)
	{
		float fcent=(double) 1200.0*log2((double)(((double) 1024.0+(double)i)/(double)1024.0));
		//float fcent=1.0+(float) i/1024.0;
		fcent=(double) 44100.0*pow(2.0,fcent/1200.0);
		FNS_Table[i]=(float) (1<<SHIFT) *fcent;
		//FNS_Table[i]=(i>>(10-SHIFT))|(1<<SHIFT);
		
	}
#ifdef RB_VOLUME
	// Volume table, 1 = -0.375dB, 8 = -3dB, 256 = -96dB
	for(i = 0; i < 256; i++)
		volume[i] = 65536.0*pow(2.0, (-0.375/6.0)*i);
	for(i = 256; i < 256*4; i++)
		volume[i] = 0;

	// Pan values, units are a linear -3dB ramp, i.e. 8 places in the volume[] table.
	for(i = 0; i < 16; i++)
	{
		pan_left[i] = i*8;
		pan_left[i+16] = 0;
		pan_right[i] = 0;
		pan_right[i+16] = i*8;
	}
	// patch in the infinity values
	pan_left[15] = 256;
	pan_right[31] = 256;

#else
	for(int i=0;i<0x10000;++i)
	{
		int iTL =(i>>0x8)&0xff;
		int iPAN=(i>>0x0)&0x1f;
		int iSDL=(i>>0x5)&0x07;

		float TL=1.0;
		float SegaDB=0;
		//2^(-(TL-2^4))
		/*if(iTL&0x01) TL*=0.95760;
		if(iTL&0x02) TL*=0.91700;
		if(iTL&0x04) TL*=0.84090;
		if(iTL&0x08) TL*=0.70711;
		if(iTL&0x10) TL*=0.50000;
		if(iTL&0x20) TL*=0.25000;
		if(iTL&0x40) TL*=0.06250;
		if(iTL&0x80) TL*=0.00391;*/
		if(iTL&0x01) SegaDB-=0.4;
		if(iTL&0x02) SegaDB-=0.8;
		if(iTL&0x04) SegaDB-=1.5;
		if(iTL&0x08) SegaDB-=3;
		if(iTL&0x10) SegaDB-=6;
		if(iTL&0x20) SegaDB-=12;
		if(iTL&0x40) SegaDB-=24;
		if(iTL&0x80) SegaDB-=48;

		TL=pow(10.0,SegaDB/20.0);

		float PAN=1.0;
		//2^(-2^(PAN-2))
		/*if(iPAN&0x1) PAN*=0.70711;
		if(iPAN&0x2) PAN*=0.50000;
		if(iPAN&0x4) PAN*=0.25000;
		if(iPAN&0x8) PAN*=0.06250;
		if(iPAN==0xf) PAN=0.0;*/

		SegaDB=0;
		if(iPAN&0x1) SegaDB-=3;
		if(iPAN&0x2) SegaDB-=6;
		if(iPAN&0x4) SegaDB-=12;
		if(iPAN&0x8) SegaDB-=24;

		if(iPAN==0xf) PAN=0.0;
		else if(iPAN==0x1f) PAN=0.0;
		else PAN=pow(10.0,SegaDB/20.0);

		float LPAN,RPAN;

		if(iPAN<0x10)
		{
			LPAN=PAN;
			RPAN=1.0;
		}
		else
		{
			RPAN=PAN;
			LPAN=1.0;
		}

		float SDL=1.0;
		if(iSDL)
			SDL=pow(10.0,(SDLT[iSDL])/20.0);
		else
			SDL=0.0;

		if(iSDL==0x6)
			int a=1;
		if(iTL==0x3a)
			int a=1;
		
		LPANTABLE[i]=FIX((4.0*LPAN*TL*SDL));
		RPANTABLE[i]=FIX((4.0*RPAN*TL*SDL));


	}
#endif
	/*for(i=0;i<4;++i)	
		ARTABLE[i]=DRTABLE[i]=0;
	for(i=4;i<62;++i)*/

	/*for(i=2;i<62;++i)
	{
		//double t=BaseTimes[i];	//In ms
		double t=BaseTimes2[i/2]/AR2DR;	//In ms
		double step=(1023*1000.0)/((float) srate*t);
		double scale=(double) (1<<EG_SHIFT);
		ARTABLE[i]=(int) (step*scale);
		step/=AR2DR;
		DRTABLE[i]=(int) (step*scale);
	}
	*/
	ARTABLE[0]=DRTABLE[0]=0;	//Infinite time
	ARTABLE[1]=DRTABLE[1]=0;
	for(int i=2;i<64;++i)
	{
		double t,step,scale;
		t=ARTimes[i];	//In ms
		if(t!=0.0)
		{
			step=(1023*1000.0)/((float) srate*t);
			scale=(double) (1<<EG_SHIFT);
			ARTABLE[i]=(int) (step*scale);
		}
		else
			ARTABLE[i]=1024<<EG_SHIFT;

		t=DRTimes[i];	//In ms
		step=(1023*1000.0)/((float) srate*t);
		scale=(double) (1<<EG_SHIFT);
		DRTABLE[i]=(int) (step*scale);
	}
	
	for(int i=0;i<32;++i)
		SCSPs[0].Slots[i].slot=i;

#ifdef USEDSP
	//allocate 0x300 (over 1 frame) * 32 slots * 16 bit
	SCSP->MIXBuf=(signed short *) malloc(0x300*32*sizeof(signed short));
#endif

	LFO_Init();
	buffertmpl = NULL;
	buffertmpr = NULL;
	buffertmpl=(signed int*) malloc(44100*sizeof(signed int));
	if (NULL == buffertmpl)
		return ErrorLog("Insufficient memory for internal SCSP buffers.");
	buffertmpr=(signed int*) malloc(44100*sizeof(signed int));
	if (NULL == buffertmpl)
	{
		free(buffertmpl);
		return ErrorLog("Insufficient memory for internal SCSP buffers.");
	}
	memset(buffertmpl,0,44100*sizeof(signed int));
	memset(buffertmpr,0,44100*sizeof(signed int));
	
	// MIDI FIFO mutex
	MIDILock = CThread::CreateMutex();
	if (NULL == MIDILock)
	{
		free(buffertmpl);
		free(buffertmpr);
		return ErrorLog("Unable to create MIDI mutex!");
	}
	
	return OKAY;
}

void SCSP_SetRAM(int n,unsigned char *r)
{
	SCSPs[n].SCSPRAM=r;
#ifdef USEDSP
	SCSPs[n].DSP.SCSPRAM=(unsigned short*) r;
#endif
}

void SCSP_UpdateSlotReg(int s,int r)
{
	_SLOT *slot=SCSP->Slots+s;
	switch(r&0x3f)
	{
		case 0:
		case 1:
			if(KEYONEX(slot))
			{
				for(int sl=0;sl<32;++sl)
				{
					_SLOT *s2=SCSP->Slots+sl;
					if(!KEYONB(s2) && sl==cnts && s2->active)
						int a=1;
					//if(s2->EG.state!=RELEASE)
					{
						if(KEYONB(s2) && (!s2->active || (s2->active && s2->EG.state==RELEASE)))
						{
							//DebugLog("KEYON %d",sl);
							//printf("68K: KEYON %d\n",sl);
							SCSP_StartSlot(s2);
						}
						if(!KEYONB(s2) && s2->active)
						{
							//s2->active=0;
							SCSP_StopSlot(s2,1);
							//DebugLog("KEYOFF %d",sl);
						}
					}
				}
				slot->data[0]&=~0x1000;
			}
			break;
		case 0x10:
		case 0x11:
			slot->step=SCSP_Step(slot);	
			break;
		case 0xA:
		case 0xB:
			if(slot->active)
				int a=1;
//			if(RR(slot)==0x1f)
//				SCSP_StopSlot(slot,0);
			slot->EG.RR=Get_RR(0,RR(slot));
			slot->EG.DL=0x1f-DL(slot);
			break;
		case 0x12:
		case 0x13:
			Compute_LFO(slot);
			break;
	}
}

void SCSP_UpdateReg(int reg)
{
	switch(reg&0x3f)
	{
		case 0x2:
		case 0x3:
		{
#ifdef USEDSP
			unsigned int v=RBL(SCSP);
			SCSP->DSP.RBP=RBP(SCSP);
			if(v==0)
				SCSP->DSP.RBL=8*1024;
			else if(v==1)
				SCSP->DSP.RBL=16*1024;
			else if(v==2)
				SCSP->DSP.RBL=32*1024;
			else if(v==3)
				SCSP->DSP.RBL=64*1024;
#endif
		}
		break;

		case 0x6:
		case 0x7:
			SCSP_MidiOutW(SCSP->data[0x6/2]&0xff);
			break;

/*		case 0x8:
		case 0x9:
			{
				unsigned char slot=SCSP.data[0x8/2]>>11;	
				int a=1;
				
			}
			break;
*/
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			{
				int a=1;
			}
			break;
		case 0x18:
		case 0x19:
			if(SCSP->Master)	
			{
				TimPris[0]=1<<((SCSP->data[0x18/2]>>8)&0x7);
				TimCnt[0]=((SCSP->data[0x18/2]&0xff)<<8)|(TimCnt[0]&0xff);
			}
			break;
		case 0x1a:
		case 0x1b:
			if(SCSP->Master)	
			{
				TimPris[1]=1<<((SCSP->data[0x1A/2]>>8)&0x7);
				TimCnt[1]=((SCSP->data[0x1A/2]&0xff)<<8)|(TimCnt[1]&0xff);
			}
			break;
		case 0x1C:
		case 0x1D:
			if(SCSP->Master)	
			{
				TimPris[2]=1<<((SCSP->data[0x1C/2]>>8)&0x7);
				TimCnt[2]=((SCSP->data[0x1C/2]&0xff)<<8)|(TimCnt[2]&0xff);
			}
			break;
		case 0x22:	//SCIRE
		case 0x23:
			if(SCSP->Master)	
			{
				SCSP->data[0x20/2]&=~SCSP->data[0x22/2];
				CheckPendingIRQ();
			}
			break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
			if(SCSP->Master)
			{
				IrqTimA=DecodeSCI(SCITMA);
				IrqTimBC=DecodeSCI(SCITMB);
				IrqMidi=DecodeSCI(SCIMID);
			}
			break;
	}
}

void SCSP_UpdateSlotRegR(int slot,int reg)
{

}

void SCSP_UpdateRegR(int reg)
{
	switch(reg&0x3f)
	{
		case 4:
		case 5:
			{
				unsigned short v=SCSP->data[0x5/2];
				v&=0xff00;
				
				/*
				 * MIDI FIFO critical section!
				 */
				if (g_Config.multiThreaded)
					MIDILock->Lock();
					
				v|=MidiStack[MidiR];
				//printf("read MIDI\n");
				if(MidiR!=MidiW)
				{
					++MidiR;
					MidiR&=MIDI_STACK_SIZE_MASK;
					//Int68kCB(IrqMidi);
				}
				
				MidiInFill--;
				SCSP->data[0x5/2]=v;
				
				if (g_Config.multiThreaded)
					MIDILock->Unlock();
			}
			break;
		case 8:
		case 9:
			{
				unsigned char slot=SCSP->data[0x8/2]>>11;	
				unsigned int CA=SCSP->Slots[slot&0x1f].cur_addr>>(SHIFT+12);
				SCSP->data[0x8/2]&=~(0x780);
				SCSP->data[0x8/2]|=CA<<7;
			}
			break;
	}
}


void SCSP_w8(unsigned int addr,unsigned char val)
{
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		//DebugLog("Slot %02X Reg %02X write byte %04X\n",slot,addr^1,val);
		//printf("Slot %02X Reg %02X write byte %04X\n",slot,addr^1,val);
		*(unsigned char *) &(SCSP->Slots[slot].datab[addr^1]) = val;
		SCSP_UpdateSlotReg(slot,(addr^1)&0x1f);
	}
	else if(addr<0x600)
	{
		*(unsigned char *) &(SCSP->datab[(addr&0xff)^1]) = val;
		SCSP_UpdateReg((addr^1)&0xff);
	}	
	else if(addr<0x700)
		SCSP->RINGBUF[(addr-0x600)/2]=val;
	else
	{
#ifdef USEDSP
		//DSP
		if(addr<0x780)	//COEF
			((unsigned char *) SCSP->DSP.COEF)[(addr-0x700)^1]=val;
		else if(addr<0x7C0)
			((unsigned char *) SCSP->DSP.MADRS)[(addr-0x780)^1]=val;
		else if(addr>=0x800 && addr<0xC00)
			((unsigned char *) SCSP->DSP.MPRO)[(addr-0x800)^1]=val;
		else
			int a=1;
		if(addr==0xBFE)
		{
			SCSPDSP_Start(&SCSP->DSP);
		}
		int a=1;
#endif
	}
}

void SCSP_w16(unsigned int addr,unsigned short val)
{
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		//DebugLog("Slot %02X Reg %02X write word %04X\n",slot,addr,val);
		//printf("Slot %02X Reg %02X write word %04X\n",slot,addr,val);
		*(unsigned short *) &(SCSP->Slots[slot].datab[addr]) = val;
		SCSP_UpdateSlotReg(slot,addr&0x1f);
	}
	else if(addr<0x600)
	{
		*(unsigned short *) &(SCSP->datab[addr&0xff]) = val;
		SCSP_UpdateReg(addr&0xff);
	}	
	else if(addr<0x700)
		SCSP->RINGBUF[(addr-0x600)/2]=val;
	else
	{
#ifdef USEDSP
		//DSP
		if(addr<0x780)	//COEF
			*(unsigned short *) &(SCSP->DSP.COEF[(addr-0x700)/2])=val;
		else if(addr<0x800)
			*(unsigned short *) &(SCSP->DSP.MADRS[(addr-0x780)/2])=val;
		else if(addr<0xC00)
			*(unsigned short *) &(SCSP->DSP.MPRO[(addr-0x800)/2])=val;
		else
			int a=1;
		if(addr==0xBFE)
			SCSPDSP_Start(&SCSP->DSP);
		int a=1;
#endif

	}
}

void SCSP_w32(unsigned int addr,unsigned int val)
{
	addr&=0xffff;
	
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		//DebugLog("Slot %02X Reg %02X write dword %08X\n",slot,addr,val);
		//printf("Slot %02X Reg %02X write dword %08X\n",slot,addr,val);
		rotl(val, 16);

		*(unsigned int *) &(SCSP->Slots[slot].datab[addr]) = val;
		SCSP_UpdateSlotReg(slot,addr&0x1f);
		SCSP_UpdateSlotReg(slot,(addr&0x1f)+2);
	}
	else if(addr<0x600)
	{
		rotl(val, 16);

		*(unsigned int *) &(SCSP->datab[addr&0xff]) = val;
		SCSP_UpdateReg(addr&0xff);
		SCSP_UpdateReg((addr&0xff)+2);
	}	
	else if(addr<0x700)
		int a=1;
	else
	{
#ifdef USEDSP
		//DSP
		rotl(val, 16);
			if(addr<0x780)	//COEF
				*(unsigned int *) &(SCSP->DSP.COEF[(addr-0x700)/2])=val;
			else if(addr<0x800)
				*(unsigned int *) &(SCSP->DSP.MADRS[(addr-0x780)/2])=val;
			else if(addr<0xC00)
				*(unsigned int *) &(SCSP->DSP.MPRO[(addr-0x800)/2])=val;
			else
				int a=1;
			if(addr==0xBFC)
				SCSPDSP_Start(&SCSP->DSP);
			int a=1;
#endif
	}
}

unsigned char SCSP_r8(unsigned int addr)
{
	unsigned char v=0;
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		SCSP_UpdateSlotRegR(slot,(addr^1)&0x1f);
		
		v=*(unsigned char *) &(SCSP->Slots[slot].datab[addr^1]);
		//DebugLog("Slot %02X Reg %02X Read byte %02X",slot,addr^1,v);
	}
	else if(addr<0x600)
	{
		SCSP_UpdateRegR(addr&0xff);
		v= *(unsigned char *) &(SCSP->datab[(addr&0xff)^1]);
		//ErrorLogMessage("SCSP Reg %02X Read byte %02X",addr&0xff,v);		
	}	
	else if(addr<0x700)
		v=0;
	return v;
}

unsigned short SCSP_r16(unsigned int addr)
{
	unsigned short v=0;
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		SCSP_UpdateSlotRegR(slot,addr&0x1f);
		v=*(unsigned short *) &(SCSP->Slots[slot].datab[addr]);
		//DebugLog("Slot %02X Reg %02X Read word %04X",slot,addr,v);
	}
	else if(addr<0x600)
	{
		SCSP_UpdateRegR(addr&0xff);
		v= *(unsigned short *) &(SCSP->datab[addr&0xff]);
		//ErrorLogMessage("SCSP Reg %02X Read word %04X",addr&0xff,v);
	}	
	else if(addr<0x700)
		v=SCSP->RINGBUF[(addr-0x600)/2];
	return v;
}

unsigned int SCSP_r32(unsigned int addr)
{
	return 0xffffffff;
}

#define REVSIGN(v) ((~v)+1)


void SCSP_TimersAddTicks2(int ticks)
{
		//Update timers
		WORD cnt;
		WORD step;
		//Timer A
		
		if(!TimPris[0])
		{
			//cnt=SCSPs[0].data[0x18/2]&0xff;
			cnt=TimCnt[0];
			if(cnt==0xffff)
				goto noTA;
			++cnt;
			++TimCnt[0];
			if(cnt>=TIMER_LIMITSA)
			{
				/*if((SCSPs[0].data[0x20/2]&SCSPs[0].data[0x1e/2])&0x40)	//timer pending ack
					int a=1;*/
				SCSPs[0].data[0x20/2]|=0x40;
				/*if(SCSP.data[0x1e/2]&0x40)
					Int68kCB(IrqTimA);*/
				cnt=0xff;
				TimCnt[0]=0xffff;
			}
			step=1<<((SCSPs[0].data[0x18/2]>>8)&0x7);
			TimPris[0]=step;
			SCSPs[0].data[0x18/2]&=0xff00;
			SCSPs[0].data[0x18/2]|=cnt;
		}
//		else
			TimPris[0]--;
noTA:
;
		//Timer B
		
		if(!TimPris[1])
		{
			//cnt=SCSPs[0].data[0x1a/2]&0xff;
			cnt=TimCnt[1];
			if(cnt==0xffff)
				goto noTB;
			++cnt;
			++TimCnt[1];
			if(cnt>=TIMER_LIMITSB)
			{
				/*if((SCSP.data[0x20/2]&SCSP.data[0x1e/2])&0x80)	//timer pending ack
					int a=1;*/
				SCSPs[0].data[0x20/2]|=0x80;
				/*if(SCSP.data[0x1e/2]&0x80)
					Int68kCB(IrqTimBC);*/
				cnt=0xff;
				TimCnt[1]=0xffff;
			}
			step=1<<((SCSP[0].data[0x1a/2]>>8)&0x7);
			TimPris[1]=step;
			SCSPs[0].data[0x1a/2]&=0xff00;
			SCSPs[0].data[0x1a/2]|=cnt;
		}
//		else
			TimPris[1]--;
noTB:
;
		//Timer C
		
		if(!TimPris[2])
		{
			//cnt=SCSPs[0].data[0x1c/2]&0xff;
			cnt=TimCnt[2];
			if(cnt==0xffff)
				goto noTC;
			++cnt;
			++TimCnt[2];
			if(cnt>=TIMER_LIMITSC)
			{
				/*if((SCSP.data[0x20/2]&SCSP.data[0x1e/2])&0x100)	//timer pending ack
					int a=1;*/
				SCSP[0].data[0x20/2]|=0x100;
				/*if(SCSP.data[0x1e/2]&0x100)
					Int68kCB(IrqTimBC);*/
				cnt=0xff;
				TimCnt[2]=0xffff;
			}
			step=1<<((SCSPs[0].data[0x1c/2]>>8)&0x7);
			TimPris[2]=step;
			SCSPs[0].data[0x1c/2]&=0xff00;
			SCSPs[0].data[0x1c/2]|=cnt;
		}
//		else
			TimPris[2]--;
noTC:
;
}

void SCSP_TimersAddTicks(int ticks)
{
	if(TimCnt[0]<=0xff00)
	{
		TimCnt[0]+=ticks << (8-((SCSPs[0].data[0x18/2]>>8)&0x7));
		if (TimCnt[0] > 0xFE00)
		{
			TimCnt[0] = 0xFFFF;
			SCSPs[0].data[0x20/2]|=0x40;
		}
		SCSPs[0].data[0x18/2]&=0xff00;
		SCSPs[0].data[0x18/2]|=TimCnt[0]>>8;
	}
	if(TimCnt[1]<=0xff00)
	{
		TimCnt[1]+=ticks << (8-((SCSPs[0].data[0x1a/2]>>8)&0x7));
		if (TimCnt[1] > 0xFE00)
		{
			TimCnt[1] = 0xFFFF;
			SCSPs[0].data[0x20/2]|=0x80;
		}
		SCSPs[0].data[0x1a/2]&=0xff00;
		SCSPs[0].data[0x1a/2]|=TimCnt[1]>>8;

	}
	if(TimCnt[2]<=0xff00)
	{
		TimCnt[2]+=ticks << (8-((SCSPs[0].data[0x1c/2]>>8)&0x7));
		if (TimCnt[2] > 0xFE00)
		{
			TimCnt[2] = 0xFFFF;
			SCSPs[0].data[0x20/2]|=0x100;
		}
		SCSPs[0].data[0x1c/2]&=0xff00;
		SCSPs[0].data[0x1c/2]|=TimCnt[2]>>8;
	}

}


#ifdef NEWSCSP

#ifdef USEDSP
const bool hasDSP=true;

#else
const bool hasDSP=false;
#endif
signed short *bufmix;

signed int *bufl1,*bufr1;
#define SCSPNAME(_8bit,lfo,alfo,loop) \
void SCSP_Update##_8bit##lfo##alfo##loop(_SLOT *slot,unsigned int Enc,unsigned int nsamples)

//TRUST ON THE COMPILER OPTIMIZATIONS
#define SCSPTMPL(_8bit,lfo,alfo,loop) \
SCSPNAME(_8bit,lfo,alfo,loop)\
{\
	signed int sample;\
	DWORD addr;\
	for(unsigned int s=0;s<nsamples;++s)\
	{\
		int step=slot->step;\
		if(!slot->active)\
			return;\
		if(lfo) \
		{\
			step=step*PLFO_Step(&(slot->PLFO));\
			step>>=SHIFT; \
		}\
		if(_8bit)\
		{\
			unsigned int offs=(slot->cur_addr>>SHIFT);\
			signed char *p=(signed char *) (slot->base);\
			int s;\
			signed int fpart=slot->cur_addr&((1<<SHIFT)-1);\
			s=(int) p[offs^1]*((1<<SHIFT)-fpart)+(int) p[(offs+1)^1]*fpart;\
			sample=(s>>SHIFT)<<8;\
		}\
		else\
		{\
			signed short *p=(signed short *) &(slot->base[(slot->cur_addr>>(SHIFT-1))&(~1)]);\
			signed int fpart=slot->cur_addr&((1<<SHIFT)-1);\
			sample=(p[0]);\
		}\
		if(loop==0)\
		{\
			slot->cur_addr+=step;\
			addr=slot->cur_addr>>SHIFT;\
			if(addr>LEA(slot))\
			{\
				SCSP_StopSlot(slot,0);\
			}\
		}\
		if(loop==1)\
		{\
			slot->cur_addr+=step;\
			addr=slot->cur_addr>>SHIFT;\
			if(addr>LEA(slot))\
				slot->cur_addr=(LSA(slot)+1)<<SHIFT;\
		}\
		if(loop==2)\
		{\
			if(slot->Back)\
				slot->cur_addr+=REVSIGN(step);\
			else\
				slot->cur_addr+=step;\
			addr=slot->cur_addr>>SHIFT;\
			if(addr>=LEA(slot))\
			{\
				slot->cur_addr=LEA(slot)<<SHIFT;\
				slot->Back=1;\
			}\
			if((addr<LSA(slot) || (addr&0x80000000)) && slot->Back)\
				slot->cur_addr=LEA(slot)<<SHIFT;\
		}\
		if(loop==3)\
		{\
			if(slot->Back)\
				slot->cur_addr+=REVSIGN(step);\
			else\
				slot->cur_addr+=step;\
			addr=slot->cur_addr>>SHIFT;\
			if(addr>=LEA(slot)) /*reached end, reverse till start*/ \
			{\
				slot->cur_addr=LEA(slot)<<SHIFT;\
				slot->Back=1;\
			}\
			if((addr<=LSA(slot) || (addr&0x80000000)) && slot->Back) /*reached start or negative*/\
			{\
				slot->cur_addr=LSA(slot)<<SHIFT;\
				slot->Back=0;\
			}\
		}\
		if(alfo)\
		{\
			sample=sample*ALFO_Step(&(slot->ALFO));\
			sample>>=SHIFT;\
		}\
		*RBUFDST=sample;\
		\
		sample=(sample*EG_Update(slot))>>SHIFT;\
		if(hasDSP)\
			*bufmix++=((sample*LPANTABLE[(Enc|0xE0)&0xFFE0])>>(SHIFT+3));\
	\
		*bufl1=*bufl1 + ((sample*LPANTABLE[Enc])>>SHIFT);\
		*bufr1=*bufr1 + ((sample*RPANTABLE[Enc])>>SHIFT);\
		++bufl1;\
		++bufr1;\
	}\
}

SCSPTMPL(0,0,0,0) SCSPTMPL(0,0,0,1) SCSPTMPL(0,0,0,2) SCSPTMPL(0,0,0,3)
SCSPTMPL(0,0,1,0) SCSPTMPL(0,0,1,1) SCSPTMPL(0,0,1,2) SCSPTMPL(0,0,1,3)
SCSPTMPL(0,1,0,0) SCSPTMPL(0,1,0,1) SCSPTMPL(0,1,0,2) SCSPTMPL(0,1,0,3)
SCSPTMPL(0,1,1,0) SCSPTMPL(0,1,1,1) SCSPTMPL(0,1,1,2) SCSPTMPL(0,1,1,3)
SCSPTMPL(1,0,0,0) SCSPTMPL(1,0,0,1) SCSPTMPL(1,0,0,2) SCSPTMPL(1,0,0,3)
SCSPTMPL(1,0,1,0) SCSPTMPL(1,0,1,1) SCSPTMPL(1,0,1,2) SCSPTMPL(1,0,1,3)
SCSPTMPL(1,1,0,0) SCSPTMPL(1,1,0,1) SCSPTMPL(1,1,0,2) SCSPTMPL(1,1,0,3)
SCSPTMPL(1,1,1,0) SCSPTMPL(1,1,1,1) SCSPTMPL(1,1,1,2) SCSPTMPL(1,1,1,3)

#undef SCSPTMPL
#define SCSPTMPL(_8bit,lfo,alfo,loop) \
 SCSP_Update##_8bit##lfo##alfo##loop ,


typedef void (*_SCSPUpdateModes)(_SLOT *,unsigned int,unsigned int);

_SCSPUpdateModes SCSPUpdateModes[]=
{
	SCSPTMPL(0,0,0,0) SCSPTMPL(0,0,0,1) SCSPTMPL(0,0,0,2) SCSPTMPL(0,0,0,3)
	SCSPTMPL(0,0,1,0) SCSPTMPL(0,0,1,1) SCSPTMPL(0,0,1,2) SCSPTMPL(0,0,1,3)
	SCSPTMPL(0,1,0,0) SCSPTMPL(0,1,0,1) SCSPTMPL(0,1,0,2) SCSPTMPL(0,1,0,3)
	SCSPTMPL(0,1,1,0) SCSPTMPL(0,1,1,1) SCSPTMPL(0,1,1,2) SCSPTMPL(0,1,1,3)
	SCSPTMPL(1,0,0,0) SCSPTMPL(1,0,0,1) SCSPTMPL(1,0,0,2) SCSPTMPL(1,0,0,3)
	SCSPTMPL(1,0,1,0) SCSPTMPL(1,0,1,1) SCSPTMPL(1,0,1,2) SCSPTMPL(1,0,1,3)
	SCSPTMPL(1,1,0,0) SCSPTMPL(1,1,0,1) SCSPTMPL(1,1,0,2) SCSPTMPL(1,1,0,3)
	SCSPTMPL(1,1,1,0) SCSPTMPL(1,1,1,1) SCSPTMPL(1,1,1,2) SCSPTMPL(1,1,1,3)

};

#define SCANLINES	210

void SCSP_CpuRunScanline()
{
	int slice=12000000/(44100);
	static unsigned int smp=0;
	smp+=(unsigned int) ((256.0*44100.0)/((float) SCANLINES*SysFPS));
	int lastdiff=0;
	for(;smp&0xffffff00;)
	{
		lastdiff=Run68kCB(slice+lastdiff);
		/*while(slice>0)
		{
			lastdiff=Run68kCB(1);
			slice+=lastdiff;
			if(M68000_regs.pc==0x0602620)
				int a=1;
		}*/
		SCSP_TimersAddTicks(1);
		CheckPendingIRQ();
		smp-=0x100;
	}
}

void SCSP_DoMasterSamples(int nsamples)
{

	static int lastdiff=0;
	signed short *bufr,*bufl;
	
	
	for(int sl=0;sl<32;++sl)
	{
		bufr1=buffertmpr;
		bufl1=buffertmpl;
//		if(sl!=0x3)
//			continue;

		if(SCSPs[0].Slots[sl].active)
		{
			_SLOT *slot=SCSPs[0].Slots+sl;
			unsigned int disdl=DISDL(slot);
			unsigned int efsdl=EFSDL(slot);
			unsigned int tl=TL(slot);
			unsigned short Enc=((TL(slot))<<0x8)|((DIPAN(slot))<<0x0)|((DISDL(slot))<<0x5);
			//unsigned short Enc=(0x00)|((DIPAN(slot))<<0x8)|((0x7)<<0xd);
			unsigned int mode=LPCTL(slot);

/*			if(SSCTL(slot)!=0)	//no FM or noise yet
				int a=1;
			int MDX=MDXSL(slot);
			int MDY=MDYSL(slot);
			int LEV=MDL(slot);
			if(LEV!=0 || MDX!=0 || MDY!=0)
				goto norender;
			if(SBCTL(slot))
				int a=1;

*/
			RBUFDST=SCSPs[0].RINGBUF+SCSPs[0].BUFPTR;
			if(sl==0x15)
				int a=1;
			if(PLFOS(slot))
				mode|=8;
			if(ALFOS(slot))
				mode|=4;
			if(PCM8B(slot))
				mode|=0x10;
#ifdef USEDSP
			bufmix=SCSPs[0].MIXBuf+0x300*slot->slot;
#endif

			SCSPUpdateModes[mode](slot,Enc,nsamples);
//norender:
//;
			
		}
		++SCSPs[0].BUFPTR;
		SCSPs[0].BUFPTR&=63;
	}
	if(HasSlaveSCSP)
	{
		for(int sl=0;sl<32;++sl)
		{
			bufr1=buffertmpr;
			bufl1=buffertmpl;
			if(SCSPs[1].Slots[sl].active)
			{
				_SLOT *slot=SCSPs[1].Slots+sl;
				//unsigned short Enc=((TL(slot))<<0x0)|((DIPAN(slot))<<0x8)|((0x7)<<0xd);
				unsigned short Enc=((TL(slot))<<0x8)|((DIPAN(slot))<<0x0)|((DISDL(slot))<<0x5);
				unsigned int mode=LPCTL(slot);
				
				if(PLFOS(slot))
					mode|=8;
				if(ALFOS(slot))
					mode|=4;
				if(PCM8B(slot))
					mode|=0x10;
				RBUFDST=SCSPs[1].RINGBUF+SCSPs[1].BUFPTR;
#ifdef USEDSP
				bufmix=SCSPs[1].MIXBuf+0x300*slot->slot;
#endif
				SCSPUpdateModes[mode](slot,Enc,nsamples);
				
			}
			++SCSPs[1].BUFPTR;
			SCSPs[1].BUFPTR&=63;
		}
	}
	bufr=bufferr;
	bufl=bufferl;
	bufr1=buffertmpr;
	bufl1=buffertmpl;
	for(int s=0;s<nsamples;++s)
	{
#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)
		signed int smpl=*bufl1;
		signed int smpr=*bufr1;
#ifdef USEDSP
		signed short *pt=SCSPs[0].MIXBuf+s;
		for(int sl=0;sl<32;++sl)
		{
			_SLOT *slot=SCSPs[0].Slots+sl;
			if(slot->active)
			{
				SCSPDSP_SetSample(&SCSP[0].DSP,pt[0],ISEL(slot),IMXL(slot));
			}
			pt+=0x300;
		}
		SCSPDSP_Step(&SCSP[0].DSP);
		if(HasSlaveSCSP)
		{
			pt=SCSPs[1].MIXBuf+s;
			for(int sl=0;sl<32;++sl)
			{
				_SLOT *slot=SCSPs[1].Slots+sl;
				if(slot->active)
				{
					SCSPDSP_SetSample(&SCSP[1].DSP,pt[0],ISEL(slot),IMXL(slot));
				}
				pt+=0x300;
			}
			SCSPDSP_Step(&SCSP[1].DSP);
		}

		//		smpl=0;
		//		smpr=0;
		for(int i=0;i<16;++i)
		{
			_SLOT *slot=SCSPs[0].Slots+i;
			int ef=EFSDL(slot);
			if(ef)
			{
				unsigned short Enc=0|((EFPAN(slot))<<0x0)|((EFSDL(slot))<<0x5);
				smpl+=(SCSPs[0].DSP.EFREG[i]*LPANTABLE[Enc])>>SHIFT;
				smpr+=(SCSPs[0].DSP.EFREG[i]*RPANTABLE[Enc])>>SHIFT;
			}
			
			if(HasSlaveSCSP)
			{
				_SLOT *slot=SCSPs[1].Slots+i;
				ef=EFSDL(slot);
				if(ef)
				{
					unsigned short Enc=0|((EFPAN(slot))<<0x0)|((EFSDL(slot))<<0x5);
					smpl+=(SCSPs[1].DSP.EFREG[i]*LPANTABLE[Enc])>>SHIFT;
					smpr+=(SCSPs[1].DSP.EFREG[i]*RPANTABLE[Enc])>>SHIFT;
				}
			}
		}
#endif
#ifdef REVERB
		smpl+=bufferrevl[RevR];
		smpr+=bufferrevr[RevR];
		bufferrevl[RevW]=((smpl<<0)/REVERB_DIV)>>0;
		bufferrevr[RevW]=((smpr<<0)/REVERB_DIV)>>0;
		++RevW;
		if(RevW==REVERB_LEN)
			RevW=0;
		++RevR;
		if(RevR==REVERB_LEN)
			RevR=0;
#endif
		*bufl=ICLIP16(smpl);
		*bufr=ICLIP16(smpr);
		*bufl1=0;
		*bufr1=0;
		++bufl;
		++bufr;
		++bufl1;
		++bufr1;

	}
}


#else

signed int inline SCSP_UpdateSlot(_SLOT *slot)
{
	signed int sample;
	int step=slot->step;
	DWORD addr;

	if(SSCTL(slot)!=0)	//no FM or noise yet
		return 0;
	
	if(PLFOS(slot)!=0)
	{
		step=step*PLFO_Step(&(slot->PLFO));
		step>>=SHIFT;
	}

	if(PCM8B(slot))
		addr=slot->cur_addr>>SHIFT;
	else
		addr=(slot->cur_addr>>(SHIFT-1))&(~1);

	if(MDL(slot)!=0 || MDXSL(slot)!=0 || MDYSL(slot)!=0)
	{
//TODO: is this correct? SCSP is not necessarily set to the slave SCSP. ElSemi may not have noticed this.
		unsigned char v;
		signed int smp=(SCSP->RINGBUF[(SCSP->BUFPTR+MDXSL(slot))&63]+SCSP->RINGBUF[(SCSP->BUFPTR+MDYSL(slot))&63])/2;
		
		smp>>=11;
		// Check for underflow before adding to addr
		if (smp >= 0 || (DWORD)(-smp) < addr)
			addr+=smp;
		else 
			addr = 0;
		if(!PCM8B(slot))
			addr&=~1;
	}

	if(PCM8B(slot))	//8 bit signed
	{	
		signed char *p=(signed char *) &(slot->base[addr^1]);
		int s;
		signed int fpart=slot->cur_addr&((1<<SHIFT)-1);
		sample=(p[0])<<8;
		
		//if (p>=(signed char *) &SCSP->SCSPRAM[0x200000])
		//	printf("%X %X %X %p %p %p\n", addr, SA(slot), LEA(slot), p, slot->base, SCSP->SCSPRAM);
		
		/*s=(int) p[0]*((1<<SHIFT)-fpart)+(int) p[1]*fpart;
		sample=(s>>SHIFT)<<8;
		*/
/*		if(SBCTL(slot)&1)	//reverse data
			sample^=0x7f;
		if(SBCTL(slot)&2)	//reverse sign
			sample^=0x80;
*/		
	}
	else	//16 bit signed (endianness?)
	{
		signed short *p=(signed short *) &(slot->base[addr]);
		int s;
		signed int fpart=slot->cur_addr&((1<<SHIFT)-1);
		sample=(p[0]);
		//sample=((p[0]>>8)&0xFF)|(p[0]<<8);
		//s=(int) p[0]*((1<<SHIFT)-fpart)+(int) p[1]*fpart;
		//sample=s>>SHIFT;

		/*		if(SBCTL(slot)&1)	//reverse data
		sample^=0x7fff;
		if(SBCTL(slot)&2)	//reverse sign
		sample^=0x8000;
		*/
	}

	
	switch(LPCTL(slot))
	{
	case 0:	//no loop
		slot->cur_addr+=step;
		addr=slot->cur_addr>>SHIFT;
		if(addr>LEA(slot))
		{
			//slot->active=0;
			SCSP_StopSlot(slot,0);
		}
		break;
	case 1: //normal loop
		slot->cur_addr+=step;
		addr=slot->cur_addr>>SHIFT;
		if(addr>LEA(slot))
			slot->cur_addr=LSA(slot)<<SHIFT;
		break;
	case 2:	//reverse loop
		if(slot->Back)
			slot->cur_addr+=REVSIGN(step);
		else
			slot->cur_addr+=step;
		addr=slot->cur_addr>>SHIFT;
		if(addr>LEA(slot))
		{
			slot->cur_addr=LEA(slot)<<SHIFT;
			slot->Back=1;
		}
		if((addr<LSA(slot) || (addr&0x80000000)) && slot->Back)
			slot->cur_addr=LEA(slot)<<SHIFT;
		break;
	case 3: //ping-pong
		if(slot->Back)
			slot->cur_addr+=REVSIGN(step);
		else
			slot->cur_addr+=step;
		addr=slot->cur_addr>>SHIFT;
		if(addr>LEA(slot)) //reached end, reverse till start
		{
			slot->cur_addr=LEA(slot)<<SHIFT;
			//slot->step=REVSIGN(slot->step);
			slot->Back=1;
		}
		if((addr<LSA(slot) || (addr&0x80000000)) && (slot->Back)) //reached start or negative
		{
			slot->cur_addr=LSA(slot)<<SHIFT;
			//slot->step=REVSIGN(slot->step);
			slot->Back=0;
		}
		break;
	}

	if(ALFOS(slot)!=0)
	{
		sample=sample*ALFO_Step(&(slot->ALFO));
		sample>>=SHIFT;
	}

	if(!STWINH(slot))
		*RBUFDST=sample;
	else 
		int a=1;

	sample=(sample*EG_Update(slot))>>SHIFT;

	return sample;
}

void SCSP_CpuRunScanline()
{

}

void SCSP_DoMasterSamples(int nsamples)
{
	int slice=12000000/(SysFPS*nsamples);	// 68K cycles/sample
	static int lastdiff=0;
	
	for(int s=0;s<nsamples;++s)
	{
		signed int smpl=0;
		signed int smpr=0;

		for(int sl=0;sl<32;++sl)
		{
			if(SCSPs[0].Slots[sl].active)
			{
				_SLOT *slot=SCSPs[0].Slots+sl;
				unsigned short Enc=((TL(slot))<<0x8)|((DIPAN(slot))<<0x0)|((DISDL(slot))<<0x5);
				RBUFDST=SCSPs[0].RINGBUF+SCSPs[0].BUFPTR;
				signed int sample;
				//signed int sample=SCSP_UpdateSlot(slot);
				//if(SA(slot)!=0x2ccf4)
				/*if(SA(slot)!=0x1c77e)
					sample=0;
				else*/
					sample=SCSP_UpdateSlot(slot);

				
				/*unsigned char ef=EFSDL(slot);
				ef+=DISDL(slot);
				if(ef>0xf) ef=0xf;
				unsigned short Enc=((TL(slot))<<0x0)|((DIPAN(slot))<<0x8)|((ef)<<0xd);
				*/
#ifdef USEDSP
				SCSPDSP_SetSample(&SCSPs[0].DSP,/*sample>>5*/(sample*LPANTABLE[(Enc|0xE0)&0xFFE0])>>(SHIFT+3)/*>>SHIFT*/,ISEL(slot),IMXL(slot));
#endif

#ifdef RB_VOLUME
				smpl += (sample * volume[TL(slot)+pan_left [DIPAN(slot)]])>>17;
				smpr += (sample * volume[TL(slot)+pan_right[DIPAN(slot)]])>>17;
#else				

				//if(sl==cnts)
				{
					smpl+=(sample*LPANTABLE[Enc])>>SHIFT;
					smpr+=(sample*RPANTABLE[Enc])>>SHIFT;
				}
#endif
			}
			++SCSPs[0].BUFPTR;
			SCSPs[0].BUFPTR&=63;
			if(HasSlaveSCSP)
			{
				if(SCSPs[1].Slots[sl].active)
				{
					_SLOT *slot=SCSPs[1].Slots+sl;
					unsigned short Enc=((TL(slot))<<0x8)|((DIPAN(slot))<<0x0)|((DISDL(slot))<<0x5);
					RBUFDST=SCSPs[1].RINGBUF+SCSPs[1].BUFPTR;
					signed int sample=SCSP_UpdateSlot(slot);
#ifdef USEDSP
					SCSPDSP_SetSample(&SCSPs[1].DSP,(sample*LPANTABLE[(Enc|0xE0)&0xFFE0])>>(SHIFT+3),ISEL(slot),IMXL(slot));
#endif
#ifdef RB_VOLUME
					smpl += (sample * volume[TL(slot)+pan_left [DIPAN(slot)]])>>17;
					smpr += (sample * volume[TL(slot)+pan_right[DIPAN(slot)]])>>17;
#else				
					smpl+=(sample*LPANTABLE[Enc])>>SHIFT;
					smpr+=(sample*RPANTABLE[Enc])>>SHIFT;
#endif
				}
				++SCSPs[1].BUFPTR;
				SCSPs[1].BUFPTR&=63;
			}
		}
#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)
#ifdef USEDSP
		SCSPDSP_Step(&SCSPs[0].DSP);
		if(HasSlaveSCSP)
			SCSPDSP_Step(&SCSPs[1].DSP);

//		smpl=0;
//		smpr=0;
		for(int i=0;i<16;++i)
		{
			_SLOT *slot=SCSPs[0].Slots+i;
			int ef=EFSDL(slot);
			if(ef)
			{
				unsigned short Enc=0|((EFPAN(slot))<<0x0)|((EFSDL(slot))<<0x5);
				smpl+=(SCSPs[0].DSP.EFREG[i]*LPANTABLE[Enc])>>SHIFT;
				smpr+=(SCSPs[0].DSP.EFREG[i]*RPANTABLE[Enc])>>SHIFT;
			}
			if(HasSlaveSCSP)
			{
				_SLOT *slot=SCSPs[1].Slots+i;
				ef=EFSDL(slot);
				if(ef)
				{
					unsigned short Enc=0|((EFPAN(slot))<<0x0)|((EFSDL(slot))<<0x5);
					smpl+=(SCSPs[1].DSP.EFREG[i]*LPANTABLE[Enc])>>SHIFT;
					smpr+=(SCSPs[1].DSP.EFREG[i]*RPANTABLE[Enc])>>SHIFT;
				}
			}
		}
#endif
#ifdef REVERB
		smpl+=bufferrevl[RevR];
		smpr+=bufferrevr[RevR];
		bufferrevl[RevW]=((smpl<<0)/REVERB_DIV)>>0;
		bufferrevr[RevW]=((smpr<<0)/REVERB_DIV)>>0;
		++RevW;
		if(RevW==REVERB_LEN)
			RevW=0;
		++RevR;
		if(RevR==REVERB_LEN)
			RevR=0;
#endif
		if(smpl<-32768)
			smpl=-32768;
		else if(smpl>32767)
			smpl=32767;
		bufferl[s]=smpl;
		//bufferl[s]=ICLIP16(smpl);
		bufferr[s]=ICLIP16(smpr);

		SCSP_TimersAddTicks(1);
		CheckPendingIRQ();

		/*for(int nc=slice;nc;nc--)
		{
		Run68kCB(1);		
		if(M68000_regs.pc==0x6035A6)
		{

		int a=1;
		}
		}
		*/


		lastdiff=Run68kCB(slice-lastdiff);
	}
}
#endif

void SCSP_Update()
{
	SCSP_DoMasterSamples(length);
}

void SCSP_SetCB(int (*Run68k)(int cycles),void (*Int68k)(int irq))
{
	Int68kCB=Int68k;
	Run68kCB=Run68k;
}

void SCSP_MidiIn(BYTE val)
{
	/*
	 * MIDI FIFO critical section
	 */
	if (g_Config.multiThreaded)
		MIDILock->Lock();
		
	//DebugLog("Midi Buffer push %02X",val);
	MidiStack[MidiW++]=val;
	MidiW&=MIDI_STACK_SIZE_MASK;
	MidiInFill++;
	//Int68kCB(IrqMidi);
//	SCSP.data[0x20/2]|=0x8;

	if (g_Config.multiThreaded)
		MIDILock->Unlock();
}

void SCSP_MidiOutW(BYTE val)
{
	/*
	 * MIDI FIFO critical section
	 */
	if (g_Config.multiThreaded)
		MIDILock->Lock();

	//printf("68K: MIDI out\n");
	//DebugLog("Midi Out Buffer push %02X",val);
	MidiStack[MidiOutW++]=val;
	MidiOutW&=7;
	++MidiOutFill;
	
	if (g_Config.multiThreaded)
		MIDILock->Unlock();
}


unsigned char SCSP_MidiOutR()
{
	unsigned char val;

	if(MidiOutR==MidiOutW)	// I don't think this needs to be a critical section...
		return 0xff;
		
	/*
	 * MIDI FIFO critical section
	 */
	if (g_Config.multiThreaded)
		MIDILock->Lock();

	val=MidiStack[MidiOutR++];
	//DebugLog("Midi Out Buffer pop %02X",val);
	MidiOutR&=7;
	--MidiOutFill;
	
	if (g_Config.multiThreaded)
		MIDILock->Unlock();
		
	return val;
}

unsigned char SCSP_MidiOutFill()
{
	unsigned char v;
	
	/*
	 * MIDI FIFO critical section
	 */
	if (g_Config.multiThreaded)
		MIDILock->Lock();

	v = MidiOutFill;
	
	if (g_Config.multiThreaded)
		MIDILock->Unlock();
	
	return v;
}

unsigned char SCSP_MidiInFill()
{
	unsigned char v;
	
	/*
	 * MIDI FIFO critical section
	 */
	if (g_Config.multiThreaded)
		MIDILock->Lock();

	v = MidiInFill;
	
	if (g_Config.multiThreaded)
		MIDILock->Unlock();
	
	return v;
}

void SCSP_RTECheck()
{
/*	unsigned short pend=SCSP.data[0x20/2]&0xfff;
	if(pend)
	{
		if(pend&0x40)
		{
			Int68kCB(IrqTimA);
			return;
		}
		if(pend&(0x80|0x100))
		{
			Int68kCB(IrqTimBC);
			return;
		}
		if(pend&0x8)
			Int68kCB(IrqMidi);
	}
*/
}

int SCSP_IRQCB(int)
{
	CheckPendingIRQ();
	return -1;
}

void SCSP_Master_w8(unsigned int addr,unsigned char val)
{
	SCSP=SCSPs+0;
	SCSP_w8(addr,val);
}

void SCSP_Master_w16(unsigned int addr,unsigned short val)
{
	SCSP=SCSPs+0;
	SCSP_w16(addr,val);
}

void SCSP_Master_w32(unsigned int addr,unsigned int val)
{
	SCSP=SCSPs+0;
	SCSP_w32(addr,val);
}

void SCSP_Slave_w8(unsigned int addr,unsigned char val)
{
	SCSP=SCSPs+1;
	SCSP_w8(addr,val);
}

void SCSP_Slave_w16(unsigned int addr,unsigned short val)
{
	SCSP=SCSPs+1;
	SCSP_w16(addr,val);
}

void SCSP_Slave_w32(unsigned int addr,unsigned int val)
{
	SCSP=SCSPs+1;
	SCSP_w32(addr,val);
}

unsigned char SCSP_Master_r8(unsigned int addr)
{
	SCSP=SCSPs+0;
	return SCSP_r8(addr);
}

unsigned short SCSP_Master_r16(unsigned int addr)
{
	SCSP=SCSPs+0;
	return SCSP_r16(addr);
}

unsigned int SCSP_Master_r32(unsigned int addr)
{
	SCSP=SCSPs+0;
	return SCSP_r32(addr);
}

unsigned char SCSP_Slave_r8(unsigned int addr)
{
	SCSP=SCSPs+1;
	return SCSP_r8(addr);
}

unsigned short SCSP_Slave_r16(unsigned int addr)
{
	SCSP=SCSPs+1;
	return SCSP_r16(addr);
}

unsigned int SCSP_Slave_r32(unsigned int addr)
{
	SCSP=SCSPs+1;
	return SCSP_r32(addr);
}


/******************************************************************************
 Supermodel Interface Functions
******************************************************************************/

void SCSP_SaveState(CBlockFile *StateFile)
{
	StateFile->NewBlock("SCSP x 2", __FILE__);
	
	/*
	 * Save global variables.
	 *
	 * Difficult to say exactly what is necessary given that many things are
	 * commented out and should not be enabled but I try to save as much as
	 * possible. 
	 *
	 * Things not saved:
	 *
	 *	- Reverb buffers and pointers
	 *	- FNS table (populated by SCSP_Init() and only read)
	 *	- RB_VOLUME stuff
	 * 	- ARTABLE, DRTABLE
	 *	- RBUFDST
	 */
	StateFile->Write(&IrqTimA, sizeof(IrqTimA));
	StateFile->Write(&IrqTimBC, sizeof(IrqTimBC));
	StateFile->Write(&IrqMidi, sizeof(IrqMidi));
	StateFile->Write(MidiOutStack, sizeof(MidiOutStack));
	StateFile->Write(&MidiOutW, sizeof(MidiOutW));
	StateFile->Write(&MidiOutR, sizeof(MidiOutR));
	StateFile->Write(MidiStack, sizeof(MidiStack));
	StateFile->Write(&MidiOutFill, sizeof(MidiOutFill));
	StateFile->Write(&MidiInFill, sizeof(MidiInFill));
	StateFile->Write(&MidiW, sizeof(MidiW));
	StateFile->Write(&MidiR, sizeof(MidiR));
	StateFile->Write(TimPris, sizeof(TimPris));
	StateFile->Write(TimCnt, sizeof(TimCnt));
	
	// Save both SCSP states
	for (int i = 0; i < 2; i++)
	{
		StateFile->Write(SCSPs[i].datab, sizeof(SCSPs[i].datab));
		StateFile->Write(&(SCSPs[i].BUFPTR), sizeof(SCSPs[i].BUFPTR));
		StateFile->Write(&(SCSPs[i].Master), sizeof(SCSPs[i].Master));
		
		// Save each slot
		for (int j = 0; j < 32; j++)
		{
			UINT64	baseOffset;
			UINT8	egState;
			
			StateFile->Write(SCSPs[i].Slots[j].datab, sizeof(SCSPs[i].Slots[j].datab));
			StateFile->Write(&(SCSPs[i].Slots[j].active), sizeof(SCSPs[i].Slots[j].active));
			baseOffset = (UINT64) (SCSPs[i].Slots[j].base - SCSPs[i].SCSPRAM);
			StateFile->Write(&baseOffset, sizeof(baseOffset));
			StateFile->Write(&(SCSPs[i].Slots[j].cur_addr), sizeof(SCSPs[i].Slots[j].cur_addr));
			StateFile->Write(&(SCSPs[i].Slots[j].step), sizeof(SCSPs[i].Slots[j].step));
			StateFile->Write(&(SCSPs[i].Slots[j].Back), sizeof(SCSPs[i].Slots[j].Back));
			StateFile->Write(&(SCSPs[i].Slots[j].slot), sizeof(SCSPs[i].Slots[j].slot));
			StateFile->Write(&(SCSPs[i].Slots[j].Prev), sizeof(SCSPs[i].Slots[j].Prev));
			
			// EG
			StateFile->Write(&(SCSPs[i].Slots[j].EG.volume), sizeof(SCSPs[i].Slots[j].EG.volume));
			egState = SCSPs[i].Slots[j].EG.state;
			StateFile->Write(&egState, sizeof(egState));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.step), sizeof(SCSPs[i].Slots[j].EG.step));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.AR), sizeof(SCSPs[i].Slots[j].EG.AR));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.D1R), sizeof(SCSPs[i].Slots[j].EG.D1R));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.D2R), sizeof(SCSPs[i].Slots[j].EG.D2R));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.RR), sizeof(SCSPs[i].Slots[j].EG.RR));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.DL), sizeof(SCSPs[i].Slots[j].EG.DL));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.EGHOLD), sizeof(SCSPs[i].Slots[j].EG.EGHOLD));
			StateFile->Write(&(SCSPs[i].Slots[j].EG.LPLINK), sizeof(SCSPs[i].Slots[j].EG.LPLINK));
			
			// PLFO
			StateFile->Write(&(SCSPs[i].Slots[j].PLFO.phase), sizeof(SCSPs[i].Slots[j].PLFO.phase));
			StateFile->Write(&(SCSPs[i].Slots[j].PLFO.phase_step), sizeof(SCSPs[i].Slots[j].PLFO.phase_step));
			
			// ALFO
			StateFile->Write(&(SCSPs[i].Slots[j].ALFO.phase), sizeof(SCSPs[i].Slots[j].ALFO.phase));
			StateFile->Write(&(SCSPs[i].Slots[j].ALFO.phase_step), sizeof(SCSPs[i].Slots[j].ALFO.phase_step));
			
			//when loading, make sure to compute lfo
		}
		
		// DSP
		StateFile->Write(&(SCSPs[i].DSP.RBP), sizeof(SCSPs[i].DSP.RBP));
		StateFile->Write(&(SCSPs[i].DSP.RBL), sizeof(SCSPs[i].DSP.RBL));
		StateFile->Write(SCSPs[i].DSP.COEF, sizeof(SCSPs[i].DSP.COEF));
		StateFile->Write(SCSPs[i].DSP.MADRS, sizeof(SCSPs[i].DSP.MADRS));
		StateFile->Write(SCSPs[i].DSP.MPRO, sizeof(SCSPs[i].DSP.MPRO));
		StateFile->Write(SCSPs[i].DSP.TEMP, sizeof(SCSPs[i].DSP.TEMP));
		StateFile->Write(SCSPs[i].DSP.MEMS, sizeof(SCSPs[i].DSP.MEMS));
		StateFile->Write(&(SCSPs[i].DSP.DEC), sizeof(SCSPs[i].DSP.DEC));
		StateFile->Write(SCSPs[i].DSP.MIXS, sizeof(SCSPs[i].DSP.MIXS));
		StateFile->Write(SCSPs[i].DSP.EXTS, sizeof(SCSPs[i].DSP.EXTS));
		StateFile->Write(SCSPs[i].DSP.EFREG, sizeof(SCSPs[i].DSP.EFREG));
		StateFile->Write(&(SCSPs[i].DSP.Stopped), sizeof(SCSPs[i].DSP.Stopped));
		StateFile->Write(&(SCSPs[i].DSP.LastStep), sizeof(SCSPs[i].DSP.LastStep));
	}
}

void SCSP_LoadState(CBlockFile *StateFile)
{
	if (OKAY != StateFile->FindBlock("SCSP x 2"))
	{
		ErrorLog("Unable to load SCSP state. Save state file is corrupt.");
		return;
	}
	
	// Load global variables
	StateFile->Read(&IrqTimA, sizeof(IrqTimA));
	StateFile->Read(&IrqTimBC, sizeof(IrqTimBC));
	StateFile->Read(&IrqMidi, sizeof(IrqMidi));
	StateFile->Read(MidiOutStack, sizeof(MidiOutStack));
	StateFile->Read(&MidiOutW, sizeof(MidiOutW));
	StateFile->Read(&MidiOutR, sizeof(MidiOutR));
	StateFile->Read(MidiStack, sizeof(MidiStack));
	StateFile->Read(&MidiOutFill, sizeof(MidiOutFill));
	StateFile->Read(&MidiInFill, sizeof(MidiInFill));
	StateFile->Read(&MidiW, sizeof(MidiW));
	StateFile->Read(&MidiR, sizeof(MidiR));
	StateFile->Read(TimPris, sizeof(TimPris));
	StateFile->Read(TimCnt, sizeof(TimCnt));
	
	// Load both SCSP states
	for (int i = 0; i < 2; i++)
	{
		StateFile->Read(SCSPs[i].datab, sizeof(SCSPs[i].datab));
		StateFile->Read(&(SCSPs[i].BUFPTR), sizeof(SCSPs[i].BUFPTR));
		StateFile->Read(&(SCSPs[i].Master), sizeof(SCSPs[i].Master));
		
		// Load each slot
		for (int j = 0; j < 32; j++)
		{
			UINT64	baseOffset;
			UINT8	egState;
			
			StateFile->Read(SCSPs[i].Slots[j].datab, sizeof(SCSPs[i].Slots[j].datab));
			StateFile->Read(&(SCSPs[i].Slots[j].active), sizeof(SCSPs[i].Slots[j].active));
			StateFile->Read(&baseOffset, sizeof(baseOffset));
			SCSPs[i].Slots[j].base = &(SCSPs[i].SCSPRAM[baseOffset&0xFFFFF]);	// clamp to 1 MB
			StateFile->Read(&(SCSPs[i].Slots[j].cur_addr), sizeof(SCSPs[i].Slots[j].cur_addr));
			StateFile->Read(&(SCSPs[i].Slots[j].step), sizeof(SCSPs[i].Slots[j].step));
			StateFile->Read(&(SCSPs[i].Slots[j].Back), sizeof(SCSPs[i].Slots[j].Back));
			StateFile->Read(&(SCSPs[i].Slots[j].slot), sizeof(SCSPs[i].Slots[j].slot));
			StateFile->Read(&(SCSPs[i].Slots[j].Prev), sizeof(SCSPs[i].Slots[j].Prev));
			
			// EG
			StateFile->Read(&(SCSPs[i].Slots[j].EG.volume), sizeof(SCSPs[i].Slots[j].EG.volume));
			StateFile->Read(&egState, sizeof(egState));
			SCSPs[i].Slots[j].EG.state = (_STATE) egState;
			StateFile->Read(&(SCSPs[i].Slots[j].EG.step), sizeof(SCSPs[i].Slots[j].EG.step));
			StateFile->Read(&(SCSPs[i].Slots[j].EG.AR), sizeof(SCSPs[i].Slots[j].EG.AR));
			StateFile->Read(&(SCSPs[i].Slots[j].EG.D1R), sizeof(SCSPs[i].Slots[j].EG.D1R));
			StateFile->Read(&(SCSPs[i].Slots[j].EG.D2R), sizeof(SCSPs[i].Slots[j].EG.D2R));
			StateFile->Read(&(SCSPs[i].Slots[j].EG.RR), sizeof(SCSPs[i].Slots[j].EG.RR));
			StateFile->Read(&(SCSPs[i].Slots[j].EG.DL), sizeof(SCSPs[i].Slots[j].EG.DL));
			StateFile->Read(&(SCSPs[i].Slots[j].EG.EGHOLD), sizeof(SCSPs[i].Slots[j].EG.EGHOLD));
			StateFile->Read(&(SCSPs[i].Slots[j].EG.LPLINK), sizeof(SCSPs[i].Slots[j].EG.LPLINK));
			
			// PLFO
			StateFile->Read(&(SCSPs[i].Slots[j].PLFO.phase), sizeof(SCSPs[i].Slots[j].PLFO.phase));
			StateFile->Read(&(SCSPs[i].Slots[j].PLFO.phase_step), sizeof(SCSPs[i].Slots[j].PLFO.phase_step));
			
			// ALFO
			StateFile->Read(&(SCSPs[i].Slots[j].ALFO.phase), sizeof(SCSPs[i].Slots[j].ALFO.phase));
			StateFile->Read(&(SCSPs[i].Slots[j].ALFO.phase_step), sizeof(SCSPs[i].Slots[j].ALFO.phase_step));

			// Recompute LFOs
			Compute_LFO(&(SCSPs[i].Slots[j]));
		}
		
		// DSP
		StateFile->Read(&(SCSPs[i].DSP.RBP), sizeof(SCSPs[i].DSP.RBP));
		StateFile->Read(&(SCSPs[i].DSP.RBL), sizeof(SCSPs[i].DSP.RBL));
		StateFile->Read(SCSPs[i].DSP.COEF, sizeof(SCSPs[i].DSP.COEF));
		StateFile->Read(SCSPs[i].DSP.MADRS, sizeof(SCSPs[i].DSP.MADRS));
		StateFile->Read(SCSPs[i].DSP.MPRO, sizeof(SCSPs[i].DSP.MPRO));
		StateFile->Read(SCSPs[i].DSP.TEMP, sizeof(SCSPs[i].DSP.TEMP));
		StateFile->Read(SCSPs[i].DSP.MEMS, sizeof(SCSPs[i].DSP.MEMS));
		StateFile->Read(&(SCSPs[i].DSP.DEC), sizeof(SCSPs[i].DSP.DEC));
		StateFile->Read(SCSPs[i].DSP.MIXS, sizeof(SCSPs[i].DSP.MIXS));
		StateFile->Read(SCSPs[i].DSP.EXTS, sizeof(SCSPs[i].DSP.EXTS));
		StateFile->Read(SCSPs[i].DSP.EFREG, sizeof(SCSPs[i].DSP.EFREG));
		StateFile->Read(&(SCSPs[i].DSP.Stopped), sizeof(SCSPs[i].DSP.Stopped));
		StateFile->Read(&(SCSPs[i].DSP.LastStep), sizeof(SCSPs[i].DSP.LastStep));
	}
}

void SCSP_SetBuffers(INT16 *leftBufferPtr, INT16 *rightBufferPtr, int bufferLength)
{
	SysFPS = 60.0;	// should this be updated to reflect actual FPS?
	bufferl = leftBufferPtr;
	bufferr = rightBufferPtr;
	length = bufferLength;
	
	cnts = 0;		// what is this for? seems unimportant but need to find out
}

void SCSP_Deinit(void)
{
#ifdef USEDSP
	free(SCSP->MIXBuf);
#endif
	free(buffertmpl);
	free(buffertmpr);
	delete MIDILock;
	buffertmpl = NULL;
	buffertmpr = NULL;
	MIDILock = NULL;
}
