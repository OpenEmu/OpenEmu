/*
   Hyllian's Data Dependent Triangulation 3x
   
   Copyright (C) 2011, 2012 Hyllian/Jararaca - sergiogdb@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <stdlib.h>

extern "C"
{
    static unsigned char initialized = 0;
//	unsigned int	RGBtoYUV[65536];
//    static unsigned int tbl_5_to_8[32]={0, 8, 16, 25, 33, 41, 49,  58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189,  197, 206, 214, 222, 230, 239, 247, 255};
//    static unsigned int tbl_6_to_8[64]={0, 4, 8, 12, 16, 20, 24,  28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101,  105, 109, 113, 117, 121, 125, 130, 134, 138, 142, 146, 150, 154, 158, 162, 166,  170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 215, 219, 223, 227, 231,  235, 239, 243, 247, 251, 255};
}

unsigned short int ddt_red_blue_mask;
unsigned short int ddt_green_mask;

#define RED_BLUE_MASK565   0xF81F
#define RED_MASK565   0xF800
#define GREEN_MASK565 0x07E0

#define RGB_MASK 0x07E0F81F


#define ALPHA_BLEND_X_W(dst, src, alpha) \
    ts = src; td = dst;\
    td = ((td|(td<<16)) & RGB_MASK); ts = ((ts|(ts<<16)) & RGB_MASK);\
	td = ((( ( (ts-td)*alpha ) >> 5 ) + td ) & RGB_MASK); \
	dst= (td|(td>>16));\


#define BIL3X(PF, PH, PI, N5, N7, N8) \
       ALPHA_BLEND_X_W(E[N5], PF, 5); \
       ALPHA_BLEND_X_W(E[N7], PH, 5); \
       ALPHA_BLEND_X_W(E[N8], PF, 9); \
       ALPHA_BLEND_X_W(E[N8], PH, 7); \
       ALPHA_BLEND_X_W(E[N8], PI, 3); \


#define DDT3X_BC(PF, PH, PI, N5, N7, N8) \
       ALPHA_BLEND_X_W(E[N5], PF, 6); \
       ALPHA_BLEND_X_W(E[N7], PH, 6); \
       ALPHA_BLEND_X_W(E[N8], PI, 12); \


#define DDT3X_D(PF, PH, N5, N7, N8) \
       ALPHA_BLEND_X_W(E[N5], PF, 6); \
       ALPHA_BLEND_X_W(E[N7], PH, 6); \
       ALPHA_BLEND_X_W(E[N8], PF, 12); \
       ALPHA_BLEND_X_W(E[N8], PH, 12); \


#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
{\
  if (PE!=PF || PE!=PH || PE!=PI) \
  {\
    ad = abs(PE-PI); bc = abs(PF-PH);\
	if (ad < bc)\
	{\
        DDT3X_BC(PF, PH, PI, N5, N7, N8);\
	}\
	else if (ad > bc)\
	{\
        DDT3X_D(PF, PH, N5, N7, N8);\
	}\
	else\
	{\
        BIL3X(PF, PH, PI, N5, N7, N8) \
    }\
 }\
}\

        
void ddt3x(unsigned char * src,  unsigned int srcPitch,
			unsigned char * dest, unsigned int dstPitch,
			int Xres, int Yres)
{	
	unsigned short int x, y;
	unsigned short int PA, PB, PC, PD, PE, PF, PG, PH, PI;
	register unsigned short int *sa1, *sa2, *sa3; // sa = start_address
	unsigned short int nl, nl_src; // nl = new_line
    unsigned short int nl1;	
	unsigned short int *E;  // E = dst_pixel
	unsigned short int src_width = (unsigned short int)Xres; 
	unsigned short int src_height = (unsigned short int)Yres;
	unsigned short int dst_width = src_width * 3;
//	unsigned short int dst_height = src_height * 3;
	unsigned short int complete_line_src, complete_line_dst;
	unsigned short int src_pitch = (unsigned short int)srcPitch;
	unsigned char pprev;
    unsigned int ad, bc;
    unsigned int td, ts;

    if (!initialized)
    {
       ddt_red_blue_mask   = RED_BLUE_MASK565;
       ddt_green_mask = GREEN_MASK565;
       
       initialized = 1;
    }
	
	nl_src = src_pitch >> 1;
	nl = (unsigned short int)dstPitch >> 1;
	nl1= (unsigned short int)dstPitch;

	// fixed by Steve Snake
	complete_line_src = (src_pitch>>1) - src_width;
	complete_line_dst = ((dstPitch*3)>>1) - dst_width;
	
	sa2 = (unsigned short int *)(src - 4);
    sa1 = sa2;
	sa3 = sa2 + src_pitch;
	
	E = (unsigned short int *)(dest);	
	
	y = src_height;

	while(y--)
	{	
        if (!y) sa3 = sa2;
        pprev = 2;
        x = src_width;
		
		while(x--) 
		{			
            PB = sa1[2];
			PE = sa2[2];			
			PH = sa3[2];
			
			PA = sa1[pprev];
			PD = sa2[pprev];			
			PG = sa3[pprev];
			
    		PC = sa1[3];
			PF = sa2[3];
			PI = sa3[3];
				
            if (!x)				
            {
   				PC = sa1[2];
   				PF = sa2[2];
   				PI = sa3[2];
   			}

			E[0]   = E[1]     = E[2]     = PE;
			E[nl]  = E[nl+1]  = E[nl+2]  = PE; // 3, 4, 5
			E[nl1] = E[nl1+1] = E[nl1+2] = PE; // 6, 7, 8
			
if (PE!=PH || PE!=PI || PE!=PF || PE!=PC || PE!=PB || PE!=PA || PE!=PD || PE!=PG) 
{
			
FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, 0, 1, 2, nl, nl+1, nl+2, nl1, nl1+1, nl1+2);
FILTRO(PE, PC, PF, PB, PI, PA, PH, PD, PG, nl1, nl, 0, nl1+1, nl+1, 1, nl1+2, nl+2, 2);
FILTRO(PE, PA, PB, PD, PC, PG, PF, PH, PI, nl1+2, nl1+1, nl1, nl+2, nl+1, nl, 2, 1, 0);
FILTRO(PE, PG, PD, PH, PA, PI, PB, PF, PC, 2, nl+2, nl1+2, 1, nl+1, nl1+1, 0, nl, nl1);

}
			sa1++;
			sa2++;
			sa3++;
			
            E+=3;
			
            pprev = 1;
		}
		
        sa2 += complete_line_src;
		sa1 = sa2 - nl_src;		
		sa3 = sa2 + nl_src;
		
        E += complete_line_dst;				
	}
}
