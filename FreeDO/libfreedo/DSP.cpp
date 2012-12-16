/*
  www.freedo.org
The first and only working 3DO multiplayer emulator.

The FreeDO licensed under modified GNU LGPL, with following notes:

*   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
*   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
*   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
    or any other material published by FreeDO is strictly forbidden without owners approval.

The above notes are taking precedence over GNU LGPL in conflicting situations.

Project authors:

Alexander Troosh
Maxim Grishin
Allen Wright
John Sammons
Felix Lazarev
*/

// DSP.cpp: implementation of the CDSP class.
//
//////////////////////////////////////////////////////////////////////

#include "freedoconfig.h"
#include "DSP.h"
#include "Clio.h"

#if 0 //20 bit ALU
#define ALUSIZEMASK 0xFFFFf000
#else //32 bit ALU
#define ALUSIZEMASK 0xFFFFFFFF
#endif

//////////////////////////////////////////////////////////////////////
// Flag logic
//////////////////////////////////////////////////////////////////////

#define TOPBIT 0x80000000
#define ADDCFLAG(A,B,rd) (((((A)&(B))&TOPBIT) || (((A)&~rd)&TOPBIT) || (((B)&~rd)&TOPBIT)) ? 1 : 0 )
#define SUBCFLAG(A,B,rd) (((((A)&~(B))&TOPBIT) || (((A)&~rd)&TOPBIT) || ((~(B)&~rd)&TOPBIT)) ? 1 : 0 )
#define ADDVFLAG(A,B,rd) ( ((((A)&(B)&~rd)&TOPBIT) || ((~(A)&~(B)&rd)&TOPBIT)) ? 1 : 0 )
#define SUBVFLAG(A,B,rd) ( ((((A)&~(B)&~rd)&TOPBIT) || ((~(A)&(B)&rd)&TOPBIT)) ? 1 : 0 )

#define WAVELET (11025)

unsigned short __fastcall RegBase(unsigned int reg);
unsigned short __fastcall ireadh(unsigned int addr);
void __fastcall iwriteh(unsigned int addr, unsigned short val);
void __fastcall OperandLoader(int Requests);
int __fastcall OperandLoaderNWB(void);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma pack(push,1)

	struct CIFTAG{
		 unsigned int	BCH_ADDR	:10;
		 unsigned int	FLAG_MASK	:2;
		 unsigned int	FLGSEL		:1;
		 unsigned int	MODE		:2;
		 unsigned int	PAD			:1;
	};
	struct BRNTAG{
		unsigned int BCH_ADDR:10;
		unsigned int FLAGM0	:1;
		unsigned int FLAGM1	:1;
		unsigned int FLAGSEL:1;
		unsigned int MODE0	:1;
		unsigned int MODE1  :1;
		unsigned int AC		:1;
	};
	struct BRNBITS {
		unsigned int BCH_ADDR:10;
		unsigned int bits	:5;
		unsigned int AC		:1;
	};

	struct AIFTAG{
		unsigned int	BS			:4;
		unsigned int	ALU			:4;
		unsigned int	MUXB		:2;
		unsigned int	MUXA		:2;
		  signed int	M2SEL		:1;
		unsigned int	NUMOPS		:2;
		  signed int	PAD			:1;
	};
	struct IOFTAG{
		  signed int	IMMEDIATE	:13;
		  signed int	JUSTIFY		:1;
		unsigned int	TYPE		:2;
	};
	struct NROFTAG{
		unsigned int	OP_ADDR		:10;
		  signed int	DI			:1;
		unsigned int	WB1			:1;
		unsigned int	PAD			:1;
		unsigned int	TYPE		:3;
	};
	struct R2OFTAG{
		unsigned int	R1			:4;
		  signed int	R1_DI		:1;
		unsigned int	R2			:4;
		  signed int	R2_DI		:1;
		unsigned int	NUMREGS		:1;
		unsigned int	WB1			:1;
		unsigned int	WB2			:1;
		unsigned int	TYPE		:3;
	};
	struct R3OFTAG{
		unsigned int	R1			:4;
		  signed int	R1_DI		:1;
		unsigned int	R2			:4;
		  signed int	R2_DI		:1;
		unsigned int	R3			:4;
		  signed int	R3_DI		:1;
		unsigned int	TYPE		:1;
	};

	union ITAG{
		unsigned int raw;
		AIFTAG	aif;
		CIFTAG	cif;
		IOFTAG	iof;
		NROFTAG	nrof;
		R2OFTAG	r2of;
		R3OFTAG	r3of;
		BRNTAG	branch;
		BRNBITS br;
	};

	struct RQFTAG{

		unsigned int BS:1;
		unsigned int ALU2:1;
		unsigned int ALU1:1;
		unsigned int MULT2:1;
		unsigned int MULT1:1;

	};
	union _requnion
        {
		unsigned char raw;
		RQFTAG	rq;
	};

	struct __INSTTRAS
	{
		_requnion req;
		char BS;		// 4+1 bits
	}; // only for ALU command

	struct REGSTAG{
		unsigned int PC;//0x0ee
		unsigned short NOISE;//0x0ea
		unsigned short AudioOutStatus;//audlock,lftfull,rgtfull -- 0x0eb//0x3eb
		unsigned short Sema4Status;//0x0ec//0x3ec
		unsigned short Sema4Data;//0x0ed//0x3ed
		short DSPPCNT;//0x0ef
		short DSPPRLD;//0x3ef
		short AUDCNT;
		unsigned short INT;//0x3ee
	};
	struct INTAG{

		signed short MULT1;
		signed short MULT2;

		signed short ALU1;
		signed short ALU2;

		int BS;

		unsigned short RMAP;
		unsigned short nOP_MASK;

		unsigned short WRITEBACK;

		_requnion	req;

		bool Running;
		bool GenFIQ;

	};

struct DSPDatum
{
        unsigned int RBASEx4;
        __INSTTRAS INSTTRAS[0x8000];
        unsigned short REGCONV[8][16];
        bool BRCONDTAB[32][32];
        unsigned short NMem[2048];
        unsigned short IMem[1024];
        int REGi;
        REGSTAG dregs;
        INTAG flags;
        unsigned int g_seed;
        bool CPUSupply[16];
};

#pragma pack(pop)

static DSPDatum dsp;

#include <memory.h>

unsigned int _dsp_SaveSize()
{
        return sizeof(DSPDatum);
}
void _dsp_Save(void *buff)
{
        memcpy(buff,&dsp,sizeof(DSPDatum));
}
void _dsp_Load(void *buff)
{
        memcpy(&dsp,buff,sizeof(DSPDatum));
}

#define RBASEx4 dsp.RBASEx4
#define INSTTRAS dsp.INSTTRAS
#define REGCONV dsp.REGCONV
#define BRCONDTAB dsp.BRCONDTAB
#define NMem dsp.NMem
#define IMem dsp.IMem
#define REGi dsp.REGi
#define dregs dsp.dregs
#define flags dsp.flags
#define g_seed dsp.g_seed
#define CPUSupply dsp.CPUSupply




int fastrand()
{
    g_seed=69069*g_seed+1;
	return g_seed & 0xFFFF;
}


void _dsp_Init()
{

  int a,c;
  ITAG inst;
  unsigned int i;

  g_seed=0xa5a5a5a5;
  //FRAMES=0;
  for(a=0;a<16;a++)
	  for(c=0;c<8;c++)
	  {
		flags.RMAP=c;
		REGCONV[c][a]=RegBase(a);
	  }

	for(inst.raw=0;inst.raw<0x8000;inst.raw++)
	{
		flags.req.raw=0;

		if(inst.aif.BS==0x8)
			flags.req.rq.BS=1;

		switch(inst.aif.MUXA)
		{
		case 3:
			flags.req.rq.MULT1=1;
			flags.req.rq.MULT2=inst.aif.M2SEL;
			break;
		case 1:
			flags.req.rq.ALU1=1;
			break;
		case 2:
			flags.req.rq.ALU2=1;
			break;
		}
		switch(inst.aif.MUXB)
		{
		case 1:
			flags.req.rq.ALU1=1;
			break;
		case 2:
			flags.req.rq.ALU2=1;
			break;
		case 3:
			flags.req.rq.MULT1=1;
			flags.req.rq.MULT2=inst.aif.M2SEL;
			break;
		}

		INSTTRAS[inst.raw].req.raw=flags.req.raw;
		INSTTRAS[inst.raw].BS=inst.aif.BS | ((inst.aif.ALU&8)<<(4-3));

	}
	{
		bool MD0, MD1, MD2, MD3;
		bool STAT0, STAT1;
		bool NSTAT0, NSTAT1;
		bool TDCARE0, TDCARE1;
		bool RDCARE;
		bool MD12S;
		bool SUPER0, SUPER1;
		bool ALLZERO, NALLZERO;
		bool SDS;
		bool NVTest;
		bool TMPCS;
		bool CZTest, XactTest;
		bool MD3S;

//		int Flags.Zero;
//		int Flags.Nega;
//		int Flags.Carry;//not borrow
//		int Flags.Over;
		int fExact;

	union {
		struct {
			char Zero;
			char Nega;
			char Carry;//not borrow
			char Over;
		};
		unsigned int raw;
	} Flags;

	for(inst.raw=0xA000; inst.raw<=0xFFFF; inst.raw+=1024)
	 for(Flags.Zero=0;Flags.Zero<2;Flags.Zero++)
	  for(Flags.Nega=0;Flags.Nega<2;Flags.Nega++)
	   for(Flags.Carry=0;Flags.Carry<2;Flags.Carry++)
		for(Flags.Over=0;Flags.Over<2;Flags.Over++)
		 for(fExact=0;fExact<2;fExact++)
			{

				MD0=!inst.branch.MODE1 && !inst.branch.MODE0;
				MD1=!inst.branch.MODE1 && inst.branch.MODE0;
				MD2=inst.branch.MODE1 && !inst.branch.MODE0;
				MD3=inst.branch.MODE1 && inst.branch.MODE0;

				STAT0=(inst.branch.FLAGSEL && Flags.Carry) || (!inst.branch.FLAGSEL && Flags.Nega);
				STAT1=(inst.branch.FLAGSEL && Flags.Zero) || (!inst.branch.FLAGSEL && Flags.Over);
				NSTAT0=STAT0!=MD2;
				NSTAT1=STAT1!=MD2;

				TDCARE1=!inst.branch.FLAGM1 || (inst.branch.FLAGM1 && NSTAT0);
				TDCARE0=!inst.branch.FLAGM0 || (inst.branch.FLAGM0 && NSTAT1);

				RDCARE=!inst.branch.FLAGM1 && !inst.branch.FLAGM0;

				MD12S=TDCARE1 && TDCARE0 && (inst.branch.MODE1!=inst.branch.MODE0) && !RDCARE;

				SUPER0= MD1 && !inst.branch.FLAGSEL && RDCARE;
				SUPER1= MD1 && inst.branch.FLAGSEL && RDCARE;

				ALLZERO=SUPER0 && Flags.Zero && fExact;
				NALLZERO=SUPER1 && !(Flags.Zero && fExact);

				SDS=ALLZERO || NALLZERO;

				NVTest=( ((Flags.Nega!=Flags.Over) || (Flags.Zero && inst.branch.FLAGM0)) != inst.branch.FLAGM1) && !inst.branch.FLAGSEL;
				TMPCS=Flags.Carry && ! Flags.Zero;
				CZTest=(TMPCS != inst.branch.FLAGM0) && inst.branch.FLAGSEL && !inst.branch.FLAGM1;
				XactTest=(fExact != inst.branch.FLAGM0) && inst.branch.FLAGSEL && inst.branch.FLAGM1;

				MD3S=(NVTest || CZTest || XactTest)&&MD3;

				BRCONDTAB[inst.br.bits][fExact+((Flags.raw*0x10080402)>>24)] = (MD12S || MD3S || SDS)? true : false;
			 }
	}


	flags.Running=0;
	flags.GenFIQ=false;
	dregs.DSPPRLD=567;
	dregs.AUDCNT=567;

	_dsp_Reset();

	dregs.Sema4Status = 0; //?? 8-CPU last, 4-DSP last, 2-CPU ACK, 1 DSP ACK ??
	for( i=0;i<sizeof(NMem)/sizeof(NMem[0]);i++) NMem[i]=0x8380; //SLEEP
	for(i=0;i<16;i++) CPUSupply[i]=0;
}

void _dsp_Reset()
{
	dregs.DSPPCNT=dregs.DSPPRLD;
	dregs.PC=0;
	RBASEx4=0;
	REGi=0;
	flags.nOP_MASK=~0;
}

#include "freedocore.h"
extern _ext_Interface  io_interface;
void _Arithmetic_Debug(uint16 nrc, uint16 opmask)
{
 bool MULT1_RQST_L,MULT2_RQST_L,ALU1_RQST_L,ALU2_RQST_L,BS_RQST_L;
 int NUMBER_OPERANDS=0, cnt=0;

    if( ((nrc&0x300)==0x300 || (nrc&0xC00)==0xC00) && !(opmask&0x10) )
        {MULT1_RQST_L=true;cnt++;}
    else MULT1_RQST_L=false;

    if( ((nrc&0x300)==0x300 || (nrc&0xC00)==0xC00) && !(opmask&0x8) && (nrc&0x1000) )
        {MULT2_RQST_L=true;cnt++;}
    else MULT2_RQST_L=false;

    if(  ((nrc&0x300)==0x100 || (nrc&0xC00)==0x400) && !(opmask&0x4)  )
        {ALU1_RQST_L=true;cnt++;}
    else ALU1_RQST_L=false;

    if(  ((nrc&0x300)==0x200 || (nrc&0xC00)==0x800) && !(opmask&0x2)  )
        {ALU2_RQST_L=true;cnt++;}
    else ALU2_RQST_L=false;

    if( (nrc&0xf)==0x8 && !(opmask&0x1) )
        {BS_RQST_L=true;cnt++;}
    else BS_RQST_L=false;

    NUMBER_OPERANDS=(nrc>>13)&3;
    if(!NUMBER_OPERANDS && (ALU1_RQST_L || ALU2_RQST_L) )NUMBER_OPERANDS=4;

    //what if RQ is more than NUM_OPS????
    if(NUMBER_OPERANDS<cnt)io_interface(EXT_DEBUG_PRINT,(void*)">>>DSP NUM_OPS_CONFLICT!!!\n");

}



unsigned int _dsp_Loop()
{
	unsigned int AOP, BOP;	//1st & 2nd operand
	unsigned int Y;			//accumulator

	unsigned int RBSR;	//return address

	union {
		struct {
			bool Zero;
			bool Nega;
			bool Carry;//not borrow
			bool Over;
		};
		unsigned int raw;
	} Flags;
	bool fExact;

	bool Work;

	if(flags.Running&1)
	{
		_dsp_Reset();

		Flags.raw=0;
		fExact=0;

		AOP=0;
		BOP=0;
		Y=0;

		RBSR=0;

		Work=true;
		do
		{
		  ITAG inst;

			inst.raw=NMem[dregs.PC++];
			//DSPCYCLES++;

			if(inst.aif.PAD)
			{//Control instruction
				switch((inst.raw>>7)&255) //special
					{
					case 0://NOP TODO
							break;
					case 1://branch accum
							dregs.PC=(Y>>16)&0x3ff;
							break;
					case 2://set rbase
							RBASEx4=(inst.cif.BCH_ADDR&0x3f)<<2;
							break;
					case 3://set rmap
							REGi=inst.cif.BCH_ADDR&7;
							break;
					case 4://RTS
							dregs.PC=RBSR;
							break;
					case 5://set op_mask
							flags.nOP_MASK=~(inst.cif.BCH_ADDR&0x1f);
							break;
					case 6:// -not used2- ins
							break;
					case 7://SLEEP
							Work=false;
							break;
					case 8:  case 9:  case 10: case 11:
					case 12: case 13: case 14: case 15:
							//jump //branch only if not branched
							dregs.PC=inst.cif.BCH_ADDR;
							break;
					case 16: case 17: case 18: case 19:
					case 20: case 21: case 22: case 23:
							//jsr
							RBSR=dregs.PC;
							dregs.PC=inst.cif.BCH_ADDR;
							break;
					case 24: case 25: case 26: case 27:
					case 28: case 29: case 30: case 31:
							// branch only if was branched
							dregs.PC=inst.cif.BCH_ADDR;
							break;
					case 32: case 33: case 34: case 35:
					case 36: case 37: case 38: case 39:
					case 40: case 41: case 42: case 43: // ??? -not used- instr's
					case 44: case 45: case 46: case 47: // ??? -not used- instr's
							// MOVEREG
							{
								int Operand=OperandLoaderNWB();
								if(inst.r2of.R1_DI)
									iwriteh(ireadh(REGCONV[REGi][inst.r2of.R1]^RBASEx4),Operand);
								else
									iwriteh(REGCONV[REGi][inst.r2of.R1]^RBASEx4,Operand);
							}
							break;
					case 48: case 49: case 50: case 51:
					case 52: case 53: case 54: case 55:
					case 56: case 57: case 58: case 59:
					case 60: case 61: case 62: case 63:
							// MOVE
							{
								int Operand=OperandLoaderNWB();
								if(inst.nrof.DI)
									iwriteh(ireadh(inst.cif.BCH_ADDR),Operand);
								else
									iwriteh(inst.cif.BCH_ADDR,Operand);
							}
							break;
					default: // Coundition branch
							if(1&BRCONDTAB[inst.br.bits][fExact+((Flags.raw*0x10080402)>>24)]) dregs.PC=inst.cif.BCH_ADDR;
							break;
				}//switch((inst.raw>>7)&255) //special
			}
			else //if(inst.aif.PAD)
			{
				//ALU instruction

				_Arithmetic_Debug(inst.raw, ~flags.nOP_MASK);

				flags.req.raw=INSTTRAS[inst.raw].req.raw;
				flags.BS     =INSTTRAS[inst.raw].BS;

				OperandLoader(inst.aif.NUMOPS);

				switch(inst.aif.MUXA)
				{
				case 3:
					if(inst.aif.M2SEL==0)
					{
						if((inst.aif.ALU==3)||(inst.aif.ALU==5)) // ACSBU signal
							AOP=Flags.Carry? ((int)flags.MULT1<<16)&ALUSIZEMASK : 0;
						else
							AOP=( ((int)flags.MULT1*(((signed int)Y>>15)&~1))&ALUSIZEMASK );
					}
					else
						AOP=( ((int)flags.MULT1*(int)flags.MULT2*2)&ALUSIZEMASK );
					break;
				case 1:
					AOP=flags.ALU1<<16;
					break;
				case 0:
					AOP=Y;
					break;
				case 2:
					AOP=flags.ALU2<<16;
					break;
				}

				if((inst.aif.ALU==3)||(inst.aif.ALU==5)) // ACSBU signal
				{
					BOP=Flags.Carry<<16;
				}
				else
				{
					switch(inst.aif.MUXB)
					{
					case 0:
						BOP=Y;
						break;
					case 1:
						BOP=flags.ALU1<<16;
						break;
					case 2:
						BOP=flags.ALU2<<16;
						break;
					case 3:
						if(inst.aif.M2SEL==0) // ACSBU==0 here always
							BOP=( ((int)flags.MULT1*(((signed int)Y>>15))&~1)&ALUSIZEMASK );
						else
							BOP=( ((int)flags.MULT1*(int)flags.MULT2*2)&ALUSIZEMASK );
						break;
					}
				}
				//ok now ALU itself.
                                //unsigned char ctt1,ctt2;
				Flags.Over=Flags.Carry=0; // Any ALU op. change Over and possible Carry
				switch(inst.aif.ALU)
				{
				case 0:
					Y=AOP;
					break;
                                //*
				case 1:
					Y=0-BOP;
					Flags.Carry=SUBCFLAG(0,BOP,Y);
					Flags.Over=SUBVFLAG(0,BOP,Y);
					break;
				case 2:
				case 3:
					Y=AOP+BOP;
					Flags.Carry=ADDCFLAG(AOP,BOP,Y);
					Flags.Over=ADDVFLAG(AOP,BOP,Y);
					break;
				case 4:
				case 5:
					Y=AOP-BOP;
					Flags.Carry=SUBCFLAG(AOP,BOP,Y);
					Flags.Over=SUBVFLAG(AOP,BOP,Y);
					break;
				case 6:
					Y=AOP+0x1000;
					Flags.Carry=ADDCFLAG(AOP,0x1000,Y);
					Flags.Over=ADDVFLAG(AOP,0x1000,Y);
					break;
				case 7:
					Y=AOP-0x1000;
					Flags.Carry=SUBCFLAG(AOP,0x1000,Y);
					Flags.Over=SUBVFLAG(AOP,0x1000,Y);
					break;

				case 8:		// A
					Y=AOP;
					break;
				case 9:		// NOT A
					Y=AOP^ALUSIZEMASK;
					break;
				case 10:	// A AND B
					Y=AOP&BOP;
					break;
				case 11:	// A NAND B
					Y=(AOP&BOP)^ALUSIZEMASK;
					break;
				case 12:	// A OR B
					Y=AOP|BOP;
					break;
				case 13:	// A NOR B
					Y=(AOP|BOP)^ALUSIZEMASK;
					break;
				case 14:	// A XOR B
					Y=AOP^BOP;
					break;
				case 15:	// A XNOR B
					Y=AOP^BOP^ALUSIZEMASK;
					break;
				}
				Flags.Zero=(Y&0xFFFF0000)?0:1;
				Flags.Nega=(Y>>31)?1:0;
				fExact=(Y&0x0000F000)?0:1;

                //and BarrelShifter
				switch(flags.BS)
				{
					case 1:
					case 17:
						Y=Y<<1;
						break;
					case 2:
					case 18:
						Y=Y<<2;
						break;
					case 3:
					case 19:
						Y=Y<<3;
						break;
					case 4:
					case 20:
						Y=Y<<4;
						break;
					case 5:
					case 21:
						Y=Y<<5;
						break;
					case 6:
					case 22:
						Y=Y<<8;
						break;

					//arithmetic shifts
					case 9:
						Y=(signed int)Y>>16;
						Y&=ALUSIZEMASK;
						break;
					case 10:
						Y=(signed int)Y>>8;
						Y&=ALUSIZEMASK;
						break;
					case 11:
						Y=(signed int)Y>>5;
						Y&=ALUSIZEMASK;
						break;
					case 12:
						Y=(signed int)Y>>4;
						Y&=ALUSIZEMASK;
						break;
					case 13:
						Y=(signed int)Y>>3;
						Y&=ALUSIZEMASK;
						break;
					case 14:
						Y=(signed int)Y>>2;
						Y&=ALUSIZEMASK;
						break;
					case 15:
						Y=(signed int)Y>>1;
						Y&=ALUSIZEMASK;
						break;

					// logocal shift
					case 7: // CLIP ari
					case 23:// CLIP log
						if(1&Flags.Over)
						{
							if(1&Flags.Nega)	Y=0x7FFFf000;
							else				Y=0x80000000;
						}
						break;
					case 8: // Load operand load sameself again (ari)
					case 24:// same, but logicalshift
						{
                                                        //int temp=Flags.Carry;
							Flags.Carry=(signed)Y<0;	// shift out bit to Carry
							//Y=Y<<1;
							//Y|=temp<<16;
							Y=((Y<<1)&0xfffe0000)|(Flags.Carry?1<<16:0)|(Y&0xf000);
						}
						break;
					case 25:
						Y=(unsigned int)Y>>16;
						Y&=ALUSIZEMASK;
						break;
					case 26:
						Y=(unsigned int)Y>>8;
						Y&=ALUSIZEMASK;
						break;
					case 27:
						Y=(unsigned int)Y>>5;
						Y&=ALUSIZEMASK;
						break;
					case 28:
						Y=(unsigned int)Y>>4;
						Y&=ALUSIZEMASK;
						break;
					case 29:
						Y=(unsigned int)Y>>3;
						Y&=ALUSIZEMASK;
						break;
					case 30:
						Y=(unsigned int)Y>>2;
						Y&=ALUSIZEMASK;
						break;
					case 31:
						Y=(unsigned int)Y>>1;
						Y&=ALUSIZEMASK;
						break;
				}

				//now write back. (assuming in WRITEBACK there is an address where to write
				if(flags.WRITEBACK)
				{
					iwriteh(flags.WRITEBACK,((signed int)Y)>>16);
				}

				//fin :)
			}//else //if(inst.aif.PAD)

		}while(Work);//big while!!!


		if(1&flags.GenFIQ)
		{
			flags.GenFIQ=false;
			_clio_GenerateFiq(0x800,0);//AudioFIQ
			//printf("#!!! AudioFIQ Generated 0x%4.4X\n!!!",val);
		}

		dregs.DSPPCNT-=567;
		if(dregs.DSPPCNT<=0)
		   dregs.DSPPCNT+=dregs.DSPPRLD;
	}
	return ((IMem[0x3ff]<<16)|IMem[0x3fe]);
}

void __fastcall _dsp_WriteMemory(unsigned short addr, unsigned short val) //CPU writes NMEM of DSP
{
	//mwriteh(addr,val);
	//printf("#NWRITE 0x%3.3X<=0x%4.4X\n",addr,val);
	NMem[addr&0x3ff]=val;
}

unsigned short __fastcall RegBase(unsigned int reg)
{
	unsigned short res;
	unsigned char twi,x,y;

	reg&=0xf;
	x=(reg>>2)&1;
	y=(reg>>3)&1;
	switch(flags.RMAP)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			twi=x;
			break;
		case 4:
			twi=y;
			break;
		case 5:
			twi=!y;
			break;
		case 6:
			twi=x&y;
			break;
		case 7:
			twi=x|y;
			break;
	}
	res=(reg&7);
	res|=twi<<8;
	res|=(reg>>3)<<9;

	////printf("RegSel=0x%X\n",res);
	return res;
}

unsigned short __fastcall ireadh(unsigned int addr) //DSP IREAD (includes EI, I)
{

	unsigned short val;

//	addr&=0x3ff;
	switch(addr)
	{
	case 0xea:
		dregs.NOISE=fastrand();
		return dregs.NOISE;
		break;
	case 0xeb:
		//printf("#DSP read AudioOutStatus (0x%4.4X)\n",dregs.AudioOutStatus);
		return dregs.AudioOutStatus;
		break;
	case 0xec:
		//printf("#DSP read Sema4Status: 0x%4.4X\n",dregs.Sema4Status);
		return dregs.Sema4Status;
		break;
	case 0xed:
		//printf("#DSP read Sema4Data: 0x%4.4X\n",dregs.Sema4Data);
		return dregs.Sema4Data;
		break;
	case 0xee:
		//printf("#DSP read PC (0x%4.4X)\n",dregs.PC);
		return dregs.PC;
		break;
	case 0xef:
		//printf("#DSP read DSPPCNT (0x%4.4X)\n",dregs.DSPPCNT); //?? 0x4000 always
		return dregs.DSPPCNT;
		break;
	case 0xf0:	case 0xf1:	case 0xf2:	case 0xf3:
	case 0xf4:	case 0xf5:	case 0xf6:	case 0xf7:
	case 0xf8:	case 0xf9:	case 0xfa:	case 0xfb:
	case 0xfc:
		if(CPUSupply[addr-0xf0])
		{
			CPUSupply[addr-0xf0]=0;
			//printf("#DSP read from CPU!!! chan=0x%x\n",addr&0x0f);
			//val=IMem[addr-0x80];
			val=(fastrand()<<16)|fastrand();
		}
		else
			val=_clio_EIFIFO(addr&0x0f);
		//val=rand();
		//printf("#DSP read EIFifo 0x%4.4X>=0x%4.4X\n",addr&0x0f,val);
		/*FILE * flo;
		flo=fopen("z:\\inp.raw","ab");
		if(flo)
		{
			fwrite(&val,2,1,flo);
			fclose(flo);
		}*/
		return val;
		break;
	case 0x70:	case 0x71:	case 0x72:	case 0x73:
	case 0x74:	case 0x75:	case 0x76:	case 0x77:
	case 0x78:	case 0x79:	case 0x7a:	case 0x7b:
	case 0x7c:
		if(CPUSupply[addr-0x70])
		{
			CPUSupply[addr-0x70]=0;
				//printf("#DSP read from CPU!!! chan=0x%x\n",addr&0x0f);
			return IMem[addr];
		}
		else
			return _clio_EIFIFONI(addr&0x0f);
		break;

	case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:
	case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:
	case 0xd8:	case 0xd9:	case 0xda:	case 0xdb:
	case 0xdc:	case 0xdd:	case 0xde: // what is last two case's?
		//if(CPUSupply[addr&0x0f])
		//{
			//CPUSupply[addr&0x0f]=0;
			//printf("#DSP read from CPU!!! chan=0x%x\n",addr&0x0f);
			//return IMem[0x70+addr&0x0f];
		//}
		//else
		//printf("#DSP read EIFifo status 0x%4.4X\n",addr&0x0f);
		if(CPUSupply[addr&0xf])
			return 2;
		else
			return _clio_GetEIFIFOStat(addr&0xf);
			//return CPUSupply[addr-0xd0]||CClio::GetEIFIFOStat(addr-0xd0);
		break;

	case 0xe0:
	case 0xe1:
	case 0xe2:
	case 0xe3:

		//printf("#DSP read EOFifo status 0x%4.4X\n",addr&0x0f);

		return _clio_GetEOFIFOStat(addr&0x0f);
		break;


	default:
		//printf("#EIRead 0x%3.3X>=0x%4.4X\n",addr, IMem[addr&0x7f]);
	    addr-=0x100;
		if((addr<0x200))
		{
			return IMem[addr|0x100];
		}
		else
			return IMem[addr&0x7f];
	}

}


void __fastcall iwriteh(unsigned int addr, unsigned short val) //DSP IWRITE (includes EO,I)
{
	//unsigned short imem;
	addr&=0x3ff;
	switch(addr)
	{
	case 0x3eb: dregs.AudioOutStatus=val;
		//if(val&0x8000)
		//	dregs.DSPPRLD=568;
		break;
	case 0x3ec: // DSP write to Sema4ACK
				//printf("#DSP write Sema4ACK=0x%4.4X\n",val);
				dregs.Sema4Status|=0x1;
				//printf("#DSP write Sema4Status=0x%4.4X\n",val);
		break;
	case 0x3ed: dregs.Sema4Data=val;
				//printf("#DSP write Sema4Data=0x%4.4X\n",val);
				dregs.Sema4Status=0x4;  // DSP write to Sema4Data
				//printf("Sema4Status set to 0x%4.4X\n",dregs.Sema4Status);
		break;
	case 0x3ee: dregs.INT=val;
				flags.GenFIQ=true;
		break;
	case 0x3ef: dregs.DSPPRLD=val;
        //if(val>0)io_interface(EXT_DEBUG_PRINT,(void*)"DSP_CNT_REL > 0");
        //else if(val==567)io_interface(EXT_DEBUG_PRINT,(void*)"DSP_CNT_REL == 567");
        //else io_interface(EXT_DEBUG_PRINT,(void*)"DSP_CNT_REL == 0");
		break;
	case 0x3f0:
	case 0x3f1:
	case 0x3f2:
	case 0x3f3:
		_clio_EOFIFO(addr&0x0f,val);
		break;
	case 0x3fd:
		//FLUSH EOFIFO
		break;
	case 0x3fe: // DAC Left channel
	case 0x3ff: // DAC Right channel
		IMem[addr]=val;
		break;
	default:
		if(addr<0x100)
		{	//printf("#OPUS !!!DPC=0x%3.3X write to r/o == addr 0x%3.3X , val 0x%4.4X\n",dregs.PC,addr,val);
			return;
		}
		//printf("#EOWRITE 0x%3.3X<=0x%4.4X\n",addr,val);

		/*if( (addr>=0x310) && (addr<0x3eb))
			printf("# UnEmulated registers write addr=0x%3.3X, pc=0x%3.3X\n",addr,dregs.PC);
		*/

        addr-=0x100;
		if(addr<0x200)
		{
			IMem[addr|0x100]=val;
		}
		else
			IMem[addr+0x100]=val;
		return;
	}
}



void __fastcall _dsp_SetRunning(bool val)
{
	flags.Running= val;
}


void __fastcall _dsp_WriteIMem(unsigned short addr, unsigned short val)//CPU writes to EI,I of DSP
{
	switch(addr)
	{
	/*case 0xea:
	case 0x6a:
		dregs.NOISE=val;
		break;
	case 0xeb:
	case 0x6b:
		dregs.AudioOutStatus=val;
		break;
	case 0xec:
	case 0x6c:
		//printf("#Arm write Sema4ACK=0x%4.4X\n",val);
		dregs.Sema4Status |= 0x2;
		break;
	case 0xed:
	case 0x6d:
		dregs.Sema4Data=val; // ARM write to Sema4Data
		dregs.Sema4Status = 0x8;
				//printf("#Arm write Sema4Data=0x%4.4X\n",val);
				//printf("#Sema4Status set to 0x%4.4X\n",dregs.Sema4Status);
		break;
	case 0xee:
        //io_interface(EXT_DEBUG_PRINT,(void*)">>>DSP PC WRITED 0xEE!!!\n");
	case 0x6e:
		dregs.PC=val;
		//io_interface(EXT_DEBUG_PRINT,(void*)">>>DSP PC WRITED 0x6E!!!\n");
		break;
	case 0xef:
	case 0x6f:
		dregs.DSPPCNT=val;
		break;*/

	case 0x70:	case 0x71:	case 0x72:	case 0x73:
	case 0x74:	case 0x75:	case 0x76:	case 0x77:
	case 0x78:	case 0x79:	case 0x7a:	case 0x7b:
	case 0x7c:
		CPUSupply[addr-0x70]=1;
		//printf("# Coeff ARM write=0x%3.3X, val=0x%4.4X\n",addr,val);
		IMem[addr&0x7f]=val;
        io_interface(EXT_DEBUG_PRINT,(void*)">>>ARM TO DSP FIFO DIRECT!!!\n");
		break;


	default:
		//if( (addr>=0x0) && (addr<0x6f))
		//	printf("# Coeff ARM write=0x%3.3X, val=0x%4.4X\n",addr,val);

		if(!(addr&0x80))IMem[addr&0x7f]=val;
		else io_interface(EXT_DEBUG_PRINT,(void*)">>>ARM TO DSP HZ!!!\n");
		break;
	}

}

void __fastcall _dsp_ARMwrite2sema4(unsigned int val)
{
    // How about Sema4ACK? Now don't think about it
	dregs.Sema4Data=val&0xffff;	// ARM write to Sema4Data low 16 bits
	dregs.Sema4Status=0x8;		// ARM be last
	//printf("#Arm write Sema4Data=0x%4.4X\n",val);
	//printf("Sema4Status set to 0x%4.4X\n",dregs.Sema4Status);
}


unsigned short __fastcall _dsp_ReadIMem(unsigned short addr) //CPU reads from EO,I of DSP
{

	switch(addr)
	{
	case 0x3eb:
		//printf("#ARM read AudioOutStatus (0x%4.4X)\n",dregs.AudioOutStatus);
		return dregs.AudioOutStatus;
		break;
	case 0x3ec:
		//printf("#Arm read Sema4Status = 0x%4.4X\n",dregs.Sema4Status);
		return dregs.Sema4Status;
		break;
	case 0x3ed:
		//printf("#Arm read Sema4Data = 0x%4.4X\n",dregs.Sema4Status);
		return dregs.Sema4Data;
		break;
	case 0x3ee:
		//printf("#ARM read dregs.INT (0x%04X)\n",dregs.INT);
		return dregs.INT;
		break;
	case 0x3ef:
		//printf("#ARM read DSPPRLD (0x%4.4X)\n",dregs.DSPPRLD);
		return dregs.DSPPRLD;
		break;
	default:
	//	printf("#Arm read IMem[0x%3.3X]=0x%4.4X\n",addr, IMem[addr]);
		return IMem[addr];
		break;

	}

}

unsigned int _dsp_ARMread2sema4(void)
{
	//printf("#Arm read both Sema4Status & Sema4Data\n");
	return (dregs.Sema4Status<<16) | dregs.Sema4Data;
}

 void __fastcall OperandLoader(int Requests)
{
	int Operands;//total of operands
	int Ptr;
	unsigned short OperandPool[6]; // c'mon -- 5 is real maximum
	unsigned short GWRITEBACK;

	ITAG operand;

	flags.WRITEBACK=0;

	if(Requests==0)
	{
		if(flags.req.raw)
			Requests=4;
		else
			return;
	}

	//DSPCYCLES+=Requests;
	Operands=0;
	GWRITEBACK=0;

	do
	{
		operand.raw=NMem[dregs.PC++];
		if(operand.nrof.TYPE==4)
		{
				//non reg format ///IT'S an address!!!
				if(operand.nrof.DI)
					OperandPool[Operands]=ireadh(flags.WRITEBACK=ireadh(operand.nrof.OP_ADDR));
				else
					OperandPool[Operands]=ireadh(flags.WRITEBACK=       operand.nrof.OP_ADDR);
				Operands++;

				if(operand.nrof.WB1)
					GWRITEBACK=flags.WRITEBACK;

		}else if ((operand.nrof.TYPE&6)==6)
		{
				//case 6: and case 7:  immediate format
				OperandPool[Operands]=operand.iof.IMMEDIATE<<(operand.iof.JUSTIFY&3);
				flags.WRITEBACK=OperandPool[Operands++];

		}else if(!(operand.nrof.TYPE&4))  // case 0..3
		{
				if(operand.r3of.R3_DI)
					OperandPool[Operands]=ireadh(ireadh(REGCONV[REGi][operand.r3of.R3]^RBASEx4));
				else
					OperandPool[Operands]=ireadh(       REGCONV[REGi][operand.r3of.R3]^RBASEx4 );
				Operands++;

				if(operand.r3of.R2_DI)
					OperandPool[Operands]=ireadh(ireadh(REGCONV[REGi][operand.r3of.R2]^RBASEx4));
				else
					OperandPool[Operands]=ireadh(       REGCONV[REGi][operand.r3of.R2]^RBASEx4 );
				Operands++;

				// only R1 can be WRITEBACK
				if(operand.r3of.R1_DI)
					OperandPool[Operands]=ireadh(flags.WRITEBACK=ireadh(REGCONV[REGi][operand.r3of.R1]^RBASEx4));
				else
					OperandPool[Operands]=ireadh(flags.WRITEBACK=       REGCONV[REGi][operand.r3of.R1]^RBASEx4 );
				Operands++;

		}else //if(operand.nrof.TYPE==5)
		{
				//regged 1/2 format
				if(operand.r2of.NUMREGS)
				{
					if(operand.r2of.R2_DI)
						OperandPool[Operands]=ireadh(flags.WRITEBACK=ireadh(REGCONV[REGi][operand.r2of.R2]^RBASEx4));
					else
						OperandPool[Operands]=ireadh(flags.WRITEBACK=       REGCONV[REGi][operand.r2of.R2]^RBASEx4 );
					Operands++;

					if(operand.r2of.WB2)
						GWRITEBACK=flags.WRITEBACK;
				}

				if(operand.r2of.R1_DI)
					OperandPool[Operands]=ireadh(flags.WRITEBACK=ireadh(REGCONV[REGi][operand.r2of.R1]^RBASEx4));
				else
					OperandPool[Operands]=ireadh(flags.WRITEBACK=       REGCONV[REGi][operand.r2of.R1]^RBASEx4 );
				Operands++;

				if(operand.r2of.WB1)
					GWRITEBACK=flags.WRITEBACK;
		}//if
	}while(Operands<Requests);


	//ok let's clean out Requests (using op_mask)
	flags.req.raw&=flags.nOP_MASK;

	Ptr=0;

	if(flags.req.rq.MULT1)
		flags.MULT1=OperandPool[Ptr++];
	if(flags.req.rq.MULT2)
		flags.MULT2=OperandPool[Ptr++];

	if(flags.req.rq.ALU1)
		flags.ALU1=OperandPool[Ptr++];
	if(flags.req.rq.ALU2)
		flags.ALU2=OperandPool[Ptr++];

	if(flags.req.rq.BS)
		flags.BS=OperandPool[Ptr++];

	if(Operands!=Ptr)
	{
		if(GWRITEBACK)
			flags.WRITEBACK=GWRITEBACK;
        //else flags.WRITEBACK=0;
	}
	else
		flags.WRITEBACK=GWRITEBACK;
}



int __fastcall OperandLoaderNWB(void)
{
	int Operand;

	ITAG operand;

		operand.raw=NMem[dregs.PC++];
		if(operand.nrof.TYPE==4)
		{
				//non reg format ///IT'S an address!!!
				if(operand.nrof.DI)
					Operand=ireadh(ireadh(operand.nrof.OP_ADDR));
				else
					Operand=ireadh(       operand.nrof.OP_ADDR);


		}else if(!(operand.nrof.TYPE&4))  // case 0..3
		{
				if(operand.r3of.R3_DI) // ???
					Operand=ireadh(ireadh(REGCONV[REGi][operand.r3of.R3]^RBASEx4));
				else
					Operand=ireadh(       REGCONV[REGi][operand.r3of.R3]^RBASEx4 );
		}else if ((operand.nrof.TYPE&6)==6)
		{
				//case 6: and case 7:  immediate format
				Operand=operand.iof.IMMEDIATE<<(operand.iof.JUSTIFY&3);

		}else if(operand.nrof.TYPE==5)
		{ //if(operand.r2of.NUMREGS) ignore... It's right?
				if(operand.r2of.R1_DI)
					Operand=ireadh(ireadh(REGCONV[REGi][operand.r2of.R1]^RBASEx4));
				else
					Operand=ireadh(       REGCONV[REGi][operand.r2of.R1]^RBASEx4 );
		}

		return Operand;
}
