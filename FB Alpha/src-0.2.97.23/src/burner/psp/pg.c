// primitive graphics for Hello World sce

#include "pg.h"
#include "font.c"

#include <math.h>

#define pg_vramtop ((unsigned char *)0x04000000)
long pg_screenmode;
long pg_showframe;
long pg_drawframe=0;

void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
	sceDisplayWaitVblankStart();
}


unsigned char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+pg_drawframe*FRAMESIZE+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
}


void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str,int bg)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,bg,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgPrintCenter(unsigned long y,unsigned long color,const char *str,int bg)
{
	unsigned long x=(480-strlen(str)*8)>>4;
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,bg,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = color;
		((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = color;
		((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = color;
	}
}

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgFillvram(unsigned long color)
{

	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}

}

void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;

	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}

}

//‚æ‚­‚í‚©‚ñ‚È‚¢x2 - LCK
void pgBitBltN2(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

//by z-rwt
void pgBitBltStScan(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

void pgBitBltSt2wotop(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

void pgBitBltSt2wobot(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<h-16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
}

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	cfont=font+ch*8;
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}

void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}

void pgScreenFlip()
{
	pg_showframe=pg_drawframe;
	pg_drawframe++;
	pg_drawframe&=1;	
	sceDisplaySetFrameBuf(pg_vramtop+pg_showframe*FRAMESIZE,LINESIZE,PIXELSIZE,0);
}


void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}
/******************************************************************************/

static unsigned Seed1;   
static unsigned Seed2;

unsigned _rand(unsigned long MAX)
{
	Seed1 = (Seed1 + 46906481) ^ Seed2;
	Seed2 = Seed1 ^ ( ((Seed2<<3) | (Seed2 >> 29)) + 103995407);
   
	return (Seed1 - Seed2)%MAX;
} 

void PutGfx(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;
	
	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=2;
				}
				dd++;
			}
			vptr0+=512*2;
		}
		d+=w;
	}
	
}

