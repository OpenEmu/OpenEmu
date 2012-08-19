#ifndef _XBR_H
#define _XBR_H

void xbr2x_a(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);
void xbr2x_b(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);
void xbr2x_c(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);
			
void xbr3x_a(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);
void xbr3x_b(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);
void xbr3x_c(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);

void xbr4x_a(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);
void xbr4x_b(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);
void xbr4x_c(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres);

#endif
