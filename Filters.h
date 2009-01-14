/*
 *  Filters.h
 *  OpenEmu
 *
 *  Created by Josh Weinberg on 9/19/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

void InitLUTs();
void hq3x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
void hq2x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );

#ifdef __cplusplus
extern "C" {
#endif
	
int scale_precondition(unsigned scale, unsigned pixel, unsigned width, unsigned height);
void scale(unsigned scale, void* void_dst, unsigned dst_slice, const void* void_src, unsigned src_slice, unsigned pixel, unsigned width, unsigned height);

#ifdef __cplusplus
}
#endif