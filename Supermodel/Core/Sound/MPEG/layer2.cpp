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
 * layer2.cpp
 * 
 * Amp library internal module.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/

/* layer2.c   MPEG audio layer2 support
 *
 * Created by: Tomislav Uzelac  Mar 1996
 * merged with amp, May 19 1997
 */
#include "amp.h"
#include "audio.h"
#include "getbits.h"
#include "transform.h"

#define LAYER2
#include "layer2.h"

#include "Supermodel.h"

int layer2_frame(struct AUDIO_HEADER *header,int cnt)
{
int i,s,sb,ch,gr,bitrate,bound=0;
char (*nbal)[],(*bit_alloc_index)[][16];
unsigned char allocation[2][32];
unsigned char scfsi[2][32];
float scalefactor[2][32][3];
float subband_sample[2][32][36];
int sblimit,nlevels,grouping; 

float  c,d;
int no_of_bits,mpi;				
unsigned short sb_sample_buf[3];	

int hsize,fs,mean_frame_size;

	bit_alloc_index=(char (*)[][16])&t_alloc0;
	nbal=(char (*)[])&t_nbal0; // shut up compiler
	sblimit = 0;

	hsize=4;
	if (header->protection_bit==0) hsize+=2;

	bitrate=t_bitrate[header->ID][3-header->layer][header->bitrate_index];
        fs=t_sampling_frequency[header->ID][header->sampling_frequency];
        if (header->ID) mean_frame_size=144000*bitrate/fs;
        else mean_frame_size=72000*bitrate/fs;

	/* layers 1 and 2 do not have a 'bit reservoir'
	 */
	append=data=0;

	fillbfr(mean_frame_size + header->padding_bit - hsize);

	switch (header->mode)
		{
		case 0 : 
		case 2 : nch=2; bound=32; bitrate=bitrate/2;  
			break;
		case 3 : nch=1; bound=32; 
			break;
		case 1 : nch=2; bitrate=bitrate/2; bound=(header->mode_extension+1)*4; 
		}
		
	if (header->ID==1) switch (header->sampling_frequency) {
		case 0 : switch (bitrate)	/* 0 = 44.1 kHz */
				{
				case 56  :
				case 64  :
				case 80  : bit_alloc_index=(char (*)[][16])&t_alloc0;
					   nbal=(char (*)[])&t_nbal0;
					   sblimit=27;
					   break;
				case 96  :
				case 112 :
				case 128 :
				case 160 :
				case 192 : bit_alloc_index=(char (*)[][16])&t_alloc1;
					   nbal=(char (*)[])&t_nbal1;
					   sblimit=30;  
					   break;
				case 32  :
				case 48  : bit_alloc_index=(char (*)[][16])&t_alloc2;
					   nbal=(char (*)[])&t_nbal2;
					   sblimit=8;
					   break;
				default  : ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
				}
				break;
		case 1 : switch (bitrate)	/* 1 = 48 kHz */
				{
				case 56  : 
				case 64  :
				case 80  :
				case 96  :
				case 112 :
				case 128 :
				case 160 :
				case 192 : bit_alloc_index=(char (*)[][16])&t_alloc0;
					   nbal=(char (*)[])&t_nbal0;
					   sblimit=27;
					   break;
				case 32  :
				case 48  : bit_alloc_index=(char (*)[][16])&t_alloc2;
					   nbal=(char (*)[])&t_nbal2;
					   sblimit=8;
					   break;
				default  : ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
				}
				break;
		case 2 : switch (bitrate)	/* 2 = 32 kHz */
				{
			case 56  :
                        case 64  :
                        case 80  : bit_alloc_index=(char (*)[][16])&t_alloc0;
                                   nbal=(char (*)[])&t_nbal0;
                                   sblimit=27;
                                   break;
                        case 96  :
                        case 112 :
                        case 128 :
                        case 160 :
                        case 192 : bit_alloc_index=(char (*)[][16])&t_alloc1;
                                   nbal=(char (*)[])&t_nbal1;
                                   sblimit=30;
                                   break;
			case 32  :
			case 48  : bit_alloc_index=(char (*)[][16])&t_alloc3;
                                   nbal=(char (*)[])&t_nbal3;
                                   sblimit=12;
				   break;
			default  : ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
			}
	                break;                                                    
		default  : ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);	// sampling frequency no good
	} else {
		bit_alloc_index=(char (*)[][16])&t_allocMPG2;
		nbal=(char (*)[])&t_nbalMPG2;
		sblimit=30;
	}

/*
 * bit allocation per subband per channel decoding *****************************
 */

	if (bound==32) bound=sblimit;	/* bound=32 means there is no intensity stereo */
	 
	for (sb=0;sb<bound;sb++)
		for (ch=0;ch<nch;ch++)
			allocation[ch][sb]=getbits((*nbal)[sb]);

	for (sb=bound;sb<sblimit;sb++)
		allocation[1][sb] = allocation[0][sb] = getbits((*nbal)[sb]);


/*
 * scfsi ***********************************************************************
 */

	for (sb=0;sb<sblimit;sb++)
		for (ch=0;ch<nch;ch++)
			if (allocation[ch][sb]!=0) scfsi[ch][sb]=getbits(2);
			else scfsi[ch][sb]=0;

/*
 * scalefactors ****************************************************************
 */

	for (sb=0;sb<sblimit;sb++)
	for (ch=0;ch<nch;ch++)
		if (allocation[ch][sb]!=0) {
			scalefactor[ch][sb][0]=(float)t_scalefactor[getbits(6)];
			switch (scfsi[ch][sb])
			{
			case 0: scalefactor[ch][sb][1]=(float)t_scalefactor[getbits(6)];
				scalefactor[ch][sb][2]=(float)t_scalefactor[getbits(6)];
				break;
			case 1: scalefactor[ch][sb][2]=(float)t_scalefactor[getbits(6)];
				scalefactor[ch][sb][1]=scalefactor[ch][sb][0];
				break;
			case 2: scalefactor[ch][sb][1]=(float)scalefactor[ch][sb][0];
				scalefactor[ch][sb][2]=scalefactor[ch][sb][0];
				break;
			case 3: scalefactor[ch][sb][2]=(float)t_scalefactor[getbits(6)];
				scalefactor[ch][sb][1]=scalefactor[ch][sb][2];
			}		 	
		} 
		else scalefactor[ch][sb][0]=scalefactor[ch][sb][1]=\
			scalefactor[ch][sb][2]=0.0;


/*
 * samples *********************************************************************
 */

	for (gr=0;gr<12;gr++) {
		/*
		 * normal ********************************
		 */
   	
		for (sb=0;sb<bound;sb++)
		for (ch=0;ch<nch;ch++)
		if (allocation[ch][sb]!=0) {
			mpi=(*bit_alloc_index)[sb][allocation[ch][sb]];	
			no_of_bits=t_bpc[mpi];
			c=(float)t_c[mpi];
			d=(float)t_d[mpi];
			grouping=t_grouping[mpi];
			nlevels=t_nlevels[mpi];

			if (grouping) {
				int samplecode=getbits(no_of_bits);
				convert_samplecode(samplecode,grouping,sb_sample_buf);

                        	for (s=0;s<3;s++)
                                	subband_sample[ch][sb][3*gr+s]=requantize_sample (sb_sample_buf[s],nlevels,c,d,scalefactor[ch][sb][gr/4]);
			} else {
				for (s=0;s<3;s++) sb_sample_buf[s]=getbits(no_of_bits);
				
				for (s=0;s<3;s++) { 
					/*subband_sample[ch][sb][3*gr+s]=requantize_sample (sb_sample_buf[s],nlevels,c,d,scalefactor[ch][sb][gr/4]);*/
					subband_sample[ch][sb][3*gr+s]=(t_dd[mpi]+sb_sample_buf[s]*t_nli[mpi])*c*scalefactor[ch][sb][gr>>2];
				}
			}
	    	} else 
			for (s=0;s<3;s++) subband_sample[ch][sb][3*gr+s]=0;


		/*
		 * joint stereo ********************************************
		 */
      
		for (sb=bound;sb<sblimit;sb++)
   		if (allocation[0][sb]!=0) {
			/*ispravka!
			*/
			mpi=(*bit_alloc_index)[sb][allocation[0][sb]];	
			no_of_bits=t_bpc[mpi];
			c=(float)t_c[mpi];
			d=(float)t_d[mpi];
			grouping=t_grouping[mpi];
			nlevels=t_nlevels[mpi];	
	    	   
			if (grouping) {
				int samplecode=getbits(no_of_bits);
				convert_samplecode(samplecode,grouping,sb_sample_buf);

				for (s=0;s<3;s++) {
					subband_sample[0][sb][3*gr+s]=requantize_sample (sb_sample_buf[s],nlevels,c,d,scalefactor[0][sb][gr/4]);
					subband_sample[1][sb][3*gr+s]=subband_sample[0][sb][3*gr+s];
				}
			} else {
				for (s=0;s<3;s++) sb_sample_buf[s]=getbits(no_of_bits);

				for (s=0;s<3;s++) { 
					subband_sample[0][sb][3*gr+s]=subband_sample[1][sb][3*gr+s]=\
						(t_dd[mpi]+sb_sample_buf[s]*t_nli[mpi])*c*scalefactor[0][sb][gr>>2];
				}
			}

		} else for (s=0;s<3;s++) {
			subband_sample[0][sb][3*gr+s]=0;
			subband_sample[1][sb][3*gr+s]=0;
		}

		/*
		 * the rest *******************************************
		 */
		for (sb=sblimit;sb<32;sb++)
			for (ch=0;ch<nch;ch++)

				for (s=0;s<3;s++) subband_sample[ch][sb][3*gr+s]=0;
	}	

	/*
	 * this is, in fact, horrible, but I had to adjust it to amp/mp3. The hack to make downmixing
	 * work is as ugly as possible.
	 */

	if (A_DOWNMIX && header->mode!=3) {
		for (ch=0;ch<nch;ch++) 
			for (sb=0;sb<32;sb++)
				for (i=0;i<36;i++)
					subband_sample[0][sb][i]=(subband_sample[0][sb][i]+subband_sample[1][sb][i])*0.5f;
		nch=1;
	}

	for (ch=0;ch<nch;ch++) {
		for (sb=0;sb<32;sb++) 
			for (i=0;i<18;i++) res[sb][i]=subband_sample[ch][sb][i]; 
	   	for (i=0;i<18;i++)
			poly(ch,i);
	}
	printout();
	for (ch=0;ch<nch;ch++) {
                for (sb=0;sb<32;sb++)
                        for (i=0;i<18;i++) res[sb][i]=subband_sample[ch][sb][i+18];
                for (i=0;i<18;i++)
                        poly(ch,i);
        }
        printout();

	if (A_DOWNMIX && header->mode!=3) nch=2;

	return 0;
}                        
/****************************************************************************/	
/****************************************************************************/

void convert_samplecode(unsigned int samplecode,unsigned int nlevels,unsigned short* sb_sample_buf)
{
int i;

	for (i=0;i<3;i++) {
		*sb_sample_buf=samplecode%nlevels;
		samplecode=samplecode/nlevels;
		sb_sample_buf++;
	}
}

float requantize_sample(unsigned short s4,unsigned short nlevels,float c,float d,float factor)
{
register float s,s2,s3;
s3=(float) (-1.0+s4*2.0/(nlevels+1));
s2=c*(s3+d);
s=factor*s2;
return s;
} 
