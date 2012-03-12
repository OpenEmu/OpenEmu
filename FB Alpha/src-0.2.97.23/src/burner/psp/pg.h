// primitive graphics for Hello World PSP
#ifndef __PG_H__
#define __PG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RGB(r,g,b) (short)((((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|(((r>>3) & 0x1F)<<0)|0x8000)
#define RGB_WHITE  RGB(255,255,255)
#define RGB_BLACK  RGB(0,0,0)
#define RGB_BLUE   RGB(0,0,255)
#define RGB_GREEN  RGB(0,255,0)
#define RGB_RED    RGB(255,0,0)
#define RGB_YELLOW RGB(255,255,0)

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define		PIXELSIZE	1				//in short
#define		LINESIZE	512				//in short
#define		FRAMESIZE	0x44000			//in byte
#define CMAX_X 60
#define CMAX_Y 34
//480*272 = 60*34

void pgWaitVn(unsigned long count);
void pgWaitV();
unsigned char *pgGetVramAddr(unsigned long x,unsigned long y);
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str,int bg);
void pgPrintCenter(unsigned long y,unsigned long color,const char *str,int bg);
void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
void pgFillvram(unsigned long color);
void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d);
void pgBitBltN2(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgBitBltStScan(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgBitBltSt2wotop(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgBitBltSt2wobot(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);
void pgScreenFrame(long mode,long frame);
void pgScreenFlip();
void pgScreenFlipV();
unsigned _rand(unsigned long MAX);
void PutGfx(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
