/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * getdata.cpp
 * 
 * Amp library internal module.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* getdata.c  scalefactor & huffman data extraction
 *
 * Created by: tomislav uzelac  Apr 1996
 * Last modified by: tomislav uzelac  Feb 27 1997
 */
#include "amp.h"
#include "audio.h"
#include "getbits.h"
#include "huffman.h"

#define		GETDATA
#include "getdata.h"

/* layer3 scalefactor decoding. should we check for the number
 * of bits read, just in case?
 */
int decode_scalefactors(struct SIDE_INFO *info,struct AUDIO_HEADER *header,int gr,int ch)
{
int sfb,window;
int slen1,slen2;
int i1,i2,i=0;
int j,k;
	if (header->ID==1) {
	/* this is MPEG-1 scalefactors format, quite different than 
	 * the MPEG-2 format. 
	 */
		slen1=t_slen1[info->scalefac_compress[gr][ch]];
		slen2=t_slen2[info->scalefac_compress[gr][ch]];
		i1=3*slen1;
		i2=3*slen2;
		
		if (info->window_switching_flag[gr][ch] && info->block_type[gr][ch]==2) {
			if (info->mixed_block_flag[gr][ch]) {
				for (sfb=0;sfb<8;sfb++) {
					scalefac_l[gr][ch][sfb]=getbits(slen1);	
					i+=slen1;
				}
				for (sfb=3;sfb<6;sfb++) {
					for (window=0;window<3;window++)
						scalefac_s[gr][ch][sfb][window]=getbits(slen1);
					i+=i1;
				}
				for (;sfb<12;sfb++) {
					for (window=0;window<3;window++)
						scalefac_s[gr][ch][sfb][window]=getbits(slen2);
					i+=i2;
				}
			} else { /* !mixed_block_flag */
				for (sfb=0;sfb<6;sfb++) {
					for (window=0;window<3;window++)
						scalefac_s[gr][ch][sfb][window]=getbits(slen1);
					i+=i1;
				}	
				for (;sfb<12;sfb++) {
					for (window=0;window<3;window++)
						scalefac_s[gr][ch][sfb][window]=getbits(slen2);
					i+=i2;
				}	
			}	
			for (window=0;window<3;window++) 
				scalefac_s[gr][ch][12][window]=0;
		} else { /* block_type!=2 */
			if ( !info->scfsi[ch][0] || !gr )
				for (sfb=0;sfb<6;sfb++) {
					scalefac_l[gr][ch][sfb]=getbits(slen1);
					i+=slen1;
				}	
			else for (sfb=0;sfb<6;sfb++) {
				scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];
			}	
			if ( !info->scfsi[ch][1] || !gr )
				for (sfb=6;sfb<11;sfb++) {
					scalefac_l[gr][ch][sfb]=getbits(slen1);
					i+=slen1;
				}
			else for (sfb=6;sfb<11;sfb++) {
				scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];				
			}
			if ( !info->scfsi[ch][2] || !gr )
				for (sfb=11;sfb<16;sfb++) {
					scalefac_l[gr][ch][sfb]=getbits(slen2);
					i+=slen2;
				}
			else for (sfb=11;sfb<16;sfb++) {
				scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];				
			}	
			if ( !info->scfsi[ch][3] || !gr )
				for (sfb=16;sfb<21;sfb++) {
					scalefac_l[gr][ch][sfb]=getbits(slen2);
					i+=slen2;
				}	
			else for (sfb=16;sfb<21;sfb++) {
				scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];
			}
			scalefac_l[gr][ch][21]=0;
		}
	} else { /* ID==0 */
		int index=0,index2,spooky_index;
		int slen[6],nr_of_sfb[6]; /* actually, there's four of each, not five, labelled 1 through 4, but
					   * what's a word of storage compared to one's sanity. so [0] is irellevant.
					   */

		/* ok, so we got 3 indexes. 
		 * spooky_index - indicates whether we use the normal set of slen eqs and nr_of_sfb tables
		 *                or the one for the right channel of intensity stereo coded frame
		 * index        - corresponds to the value of scalefac_compress, as listed in the standard
		 * index2	- 0 for long blocks, 1 for short wo/ mixed_block_flag, 2 for short with it
		 */
		if ( (header->mode_extension==1 || header->mode_extension==3) && ch==1) { /* right ch... */
			int int_scalefac_compress=info->scalefac_compress[0][ch]>>1;
			intensity_scale=info->scalefac_compress[0][1]&1;
			spooky_index=1;
			if (int_scalefac_compress < 180) {
				slen[1]=int_scalefac_compress/36;
				slen[2]=(int_scalefac_compress%36)/6;
				slen[3]=(int_scalefac_compress%36)%6;
				slen[4]=0;
				info->preflag[0][ch]=0;
				index=0;
			}
			if ( 180 <= int_scalefac_compress && int_scalefac_compress < 244) {
				slen[1]=((int_scalefac_compress-180)%64)>>4;
				slen[2]=((int_scalefac_compress-180)%16)>>2;
				slen[3]=(int_scalefac_compress-180)%4;
				slen[4]=0;
				info->preflag[0][ch]=0;
				index=1;
			}
			if ( 244 <= int_scalefac_compress && int_scalefac_compress < 255) {
				slen[1]=(int_scalefac_compress-244)/3;
				slen[2]=(int_scalefac_compress-244)%3;
				slen[3]=0;
				slen[4]=0;
				info->preflag[0][ch]=0;
				index=2;
			}
		} else { /* the usual */
			spooky_index=0;
			if (info->scalefac_compress[0][ch] < 400) {
				slen[1]=(info->scalefac_compress[0][ch]>>4)/5;
				slen[2]=(info->scalefac_compress[0][ch]>>4)%5;
				slen[3]=(info->scalefac_compress[0][ch]%16)>>2;
				slen[4]=info->scalefac_compress[0][ch]%4;
				info->preflag[0][ch]=0;
				index=0;
			}
			if (info->scalefac_compress[0][ch] >= 400 && info->scalefac_compress[0][ch] < 500) {
				slen[1]=((info->scalefac_compress[0][ch]-400)>>2)/5;
				slen[2]=((info->scalefac_compress[0][ch]-400)>>2)%5;
				slen[3]=(info->scalefac_compress[0][ch]-400)%4;
				slen[4]=0;
				info->preflag[0][ch]=0;
				index=1;
			} 
			if (info->scalefac_compress[0][ch] >= 500 && info->scalefac_compress[0][ch] < 512) {
				slen[1]=(info->scalefac_compress[0][ch]-500)/3;
				slen[2]=(info->scalefac_compress[0][ch]-500)%3;
				slen[3]=0;
				slen[4]=0;
				info->preflag[0][ch]=1;
				index=2;
			}
		}

		if (info->window_switching_flag[0][ch] && info->block_type[0][ch]==2)
			if (info->mixed_block_flag[0][ch]) index2=2;
			else index2=1;
		else index2=0;

		for (j=1;j<=4;j++) nr_of_sfb[j]=spooky_table[spooky_index][index][index2][j-1];

	/* now we'll do some actual scalefactor extraction, and a little more.
	 * for each scalefactor band we'll set the value of is_max to indicate
	 * illegal is_pos, since with MPEG2 it's not 'hardcoded' to 7.
	 */
		if (!info->window_switching_flag[0][ch] || (info->window_switching_flag[0][ch] && info->block_type[0][ch]!=2)) {
			sfb=0;
			for (j=1;j<=4;j++) 
				for (k=0;k<nr_of_sfb[j];k++) {
					scalefac_l[0][ch][sfb]=getbits(slen[j]);
					i+=slen[j];
					if (ch) is_max[sfb]=(1<<slen[j])-1;
					sfb++;
				}
		} else if (info->block_type[0][ch]==2)
		       {
			if (!info->mixed_block_flag[0][ch]) {
				sfb=0;
				for (j=1;j<=4;j++)
					for (k=0;k<nr_of_sfb[j];k+=3) {
						/* we assume here that nr_of_sfb is divisible by 3. it is.
						 */
						scalefac_s[0][ch][sfb][0]=getbits(slen[j]);
						scalefac_s[0][ch][sfb][1]=getbits(slen[j]);
						scalefac_s[0][ch][sfb][2]=getbits(slen[j]);
						i+=3*slen[j];
						if (ch) is_max[sfb+6]=(1<<slen[j])-1;
						sfb++;
					}
			} else {
				/* what we do here is:
				 * 1. assume that for every fs, the two lowest subbands are equal to the
				 *    six lowest scalefactor bands for long blocks/MPEG2. they are.
				 * 2. assume that for every fs, the two lowest subbands are equal to the
				 *    three lowest scalefactor bands for short blocks. they are.
				 */
				sfb=0;
				for (k=0;k<6;k++) {
					scalefac_l[0][ch][sfb]=getbits(slen[1]);
					i+=slen[j];
					if (ch) is_max[sfb]=(1<<slen[1])-1;
					sfb++;
				}
				nr_of_sfb[1]-=6;
				sfb=3;
				for (j=1;j<=4;j++)
					for (k=0;k<nr_of_sfb[j];k+=3) {
						scalefac_s[0][ch][sfb][0]=getbits(slen[j]);
						scalefac_s[0][ch][sfb][1]=getbits(slen[j]);
						scalefac_s[0][ch][sfb][2]=getbits(slen[j]);
						i+=3*slen[j];
						if (ch) is_max[sfb+6]=(1<<slen[j])-1;
						sfb++;
					}
			}
		} 
	}
return i;
}
