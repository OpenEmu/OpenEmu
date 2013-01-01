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

// VDLP.cpp: implementation of the CVDLP class.
//
//////////////////////////////////////////////////////////////////////

#include "freedoconfig.h"

#include "vdlp.h"
#include "arm.h"
#include <memory.h>

#include "freedocore.h"

extern _ext_Interface  io_interface;

/* === VDL Palette data === */
#define VDL_CONTROL     0x80000000
#define VDL_BACKGROUND	0xE0000000
#define VDL_RGBCTL_MASK 0x60000000
#define VDL_PEN_MASK    0x1F000000
#define VDL_R_MASK      0x00FF0000
#define VDL_G_MASK      0x0000FF00
#define VDL_B_MASK      0x000000FF

#define VDL_B_SHIFT       0
#define VDL_G_SHIFT       8
#define VDL_R_SHIFT       16
#define VDL_PEN_SHIFT     24
#define VDL_RGBSEL_SHIFT  29

/* VDL_RGBCTL_MASK definitions */
#define VDL_FULLRGB     0x00000000
#define VDL_REDONLY     0x60000000
#define VDL_GREENONLY   0x40000000
#define VDL_BLUEONLY    0x20000000


#pragma pack(push,1)

	struct cdmaw
	{
		unsigned int	lines:9;//0-8
		unsigned int	numword:6;//9-14
		unsigned int	prevover:1;//15
		unsigned int	currover:1;//16
		unsigned int	prevtick:1;//17
		unsigned int  abs:1;//18
		unsigned int  vmode:1;//19
		unsigned int  pad0:1;//20
		unsigned int  enadma:1;//21
		unsigned int  pad1:1;//22
		unsigned int  modulo:3;//23-25
		unsigned int  pad2:6;//26-31
	};
	union CDMW
	{
		unsigned int raw;
		cdmaw  dmaw;
	};

struct VDLDatum
{
        unsigned char CLUTB[32];
        unsigned char CLUTG[32];
        unsigned char CLUTR[32];
        unsigned int BACKGROUND;
        unsigned int HEADVDL;
        unsigned int MODULO;
        unsigned int CURRENTVDL;
        unsigned int CURRENTBMP;
        unsigned int PREVIOUSBMP;
        unsigned int OUTCONTROLL;
        CDMW CLUTDMA;
        int linedelay;
};
#pragma pack(pop)

static VDLDatum vdl;
static unsigned char * vram;

unsigned int _vdl_SaveSize()
{
        return sizeof(VDLDatum);
}
void _vdl_Save(void *buff)
{
        memcpy(buff,&vdl,sizeof(VDLDatum));
}
void _vdl_Load(void *buff)
{
        memcpy(&vdl,buff,sizeof(VDLDatum));
}

#define CLUTB vdl.CLUTB
#define CLUTG vdl.CLUTG
#define CLUTR vdl.CLUTR
#define BACKGROUND vdl.BACKGROUND
#define HEADVDL vdl.HEADVDL
#define MODULO vdl.MODULO
#define CURRENTVDL vdl.CURRENTVDL
#define CURRENTBMP vdl.CURRENTBMP
#define PREVIOUSBMP vdl.PREVIOUSBMP
#define OUTCONTROLL vdl.OUTCONTROLL
#define CLUTDMA vdl.CLUTDMA
#define linedelay vdl.linedelay


unsigned int vmreadw(unsigned int addr);
//static AString str;

void _vdl_ProcessVDL( unsigned int addr)
{
	HEADVDL=addr;

	if((addr&0xfff00000)!=0x200000)
	{
                //io_interface(EXT_DEBUG_PRINT,(void*)str.print("!!!!VDLP!!!! VDLP code out of VRAM boundaries!!! 0x%8.8X",addr).CStr());
	}

}

static const unsigned int HOWMAYPIXELEXPECTPERLINE[8] =
	{320, 384, 512, 640, 1024, 320, 320, 320};

// ###### Per line implementation ######

bool doloadclut=false;
__inline void VDLExec()
{
 unsigned int NEXTVDL,tmp;
 unsigned char ifgnorflag=0;


		tmp=vmreadw(CURRENTVDL);
		if(tmp==0) // End of list
		{
			linedelay=511;
                        doloadclut=false;
		}
		else
		{
			CLUTDMA.raw=tmp;

			if(CLUTDMA.dmaw.currover)
				CURRENTBMP=vmreadw(CURRENTVDL+4);
			if(CLUTDMA.dmaw.prevover)
				PREVIOUSBMP=vmreadw(CURRENTVDL+8);
			if(CLUTDMA.dmaw.abs)
			{
				NEXTVDL=(CURRENTVDL+vmreadw(CURRENTVDL+12)+16);
				//CDebug::DPrint("Relative offset??\n");
			}
			else
				NEXTVDL=vmreadw(CURRENTVDL+12);

			CURRENTVDL+=16;

			int nmcmd=CLUTDMA.dmaw.numword;	//nmcmd-=4;?
			for(int i=0; i<nmcmd; i++)
			{
				int cmd=vmreadw(CURRENTVDL);
				CURRENTVDL+=4;

				        if(!(cmd&0x80000000))
					{	//color value

						unsigned int coloridx=(cmd&VDL_PEN_MASK)>>VDL_PEN_SHIFT;
						if((cmd&VDL_RGBCTL_MASK)==VDL_FULLRGB)
						{
							CLUTR[coloridx]=(cmd&VDL_R_MASK)>>VDL_R_SHIFT;
							CLUTG[coloridx]=(cmd&VDL_G_MASK)>>VDL_G_SHIFT;
							CLUTB[coloridx]=(cmd&VDL_B_MASK)>>VDL_B_SHIFT;
						}
                                                else if((cmd&VDL_RGBCTL_MASK)==VDL_REDONLY)
							CLUTR[coloridx]=(cmd&VDL_R_MASK)>>VDL_R_SHIFT;
                                                else if((cmd&VDL_RGBCTL_MASK)==VDL_GREENONLY)
							CLUTG[coloridx]=(cmd&VDL_G_MASK)>>VDL_G_SHIFT;
                                                else if((cmd&VDL_RGBCTL_MASK)==VDL_BLUEONLY)
							CLUTB[coloridx]=(cmd&VDL_B_MASK)>>VDL_B_SHIFT;
					}
					else if((cmd&0xff000000)==VDL_BACKGROUND)
					{
                                                if(ifgnorflag)continue;
                			        BACKGROUND=((     cmd&0xFF    )<<16)|
									   (( cmd&0xFF00 )) |
										(((cmd>>16)&0xFF) );
                	                }
					else if((cmd&0xE0000000)==0xc0000000)
					{
							if(ifgnorflag)continue;
							OUTCONTROLL=cmd;

                                                        //io_interface(EXT_DEBUG_PRINT,(void*)str.print("????VDLP???? Strange... DPComm=0x%8.8X",cmd).CStr());
							/*if((OUTCONTROLL>>22)&7)
							{
                                                                _3do_DPrint(str.print("????VDLP???? Strange... DPComm=0x%8.8X",cmd).CStr());
							} */
							ifgnorflag=OUTCONTROLL&2;
							//if(!ifgnorflag)break;
					}
                                        else if((unsigned int)cmd==0xffffffff)
					{
						if(ifgnorflag)continue;
						for(unsigned int j=0;j<32;j++)
						{
							CLUTB[j]=CLUTG[j]=CLUTR[j]=((j&0x1f)<<3)|((j>>2)&7);
						}
					}
					else if((cmd&0xff000000)!=0xE1000000 && (cmd&0xC0000000)!=0x80000000)
					{
                                             //   io_interface(EXT_DEBUG_PRINT,(void*)str.print("::::VDLP:::: Unknown opcode... Comm=0x%8.8X",cmd).CStr());
					}

			}//for(i<nmcmd)
			CURRENTVDL=NEXTVDL;

			MODULO=HOWMAYPIXELEXPECTPERLINE[CLUTDMA.dmaw.modulo];
                        if(MODULO!=320)
                        {
                               // io_interface(EXT_DEBUG_PRINT,(void*)str.print("::::VDLP:::: Nonstandard modulo... W=%d, DMAWORD=0x%8.8X",MODULO, CLUTDMA.raw).CStr());
                        }
                        doloadclut=((linedelay=CLUTDMA.dmaw.lines)!=0);
		}
}


__inline uint32 VRAMOffEval(uint32 addr, uint32 line)
{
        return ((((~addr)&2)<<(18+RESSCALE))+((addr>>2)<<1)+1024*512*line)<<RESSCALE;
}

void _vdl_DoLineNew(int line2x, VDLFrame *frame)
{
  int y,i;
  int line=line2x&0x7ff;


	if(line==0)
	{
                doloadclut=true;
                linedelay=0;
		CURRENTVDL=HEADVDL;
		VDLExec();
	}

	y=(line-(16));

	if(linedelay==0 /*&& doloadclut*/)
	{
	    VDLExec();
	}

	if((y>=0) && (y<240))  // 256???
	{

                if(CLUTDMA.dmaw.enadma)
                {
                        if(RESSCALE)
                        {
                                        unsigned short *dst1,*dst2;
                                        unsigned int *src1,*src2,*src3,*src4;
                                        dst1=frame->lines[(y<<1)].line;
                                        dst2=frame->lines[(y<<1)+1].line;
                                        src1=(unsigned int*)(vram+((PREVIOUSBMP^2) & 0x0FFFFF));
                                        src2=(unsigned int*)(vram+((PREVIOUSBMP^2) & 0x0FFFFF)+1024*1024);
                                        src3=(unsigned int*)(vram+((CURRENTBMP^2) & 0x0FFFFF)+2*1024*1024);
                                        src4=(unsigned int*)(vram+((CURRENTBMP^2) & 0x0FFFFF)+3*1024*1024);
                                        i=320;
                                        while(i--)
                                        {
                                                *dst1++=*(unsigned short*)(src1++);
                                                *dst1++=*(unsigned short*)(src2++);
                                                *dst2++=*(unsigned short*)(src3++);
                                                *dst2++=*(unsigned short*)(src4++);
                                        }
                        }
                        else
                        {
                                unsigned short *dst;
                                unsigned int *src;
                                dst=frame->lines[y].line;
                                src=(unsigned int*)(vram+((PREVIOUSBMP^2) & 0x0FFFFF));
                                i=320;
                                while(i--)*dst++=*(unsigned short*)(src++);
                        }
                        memcpy(frame->lines[(y<<RESSCALE)].xCLUTB,CLUTB,32);
                        memcpy(frame->lines[(y<<RESSCALE)].xCLUTG,CLUTG,32);
                        memcpy(frame->lines[(y<<RESSCALE)].xCLUTR,CLUTR,32);
                        if(RESSCALE)
                        {
                                memcpy(frame->lines[(y<<RESSCALE)+1].xCLUTB,frame->lines[(y<<RESSCALE)].xCLUTB,32*3);
                        }
                }
                frame->lines[(y<<RESSCALE)].xOUTCONTROLL=OUTCONTROLL;
                frame->lines[(y<<RESSCALE)].xCLUTDMA=CLUTDMA.raw;
                frame->lines[(y<<RESSCALE)].xBACKGROUND=BACKGROUND;
                if(RESSCALE)
                {
                        frame->lines[(y<<RESSCALE)+1].xOUTCONTROLL=OUTCONTROLL;
                        frame->lines[(y<<RESSCALE)+1].xCLUTDMA=CLUTDMA.raw;
                        frame->lines[(y<<RESSCALE)+1].xBACKGROUND=BACKGROUND;
                }

	} // //if((y>=0) && (y<240))

        if(CURRENTBMP & 2)
		CURRENTBMP+=MODULO*4 - 2;
	else
		CURRENTBMP+=2;

	if(!CLUTDMA.dmaw.prevtick)
	{
		PREVIOUSBMP=CURRENTBMP;
	}
	else
	{
		if(PREVIOUSBMP & 2)
			PREVIOUSBMP+=MODULO*4 - 2;
		else
			PREVIOUSBMP+=2;
	}


	linedelay--;
	OUTCONTROLL&=~1; //Vioff1ln
}


void _vdl_Init(unsigned char *vramstart)
{
	vram=vramstart;

	static const unsigned int StartupVDL[]=
	{ // Startup VDL at addres 0x2B0000
		0x00004410, 0x002C0000, 0x002C0000, 0x002B0098,
		0x00000000, 0x01080808, 0x02101010, 0x03191919,
		0x04212121, 0x05292929, 0x06313131, 0x073A3A3A,
		0x08424242, 0x094A4A4A, 0x0A525252, 0x0B5A5A5A,
		0x0C636363, 0x0D6B6B6B, 0x0E737373, 0x0F7B7B7B,
		0x10848484, 0x118C8C8C, 0x12949494, 0x139C9C9C,
		0x14A5A5A5, 0x15ADADAD, 0x16B5B5B5, 0x17BDBDBD,
		0x18C5C5C5, 0x19CECECE, 0x1AD6D6D6, 0x1BDEDEDE,
		0x1CE6E6E6, 0x1DEFEFEF, 0x1EF8F8F8, 0x1FFFFFFF,
		0xE0010101, 0xC001002C, 0x002180EF, 0x002C0000,
		0x002C0000, 0x002B00A8, 0x00000000, 0x002C0000,
		0x002C0000, 0x002B0000
	};
	HEADVDL=0xB0000;

        for(unsigned int i=0;i<(sizeof(StartupVDL)/4);i++)
                _mem_write32(HEADVDL+i*4+1024*1024*2,StartupVDL[i]);

	//memcpy(vram+HEADVDL, StartupVDL, sizeof(StartupVDL));


	for(unsigned int i=0;i<32;i++)
	{
		CLUTB[i]=CLUTG[i]=CLUTR[i]=((i&0x1f)<<3)|((i>>2)&7);
	}
}

unsigned int vmreadw(unsigned int addr)
{
	//unsigned int val;
	//val=*(unsigned int*)(vram+(addr&0xfffff));
	//return val;

        return _mem_read32((addr&0xfffff)+1024*1024*2);

}

