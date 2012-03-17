/**
 * GENS: Sega 32X VDP - 32-bit color functions.
 * Provided by Upth, ported from Gens Rerecording
 */


// Post_Line_32X() was originally a series of macros.
// I converted it to a function so it can be debugged easier.


// TODO: Clean up these functions.

// TODO: Maybe split up Post_Line_32X() using inline functions?


#include "g_32x_32bit.h"
#include "gens_core/vdp/vdp_32x.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"


/**
 * Post_Line_32X(): Process a 32X video line.
 */
void Post_Line_32X(void)
{
	unsigned short *VRam_32X = (unsigned short *) &_32X_VDP_Ram;
	int VRam_Ind = ((_32X_VDP.State & 1) << 16);
	VRam_Ind += VRam_32X[VRam_Ind + VDP_Current_Line];
	
	unsigned int pixel;
	unsigned char pixC; 
	unsigned short pixS; 
	
	// Used for POST_LINE_32X_M11
	unsigned char Num;
	int endPixel, curPixel, pixMax;
	
	switch (_32X_Rend_Mode)
	{
		case 0:
		case 4:
		case 8:
		case 12:
			//POST_LINE_32X_M00;
			for (pixel = TAB336[VDP_Current_Line] + 8; pixel < TAB336[VDP_Current_Line] + 336; pixel++)
			{
				MD_Screen32[pixel] = MD_Palette32[MD_Screen[pixel] & 0xFF];
				MD_Screen[pixel] = MD_Palette[MD_Screen[pixel] & 0xFF];
			}
			
			break;
		
		case 1:
			//POST_LINE_32X_M01;
			VRam_Ind *= 2;
			for (pixel = TAB336[VDP_Current_Line] + 8;
			     pixel < (TAB336[VDP_Current_Line] + 336); pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) || !(MD_Screen[pixel] & 0x0F))
				{
					MD_Screen32[pixel] = _32X_VDP_CRam_Ajusted32[pixC];
					MD_Screen[pixel] = _32X_VDP_CRam_Ajusted[pixC];
				}
				else
				{
					MD_Screen32[pixel] = MD_Palette32[MD_Screen[pixel] & 0xFF];
					MD_Screen[pixel] = MD_Palette[MD_Screen[pixel] & 0xFF];
				}
			}
			
			break;
		
		case 2:
		case 10:
			//POST_LINE_32X_M10;
			for (pixel = TAB336[VDP_Current_Line] + 8; pixel < TAB336[VDP_Current_Line] + 336; pixel++)
			{
				pixS = VRam_32X[VRam_Ind++];
				if ((pixS & 0x8000) || !(MD_Screen[pixel] & 0x0F))
				{
					MD_Screen32[pixel] = _32X_Palette_32B[pixS];
					MD_Screen[pixel] = _32X_Palette_16B[pixS];
				}
				else
				{
					MD_Screen32[pixel] = MD_Palette32[MD_Screen[pixel] & 0xFF];
					MD_Screen[pixel] = MD_Palette[MD_Screen[pixel] & 0xFF];
				}
			}
			break;
		
		case 3:
		case 7:
		case 11:
		case 15:
			//POST_LINE_32X_M11;
			curPixel = TAB336[VDP_Current_Line] + 8;
			pixMax = TAB336[VDP_Current_Line] + 336;
			while (curPixel < pixMax)
			{
				pixC = VRam_32X[VRam_Ind] & 0xFF;
				Num = VRam_32X[VRam_Ind++] >> 8;
				endPixel = curPixel + Num;
				while (curPixel <= endPixel)
				{
					MD_Screen32[curPixel] = _32X_VDP_CRam_Ajusted32[pixC];
					MD_Screen[curPixel] = _32X_VDP_CRam_Ajusted[pixC];
					curPixel++;
				}
			}
			break;
		
		case 5:
			//POST_LINE_32X_M01_P;
			VRam_Ind *= 2;
			for (pixel = TAB336[VDP_Current_Line] + 8; pixel < (TAB336[VDP_Current_Line] + 336); pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) && (MD_Screen[pixel] & 0x0F))
				{
					MD_Screen32[pixel] = MD_Palette32[MD_Screen[pixel] & 0xFF];
					MD_Screen[pixel] = MD_Palette[MD_Screen[pixel] & 0xFF];
				}
				else
				{
					MD_Screen32[pixel] = _32X_VDP_CRam_Ajusted32[pixC];
					MD_Screen[pixel] = _32X_VDP_CRam_Ajusted[pixC];
				}
			}
			break;
		
		case 6:
		case 14:
			//POST_LINE_32X_M10_P;
			for (pixel = TAB336[VDP_Current_Line] + 8; pixel < TAB336[VDP_Current_Line] + 336; pixel++)
			{
				pixS = VRam_32X[VRam_Ind++];
				if (!(pixS & 0x8000) && (MD_Screen[pixel] & 0x0F))
				{
					MD_Screen32[pixel] = _32X_Palette_32B[pixS];
					MD_Screen[pixel] = _32X_Palette_16B[pixS];
				}
				else
				{
					MD_Screen32[pixel] = MD_Palette32[MD_Screen[pixel] & 0xFF];
					MD_Screen[pixel] = MD_Palette[MD_Screen[pixel] & 0xFF];
				}
			}
			break;
		
		case 9:
			//POST_LINE_32X_SM01;
			VRam_Ind *= 2;
			for (pixel = TAB336[VDP_Current_Line] + 8; pixel < (TAB336[VDP_Current_Line] + 336); pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) || !(MD_Screen[pixel] & 0x0F))
				{
					MD_Screen32[pixel] = _32X_VDP_CRam_Ajusted32[pixC];
					MD_Screen[pixel] = _32X_VDP_CRam_Ajusted[pixC];
				}
				else
				{
					MD_Screen32[pixel] = MD_Palette32[MD_Screen[pixel] & 0xFF];
					MD_Screen[pixel] = MD_Palette[MD_Screen[pixel] & 0xFF];
				}
			}
			break;
		
		case 13:
			//POST_LINE_32X_SM01_P;
			VRam_Ind *= 2;
			for (pixel = TAB336[VDP_Current_Line] + 8; pixel < (TAB336[VDP_Current_Line] + 336); pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) && (MD_Screen[pixel] & 0x0F))
				{
					MD_Screen32[pixel] = MD_Palette32[MD_Screen[pixel] & 0xFF];
					MD_Screen[pixel] = MD_Palette[MD_Screen[pixel] & 0xFF];
				}
				else
				{
					MD_Screen32[pixel] = _32X_VDP_CRam_Ajusted32[pixC];
					MD_Screen[pixel] = _32X_VDP_CRam_Ajusted[pixC];
				}
			}
			break;
	}
}
