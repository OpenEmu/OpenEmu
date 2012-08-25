/*
   Hyllian's 2xBR v3.3b
   
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

#include <math.h>
#include <cstdlib>


static unsigned int RGBtoYUV[65536];
static unsigned int tbl_5_to_8[32]={0, 8, 16, 25, 33, 41, 49,  58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189,  197, 206, 214, 222, 230, 239, 247, 255};
static unsigned int tbl_6_to_8[64]={0, 4, 8, 12, 16, 20, 24,  28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101,  105, 109, 113, 117, 121, 125, 130, 134, 138, 142, 146, 150, 154, 158, 162, 166,  170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 215, 219, 223, 227, 231,  235, 239, 243, 247, 251, 255};

#define RED_MASK565   0xF800
#define RED_BLUE_MASK565 0xF81F
#define GREEN_MASK565 0x07E0
#define BLUE_MASK565  0x001F

#define RED_MASK555 0x7C00
#define GREEN_MASK555 0x03E0
#define BLUE_MASK555 0x001F

#define PG_LBMASK565 0xF7DE
#define PG_LBMASK555 0x7BDE

static const unsigned short int pg_red_mask = RED_MASK565;
static const unsigned short int pg_red_blue_mask = RED_BLUE_MASK565;
static const unsigned short int pg_green_mask = GREEN_MASK565;
static const unsigned short int pg_blue_mask = BLUE_MASK565;
static const unsigned short int pg_lbmask = PG_LBMASK565;

#define ALPHA_BLEND_128_W(dst, src) dst = ((src & pg_lbmask) >> 1) + ((dst & pg_lbmask) >> 1)

#define ALPHA_BLEND_32_W(dst, src) \
	dst = ( \
    (pg_red_blue_mask & ((dst & pg_red_blue_mask) + \
        ((((src & pg_red_blue_mask) - \
        (dst & pg_red_blue_mask))) >>3))) | \
    (pg_green_mask & ((dst & pg_green_mask) + \
        ((((src & pg_green_mask) - \
        (dst & pg_green_mask))) >>3))))

#define ALPHA_BLEND_64_W(dst, src) \
	dst = ( \
    (pg_red_blue_mask & ((dst & pg_red_blue_mask) + \
        ((((src & pg_red_blue_mask) - \
        (dst & pg_red_blue_mask))) >>2))) | \
    (pg_green_mask & ((dst & pg_green_mask) + \
        ((((src & pg_green_mask) - \
        (dst & pg_green_mask))) >>2))))

#define ALPHA_BLEND_192_W(dst, src) \
	dst = ( \
    (pg_red_blue_mask & ((dst & pg_red_blue_mask) + \
        ((((src & pg_red_blue_mask) - \
        (dst & pg_red_blue_mask)) * 3) >>2))) | \
    (pg_green_mask & ((dst & pg_green_mask) + \
        ((((src & pg_green_mask) - \
        (dst & pg_green_mask)) * 3) >>2))))

#define ALPHA_BLEND_224_W(dst, src) \
	dst = ( \
    (pg_red_blue_mask & ((dst & pg_red_blue_mask) + \
        ((((src & pg_red_blue_mask) - \
        (dst & pg_red_blue_mask)) * 7) >>3))) | \
    (pg_green_mask & ((dst & pg_green_mask) + \
        ((((src & pg_green_mask) - \
        (dst & pg_green_mask)) * 7) >>3))))


#define LEFT_UP_2_2X(N3, N2, N1, PIXEL)\
             ALPHA_BLEND_224_W(E[N3], PIXEL); \
             ALPHA_BLEND_64_W( E[N2], PIXEL); \
             E[N1] = E[N2]; \

        
#define LEFT_2_2X(N3, N2, PIXEL)\
             ALPHA_BLEND_192_W(E[N3], PIXEL); \
             ALPHA_BLEND_64_W( E[N2], PIXEL); \

#define UP_2_2X(N3, N1, PIXEL)\
             ALPHA_BLEND_192_W(E[N3], PIXEL); \
             ALPHA_BLEND_64_W( E[N1], PIXEL); \

#define DIA_2X(N3, PIXEL)\
             ALPHA_BLEND_128_W(E[N3], PIXEL); \

#define df(A, B)\
        abs((int)RGBtoYUV[A] - (int)RGBtoYUV[B])\

#define eq(A, B)\
        (df(A, B) < 155)\

static void initialize(){
    static int initialized;
    if (initialized){
        return;
    }
    initialized = 1;

    int format = 0;

    if (format == 0){ //565
        for (int c = 0; c < 65536; c++){
            unsigned int r = tbl_5_to_8[(c &   RED_MASK565) >> 11];
            unsigned int g = tbl_6_to_8[(c & GREEN_MASK565) >>  5];
            unsigned int b = tbl_5_to_8[(c &  BLUE_MASK565)      ];
            unsigned int y = ((r<<4) + (g<<5) + (b<<2));
            unsigned int u = (   -r  - (g<<1) + (b<<2));
            unsigned int v = ((r<<1) - (g<<1) - (b>>1));
            RGBtoYUV[c] = y + u + v;
        }
    } else if (format == 1){ //555
        for (int c = 0; c < 65536; c++) {
            unsigned int r = tbl_5_to_8[(c &   RED_MASK555) >> 10];
            unsigned int g = tbl_5_to_8[(c & GREEN_MASK555) >>  5];
            unsigned int b = tbl_5_to_8[(c &  BLUE_MASK555)      ];
            unsigned int y = ((r<<4) + (g<<5) + (b<<2));
            unsigned int u = (   -r  - (g<<1) + (b<<2));
            unsigned int v = ((r<<1) - (g<<1) - (b>>1));
            RGBtoYUV[c] = y + u + v;
        }
    }
}

#define xbr2x_do \
    initialize(); \
	\
    unsigned int e, i, px; \
    unsigned int ex, ex2, ex3; \
    unsigned int ke, ki; \
	\
    int nextOutputLine = dstPitch / 2; \
	\
    for (int y = 0; y < Yres; y++){ \
        unsigned short int * E = (unsigned short *)((char*) pOut + y * dstPitch * 2); \
		\
        unsigned short int * sa2 = (unsigned short *)((char*) pIn + y * srcPitch - 4); \
        unsigned short int * sa1 = sa2 - srcPitch / 2; \
        unsigned short int * sa0 = sa1 - srcPitch / 2; \
        unsigned short int * sa3 = sa2 + srcPitch / 2; \
        unsigned short int * sa4 = sa3 + srcPitch / 2; \
		\
        if (y <= 1){  \
            sa0 = sa1; \
            if (y == 0){ \
                sa0 = sa1 = sa2; \
            } \
        } \
		\
        if (y >= Yres - 2){ \
            sa4 = sa3; \
            if (y == Yres - 1){ \
                sa4 = sa3 = sa2; \
            } \
        } \
		\
        unsigned char pprev; \
        unsigned char pprev2; \
        pprev = pprev2 = 2; \
		\
        for (int x = 0; x < Xres; x++){ \
            unsigned short B1 = sa0[2]; \
            unsigned short PB = sa1[2]; \
            unsigned short PE = sa2[2]; \
            unsigned short PH = sa3[2]; \
            unsigned short H5 = sa4[2]; \
			\
            unsigned short A1 = sa0[pprev]; \
            unsigned short PA = sa1[pprev]; \
            unsigned short PD = sa2[pprev]; \
            unsigned short PG = sa3[pprev]; \
            unsigned short G5 = sa4[pprev]; \
			\
            unsigned short A0 = sa1[pprev2]; \
            unsigned short D0 = sa2[pprev2]; \
            unsigned short G0 = sa3[pprev2]; \
			\
            unsigned short C1 = 0; \
            unsigned short PC = 0; \
            unsigned short PF = 0; \
            unsigned short PI = 0; \
            unsigned short I5 = 0; \
			\
            unsigned short C4 = 0; \
            unsigned short F4 = 0; \
            unsigned short I4 = 0; \
			\
            if (x >= Xres - 2){ \
                if (x == Xres - 1){ \
                    C1 = sa0[2]; \
                    PC = sa1[2]; \
                    PF = sa2[2]; \
                    PI = sa3[2]; \
                    I5 = sa4[2]; \
					\
                    C4 = sa1[2]; \
                    F4 = sa2[2]; \
                    I4 = sa3[2]; \
                } else { \
                    C1 = sa0[3]; \
                    PC = sa1[3]; \
                    PF = sa2[3]; \
                    PI = sa3[3]; \
                    I5 = sa4[3]; \
					\
                    C4 = sa1[3]; \
                    F4 = sa2[3]; \
                    I4 = sa3[3]; \
                } \
            } else { \
                C1 = sa0[3]; \
                PC = sa1[3]; \
                PF = sa2[3]; \
                PI = sa3[3]; \
                I5 = sa4[3]; \
				\
                C4 = sa1[4]; \
                F4 = sa2[4]; \
                I4 = sa3[4]; \
            } \
			\
            E[0] = E[1] = E[nextOutputLine] = E[nextOutputLine + 1] = PE; \
			\
			FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, nextOutputLine, nextOutputLine+1); \
			FILTRO(PE, PC, PF, PB, PI, PA, PH, PD, PG, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, nextOutputLine, 0, nextOutputLine+1, 1); \
			FILTRO(PE, PA, PB, PD, PC, PG, PF, PH, PI, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, nextOutputLine+1, nextOutputLine, 1, 0); \
			FILTRO(PE, PG, PD, PH, PA, PI, PB, PF, PC, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 1, nextOutputLine+1, 0, nextOutputLine); \
			\
            sa0 += 1; \
            sa1 += 1; \
            sa2 += 1; \
            sa3 += 1; \
            sa4 += 1; \
			\
            E += 2; \
			\
            if (pprev2){ \
                pprev2--; \
                pprev = 1; \
            } \
        } \
    } \

void xbr2x_a(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if ((e<i)  && ( (!eq(PF,PB) && !eq(PF,PC)) || (!eq(PH,PD) && !eq(PH,PG)) || ((eq(PE,PI) && (!eq(PF,F4) && !eq(PF,I4))) || (!eq(PH,H5) && !eq(PH,I5))) || eq(PE,PG) || eq(PE,PC)) )\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2_2X(N3, N2, N1, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2_2X(N3, N2, px);\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2_2X(N3, N1, px);\
              }\
              else \
              {\
                     DIA_2X(N3, px);\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N3], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\
	 
	xbr2x_do
	
#undef FILTRO
}

void xbr2x_b(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if ((e<i)  && ( (!eq(PF,PB) && !eq(PH,PD)) || (eq(PE,PI) && (!eq(PF,I4) && !eq(PH,I5))) || eq(PE,PG) || eq(PE,PC)) )\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2_2X(N3, N2, N1, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2_2X(N3, N2, px);\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2_2X(N3, N1, px);\
              }\
              else \
              {\
                     DIA_2X(N3, px);\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N3], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\
	 
	xbr2x_do
	
#undef FILTRO
}

void xbr2x_c(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if (e<i)\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2_2X(N3, N2, N1, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2_2X(N3, N2, px);\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2_2X(N3, N1, px);\
              }\
              else \
              {\
                     DIA_2X(N3, px);\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N3], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\
	
	xbr2x_do
	
#undef FILTRO
}

#undef xbr2x_do

#define LEFT_UP_2_3X(N7, N5, N6, N2, N8, PIXEL)\
             ALPHA_BLEND_192_W(E[N7], PIXEL); \
             ALPHA_BLEND_64_W( E[N6], PIXEL); \
             E[N5] = E[N7]; \
             E[N2] = E[N6]; \
             E[N8] =  PIXEL;\

        
#define LEFT_2_3X(N7, N5, N6, N8, PIXEL)\
             ALPHA_BLEND_192_W(E[N7], PIXEL); \
             ALPHA_BLEND_64_W( E[N5], PIXEL); \
             ALPHA_BLEND_64_W( E[N6], PIXEL); \
             E[N8] =  PIXEL;\

#define UP_2_3X(N5, N7, N2, N8, PIXEL)\
             ALPHA_BLEND_192_W(E[N5], PIXEL); \
             ALPHA_BLEND_64_W( E[N7], PIXEL); \
             ALPHA_BLEND_64_W( E[N2], PIXEL); \
             E[N8] =  PIXEL;\

#define DIA_3X(N8, N5, N7, PIXEL)\
             ALPHA_BLEND_224_W(E[N8], PIXEL); \
             ALPHA_BLEND_32_W(E[N5], PIXEL); \
             ALPHA_BLEND_32_W(E[N7], PIXEL); \

#define xbr3x_do \
    initialize(); \
	\
    const int nl = dstPitch / 2; \
    const int nl1 = nl + nl; \
	\
    for (int y = 0; y < Yres; y++){ \
        unsigned short int * E = (unsigned short *)((char*) pOut + y * dstPitch * 3); \
		\
        unsigned short int * sa2 = (unsigned short *)((char*) pIn + y * srcPitch - 4); \
        unsigned short int * sa1 = sa2 - srcPitch / 2; \
        unsigned short int * sa0 = sa1 - srcPitch / 2; \
        unsigned short int * sa3 = sa2 + srcPitch / 2; \
        unsigned short int * sa4 = sa3 + srcPitch / 2; \
		\
        if (y <= 1){  \
            sa0 = sa1; \
            if (y == 0){ \
                sa0 = sa1 = sa2; \
            } \
        } \
		\
        if (y >= Yres - 2){ \
            sa4 = sa3; \
            if (y == Yres - 1){ \
                sa4 = sa3 = sa2; \
            } \
        } \
		\
        unsigned char pprev; \
        unsigned char pprev2; \
        pprev = pprev2 = 2; \
		\
        for (int x = 0; x < Xres; x++){ \
            unsigned short B1 = sa0[2]; \
            unsigned short PB = sa1[2]; \
            unsigned short PE = sa2[2]; \
            unsigned short PH = sa3[2]; \
            unsigned short H5 = sa4[2]; \
			\
            unsigned short A1 = sa0[pprev]; \
            unsigned short PA = sa1[pprev]; \
            unsigned short PD = sa2[pprev]; \
            unsigned short PG = sa3[pprev]; \
            unsigned short G5 = sa4[pprev]; \
			\
            unsigned short A0 = sa1[pprev2]; \
            unsigned short D0 = sa2[pprev2]; \
            unsigned short G0 = sa3[pprev2]; \
			\
            unsigned short C1 = 0; \
            unsigned short PC = 0; \
            unsigned short PF = 0; \
            unsigned short PI = 0; \
            unsigned short I5 = 0; \
			\
            unsigned short C4 = 0; \
            unsigned short F4 = 0; \
            unsigned short I4 = 0; \
			\
            if (x >= Xres - 2){ \
                if (x == Xres - 1){ \
                    C1 = sa0[2]; \
                    PC = sa1[2]; \
                    PF = sa2[2]; \
                    PI = sa3[2]; \
                    I5 = sa4[2]; \
					\
                    C4 = sa1[2]; \
                    F4 = sa2[2]; \
                    I4 = sa3[2]; \
                } else { \
                    C1 = sa0[3]; \
                    PC = sa1[3]; \
                    PF = sa2[3]; \
                    PI = sa3[3]; \
                    I5 = sa4[3]; \
					\
                    C4 = sa1[3]; \
                    F4 = sa2[3]; \
                    I4 = sa3[3]; \
                } \
            } else { \
                C1 = sa0[3]; \
                PC = sa1[3]; \
                PF = sa2[3]; \
                PI = sa3[3]; \
                I5 = sa4[3]; \
				\
                C4 = sa1[4]; \
                F4 = sa2[4]; \
                I4 = sa3[4]; \
            } \
			\
            unsigned int e, i, px; \
            unsigned int ex, ex2, ex3; \
            unsigned int ke, ki; \
			\
            E[0]   = E[1]     = E[2]     = PE; \
            E[nl]  = E[nl+1]  = E[nl+2]  = PE; \
            E[nl1] = E[nl1+1] = E[nl1+2] = PE; \
			\
			FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, nl, nl+1, nl+2, nl1, nl1+1, nl1+2); \
			FILTRO(PE, PC, PF, PB, PI, PA, PH, PD, PG, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, nl1, nl, 0, nl1+1, nl+1, 1, nl1+2, nl+2, 2); \
			FILTRO(PE, PA, PB, PD, PC, PG, PF, PH, PI, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, nl1+2, nl1+1, nl1, nl+2, nl+1, nl, 2, 1, 0); \
			FILTRO(PE, PG, PD, PH, PA, PI, PB, PF, PC, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 2, nl+2, nl1+2, 1, nl+1, nl1+1, 0, nl, nl1); \
			\
            sa0 += 1; \
            sa1 += 1; \
            sa2 += 1; \
            sa3 += 1; \
            sa4 += 1; \
			\
            E += 3; \
			\
            if (pprev2){ \
                pprev2--; \
                pprev = 1; \
            } \
        } \
    } \

void xbr3x_a(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if ((e<i)  && ( (!eq(PF,PB) && !eq(PF,PC)) || (!eq(PH,PD) && !eq(PH,PG)) || ((eq(PE,PI) && (!eq(PF,F4) && !eq(PF,I4))) || (!eq(PH,H5) && !eq(PH,I5))) || eq(PE,PG) || eq(PE,PC)) )\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2_3X(N7, N5, N6, N2, N8, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2_3X(N7, N5, N6, N8, px);\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2_3X(N5, N7, N2, N8, px);\
              }\
              else \
              {\
                     DIA_3X(N8, N5, N7, px);\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N8], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\
	 
	xbr3x_do
	
#undef FILTRO
}

void xbr3x_b(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if ((e<i)  && ( (!eq(PF,PB) && !eq(PH,PD)) || (eq(PE,PI) && (!eq(PF,I4) && !eq(PH,I5))) || eq(PE,PG) || eq(PE,PC)) )\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2_3X(N7, N5, N6, N2, N8, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2_3X(N7, N5, N6, N8, px);\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2_3X(N5, N7, N2, N8, px);\
              }\
              else \
              {\
                     DIA_3X(N8, N5, N7, px);\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N8], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\
	 
	xbr3x_do
	
#undef FILTRO
}

void xbr3x_c(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if (e<i)\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2_3X(N7, N5, N6, N2, N8, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2_3X(N7, N5, N6, N8, px);\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2_3X(N5, N7, N2, N8, px);\
              }\
              else \
              {\
                     DIA_3X(N8, N5, N7, px);\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N8], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\

	xbr3x_do
	
#undef FILTRO
}

#undef xbr3x_do
	
#define LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, PIXEL)\
                                ALPHA_BLEND_192_W(E[N13], PIXEL); \
                                ALPHA_BLEND_64_W( E[N12], PIXEL); \
                                E[N15] = E[N14] = E[N11] = PIXEL; \
                                E[N10] = E[N3] = E[N12]; \
                                E[N7]  = E[N13]; \

#define LEFT_2(N15, N14, N11, N13, N12, N10, PIXEL)\
                                ALPHA_BLEND_192_W(E[N11], PIXEL); \
                                ALPHA_BLEND_192_W(E[N13], PIXEL); \
                                ALPHA_BLEND_64_W( E[N10], PIXEL); \
                                ALPHA_BLEND_64_W( E[N12], PIXEL); \
                                E[N14] = PIXEL; \
                                E[N15] = PIXEL; \

#define UP_2(N15, N14, N11, N3, N7, N10, PIXEL)\
                                ALPHA_BLEND_192_W(E[N14], PIXEL); \
                                ALPHA_BLEND_192_W(E[N7 ], PIXEL); \
                                ALPHA_BLEND_64_W( E[N10], PIXEL); \
                                ALPHA_BLEND_64_W( E[N3 ], PIXEL); \
                                E[N11] = PIXEL; \
                                E[N15] = PIXEL; \

#define DIA(N15, N14, N11, PIXEL)\
                        ALPHA_BLEND_128_W(E[N11], PIXEL); \
                        ALPHA_BLEND_128_W(E[N14], PIXEL); \
                        E[N15] = PIXEL; \

#define xbr4x_do \
    initialize(); \
	\
    const int nl = dstPitch / 2; \
    const int nl1 = nl + nl; \
    const int nl2 = nl1 + nl; \
	 \
    for (int y = 0; y < Yres; y++){ \
        unsigned short int * E = (unsigned short *)((char*) pOut + y * dstPitch * 4); \
		\
        unsigned short int * sa2 = (unsigned short *)((char*) pIn + y * srcPitch - 4); \
        unsigned short int * sa1 = sa2 - srcPitch / 2; \
        unsigned short int * sa0 = sa1 - srcPitch / 2; \
        unsigned short int * sa3 = sa2 + srcPitch / 2; \
        unsigned short int * sa4 = sa3 + srcPitch / 2; \
		 \
        if (y <= 1){  \
            sa0 = sa1; \
            if (y == 0){ \
                sa0 = sa1 = sa2; \
            } \
        } \
		\
        if (y >= Yres - 2){ \
            sa4 = sa3; \
            if (y == Yres - 1){ \
                sa4 = sa3 = sa2; \
            } \
        } \
		\
        unsigned char pprev; \
        unsigned char pprev2; \
        pprev = pprev2 = 2; \
		\
        for (int x = 0; x < Xres; x++){ \
            unsigned short B1 = sa0[2]; \
            unsigned short PB = sa1[2]; \
            unsigned short PE = sa2[2]; \
            unsigned short PH = sa3[2]; \
            unsigned short H5 = sa4[2]; \
			\
            unsigned short A1 = sa0[pprev]; \
            unsigned short PA = sa1[pprev]; \
            unsigned short PD = sa2[pprev]; \
            unsigned short PG = sa3[pprev]; \
            unsigned short G5 = sa4[pprev]; \
			\
            unsigned short A0 = sa1[pprev2]; \
            unsigned short D0 = sa2[pprev2]; \
            unsigned short G0 = sa3[pprev2]; \
			 \
            unsigned short C1 = 0; \
            unsigned short PC = 0; \
            unsigned short PF = 0; \
            unsigned short PI = 0; \
            unsigned short I5 = 0; \
			\
            unsigned short C4 = 0; \
            unsigned short F4 = 0; \
            unsigned short I4 = 0; \
			\
            if (x >= Xres - 2){ \
                if (x == Xres - 1){ \
                    C1 = sa0[2]; \
                    PC = sa1[2]; \
                    PF = sa2[2]; \
                    PI = sa3[2]; \
                    I5 = sa4[2]; \
					\
                    C4 = sa1[2]; \
                    F4 = sa2[2]; \
                    I4 = sa3[2]; \
                } else { \
                    C1 = sa0[3]; \
                    PC = sa1[3]; \
                    PF = sa2[3]; \
                    PI = sa3[3]; \
                    I5 = sa4[3]; \
					\
                    C4 = sa1[3]; \
                    F4 = sa2[3]; \
                    I4 = sa3[3]; \
                } \
            } else { \
                C1 = sa0[3]; \
                PC = sa1[3]; \
                PF = sa2[3]; \
                PI = sa3[3]; \
                I5 = sa4[3]; \
				\
                C4 = sa1[4]; \
                F4 = sa2[4]; \
                I4 = sa3[4]; \
            } \
			\
            unsigned int e, i, px; \
            unsigned int ex, ex2, ex3; \
            unsigned int ke, ki; \
			\
            E[0]   = E[1]     = E[2]     = E[3]     = PE; \
            E[nl]  = E[nl+1]  = E[nl+2]  = E[nl+3]  = PE; \
            E[nl1] = E[nl1+1] = E[nl1+2] = E[nl1+3] = PE; \
            E[nl2] = E[nl2+1] = E[nl2+2] = E[nl2+3] = PE; \
			\
			FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, nl2+3, nl2+2, nl1+3,  3,  nl+3, nl1+2, nl2+1, nl2,  nl1+1,  nl+2, 2,  1, nl+1, nl1, nl, 0); \
			FILTRO(PE, PC, PF, PB, PI, PA, PH, PD, PG, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0,  3,  nl+3,  2,  0,  1,  nl+2, nl1+3, nl2+3, nl1+2,  nl+1, nl,  nl1, nl1+1,nl2+2,nl2+1,nl2); \
			FILTRO(PE, PA, PB, PD, PC, PG, PF, PH, PI, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5,  0,  1,  nl, nl2,  nl1,  nl+1,  2,  3,  nl+2,  nl1+1, nl2+1,nl2+2,nl1+2, nl+3,nl1+3,nl2+3); \
			FILTRO(PE, PG, PD, PH, PA, PI, PB, PF, PC, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, nl2,  nl1, nl2+1, nl2+3, nl2+2,  nl1+1,  nl,  0,  nl+1, nl1+2, nl1+3, nl+3, nl+2, 1, 2, 3); \
			\
            sa0 += 1; \
            sa1 += 1; \
            sa2 += 1; \
            sa3 += 1; \
            sa4 += 1; \
			\
            E += 4; \
			\
            if (pprev2){ \
                pprev2--; \
                pprev = 1; \
            } \
        } \
    } \


void xbr4x_a(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N15, N14, N11, N3, N7, N10, N13, N12, N9, N6, N2, N1, N5, N8, N4, N0) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if ((e<i)  && ( (!eq(PF,PB) && !eq(PF,PC)) || (!eq(PH,PD) && !eq(PH,PG)) || ((eq(PE,PI) && (!eq(PF,F4) && !eq(PF,I4))) || (!eq(PH,H5) && !eq(PH,I5))) || eq(PE,PG) || eq(PE,PC)) )\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2(N15, N14, N11, N13, N12, N10, px)\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2(N15, N14, N11, N3, N7, N10, px)\
              }\
              else \
              {\
                     DIA(N15, N14, N11, px)\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N15], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\

	xbr4x_do
	
#undef FILTRO
}

void xbr4x_b(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N15, N14, N11, N3, N7, N10, N13, N12, N9, N6, N2, N1, N5, N8, N4, N0) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if ((e<i)  && ( (!eq(PF,PB) && !eq(PH,PD)) || (eq(PE,PI) && (!eq(PF,I4) && !eq(PH,I5))) || eq(PE,PG) || eq(PE,PC)) )\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2(N15, N14, N11, N13, N12, N10, px)\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2(N15, N14, N11, N3, N7, N10, px)\
              }\
              else \
              {\
                     DIA(N15, N14, N11, px)\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N15], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\

	xbr4x_do
	
#undef FILTRO
}

void xbr4x_c(unsigned char * pIn,  unsigned int srcPitch, unsigned char * pOut, unsigned int dstPitch, int Xres, int Yres)
{	
#define FILTRO(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N15, N14, N11, N3, N7, N10, N13, N12, N9, N6, N2, N1, N5, N8, N4, N0) \
     ex   = (PE!=PH && PE!=PF); \
     if ( ex )\
     {\
          e = (df(PE,PC)+df(PE,PG)+df(PI,H5)+df(PI,F4))+(df(PH,PF)<<2); \
          i = (df(PH,PD)+df(PH,I5)+df(PF,I4)+df(PF,PB))+(df(PE,PI)<<2); \
          if (e<i)\
          {\
              ke=df(PF,PG); ki=df(PH,PC); \
              ex2 = (PE!=PC && PB!=PC); ex3 = (PE!=PG && PD!=PG); px = (df(PE,PF) <= df(PE,PH)) ? PF : PH; \
              if ( ((ke<<1)<=ki) && ex3 && (ke>=(ki<<1)) && ex2 ) \
              {\
                     LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, px)\
              }\
              else if ( ((ke<<1)<=ki) && ex3 ) \
              {\
                     LEFT_2(N15, N14, N11, N13, N12, N10, px)\
              }\
              else if ( (ke>=(ki<<1)) && ex2 ) \
              {\
                     UP_2(N15, N14, N11, N3, N7, N10, px)\
              }\
              else \
              {\
                     DIA(N15, N14, N11, px)\
              }\
          }\
          else if (e<=i)\
          {\
               ALPHA_BLEND_128_W( E[N15], ((df(PE,PF) <= df(PE,PH)) ? PF : PH)); \
          }\
     }\

	xbr4x_do
	
#undef FILTRO
}

#undef xbr4x_do
