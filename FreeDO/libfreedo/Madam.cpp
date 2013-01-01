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

// Madam.cpp: implementation of the CMadam class.
//
//////////////////////////////////////////////////////////////////////

#include "freedoconfig.h"
#include "Madam.h"
#include "Clio.h"
#include "vdlp.h"
#include "arm.h"
#include <math.h>
#include <memory.h>

#include "bitop.h"
BitReaderBig bitoper;

#include "freedocore.h"

extern _ext_Interface  io_interface;



/* === CCB control word flags === */
#define CCB_SKIP        0x80000000
#define CCB_LAST        0x40000000
#define CCB_NPABS       0x20000000
#define CCB_SPABS       0x10000000
#define CCB_PPABS       0x08000000
#define CCB_LDSIZE      0x04000000
#define CCB_LDPRS       0x02000000
#define CCB_LDPPMP      0x01000000
#define CCB_LDPLUT      0x00800000
#define CCB_CCBPRE      0x00400000
#define CCB_YOXY        0x00200000
#define CCB_ACSC        0x00100000
#define CCB_ALSC        0x00080000
#define CCB_ACW         0x00040000
#define CCB_ACCW        0x00020000
#define CCB_TWD         0x00010000
#define CCB_LCE         0x00008000
#define CCB_ACE         0x00004000
#define CCB_reserved13  0x00002000
#define CCB_MARIA       0x00001000
#define CCB_PXOR        0x00000800
#define CCB_USEAV       0x00000400
#define CCB_PACKED      0x00000200
#define CCB_POVER_MASK  0x00000180
#define CCB_PLUTPOS     0x00000040
#define CCB_BGND        0x00000020
#define CCB_NOBLK       0x00000010
#define CCB_PLUTA_MASK  0x0000000F

#define CCB_POVER_SHIFT  7
#define CCB_PLUTA_SHIFT  0

#define PMODE_PDC   ((0x00000000)<<CCB_POVER_SHIFT) /* Normal */
#define PMODE_ZERO  ((0x00000002)<<CCB_POVER_SHIFT)
#define PMODE_ONE   ((0x00000003)<<CCB_POVER_SHIFT)

/* === Cel first preamble word flags === */
#define PRE0_LITERAL    0x80000000
#define PRE0_BGND       0x40000000
#define PREO_reservedA  0x30000000
#define PRE0_SKIPX_MASK 0x0F000000
#define PREO_reservedB  0x00FF0000
#define PRE0_VCNT_MASK  0x0000FFC0
#define PREO_reservedC  0x00000020
#define PRE0_LINEAR     0x00000010
#define PRE0_REP8       0x00000008
#define PRE0_BPP_MASK   0x00000007

#define PRE0_SKIPX_SHIFT 24
#define PRE0_VCNT_SHIFT  6
#define PRE0_BPP_SHIFT   0

/* PRE0_BPP_MASK definitions */
#define PRE0_BPP_1   0x00000001
#define PRE0_BPP_2   0x00000002
#define PRE0_BPP_4   0x00000003
#define PRE0_BPP_6   0x00000004
#define PRE0_BPP_8   0x00000005
#define PRE0_BPP_16  0x00000006

/* Subtract this value from the actual vertical source line count */
#define PRE0_VCNT_PREFETCH    1


/* === Cel second preamble word flags === */
#define PRE1_WOFFSET8_MASK   0xFF000000
#define PRE1_WOFFSET10_MASK  0x03FF0000
#define PRE1_NOSWAP          0x00004000
#define PRE1_TLLSB_MASK      0x00003000
#define PRE1_LRFORM          0x00000800
#define PRE1_TLHPCNT_MASK    0x000007FF

#define PRE1_WOFFSET8_SHIFT   24
#define PRE1_WOFFSET10_SHIFT  16
#define PRE1_TLLSB_SHIFT      12
#define PRE1_TLHPCNT_SHIFT    0

#define PRE1_TLLSB_0     0x00000000
#define PRE1_TLLSB_PDC0  0x00001000 /* Normal */
#define PRE1_TLLSB_PDC4  0x00002000
#define PRE1_TLLSB_PDC5  0x00003000

/* Subtract this value from the actual word offset */
#define PRE1_WOFFSET_PREFETCH 2
/* Subtract this value from the actual pixel count */
#define PRE1_TLHPCNT_PREFETCH 1

#define PPMP_0_SHIFT 0
#define PPMP_1_SHIFT 16

#define PPMPC_1S_MASK  0x00008000
#define PPMPC_MS_MASK  0x00006000
#define PPMPC_MF_MASK  0x00001C00
#define PPMPC_SF_MASK  0x00000300
#define PPMPC_2S_MASK  0x000000C0
#define PPMPC_AV_MASK  0x0000003E
#define PPMPC_2D_MASK  0x00000001

#define PPMPC_MS_SHIFT  13
#define PPMPC_MF_SHIFT  10
#define PPMPC_SF_SHIFT  8
#define PPMPC_2S_SHIFT  6
#define PPMPC_AV_SHIFT  1

/* PPMPC_1S_MASK definitions */
#define PPMPC_1S_PDC   0x00000000
#define PPMPC_1S_CFBD  0x00008000

/* PPMPC_MS_MASK definitions */
#define PPMPC_MS_CCB         0x00000000
#define PPMPC_MS_PIN         0x00002000
#define PPMPC_MS_PDC_MFONLY  0x00004000
#define PPMPC_MS_PDC         0x00004000

/* PPMPC_MF_MASK definitions */
#define PPMPC_MF_1  0x00000000
#define PPMPC_MF_2  0x00000400
#define PPMPC_MF_3  0x00000800
#define PPMPC_MF_4  0x00000C00
#define PPMPC_MF_5  0x00001000
#define PPMPC_MF_6  0x00001400
#define PPMPC_MF_7  0x00001800
#define PPMPC_MF_8  0x00001C00

/* PPMPC_SF_MASK definitions */
#define PPMPC_SF_2   0x00000100
#define PPMPC_SF_4   0x00000200
#define PPMPC_SF_8   0x00000300
#define PPMPC_SF_16  0x00000000

/* PPMPC_2S_MASK definitions */
#define PPMPC_2S_0     0x00000000
#define PPMPC_2S_CCB   0x00000040
#define PPMPC_2S_CFBD  0x00000080
#define PPMPC_2S_PDC   0x000000C0

/* PPMPC_2D_MASK definitions */
#define PPMPC_2D_1  0x00000000
#define PPMPC_2D_2  0x00000001


#pragma pack(push,1)

struct cp1btag{
	unsigned short	c:1;
	unsigned short	pad:15;
};
struct cp2btag{
	unsigned short	c:2;
	unsigned short	pad:14;
};
typedef struct cp4btag{
	unsigned short	c:4;
	unsigned short	pad:12;
} cp4b;
struct cp6btag{
	unsigned short  c:5;
	unsigned short  pw:1;
	unsigned short	pad:10;
};
struct cp8btag{
	unsigned short	c:5;
	unsigned short	mpw:1;
	unsigned short	m:2;
	unsigned short	pad:8;
};
struct cp16btag{
	unsigned short	c:5;
	unsigned short	mb:3;
	unsigned short	mg:3;
	unsigned short	mr:3;
	unsigned short	pad:1;
	unsigned short	pw:1;
};
struct up8btag{
	unsigned short	b:2;
	unsigned short	g:3;
	unsigned short	r:3;
	unsigned short	pad:8;
};
struct up16btag{
	unsigned short	bw:1;
	unsigned short	b:4;
	unsigned short	g:5;
	unsigned short	r:5;
	unsigned short	p:1;
};
struct res16btag{
	unsigned short	b:5;
	unsigned short	g:5;
	unsigned short	r:5;
	unsigned short	p:1;
};
union pdeco{
	unsigned int	raw;
	cp1btag	c1b;
	cp2btag	c2b;
	cp4btag	c4b;
	cp6btag	c6b;
	cp8btag	c8b;
	cp16btag	c16b;
	up8btag	u8b;
	up16btag	u16b;
	res16btag	r16b;
};

struct avtag
{
	unsigned char NEG:1;
	unsigned char XTEND:1;
	unsigned char nCLIP:1;
	unsigned char dv3:2;
	unsigned char pad:3;
};
union AVS{
	avtag avsignal;
	unsigned int raw;
};
struct pixctag
{
	unsigned char	dv2:1;
	unsigned char	av:5;  // why int don't work???
	unsigned char	s2:2;
	unsigned char	dv1:2;
	unsigned char	mxf:3;
	unsigned char	ms:2;
	unsigned char	s1:1;
};

union	PXC
{
	pixctag	meaning;
	unsigned int raw;
};

#pragma pack(pop)


//*******************************************
#pragma pack(push,1)
struct MADAMDatum
{
 unsigned int mregs[2048+64];
 unsigned short PLUT[32];
 unsigned char PBUSQueue[20];
 int RMOD;
 int WMOD;
 unsigned int _madam_FSM;
};
#pragma pack(pop)
static MADAMDatum madam;

unsigned int Get_madam_FSM(){return madam._madam_FSM;};
void Set_madam_FSM(unsigned int val){madam._madam_FSM=val;};

unsigned int _madam_SaveSize()
{
        return sizeof(MADAMDatum);
}
void _madam_Save(void *buff)
{
        memcpy(buff,&madam,sizeof(MADAMDatum));
}
void _madam_Load(void *buff)
{
        memcpy(&madam,buff,sizeof(MADAMDatum));
}

#define mregs madam.mregs
#define PLUT madam.PLUT
#define PBUSQueue madam.PBUSQueue
#define RMOD madam.RMOD
#define WMOD madam.WMOD
#define _madam_FSM madam._madam_FSM
//*******************************************



 unsigned int PXOR1, PXOR2;


#define PDV(x) ((((x)-1)&3)+1)


#define MIN(x,y) (x)+(((signed int)((y)-(x))>>31&((y)-(x))))
#define MAX(x,y) (y)-(((signed int)((y)-(x))>>31&((y)-(x))))

#define TESTCLIP(cx,cy) ( ((int)cx>=0)&&((int)cx<=((CLIPXVAL)<<16))&&((int)cy>=0)&&((int)cy<=((CLIPYVAL)<<16)))

#define FLT(a) ((float)(int)(a)/65536.0)
#define  XY2OFF(a,b,c)   (  (((int)b>>1)*c/*bitmap width*/)   + (((int)(b)&1)<<1) +    (a)    )


#define PBMASK 0x80000000
#define KUP    0x08000000
#define KDN    0x10000000
#define KRI    0x04000000
#define KLE    0x02000000
#define KA     0x01000000
#define KB     0x00800000
#define KC     0x00400000
#define KP     0x00200000
#define KX     0x00100000
#define KRS    0x00080000
#define KLS    0x00040000
#define FIXP16_SHIFT     16
#define FIXP16_MAG       65536
#define FIXP16_DP_MASK   0x0000ffff
#define FIXP16_WP_MASK   0xffff0000
#define FIXP16_ROUND_UP  0x0000ffff //0x8000



// TYPES ///////////////////////////////////////////////////////////////////


// CLASSES /////////////////////////////////////////////////////////////////
unsigned int __fastcall mread(unsigned int addr);
void __fastcall mwrite(unsigned int addr, unsigned int val);
int TestInitVisual(int packed);
int Init_Line_Map();
void Init_Scale_Map();
void Init_Arbitrary_Map();
int __fastcall TexelDraw_Line(unsigned short CURPIX, unsigned short LAMV, int xcur, int ycur, int cnt);
int __fastcall TexelDraw_Scale(unsigned short CURPIX, unsigned short LAMV, int xcur, int ycur, int deltax, int deltay);
int __fastcall TexelDraw_Arbitrary(unsigned short CURPIX, unsigned short LAMV, int xA, int yA, int xB, int yB, int xC, int yC, int xD, int yD);
void __fastcall DrawPackedCel_New();
void __fastcall DrawLiteralCel_New();
void __fastcall DrawLRCel_New();
void HandleDMA8();
void DMAPBus();


unsigned int MAPPING;

// general 3D vertex class

#define INT1220(a)	 ((signed int)(a)>>20)
#define INT1220up(a) ((signed int)((a)+(1<<19))>>20)





 static struct
 {
//	int mode;
	unsigned int plutaCCBbits;
	unsigned int pixelBitsMask;
	unsigned int pmodeORmask;
	unsigned int pmodeANDmask;
	bool tmask;
 } pdec;

 unsigned int pbus=0;
 unsigned char * Mem;
 unsigned int retuval;
 unsigned int BITADDR;
 //static unsigned int * BITPTR;
 //static unsigned int * BITEND;
 unsigned int BITBUFLEN;
 unsigned int BITBUF;
 unsigned int CCBFLAGS,/*PLUTDATA*/PIXC,PRE0,PRE1,TARGETPROJ,SRCDATA,debug;
 int SPRWI,SPRHI;
	unsigned int PLUTF,PDATF,NCCBF;
 int CELCYCLES,__smallcicles;
 bool ADD;
 //static SDL_Event cpuevent;
 int BITCALC;


 unsigned short bitbuf; //bit buffer
 unsigned char  subbitbuf;// bit sub buffer
 int    bitcount; // bit counter
 long   compsize; // size of commpressed!!! in bytes!!! actually pixcount*bpp/8!!!
 unsigned int gFINISH;
 unsigned short RRR;
 int USECEL;

 unsigned int	const BPP[8]={1,1,2,4,6,8,16,1};

 unsigned char PSCALAR[8][4][32];

 unsigned short MAPu8b[256+64], MAPc8bAMV[256+64], MAPc16bAMV[8*8*8+64];


 int currentrow;
 unsigned int bpp;
 unsigned int pixcount;
 unsigned int type;
 unsigned int offsetl;
 unsigned int offset;
 //static unsigned int begining;
 unsigned int eor;
  int calcx;
  int nrows;

 unsigned int pix;

 unsigned short ttt;

 unsigned int OFF;

 unsigned int pSource;

bool Transparent;

//AString str;

//CelEngine STATBits
#define STATBITS	mregs[0x28]

#define SPRON		0x10
#define SPRPAU		0x20

//CelEngine Registers
#define SPRSTRT		0x100
#define SPRSTOP		0x104
#define SPRCNTU		0x108
#define SPRPAUS		0x10c

#define CCBCTL0		mregs[0x110]
#define REGCTL0		mregs[0x130]
#define REGCTL1		mregs[0x134]
#define REGCTL2		mregs[0x138]
#define REGCTL3		mregs[0x13c]

#define CLIPXVAL	((int)mregs[0x134]&0x3ff)
#define CLIPYVAL	((int)(mregs[0x134]>>16)&0x3ff)

#define PIXSOURCE	(mregs[0x138])
#define FBTARGET	(mregs[0x13c])

#define CURRENTCCB	mregs[0x5a0]
//next ccb == 0 stop the engine
#define NEXTCCB		mregs[0x5a4]
#define PLUTDATA	mregs[0x5a8]
#define PDATA		mregs[0x5ac]
#define ENGAFETCH	mregs[0x5b0]
#define ENGALEN		mregs[0x5b4]
#define	ENGBFETCH	mregs[0x5b8]
#define ENGBLEN		mregs[0x5bc]
#define PAL_EXP		(&mregs[0x5d0])
#define CHAR_BIT  (8)



int FLOAT1612(int a)
{
	return a<<4;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//void MapCoord(poly *pol);
//void RenderPoly();



unsigned int __fastcall _madam_Peek(unsigned int addr)
{

//	if((addr>=0x400)&&(addr<=0x53f))
//		printf("#Madam Peek [%X]=%X\n",addr,mregs[addr]);


	if((addr>=0x400)&&(addr<=0x53f))
		{
		   //we need to return actual fifo status!!!!
		return _clio_FIFOStruct(addr);
		}


	if(addr==0x28) // STATUS OF CEL
		{
			switch(_madam_FSM)
			{
				case FSM_IDLE:return 0x0;
				case FSM_SUSPENDED:return 0x30;
				case FSM_INPROCESS:return 0x10;
			}
		}
	if(addr>=0x580 && addr<0x5A0)
	{
		//sprintf(str,"CLUT - MADAM Read madam[0x%X]\n",addr);
		//CDebug::DPrint(str);
	}
	return mregs[addr];
}


void __fastcall _madam_Poke(unsigned int addr, unsigned int val)
{
 if(addr>0x2ff && addr<0x400)
 {
      //  io_interface(EXT_DEBUG_PRINT,(void*)str.print("MADAM Write madam[0x%X] = 0x%8.8X\n",addr,val).CStr());
 }
 /*
 if(addr==0x13c)
 {
		sprintf(str,"Switch screen SWI 0x%8X addr 0x%8X\n",last_SWI,val);
		CDebug::DPrint(str);
 }*/

	if((addr>=0x400)&&(addr<=0x53f))
	{

		_clio_SetFIFO(addr,val);

		return;

	}
	else
		switch(addr)
	{
	case 0x4:
		val=0x29;
		mregs[addr]=val;
		break;

	case 0x8:
		mregs[addr]=val;
		HandleDMA8();
		break;
	case 0x580:
		_vdl_ProcessVDL(val);
		return;
	case 0x584:
	case 0x588:
	case 0x58C:
	case 0x590:
	case 0x594:
	case 0x598:
	case 0x59C:
		//_3do_DPrint(str.print("CLUT - MADAM Write madam[0x%X] = 0x%8.8X\n",addr,val));
		mregs[addr]=val;
		return;

	case 0x0:
		io_interface(EXT_KPRINT,(void*)val);
		return;
	case SPRSTRT:
		if(_madam_FSM==FSM_IDLE)
			_madam_FSM=FSM_INPROCESS;
		return;

	case SPRSTOP:
		_madam_FSM=FSM_IDLE;
		NEXTCCB=0;
		return;

	case SPRCNTU:
		if(_madam_FSM==FSM_SUSPENDED)
			_madam_FSM=FSM_INPROCESS;
		return;

	case SPRPAUS:
		if(_madam_FSM==FSM_INPROCESS)
			_madam_FSM=FSM_SUSPENDED;
		return;

//Matrix engine macros
#define M00  ((double)(signed int)mregs[0x600])
#define M01  ((double)(signed int)mregs[0x604])
#define M02  ((double)(signed int)mregs[0x608])
#define M03  ((double)(signed int)mregs[0x60C])
#define M10  ((double)(signed int)mregs[0x610])
#define M11  ((double)(signed int)mregs[0x614])
#define M12  ((double)(signed int)mregs[0x618])
#define M13  ((double)(signed int)mregs[0x61C])
#define M20  ((double)(signed int)mregs[0x620])
#define M21  ((double)(signed int)mregs[0x624])
#define M22  ((double)(signed int)mregs[0x628])
#define M23  ((double)(signed int)mregs[0x62C])
#define M30  ((double)(signed int)mregs[0x630])
#define M31  ((double)(signed int)mregs[0x634])
#define M32  ((double)(signed int)mregs[0x638])
#define M33  ((double)(signed int)mregs[0x63C])

#define  V0  ((double)(signed int)mregs[0x640])
#define  V1  ((double)(signed int)mregs[0x644])
#define  V2  ((double)(signed int)mregs[0x648])
#define  V3  ((double)(signed int)mregs[0x64C])

#define Rez0 mregs[0x660]
#define Rez1 mregs[0x664]
#define Rez2 mregs[0x668]
#define Rez3 mregs[0x66C]

//#define Nfrac16 ((__int64)mregs[0x680]<<32|(unsigned int)mregs[0x684])
#define Nfrac16 (((__int64)mregs[0x680]<<32)|(unsigned int)mregs[0x684])

// Matix engine


    case 0x7fc:

                mregs[0x7fc]=0; // Our matrix engine always ready

		static int Rez0T,Rez1T,Rez2T,Rez3T;
               // io_interface(EXT_DEBUG_PRINT,(void*)str.print("MADAM Write madam[0x%X] = 0x%8.8X\n",addr,val).CStr());

		switch(val) // Cmd
		{
			case 0: //printf("#Matrix = NOP\n");
				Rez0=Rez0T;
				Rez1=Rez1T;
				Rez2=Rez2T;
				Rez3=Rez3T;
				return;   // NOP


			case 1: //multiply a 4x4 matrix of 16.16 values by a vector of 16.16 values

				Rez0=Rez0T;
				Rez1=Rez1T;
				Rez2=Rez2T;
				Rez3=Rez3T;


				Rez0T=(int)((M00*V0+M01*V1+M02*V2+M03*V3)/65536.0);
				Rez1T=(int)((M10*V0+M11*V1+M12*V2+M13*V3)/65536.0);
				Rez2T=(int)((M20*V0+M21*V1+M22*V2+M23*V3)/65536.0);
				Rez3T=(int)((M30*V0+M31*V1+M32*V2+M33*V3)/65536.0);

				return;
			case 2: //multiply a 3x3 matrix of 16.16 values by a vector of 16.16 values
				Rez0=Rez0T;
				Rez1=Rez1T;
				Rez2=Rez2T;
				Rez3=Rez3T;

				Rez0T=(int)((M00*V0+M01*V1+M02*V2)/65536.0);
				Rez1T=(int)((M10*V0+M11*V1+M12*V2)/65536.0);
				Rez2T=(int)((M20*V0+M21*V1+M22*V2)/65536.0);
				//printf("#Matrix CMD2, R0=0x%8.8X, R1=0x%8.8X, R2=0x%8.8X\n",Rez0,Rez1,Rez2);
				return;

			case 3: // Multiply a 3x3 matrix of 16.16 values by multiple vectors, then multiply x and y by n/z
				{   // Return the result vectors {x*n/z, y*n/z, z}


					Rez0=Rez0T;
					Rez1=Rez1T;
					Rez2=Rez2T;
					Rez3=Rez3T;

					double M;

					Rez2T=(signed int)((M20*V0+M21*V1+M22*V2)/65536.0); // z
					if(Rez2T!=0)
						M=Nfrac16/(double)Rez2T;          // n/z
					else
						{
							M=Nfrac16;
							io_interface(EXT_DEBUG_PRINT,(void*)"!!!Division by zero!!!\n");
						}

					Rez0T=(signed int)((M00*V0+M01*V1+M02*V2)/65536.0);
					Rez1T=(signed int)((M10*V0+M11*V1+M12*V2)/65536.0);


					Rez0T=(int)((Rez0T*M)/65536.0/65536.0); // x * n/z
					Rez1T=(int)((Rez1T*M)/65536.0/65536.0); // y * n/z

				}
				return;
				default:
					//io_interface(EXT_DEBUG_PRINT,(void*)str.print("??? Unknown cmd MADAM[0x7FC]==0x%x\n", val).CStr());
					return;
		}
        break;
	case 0x130:
		mregs[addr]=val;	//modulo variables :)
		RMOD=((val&1)<<7)+((val&12)<<8)+((val&0x70)<<4);
		val>>=8;
		WMOD=((val&1)<<7)+((val&12)<<8)+((val&0x70)<<4);
		break;
	default:
		mregs[addr]=val;
		break;
	}


}
unsigned int OFFSET;
unsigned int temp1;
unsigned int Flag;

double HDDX,HDDY,HDX,HDY,VDX,VDY,XPOS,YPOS,HDX_2,HDY_2;

int HDDX1616,HDDY1616,HDX1616,HDY1616,VDX1616,VDY1616,XPOS1616,YPOS1616,HDX1616_2,HDY1616_2;
char	TEXEL_FUN_NUMBER;
int TEXTURE_WI_START,TEXTURE_HI_START,TEXEL_INCX,TEXEL_INCY;
int TEXTURE_WI_LIM, TEXTURE_HI_LIM;


void LoadPLUT(unsigned int pnt,int n)
{
 int i;
 	for(i=0;i<n;i++)
	{
			PLUT[i]=_mem_read16((((pnt>>1)+i)^1)<<1);
	}
}

int CCBCOUNTER;
int _madam_HandleCEL()
{


	__smallcicles=CELCYCLES=0;
	if(NEXTCCB!=0)CCBCOUNTER=0;
        STATBITS|=SPRON;
	Flag=0;


        while((NEXTCCB!=0)&&(!Flag))
	//if(_madam_FSM==FSM_INPROCESS)
	{
		CCBCOUNTER++;
		if((NEXTCCB==0)||(Flag))
			{
				_madam_FSM=FSM_IDLE;
				return CELCYCLES;
			}
//1st step -- parce CCB and load it into registers
		CURRENTCCB=NEXTCCB&0xfffffc;
		if((CURRENTCCB>>20)>2)
		{
				_madam_FSM=FSM_IDLE;
				return CELCYCLES;
		}
		OFFSET=CURRENTCCB;


		CCBFLAGS=mread(CURRENTCCB);

		CURRENTCCB+=4;


		if(CCBFLAGS&CCB_PXOR)
		{
			PXOR1=0;
			PXOR2=0x1f1f1f1f;
		}
		else
		{
			PXOR1=0xFFffFFff;
			PXOR2=0;
		}
		Flag=0;
		PLUTF=PDATF=NCCBF=0;

		NEXTCCB=mread(CURRENTCCB)&(~3);


		if(!(CCBFLAGS&CCB_NPABS))
		{
				NEXTCCB+=CURRENTCCB+4;
				NEXTCCB&=0xffffff;
		}
		if((NEXTCCB==0))
			NCCBF=1;
		if((NEXTCCB>>20)>2)
			NCCBF=1;

		CURRENTCCB+=4;


		PDATA=mread(CURRENTCCB)&(~3);
		//if((PDATA==0))
		//	PDATF=1;
		if(!(CCBFLAGS&CCB_SPABS))
		{
			PDATA+=CURRENTCCB+4;
			PDATA&=0xffffff;
		}
		if((PDATA>>20)>2)
			PDATF=1;
		CURRENTCCB+=4;

        if((CCBFLAGS&CCB_LDPLUT))
        {
               PLUTDATA=mread(CURRENTCCB)&(~3);
               //if((PLUTDATA==0))
               //    PLUTF=1;
               if(!(CCBFLAGS&CCB_PPABS))
			   {
                   PLUTDATA+=CURRENTCCB+4;
				   PLUTDATA&=0xffffff;
			   }
               if((PLUTDATA>>20)>2)
				   PLUTF=1;
        }
	    CURRENTCCB+=4;


		if(NCCBF)
			CCBFLAGS|=CCB_LAST;


		if(CCBFLAGS&CCB_LAST)
			Flag=1;


		if(CCBFLAGS&CCB_YOXY)
		{
			XPOS1616=mread(CURRENTCCB);
                        XPOS=XPOS1616/65536.0;
			CURRENTCCB+=4;
			YPOS1616=mread(CURRENTCCB);
                        YPOS=YPOS1616/65536.0;
			CURRENTCCB+=4;
		}
		else
			CURRENTCCB+=8;

		//if((CCBFLAGS&CCB_SKIP)&& debug)
		//	printf("###Cel skipped!!! PDATF=%d PLUTF=%d NCCBF=%d\n",PDATF,PLUTF,NCCBF);


			if(CCBFLAGS&CCB_LAST)
				NEXTCCB=0;
			if(CCBFLAGS&CCB_LDSIZE)
			{
				HDX1616=((int)mread(CURRENTCCB))>>4;
                                HDX=HDX1616/65536.0;
				CURRENTCCB+=4;
				HDY1616=((int)mread(CURRENTCCB))>>4;
                                HDY=HDY1616/65536.0;
				CURRENTCCB+=4;
				VDX1616=mread(CURRENTCCB);
                                VDX=VDX1616/65536.0;
				CURRENTCCB+=4;
				VDY1616=mread(CURRENTCCB);
                                VDY=VDY1616/65536.0;
				CURRENTCCB+=4;
			}
			if(CCBFLAGS&CCB_LDPRS)
			{
				HDDX1616=((int)mread(CURRENTCCB))>>4;
                                HDDX=HDDX1616/65536.0;
				CURRENTCCB+=4;
				HDDY1616=((int)mread(CURRENTCCB))>>4;
                                HDDY=HDDY1616/65536.0;
				CURRENTCCB+=4;
			}
			if(CCBFLAGS&CCB_LDPPMP)
			{
				PIXC=mread(CURRENTCCB);
				CURRENTCCB+=4;
			}
			if(CCBFLAGS&CCB_CCBPRE)
			{
				PRE0=mread(CURRENTCCB);
				CURRENTCCB+=4;
				if(!(CCBFLAGS&CCB_PACKED))
				{
					PRE1=mread(CURRENTCCB);
					CURRENTCCB+=4;
				}
			}
			else if(!PDATF)
			{
				PRE0=mread(PDATA);
				PDATA+=4;
				if(!(CCBFLAGS&CCB_PACKED))
				{
					PRE1=mread(PDATA);
					PDATA+=4;
				}
			}

			{// PDEC data compute
				//pdec.mode=PRE0&PRE0_BPP_MASK;
				switch(PRE0&PRE0_BPP_MASK)
					{
						case 0:
						case 7:
							continue;
						case 1:
							pdec.plutaCCBbits=(CCBFLAGS&0xf)*4;
							pdec.pixelBitsMask=1; // 1 bit
							break;
						case 2:
							pdec.plutaCCBbits=(CCBFLAGS&0xe)*4;
							pdec.pixelBitsMask=3; // 2 bit
							break;
						default://case 3:
							pdec.plutaCCBbits=(CCBFLAGS&0x8)*4;
							pdec.pixelBitsMask=15; // 4 bit
							break;
				}
				pdec.tmask=!(CCBFLAGS&CCB_BGND);
				pdec.pmodeORmask= ((CCBFLAGS&CCB_POVER_MASK)==PMODE_ONE )? 0x8000:0x0000;
				pdec.pmodeANDmask=((CCBFLAGS&CCB_POVER_MASK)!=PMODE_ZERO)? 0xFFFF:0x7FFF;

			}

			if((CCBFLAGS&CCB_LDPLUT) && !PLUTF) //load PLUT
		    {
			   switch(PRE0&PRE0_BPP_MASK)
			   {
			   case 1:
				   LoadPLUT(PLUTDATA,2);
				   break;
			   case 2:
				   LoadPLUT(PLUTDATA,4);
				   break;
			   case 3:
				   LoadPLUT(PLUTDATA,16);
				   break;
			   default:
				   LoadPLUT(PLUTDATA,32);
			   };
		    }

//ok -- CCB decoded -- let's print out our current status
//step#2 -- getting CEL data
			//*
		if(!(CCBFLAGS&CCB_SKIP) && !PDATF)
		{
			if(CCBFLAGS&CCB_PACKED)
			{
				DrawPackedCel_New();
			}
			else
			{

				if((PRE1&PRE1_LRFORM)&&(BPP[PRE0&PRE0_BPP_MASK]==16))
					DrawLRCel_New();
				else
					DrawLiteralCel_New();

			}

		}//if(!(CCBFLAGS& CCB_SKIP))
	}//while

	//STATBITS&=~SPRON;
		if((NEXTCCB==0)||(Flag))
			{
				_madam_FSM=FSM_IDLE;
			}

        return CELCYCLES;
}//HandleCEL



void HandleDMA8()
{
	if(mregs[0x8]&0x8000)// pbus transfer
	{
		DMAPBus();
		mregs[0x8]&=~0x8000; // dma done

		_clio_GenerateFiq(0,1);
	}
}


void DMAPBus()
{
    unsigned int i=0;

	if((int)mregs[0x574]<0)
		return;

	mregs[0x574]-=4;
	mregs[0x570]+=4;
	mregs[0x578]+=4;

	while((int)mregs[0x574]>0)
	{
			if(i<5) WriteIO(mregs[0x570],((unsigned int*)PBUSQueue)[i]);
			else WriteIO(mregs[0x570],0xffffffff);
			mregs[0x574]-=4;
			mregs[0x570]+=4;
			mregs[0x578]+=4;
			i++;
	}

	mregs[0x574]=0xfffffffc;
}

void _madam_KeyPressed(unsigned char* data, unsigned int num)
{
	if(num>16)num=16;
	if(num)memcpy(PBUSQueue,data,num);
	memset(&PBUSQueue[num],-1,20-num);
}


void _madam_Init(unsigned char *memory)
{
	int i,j,n;
	ADD=0;
	debug=0;
	USECEL=1;
	CELCYCLES=0;
	Mem=memory;

	MAPPING=1;

	_madam_FSM=FSM_IDLE;

	for(i=0;i<2048;i++)
		mregs[i]=0;

    mregs[004]=0x29;		// DRAM dux init
    mregs[574]=0xfffffffc;

#if 1
	mregs[000]=0x01020000; // for Green matrix engine autodetect
        //mregs[000]=0x02022000; // for Green matrix engine autodetect
#else
	mregs[000]=0x01020001; // for ARM soft emu of matrix engine
#endif



	for(i=0;i<32;i++)
		for(j=0;j<8;j++)
			for(n=0;n<4;n++)
			{
				PSCALAR[j][n][i]=((i*(j+1))>>PDV(n));
			}

	for(i=0;i<256;i++)
	{
		pdeco	pix1,pix2;
		unsigned short pres, resamv;

			pix1.raw=i;
			pix2.r16b.b=(pix1.u8b.b<<3)+(pix1.u8b.b<<1)+(pix1.u8b.b>>1);
			pix2.r16b.g=(pix1.u8b.g<<2)+(pix1.u8b.g>>1);
			pix2.r16b.r=(pix1.u8b.r<<2)+(pix1.u8b.r>>1);
			pres=pix2.raw;
			pres&=0x7fff; //pmode=0;
			MAPu8b[i]=pres;

			resamv=(pix1.c8b.m<<1)+pix1.c8b.mpw;
			resamv=(resamv<<6)+(resamv<<3)+resamv;
			MAPc8bAMV[i]=resamv;
	}
	for(i=0;i<(8*8*8);i++)
	{
		pdeco pix1;

			pix1.raw=i<<5;
			MAPc16bAMV[i]=(pix1.c16b.mr<<6)+(pix1.c16b.mg<<3)+pix1.c16b.mb;
	}



}

extern void _3do_InternalFrame(int cicles);
void exteraclocker()
{
        if((CELCYCLES-__smallcicles)>>7)
        {
                __smallcicles=CELCYCLES;
                //_3do_InternalFrame(64);
        }
}

unsigned int __fastcall mread(unsigned int addr)
{
	unsigned int val;
#ifdef SAFEMEMACCESS
//	addr&=0x3FFFFF;
#endif
	val=_mem_read32(addr);
	CELCYCLES+=1;
        //exteraclocker();
	return val;
}

void __fastcall mwrite(unsigned int addr, unsigned int val)
{
#ifdef SAFEMEMACCESS
	addr&=0x3FFFFF;
#endif
	_mem_write32(addr,val);
	CELCYCLES+=2;
        //exteraclocker();

}

void __fastcall mwriteh(unsigned int addr, unsigned short val)
{
#ifdef SAFEMEMACCESS
	addr&=0x3fffff;
#endif
	CELCYCLES+=2;
	_mem_write16((addr^2),val);
        //exteraclocker();
}

unsigned short __fastcall mreadh(unsigned int addr)
{
#ifdef SAFEMEMACCESS
//	addr&=0x3FFFFF;
#endif
	CELCYCLES+=1;
        //exteraclocker();
	return _mem_read16((addr^2));
}

unsigned int __fastcall readPLUTDATA(unsigned int offset)
{
	CELCYCLES+=4;
	if(PLUTDATA==0)
		return 0;
	return *(unsigned short*)(PLUTDATA+(offset^2));
	//return ((unsigned short*)PAL_EXP)[((offset^2)>>1)];
}

unsigned int __fastcall PDEC(unsigned int pixel, unsigned short * amv)
{


        pdeco	pix1;
	unsigned short resamv,pres;


	pix1.raw=pixel;

	switch(PRE0&PRE0_BPP_MASK/*pdec.mode*/)
	{
		default:
		//case 1: // 1 bit
		//case 2: // 2 bits
		//case 3: // 4 bits


			pres=PLUT[(pdec.plutaCCBbits+((pix1.raw&pdec.pixelBitsMask)*2))>>1];
			//felix disable

			resamv=0x49;
			break;

		case 4:   // 6 bits

			pres=PLUT[pix1.c6b.c];
			pres=(pres&0x7FFF)+(pix1.c6b.pw<<15); //pmode=pix1.c6b.pw; ???

			resamv=0x49;
			break;

		case 5:   // 8 bits

			if(PRE0&PRE0_LINEAR)
			{
				pres=MAPu8b[pix1.raw&0xFF];

				resamv=0x49;
			}
			else
			{
				pres=PLUT[pix1.c8b.c];

				resamv=MAPc8bAMV[pix1.raw&0xFF];
			}
			break;

		case 6:  // 16 bits
		case 7:
			//*amv=0;
			//pres=0;
		    //Transparent=0;
			if((PRE0&PRE0_LINEAR))
			{

				pres=pix1.raw;
				//pres&=0x7ffe;

				//pres=0x11;
				// pres=(pres&0x7fff)+(pix1.u16b.p<<15);//pmode=pix1.u16b.p; ???
				resamv=0x49;

			}
			else
			{
				pres=PLUT[pix1.c16b.c];
				pres=(pres&0x7fff)|(pixel&0x8000);
				resamv=MAPc16bAMV[(pix1.raw>>5)&0x1FF];
				//nop: pres=(pres&0x7fff)+(pix1.c16b.pw<<15);//pmode=pix1.c16b.pw; ???
			}

			break;
	}

	*amv=resamv;

	//if(!(PRE1&PRE1_NOSWAP) && (CCBCTL0&(1<<27)))
	//			pres=(pres&0x7ffe)|((pres&0x8000)>>15)|((pres&1)<<15);

	//if(!(CCBCTL0&0x80000000))pres=(pres&0x7fff)|((CCBCTL0>>15)&0x8000);

	pres=(pres|pdec.pmodeORmask)&pdec.pmodeANDmask;


	Transparent=( ((pres&0x7fff)==0x0) & pdec.tmask );


	return pres;
}




unsigned int __fastcall PPROC(unsigned int pixel, unsigned int fpix, unsigned int amv)
{
	AVS AV;
	PXC pixc;

	pdeco	input1,out,pix1;

	//return pixel;


	pixc.raw=PIXC&0xffff;
	if((pixel&0x8000))
	{
		pixc.raw=PIXC>>16;
	}

	//pres,fpix

	//now let's select the sources
	//1. av
	//2. input1
	//3. input2
	//pixc.raw=0;

	if(CCBFLAGS&CCB_USEAV)
	{
		AV.raw=pixc.meaning.av;
	}
	else
	{
		AV.avsignal.dv3=0;
		AV.avsignal.nCLIP=0;
		AV.avsignal.XTEND=0;
		AV.avsignal.NEG=0;
	}

	if(!pixc.meaning.s1)
		input1.raw=pixel;
	else
		input1.raw=fpix;


#pragma pack(push,1)
	union
	{
		unsigned int raw;
		struct
		{
			char R;
			char B;
			char G;
			char a;
		};
	} color1, color2, AOP, BOP;
#pragma pack(pop)

	switch(pixc.meaning.s2)
	{
	case 0:
		color2.raw=0;
		break;
	case 1:
		color2.R=color2.G=color2.B=(pixc.meaning.av>>AV.avsignal.dv3);
                break;
	case 2:
		pix1.raw=fpix;
		color2.R=(pix1.r16b.r)>>AV.avsignal.dv3;
		color2.G=(pix1.r16b.g)>>AV.avsignal.dv3;
		color2.B=(pix1.r16b.b)>>AV.avsignal.dv3;
		break;
	case 3:
		pix1.raw=pixel;
		color2.R=(pix1.r16b.r)>>AV.avsignal.dv3;
		color2.G=(pix1.r16b.g)>>AV.avsignal.dv3;
		color2.B=(pix1.r16b.b)>>AV.avsignal.dv3;
		break;
	}


	switch(pixc.meaning.ms)
	{
	case 0:
		color1.R=PSCALAR[pixc.meaning.mxf][pixc.meaning.dv1][input1.r16b.r];
		color1.G=PSCALAR[pixc.meaning.mxf][pixc.meaning.dv1][input1.r16b.g];
		color1.B=PSCALAR[pixc.meaning.mxf][pixc.meaning.dv1][input1.r16b.b];
		break;
	case 1:
		color1.R=PSCALAR[(amv>>6)&7][pixc.meaning.dv1][input1.r16b.r];
		color1.G=PSCALAR[(amv>>3)&7][pixc.meaning.dv1][input1.r16b.g];
		color1.B=PSCALAR[amv&7][pixc.meaning.dv1][input1.r16b.b];
		break;
	case 2:
		pix1.raw=pixel;
		color1.R=PSCALAR[pix1.r16b.r>>2][pix1.r16b.r&3][input1.r16b.r];
		color1.G=PSCALAR[pix1.r16b.g>>2][pix1.r16b.g&3][input1.r16b.g];
		color1.B=PSCALAR[pix1.r16b.b>>2][pix1.r16b.b&3][input1.r16b.b];
		break;
	case 3:
		color1.R=PSCALAR[0][pixc.meaning.dv1][input1.r16b.r];
		color1.G=PSCALAR[0][pixc.meaning.dv1][input1.r16b.g];
		color1.B=PSCALAR[0][pixc.meaning.dv1][input1.r16b.b];
		break;
	}

	//ok -- we got the sources -- now RGB processing
	//AOP/BOP calculation
	AOP.raw=color1.raw&PXOR1;
	color1.raw&=PXOR2;


	if(AV.avsignal.NEG)
		BOP.raw=color2.raw^0x00ffffff;
	else
	{
		BOP.raw=color2.raw^color1.raw;
	}

	if(AV.avsignal.XTEND)
	{
		BOP.R=(BOP.R<<3)>>3;
		BOP.B=(BOP.B<<3)>>3;
		BOP.G=(BOP.G<<3)>>3;
	}

	color2.R=(AOP.R+BOP.R+AV.avsignal.NEG)>>pixc.meaning.dv2;
	color2.G=(AOP.G+BOP.G+AV.avsignal.NEG)>>pixc.meaning.dv2;
	color2.B=(AOP.B+BOP.B+AV.avsignal.NEG)>>pixc.meaning.dv2;


//fprintf(flog,"%d %d %02x\t%02d %02d %02d\n", pixc.meaning.s2, pixc.meaning.ms, AV.raw, color2.R, color2.G, color2.B);

	if(!AV.avsignal.nCLIP)
	{
			if(color2.R<0)  color2.R=0;
			else if(color2.R>31) color2.R=31;

			if(color2.G<0)  color2.G=0;
			else if(color2.G>31) color2.G=31;

			if(color2.B<0)  color2.B=0;
			else if(color2.B>31) color2.B=31;

	}



	out.raw=0;
	out.r16b.r=color2.R;
	out.r16b.g=color2.G;
	out.r16b.b=color2.B;

	if(!(CCBFLAGS&CCB_NOBLK) && out.raw==0) out.raw=1<<10;

	//if(!(PRE1&PRE1_NOSWAP) && (CCBCTL0&(1<<27)))
	//			out.raw=(out.raw&0x7ffe)|((out.raw&0x8000)>>15)|((out.raw&1)<<15);

	//if(!(CCBCTL0&0x80000000))out.raw=(out.raw&0x7fff)|((CCBCTL0>>15)&0x8000);

	return out.raw;
}



unsigned int * _madam_GetRegs()
{
	return mregs;
}



void __fastcall DrawPackedCel_New()
{

	unsigned int start;
	unsigned short CURPIX,LAMV;
        //int i,j;
	// RMOD=RMODULO[REGCTL0];
	// WMOD=WMODULO[REGCTL0];
        unsigned int lastaddr;
	int xcur,ycur,xvert,yvert,xdown,ydown,hdx,hdy, scipw, wcnt;
        //int accx, accy, scipstr;

        //double dxcur,dycur,dxvert,dyvert,dxdown,dydown,dhdx,dhdy;

	start = PDATA;

	nrows=(PRE0&PRE0_VCNT_MASK)>>PRE0_VCNT_SHIFT;

	bpp=BPP[PRE0&PRE0_BPP_MASK];
	offsetl=2;	if(bpp < 8) offsetl=1;

	pixcount=0;


	compsize=30;

	SPRHI=nrows+1;
	calcx=0;

	//SPRWI=0;
	if(TestInitVisual(1))return;
	xvert=XPOS1616;
	yvert=YPOS1616;
	//return;

if(TEXEL_FUN_NUMBER==0)
{
	//return;
	for(currentrow=0;currentrow<(TEXTURE_HI_LIM);currentrow++)
	{

		//Initbitoper.Read(start,300);
		bitoper.AttachBuffer(start);
		offset=bitoper.Read(offsetl<<3);
		//bitoper.Read(offsetl<<3);

		//BITCALC=((offset+2)<<2)<<5;
		lastaddr=start+((offset+2)<<2);
	//	calcx=0;
		eor=0;


		xcur=xvert;
		ycur=yvert;
		xvert+=VDX1616;
		yvert+=VDY1616;
		//if((ycur>>16)>CLIPXVAL)ycur=CLIPXVAL<<16;
		//if(ycur<0)ycur=0;

		if(TEXTURE_HI_START){TEXTURE_HI_START--;start=lastaddr;continue;}
		scipw=TEXTURE_WI_START;
		wcnt=scipw;
		while(!eor)//while not end of row
		{

			type=bitoper.Read(2);//bitoper.Read(2);
                        if( (unsigned int)(bitoper.GetBytePose()+start) >= (lastaddr))type=0;

			//pixcount=bitoper.Read(6)+1;
			pixcount=bitoper.Read(6)+1;

			if(scipw)
			{
				if(type==0) break;
				if(scipw>=(int)(pixcount))
				{
					scipw-=(pixcount);
					if(HDX1616)xcur+=HDX1616*(pixcount);
					if(HDY1616)ycur+=HDY1616*(pixcount);
					if(type==1)bitoper.Skip(bpp*pixcount);//bitoper.Skip(bpp*(pixcount));
					else if(type==3)bitoper.Skip(bpp);//bitoper.Skip(bpp);
					continue;
				}
				else
				{
					if(HDX1616)xcur+=HDX1616*(scipw);
					if(HDY1616)ycur+=HDY1616*(scipw);
					pixcount-=scipw;
					if(type==1)bitoper.Skip(bpp*scipw);//bitoper.Skip(bpp*scipw);
					scipw=0;
				}
			}
			//if(wcnt>=TEXTURE_WI_LIM)break;
			wcnt+=(pixcount);
			if(wcnt>TEXTURE_WI_LIM)
			{
				pixcount-=(wcnt-TEXTURE_WI_LIM);
				//if(pixcount>>31)break;
			}
			switch(type)
			{
				case 0: //end of row
					eor=1;
					break;
				case 1: //PACK_LITERAL
					for(pix=0;pix<pixcount;pix++)
					{
						//CURPIX=PDEC(bitoper.Read(bpp),&LAMV);
						CURPIX=PDEC(bitoper.Read(bpp),&LAMV);

						if(!Transparent)
						{

								//TexelDraw_Line(CURPIX, LAMV, xcur, ycur, 1);
								mwriteh((FBTARGET+XY2OFF((xcur>>16)<<2,ycur>>16,WMOD)),PPROC(CURPIX,mreadh((PIXSOURCE+XY2OFF((xcur>>16)<<2,ycur>>16,RMOD))),LAMV));

						}
						xcur+=HDX1616;
						ycur+=HDY1616;

					}

					break;
				case 2: //PACK_TRANSPARENT

					//	calcx+=(pixcount+1);

					if(HDX1616)xcur+=HDX1616*(pixcount);
					if(HDY1616)ycur+=HDY1616*(pixcount);

					break;
				case 3: //PACK_REPEAT

					//CURPIX=PDEC(bitoper.Read(bpp),&LAMV);
					CURPIX=PDEC(bitoper.Read(bpp),&LAMV);

					if(!Transparent)
					{

							TexelDraw_Line(CURPIX, LAMV, xcur, ycur, (pixcount));

					}
					if(HDX1616)xcur+=HDX1616*(pixcount);
					if(HDY1616)ycur+=HDY1616*(pixcount);

				break;
			}//type
			if(wcnt>=TEXTURE_WI_LIM)break;
		}//eor

		start=lastaddr;

	}
}
else if(TEXEL_FUN_NUMBER==1)
{

	for(currentrow=0;currentrow<SPRHI;currentrow++)
	{

		bitoper.AttachBuffer(start);
		offset=bitoper.Read(offsetl<<3);

		BITCALC=((offset+2)<<2)<<5;
		lastaddr=start+((offset+2)<<2);

		eor=0;

		xcur=xvert;
		ycur=yvert;
		xvert+=VDX1616;
		yvert+=VDY1616;

		while(!eor)//while not end of row
		{

			type=bitoper.Read(2);
                        if( (bitoper.GetBytePose()+start) >= (lastaddr))type=0;

			int __pix=bitoper.Read(6)+1;
			switch(type)
			{
				case 0: //end of row
					eor=1;
					break;
				case 1: //PACK_LITERAL

					while(__pix)
					{
                                                __pix--;
						CURPIX=PDEC(bitoper.Read(bpp),&LAMV);

						if(!Transparent)
						{

								if(TexelDraw_Scale(CURPIX, LAMV, xcur>>16, ycur>>16, (xcur+(HDX1616+VDX1616))>>16, (ycur+(HDY1616+VDY1616))>>16))break;

						}
						xcur+=HDX1616;
						ycur+=HDY1616;

					}

					break;
				case 2: //PACK_TRANSPARENT

					//	calcx+=(pixcount+1);
					xcur+=HDX1616*(__pix);
					ycur+=HDY1616*(__pix);
                                        __pix=0;

					break;
				case 3: //PACK_REPEAT
					CURPIX=PDEC(bitoper.Read(bpp),&LAMV);
					if(!Transparent)
					{

							if(TexelDraw_Scale(CURPIX, LAMV, xcur>>16, ycur>>16, (xcur+(HDX1616*(__pix))+VDX1616)>>16, (ycur+(HDY1616*(__pix))+VDY1616)>>16))break;

					}
					xcur+=HDX1616*(__pix);
					ycur+=HDY1616*(__pix);
                                        __pix=0;
				break;
			}//type
			if(__pix)break;
		}//eor


		start=lastaddr;


	}

}
else
{

        for(currentrow=0;currentrow<SPRHI;currentrow++)
	{

		bitoper.AttachBuffer(start);
		offset=bitoper.Read(offsetl<<3);

		BITCALC=((offset+2)<<2)<<5;
		lastaddr=start+((offset+2)<<2);

		eor=0;

		xcur=xvert;
		ycur=yvert;
		hdx=HDX1616;
		hdy=HDY1616;

		xvert+=VDX1616;
		yvert+=VDY1616;
		HDX1616+=HDDX1616;
		HDY1616+=HDDY1616;


                xdown=xvert;
		ydown=yvert;

		while(!eor)//while not end of row
		{

			type=bitoper.Read(2);
                        if( (bitoper.GetBytePose()+start) >= (lastaddr))type=0;

			int __pix=bitoper.Read(6)+1;

			switch(type)
			{
				case 0: //end of row
					eor=1;
					break;
				case 1: //PACK_LITERAL

					while(__pix)
					{
						CURPIX=PDEC(bitoper.Read(bpp),&LAMV);
                                                __pix--;
						if(!Transparent)
						{

								if(TexelDraw_Arbitrary(CURPIX, LAMV, xcur, ycur, xcur+hdx, ycur+hdy, xdown+HDX1616, ydown+HDY1616, xdown, ydown))break;

						}
						xcur+=hdx;
						ycur+=hdy;
						xdown+=HDX1616;
						ydown+=HDY1616;
					}
                                        //pixcount=0;
					break;
				case 2: //PACK_TRANSPARENT

					//	calcx+=(pixcount+1);
					xcur+=hdx*(__pix);
					ycur+=hdy*(__pix);
					xdown+=HDX1616*(__pix);
					ydown+=HDY1616*(__pix);
                                        __pix=0;

					break;
				case 3: //PACK_REPEAT
					CURPIX=PDEC(bitoper.Read(bpp),&LAMV);
					if(!Transparent)
					{
							while(__pix)
                                                        {
                                                                __pix--;
                                                                if(TexelDraw_Arbitrary(CURPIX, LAMV, xcur, ycur, xcur+hdx, ycur+hdy, xdown+HDX1616, ydown+HDY1616, xdown, ydown))break;
                                                                xcur+=hdx;
					                        ycur+=hdy;
					                        xdown+=HDX1616;
					                        ydown+=HDY1616;
                                                        }
					}
                                        else
                                        {
                                                xcur+=hdx*__pix;
                                                ycur+=hdy*__pix;
                                                xdown+=HDX1616*__pix;
                                                ydown+=HDY1616*__pix;
                                                __pix=0;
                                        }
                                        //pixcount=0;

				break;
			};//type
			if(__pix) break;
		}//eor

		start=lastaddr;

	}
}
	SPRWI++;
        XPOS1616=xcur;
        XPOS=XPOS1616/65536.0;
        //YPOS1616=ycur;
}

void __fastcall DrawLiteralCel_New()
{
        int i,j,xcur,ycur,xvert,yvert,xdown,ydown,hdx,hdy;
 	unsigned short CURPIX,LAMV;
//	int get1,get2;
	// RMOD=RMODULO[REGCTL0];
	// WMOD=WMODULO[REGCTL0];



	bpp=BPP[PRE0&PRE0_BPP_MASK];
	offsetl=2;	if(bpp < 8)	offsetl=1;
	pixcount=0;
	offset=(offsetl==1)?((PRE1&PRE1_WOFFSET8_MASK)>>PRE1_WOFFSET8_SHIFT):((PRE1&PRE1_WOFFSET10_MASK)>>PRE1_WOFFSET10_SHIFT);


	SPRWI=1+(PRE1&PRE1_TLHPCNT_MASK);
	SPRHI=((PRE0&PRE0_VCNT_MASK)>>PRE0_VCNT_SHIFT)+1;

	if(TestInitVisual(0))return;
	xvert=XPOS1616;
	yvert=YPOS1616;



if(TEXEL_FUN_NUMBER==0)
{
        //רנטפעû NFS
        SPRWI-=((PRE0>>24)&0xf);
	xvert+=TEXTURE_HI_START*VDX1616;
	yvert+=TEXTURE_HI_START*VDY1616;
	PDATA+=((offset+2)<<2)*TEXTURE_HI_START;
	if(SPRWI>TEXTURE_WI_LIM)SPRWI=TEXTURE_WI_LIM;
	for(i=TEXTURE_HI_START;i<TEXTURE_HI_LIM;i++)
	{

		bitoper.AttachBuffer(PDATA);
		BITCALC=((offset+2)<<2)<<5;
		xcur=xvert+TEXTURE_WI_START*HDX1616;
		ycur=yvert+TEXTURE_WI_START*HDY1616;
                bitoper.Skip(bpp*(((PRE0>>24)&0xf)));
		if(TEXTURE_WI_START)bitoper.Skip(bpp*(TEXTURE_WI_START));

		xvert+=VDX1616;
		yvert+=VDY1616;


		for(j=TEXTURE_WI_START;j<SPRWI;j++)
		{
			CURPIX=PDEC(bitoper.Read(bpp),&LAMV);

			if(!Transparent)
			{

					//TexelDraw_Line(CURPIX, LAMV, xcur, ycur, 1);
					mwriteh((FBTARGET+XY2OFF((xcur>>16)<<2,ycur>>16,WMOD)),PPROC(CURPIX,mreadh((PIXSOURCE+XY2OFF((xcur>>16)<<2,ycur>>16,RMOD))),LAMV));

			}
			xcur+=HDX1616;
			ycur+=HDY1616;

		}
		PDATA+=(offset+2)<<2;

	}
}
else if(TEXEL_FUN_NUMBER==1)
{


        SPRWI-=((PRE0>>24)&0xf);
	for(i=0;i<SPRHI;i++)
	{

		bitoper.AttachBuffer(PDATA);
		BITCALC=((offset+2)<<2)<<5;
		xcur=xvert;
		ycur=yvert;
		xvert+=VDX1616;
		yvert+=VDY1616;
                bitoper.Skip(bpp*(((PRE0>>24)&0xf)));


		for(j=0;j<SPRWI;j++)
		{

			CURPIX=PDEC(bitoper.Read(bpp),&LAMV);


			if(!Transparent)
			{

					if(TexelDraw_Scale(CURPIX, LAMV, xcur>>16, ycur>>16, (xcur+HDX1616+VDX1616)>>16, (ycur+HDY1616+VDY1616)>>16))break;

			}
			xcur+=HDX1616;
			ycur+=HDY1616;

		}
		PDATA+=(offset+2)<<2;

	}
}
else
{


        SPRWI-=((PRE0>>24)&0xf);
	for(i=0;i<SPRHI;i++)
	{
		bitoper.AttachBuffer(PDATA);
		BITCALC=((offset+2)<<2)<<5;

		xcur=xvert;
		ycur=yvert;
		hdx=HDX1616;
		hdy=HDY1616;

		xvert+=VDX1616;
		yvert+=VDY1616;
		HDX1616+=HDDX1616;
		HDY1616+=HDDY1616;

                bitoper.Skip(bpp*(((PRE0>>24)&0xf)));


		xdown=xvert;
		ydown=yvert;

		for(j=0;j<SPRWI;j++)
		{

                        CURPIX=PDEC(bitoper.Read(bpp),&LAMV);


			if(!Transparent)
			{
					if(TexelDraw_Arbitrary(CURPIX, LAMV, xcur, ycur, xcur+hdx, ycur+hdy, xdown+HDX1616, ydown+HDY1616, xdown, ydown))break;

			}
			xcur+=hdx;
			ycur+=hdy;
			xdown+=HDX1616;
			ydown+=HDY1616;
		}
		PDATA+=(((offset+2)<<2)/*scipstr*/);


	}
}

        XPOS1616=xcur;
        XPOS=XPOS1616/65536.0;
        //YPOS1616=ycur;
}

void __fastcall DrawLRCel_New()
{
	int i,j,xcur,ycur,xvert,yvert,xdown,ydown,hdx,hdy;
 	unsigned short CURPIX,LAMV;


	bpp=BPP[PRE0&PRE0_BPP_MASK];
	offsetl=2;	if(bpp < 8)	offsetl=1;
	pixcount=0;
	offset=(offsetl==1)?((PRE1&PRE1_WOFFSET8_MASK)>>PRE1_WOFFSET8_SHIFT):((PRE1&PRE1_WOFFSET10_MASK)>>PRE1_WOFFSET10_SHIFT);
	offset+=2;

	SPRWI=1+(PRE1&PRE1_TLHPCNT_MASK);
	SPRHI=(((PRE0&PRE0_VCNT_MASK)>>PRE0_VCNT_SHIFT)<<1)+1;

	if(TestInitVisual(0))return;
	xvert=XPOS1616;
	yvert=YPOS1616;

if(TEXEL_FUN_NUMBER==0)
{
	xvert+=TEXTURE_HI_START*VDX1616;
	yvert+=TEXTURE_HI_START*VDY1616;
	//if(SPRHI>TEXTURE_HI_LIM)SPRHI=TEXTURE_HI_LIM;
	if(SPRWI>TEXTURE_WI_LIM)SPRWI=TEXTURE_WI_LIM;
	for(i=TEXTURE_HI_START;i<TEXTURE_HI_LIM;i++)
	{
		xcur=xvert+TEXTURE_WI_START*HDX1616;
		ycur=yvert+TEXTURE_WI_START*HDY1616;
		xvert+=VDX1616;
		yvert+=VDY1616;


		for(j=TEXTURE_WI_START;j<SPRWI;j++)
		{
			CURPIX=PDEC(mreadh((PDATA+XY2OFF(j<<2,i,offset<<2))),&LAMV);

			if(!Transparent)
			{
					//TexelDraw_Line(CURPIX, LAMV, xcur, ycur, 1);
					mwriteh((FBTARGET+XY2OFF((xcur>>16)<<2,ycur>>16,WMOD)),PPROC(CURPIX,mreadh((PIXSOURCE+XY2OFF((xcur>>16)<<2,ycur>>16,RMOD))),LAMV));
			}

			xcur+=HDX1616;
			ycur+=HDY1616;
		}

	}
}
else if(TEXEL_FUN_NUMBER==1)
{


	for(i=0;i<SPRHI;i++)
	{


		xcur=xvert;
		ycur=yvert;
		xvert+=VDX1616;
		yvert+=VDY1616;


		for(j=0;j<SPRWI;j++)
		{

			CURPIX=PDEC(mreadh((PDATA+XY2OFF(j<<2,i,offset<<2))),&LAMV);

			if(!Transparent)
			{

					if(TexelDraw_Scale(CURPIX, LAMV, xcur>>16, ycur>>16, (xcur+HDX1616+VDX1616)>>16, (ycur+HDY1616+VDY1616)>>16))break;

			}
			xcur+=HDX1616;
			ycur+=HDY1616;


		}

	}
}
else
{
        //return;

        for(i=0;i<SPRHI;i++)
	{


		xcur=xvert;
		ycur=yvert;
		xvert+=VDX1616;
		yvert+=VDY1616;
		xdown=xvert;
		ydown=yvert;
		hdx=HDX1616;
		hdy=HDY1616;
		HDX1616+=HDDX1616;
		HDY1616+=HDDY1616;


		for(j=0;j<SPRWI;j++)
		{
                        CURPIX=PDEC(mreadh((PDATA+XY2OFF(j<<2,i,offset<<2))),&LAMV);

			if(!Transparent)
			{

					if(TexelDraw_Arbitrary(CURPIX, LAMV, xcur, ycur, xcur+hdx, ycur+hdy, xdown+HDX1616, ydown+HDY1616, xdown, ydown))break;

			}

			xcur+=hdx;
			ycur+=hdy;
			xdown+=HDX1616;
			ydown+=HDY1616;


		}


	}
}

        XPOS1616=xcur;
        XPOS=XPOS1616/65536.0;
        //YPOS1616=ycur;

}



unsigned int _madam_GetCELCycles()
{
	unsigned int val=CELCYCLES; // 1 word = 2 CELCYCLES, 1 hword= 1 CELCYCLE, 8 CELCYCLE=1 CPU SCYCLE
	CELCYCLES=0;
	return val;
}


void _madam_Reset()
{
int i;
for(i=0;i<2048;i++)
	mregs[i]=0;
}


void _madam_SetMapping(unsigned int flag)
{
	MAPPING=flag;
}



#define ROAN_SHIFT 16
#define ROAN_TYPE int

#include <math.h>

unsigned int TexelCCWTest(double hdx, double hdy, double vdx, double vdy)
{
        if(((hdx+vdx)*(hdy-vdy)+vdx*vdy-hdx*hdy)<0.0)return CCB_ACCW;
        return CCB_ACW;
}
bool QuardCCWTest(int wdt)
{
 unsigned int tmp;
        if(((CCBFLAGS&CCB_ACCW)) && ((CCBFLAGS&CCB_ACW)))return false;

        tmp=TexelCCWTest(HDX,HDY,VDX,VDY);
        if(tmp!=TexelCCWTest(HDX,HDY,VDX+(HDDX)*(float)wdt,VDY+(HDDY)*(float)wdt))return false;
        if(tmp!=TexelCCWTest(HDX+(HDDX)*SPRHI,HDY+(HDDY)*SPRHI,VDX,VDY))return false;
        if(tmp!=TexelCCWTest(HDX+(HDDX)*SPRHI,HDY+(HDDY)*SPRHI,VDX+(HDDX)*(float)SPRHI*(float)wdt,VDY+(HDDY)*(float)SPRHI*(float)wdt))return false;
        if(tmp==(CCBFLAGS&(CCB_ACCW|CCB_ACW)))
                return true;
        return false;
}

__inline int __abs(int val)
{
	if(val>0)return val;
	return -val;
}
int TestInitVisual(int packed)
{
	int xpoints[4],ypoints[4];

	if((!(CCBFLAGS&CCB_ACCW)) && (!(CCBFLAGS&CCB_ACW)))return -1;


	if(!packed)
        {
		xpoints[0]=XPOS1616>>16;
		xpoints[1]=(XPOS1616+HDX1616*SPRWI)>>16;
		xpoints[2]=(XPOS1616+VDX1616*SPRHI)>>16;
		xpoints[3]=(XPOS1616+VDX1616*SPRHI+
			(HDX1616+HDDX1616*SPRHI)*SPRWI)>>16;
		if(xpoints[0]<0 && xpoints[1]<0 && xpoints[2]<0 && xpoints[3]<0) return -1;
		if(xpoints[0]>CLIPXVAL && xpoints[1]>CLIPXVAL && xpoints[2]>CLIPXVAL && xpoints[3]>CLIPXVAL) return -1;


		ypoints[0]=YPOS1616>>16;
		ypoints[1]=(YPOS1616+HDY1616*SPRWI)>>16;
		ypoints[2]=(YPOS1616+VDY1616*SPRHI)>>16;
		ypoints[3]=(YPOS1616+VDY1616*SPRHI+
			(HDY1616+HDDY1616*SPRHI)*SPRWI)>>16;
		if(ypoints[0]<0 && ypoints[1]<0 && ypoints[2]<0 && ypoints[3]<0) return -1;
		if(ypoints[0]>CLIPYVAL && ypoints[1]>CLIPYVAL && ypoints[2]>CLIPYVAL && ypoints[3]>CLIPYVAL) return -1;
	}
        else
        {
                xpoints[0]=XPOS1616>>16;
		xpoints[1]=(XPOS1616+VDX1616*SPRHI)>>16;
                if( xpoints[0]<0 && xpoints[1]<0 && HDX1616<=0 && HDDX1616<=0 ) return -1;
                if(xpoints[0]>CLIPXVAL && xpoints[1]>CLIPXVAL && HDX1616>=0 && HDDX1616>=0 ) return -1;

                ypoints[0]=YPOS1616>>16;
		ypoints[1]=(YPOS1616+VDY1616*SPRHI)>>16;
                if(ypoints[0]<0 && ypoints[1]<0 && HDY1616<=0 && HDDY1616<=0 ) return -1;
                if(ypoints[0]>CLIPYVAL && ypoints[1]>CLIPYVAL && HDY1616>=0 && HDDY1616>=0 ) return -1;
        }

	//*
	if(HDDX1616==0 && HDDY1616==0)
	{
		if(HDX1616==0 && VDY1616==0)
		{
					if((HDY1616<0 && VDX1616>0)||(HDY1616>0 && VDX1616<0))
					{
						if((CCBFLAGS&CCB_ACW))
						{
							if(__abs(HDY1616)==0x10000 && __abs(VDX1616)==0x10000 && !((YPOS1616|XPOS1616)&0xffff))
							{
								return Init_Line_Map();
								//return 0;
							}
							else
							{
								Init_Scale_Map();
								return 0;
							}
						}
					}
					else
					{
						if((CCBFLAGS&CCB_ACCW))
						{
							if(__abs(HDY1616)==0x10000 && __abs(VDX1616)==0x10000 && !((YPOS1616|XPOS1616)&0xffff))
							{
								return Init_Line_Map();
								//return 0;
							}
							else
							{
								Init_Scale_Map();
								return 0;
							}
						}

					}
					return -1;


		}
		else if(HDY1616==0 && VDX1616==0)
		{

					if((HDX1616<0 && VDY1616>0)||(HDX1616>0 && VDY1616<0))
					{
						if((CCBFLAGS&CCB_ACCW))
						{
							if(__abs(HDX1616)==0x10000 &&	__abs(VDY1616)==0x10000 && !((YPOS1616|XPOS1616)&0xffff))
							{
								return Init_Line_Map();
								//return 0;
							}
							else
							{
								Init_Scale_Map();
								return 0;
							}
						}
					}
					else
					{
						if((CCBFLAGS&CCB_ACW))
						{
							if(__abs(HDX1616)==0x10000 &&	__abs(VDY1616)==0x10000 && !((YPOS1616|XPOS1616)&0xffff))
							{
								return Init_Line_Map();
								//return 0;
							}
							else
							{
								Init_Scale_Map();
								return 0;
							}
						}
					}
					return -1;


		}
	} //*/

        if(QuardCCWTest((!packed)?SPRWI:2048))return -1;
	Init_Arbitrary_Map();


	return 0;

}

int Init_Line_Map()
{
	TEXEL_FUN_NUMBER=0;
	TEXTURE_WI_START=0;
	TEXTURE_HI_START=0;
	TEXTURE_HI_LIM=SPRHI;
	if(HDX1616<0)
		XPOS1616-=0x8000;
	else if(VDX1616<0)
		XPOS1616-=0x8000;

	if(HDY1616<0)
		YPOS1616-=0x8000;
	else if(VDY1616<0)
		YPOS1616-=0x8000;

	if(VDX1616<0)
	{
                if(((XPOS1616)-(((SPRHI-1)<<16)>>16))<0)
			TEXTURE_HI_LIM=(XPOS1616>>16)+1;
		if(TEXTURE_HI_LIM>SPRHI)TEXTURE_HI_LIM=SPRHI;
	}
	else if(VDX1616>0)
	{
		if(((XPOS1616+(SPRHI<<16))>>16)>CLIPXVAL)
			TEXTURE_HI_LIM=CLIPXVAL-(XPOS1616>>16)+1;
	}
	if(VDY1616<0)
	{
                if(((YPOS1616)-(((SPRHI-1)<<16)>>16))<0)
			TEXTURE_HI_LIM=(YPOS1616>>16)+1;
		if(TEXTURE_HI_LIM>SPRHI)TEXTURE_HI_LIM=SPRHI;
	}
	else if(VDY1616>0)
	{
		if(((YPOS1616+(SPRHI<<16))>>16)>CLIPYVAL)
			TEXTURE_HI_LIM=CLIPYVAL-(YPOS1616>>16)+1;
	}

	if(HDX1616<0)
		TEXTURE_WI_LIM=(XPOS1616>>16)+1;
	else if(HDX1616>0)
		TEXTURE_WI_LIM=CLIPXVAL-(XPOS1616>>16)+1;

	if(HDY1616<0)
		TEXTURE_WI_LIM=(YPOS1616>>16)+1;
	else if(HDY1616>0)
		TEXTURE_WI_LIM=CLIPYVAL-(YPOS1616>>16)+1;


	if(XPOS1616<0)
	{
		if(HDX1616<0)return -1;
		else if(HDX1616>0)
			TEXTURE_WI_START=-(XPOS1616>>16);

		if(VDX1616<0)return -1;
		else if(VDX1616>0)
			TEXTURE_HI_START=-(XPOS1616>>16);
	}
	else if((XPOS1616>>16)>CLIPXVAL)
	{
		if(HDX1616>0)return -1;
		else if(HDX1616<0)
			TEXTURE_WI_START=(XPOS1616>>16)-CLIPXVAL;

		if(VDX1616>0)return -1;
		else if(VDX1616<0)
			TEXTURE_HI_START=(XPOS1616>>16)-CLIPXVAL;

	}
	if(YPOS1616<0)
	{
		if(HDY1616<0)return -1;
		else if(HDY1616>0)
			TEXTURE_WI_START=-(YPOS1616>>16);

		if(VDY1616<0)return -1;
		else if(VDY1616>0)
			TEXTURE_HI_START=-(YPOS1616>>16);
	}
	else if((YPOS1616>>16)>CLIPYVAL)
	{
		if(HDY1616>0)return -1;
		else if(HDY1616<0)
			TEXTURE_WI_START=(YPOS1616>>16)-CLIPYVAL;

		if(VDY1616>0)return -1;
		else if(VDY1616<0)
			TEXTURE_HI_START=(YPOS1616>>16)-CLIPYVAL;
	}
        //if(TEXTURE_WI_START<((PRE0>>24)&0xf))
        //        TEXTURE_WI_START=((PRE0>>24)&0xf);
	//TEXTURE_WI_START+=(PRE0>>24)&0xf;
	//if(TEXTURE_WI_START<0)TEXTURE_WI_START=0;
	//if(TEXTURE_HI_START<0)TEXTURE_HI_START=0;
	//if(TEXTURE_HI_LIM>SPRHI)TEXTURE_HI_LIM=SPRHI;
	if(TEXTURE_WI_LIM<=0)return -1;
	return 0;
}

void Init_Scale_Map()
{
 int deltax,deltay;
	TEXEL_FUN_NUMBER=1;
	if(HDX1616<0)
		XPOS1616-=0x8000;
	else if(VDX1616<0)
		XPOS1616-=0x8000;

	if(HDY1616<0)
		YPOS1616-=0x8000;
	else if(VDY1616<0)
		YPOS1616-=0x8000;

	deltax=HDX1616+VDX1616;
	deltay=HDY1616+VDY1616;
	if(deltax<0)TEXEL_INCX=-1;
	else TEXEL_INCX=1;
	if(deltay<0)TEXEL_INCY=-1;
	else TEXEL_INCY=1;

	TEXEL_INCX<<=2;

	TEXTURE_WI_START=0;
	TEXTURE_HI_START=0;
}

void Init_Arbitrary_Map()
{
	TEXEL_FUN_NUMBER=2;
	TEXTURE_WI_START=0;
	TEXTURE_HI_START=0;
}

int __fastcall TexelDraw_Line(unsigned short CURPIX, unsigned short LAMV, int xcur, int ycur, int cnt)
{
        int i=0;
	unsigned int pixel;
	unsigned int curr=0xffffffff, next;

        xcur>>=16;
        ycur>>=16;

	for(i=0;i<cnt;i++,xcur+=(HDX1616>>16),ycur+=(HDY1616>>16))
	{
                next=mreadh((PIXSOURCE+XY2OFF((xcur)<<2,ycur,RMOD)));
                if(next!=curr){curr=next;pixel=PPROC(CURPIX,next,LAMV);}
                //pixel=PPROC(CURPIX,mreadh((PIXSOURCE+XY2OFF((xcur>>16)<<2,ycur>>16,RMOD))),LAMV);
                mwriteh((FBTARGET+XY2OFF((xcur)<<2,ycur,WMOD)),pixel);
	}
 return 0;
}


__inline uint16 readPIX(uint32 src, int i, int j)
{
        src+=XY2OFF((((j)>>(RESSCALE))<<2),(i>>RESSCALE),WMOD);
        if(RESSCALE)
          return *((uint16*)&Mem[(src^2)+(((i&1)<<1)+((j)&1))*1024*1024]);
        return *((uint16*)&Mem[src^2]);
}

__inline void writePIX(uint32 src, int i, int j, uint16 pix)
{
        src+=XY2OFF((((j)>>(RESSCALE))<<2),(i>>RESSCALE),WMOD);
        if(RESSCALE)
                *((uint16*)&Mem[(src^2)+(((i&1)<<1)+((j)&1))*1024*1024])=pix;
        else
                *((uint16*)&Mem[src^2])=pix;
}


int __fastcall TexelDraw_Scale(unsigned short CURPIX, unsigned short LAMV, int xcur, int ycur, int deltax, int deltay)
{
	int i,j;
	unsigned int pixel;
//	unsigned int curr=-1, next;

						if((HDX1616<0) && (deltax)<0 && xcur<0)
						{
							return -1;
						}
						else if((HDY1616<0) && (deltay)<0 && ycur<0 )
						{
							return -1;
						}
						else if((HDX1616>0) && (deltax)>(CLIPXVAL) && (xcur)>(CLIPXVAL))
						{
							return -1;
						}
						else if((HDY1616>0) && ((deltay))>(CLIPYVAL) && (ycur)>(CLIPYVAL))
						{
							return -1;
						}
						else
						{
							if((((int)xcur))==(((int)deltax)))return 0;

							for(i=((int)ycur);i!=(((int)deltay));i+=TEXEL_INCY)
								for(j=(((int)xcur))<<2;j!=((((int)deltax))<<2);j+=TEXEL_INCX)
									if((TESTCLIP((j<<14),(i<<16))))
									{
										pixel=PPROC(CURPIX,mreadh((PIXSOURCE+XY2OFF(j,i,RMOD))),LAMV);
										//next=mreadh((PIXSOURCE+XY2OFF(j,i,RMOD)));
										//if(next!=curr){curr=next;pixel=PPROC(CURPIX,next,LAMV);}
										mwriteh((FBTARGET+XY2OFF(j,i,WMOD)),pixel);

									}
						}
						return 0;
}


__inline int TexelCCWTestSmp(int hdx, int hdy, int vdx, int vdy)
{
        if(((hdx+vdx)*(hdy-vdy)+vdx*vdy-hdx*hdy)<0)return CCB_ACCW;
        return CCB_ACW;
}

int __fastcall TexelDraw_Arbitrary(unsigned short CURPIX, unsigned short LAMV, int xA, int yA, int xB, int yB, int xC, int yC, int xD, int yD)
{
        int miny, maxy, i, xpoints[4], j, maxyt, maxxt, maxx;
	int updowns[4],cnt_cross, jtmp;
	unsigned int pixel;
	unsigned int curr=-1, next;

                                         xA>>=(16-RESSCALE);
                                         xB>>=(16-RESSCALE);
                                         xC>>=(16-RESSCALE);
                                         xD>>=(16-RESSCALE);
                                         yA>>=(16-RESSCALE);
                                         yB>>=(16-RESSCALE);
                                         yC>>=(16-RESSCALE);
                                         yD>>=(16-RESSCALE);

                                         if((xA)==(xB) && (xB)==(xC) && (xC)==(xD)) return 0;

                                         maxxt=((CLIPXVAL+1)<<RESSCALE);
                                         maxyt=((CLIPYVAL+1)<<RESSCALE);

                                         if(HDX1616<0 && HDDX1616<0)
                                         {
	                                        if((xA<0) && (xB<0) && (xC<0) && (xD<0))
					                return -1;
					 }
                                         if(HDX1616>0 && HDDX1616>0)
                                         {
					        if(((xA)>=maxxt) && ((xB)>=maxxt) && ((xC)>=maxxt) && ((xD)>=maxxt))
                                                        return -1;
                                         }
                                         if(HDY1616<0 && HDDY1616<0)
                                         {
					        if((yA<0) && (yB<0) && (yC<0) && (yD<0))
                                                        return -1;
					 }
                                         if(HDY1616>0 && HDDY1616>0)
                                         {
					        if(((yA)>=maxyt) && ((yB)>=maxyt) && ((yC)>=maxyt) && ((yD)>=maxyt))
					 	        return -1;
                                         }

					 {

						miny=yA;
						if(miny>yB)miny=yB;
						if(miny>yC)miny=yC;
						if(miny>yD)miny=yD;
						maxy=yA;
						if(maxy<yB)maxy=yB;
						if(maxy<yC)maxy=yC;
						if(maxy<yD)maxy=yD;

						i=(miny);
						if(i<0)i=0;
                                                if(maxy<maxyt)maxyt=maxy;


						for(;i<maxyt;i++)
						{

							cnt_cross=0;
							if(i<(yB) && i>=(yA))
							{
									xpoints[cnt_cross]=(int)((((xB-xA)*(i-yA))/(yB-yA)+xA));
									updowns[cnt_cross++]=1;
							}
							else if(i>=(yB) && i<(yA))
							{
									xpoints[cnt_cross]=(int)((((xA-xB)*(i-yB))/(yA-yB)+xB));
									updowns[cnt_cross++]=0;
							}

							if(i<(yC) && i>=(yB))
							{
									xpoints[cnt_cross]=(int)((((xC-xB)*(i-yB))/(yC-yB)+xB));
									updowns[cnt_cross++]=1;
							}
							else if(i>=(yC) && i<(yB))
							{
									xpoints[cnt_cross]=(int)((((xB-xC)*(i-yC))/(yB-yC)+xC));
									updowns[cnt_cross++]=0;
							}

							if(i<(yD) && i>=(yC))
							{
									xpoints[cnt_cross]=(int)((((xD-xC)*(i-yC))/(yD-yC)+xC));
									updowns[cnt_cross++]=1;
							}
							else if(i>=(yD) && i<(yC))
							{
									xpoints[cnt_cross]=(int)((((xC-xD)*(i-yD))/(yC-yD)+xD));
									updowns[cnt_cross++]=0;
							}

							if(cnt_cross&1)
							{
								if(i<(yA) && i>=(yD))
								{
										xpoints[cnt_cross]=(int)((((xA-xD)*(i-yD))/(yA-yD)+xD));
										updowns[cnt_cross]=1;
								}
								else if(i>=(yA) && i<(yD))
								{
										xpoints[cnt_cross]=(int)((((xD-xA)*(i-yA))/(yD-yA)+xA));
										updowns[cnt_cross]=0;
								}
							}

							if(cnt_cross!=0)
							{

								if(xpoints[0]>xpoints[1])
								{
									xpoints[1]+=xpoints[0];
									xpoints[0]=xpoints[1]-xpoints[0];
									xpoints[1]=xpoints[1]-xpoints[0];

									jtmp=updowns[0];
									updowns[0]=updowns[1];
									updowns[1]=jtmp;
								}
								if(cnt_cross>2)
								{

									if(xpoints[1]>xpoints[2])
									{

										xpoints[1]+=xpoints[2];
										xpoints[2]=xpoints[1]-xpoints[2];
										xpoints[1]=xpoints[1]-xpoints[2];

										jtmp=updowns[1];
										updowns[1]=updowns[2];
										updowns[2]=jtmp;

										if(xpoints[0]>xpoints[1])
										{
											xpoints[1]+=xpoints[0];
											xpoints[0]=xpoints[1]-xpoints[0];
											xpoints[1]=xpoints[1]-xpoints[0];

											jtmp=updowns[0];
											updowns[0]=updowns[1];
											updowns[1]=jtmp;
										}
									}
									if(xpoints[2]>xpoints[3])
									{
										xpoints[3]+=xpoints[2];
										xpoints[2]=xpoints[3]-xpoints[2];
										xpoints[3]=xpoints[3]-xpoints[2];

										jtmp=updowns[2];
										updowns[2]=updowns[3];
										updowns[3]=jtmp;
										if(xpoints[1]>xpoints[2])
										{
											xpoints[1]+=xpoints[2];
											xpoints[2]=xpoints[1]-xpoints[2];
											xpoints[1]=xpoints[1]-xpoints[2];

											jtmp=updowns[1];
											updowns[1]=updowns[2];
											updowns[2]=jtmp;
											if(xpoints[0]>xpoints[1])
											{
												xpoints[1]+=xpoints[0];
												xpoints[0]=xpoints[1]-xpoints[0];
												xpoints[1]=xpoints[1]-xpoints[0];

												jtmp=updowns[0];
												updowns[0]=updowns[1];
												updowns[1]=jtmp;
											}
										}
									}

									if( ((CCBFLAGS&CCB_ACW)&&updowns[2]==0) ||
										((CCBFLAGS&CCB_ACCW)&&updowns[2]==1))
									{
										j=xpoints[2];
										if(j<0)j=0;
										maxx=xpoints[3];
										if(maxx>maxxt)maxx=maxxt;
                                                                                for(;j<maxx;j++)
									        {
                                                                                        next=readPIX(PIXSOURCE, i, j);
									                if(next!=curr){curr=next;
                                                                                                pixel=PPROC(CURPIX,next,LAMV);
                                                                                                }
                                                                                        writePIX(FBTARGET, i, j, pixel);
									        }
									}

								}

								if( ((CCBFLAGS&CCB_ACW)&&updowns[0]==0) ||
										((CCBFLAGS&CCB_ACCW)&&updowns[0]==1))
								{
									j=xpoints[0];
									if(j<0)j=0;
									maxx=xpoints[1];
									if(maxx>maxxt)maxx=maxxt;

                                                                        for(;j<maxx;j++)
									{
                                                                                next=readPIX(PIXSOURCE, i, j);
									        if(next!=curr){curr=next;
                                                                                        pixel=PPROC(CURPIX,next,LAMV);
                                                                                        }
                                                                                writePIX(FBTARGET, i, j, pixel);
									}


								}

							}

						}


					 }
					 return 0;
}


