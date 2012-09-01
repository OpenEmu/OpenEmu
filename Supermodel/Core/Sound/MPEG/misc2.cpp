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
 * misc2.cpp
 * 
 * Amp library internal module.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* misc2.c  requantization, stereo processing, reordering(shortbl) and antialiasing butterflies
 *
 * misc.c was created by tomislav uzelac in May 1996, and was completely awful
 * Created by: tomislav uzelac Dec 22 1996
 * some more speed injected, cca. Jun 1 1997
 */
#include <math.h>
#include <string.h>

#include "amp.h"
#include "audio.h"
#include "getdata.h"
#include "huffman.h"

#define MISC2
#include "misc2.h"

#include "Supermodel.h"

/* 
 * fras == Formula for Requantization and All Scaling **************************
 */
static inline float fras_l(int sfb,int global_gain,int scalefac_scale,int scalefac,int preflag)
{
register int a,scale;
	/*
        if (scalefac_scale) scale=2;
	else scale=1;
	*/
	scale=scalefac_scale+1;
	a= global_gain -210 -(scalefac << scale);
        if (preflag) a-=(t_pretab[sfb] << scale);

/* bugfix, Mar 13 97: shifting won't produce a legal result if we shift by more than 31
 * since global_gain<256, this can only occur for (very) negative values of a.
*/
	if (a < -127) return 0;

/* a minor change here as well, no point in abs() if we now that a<0
*/
        if (a>=0) return tab[a&3]*(1 << (a>>2));
	else return tabi[(-a)&3]/(1 << ((-a) >> 2));
}

static inline float fras_s(int global_gain,int subblock_gain,int scalefac_scale,int scalefac)
{
int a;
        a=global_gain - 210 - (subblock_gain << 3);
	if (scalefac_scale) a-= (scalefac << 2);
	else a-= (scalefac << 1);

        if (a < -127) return 0;

        if (a>=0) return tab[a&3]*(1 << (a>>2));
        else return tabi[(-a)&3]/(1 << ((-a) >> 2));
}

/* this should be faster than pow()
 */
static inline float fras2(int is,float a)
{
        if (is > 0) return t_43[is]*a;
        else return -t_43[-is]*a;
}

/*
 * requantize_mono *************************************************************
 */

/* generally, the two channels do not have to be of the same block type - that's why we do two passes with requantize_mono.
 * if ms or intensity stereo is enabled we do a single pass with requantize_ms because both channels are same block type
 */

void requantize_mono(int gr,int ch,struct SIDE_INFO *info,struct AUDIO_HEADER *header)
{
int l,i,sfb;
float a;
int global_gain=info->global_gain[gr][ch];
int scalefac_scale=info->scalefac_scale[gr][ch];
int sfreq=header->sampling_frequency;


	no_of_imdcts[0]=no_of_imdcts[1]=32;

	if (info->window_switching_flag[gr][ch] && info->block_type[gr][ch]==2)	
		if (info->mixed_block_flag[gr][ch]) {
	/*
	 * requantize_mono - mixed blocks/long block part **********************
	 */
	                int window,window_len,preflag=0; /* pretab is all zero in this low frequency area */
	                int scalefac=scalefac_l[gr][ch][0];

			l=0;sfb=0;
			a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
	                while (l<36) {
	                        xr[ch][0][l]=fras2(is[ch][l],a);
	                        if (l==t_l[sfb]) {
					scalefac=scalefac_l[gr][ch][++sfb];
					a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
				}
	                        l++;
	                }
	/*
	 * requantize_mono - mixed blocks/short block part *********************
	 */
			sfb=3; 
			window_len=t_s[sfb]-t_s[sfb-1];
	                while (l<non_zero[ch]) {
	                	for (window=0;window<3;window++) {
	                        	int scalefac=scalefac_s[gr][ch][sfb][window];
	                                int subblock_gain=info->subblock_gain[gr][ch][window];
					a=fras_s(global_gain,subblock_gain,scalefac_scale,scalefac);
	                                for (i=0;i<window_len;i++) {
      		                                xr[ch][0][t_reorder[header->ID][sfreq][l]]=fras2(is[ch][l],a);
	                                	l++;
                                       	}
                               	}
                               	sfb++;
                               	window_len=t_s[sfb]-t_s[sfb-1];
			}
			while (l<576) xr[ch][0][t_reorder[header->ID][sfreq][l++]]=0;
		} else {
	/*
	 * requantize mono - short blocks **************************************
	 */
			int window,window_len;

			sfb=0; l=0;
			window_len=t_s[0]+1;
			while (l<non_zero[ch]) {
				for (window=0;window<3;window++) {
					int scalefac=scalefac_s[gr][ch][sfb][window];
					int subblock_gain=info->subblock_gain[gr][ch][window];
					float a=fras_s(global_gain,subblock_gain,scalefac_scale,scalefac);
					for (i=0;i<window_len;i++) {
						xr[ch][0][t_reorder[header->ID][sfreq][l]]=fras2(is[ch][l],a);
						l++;
					}
				}
				sfb++;
				window_len=t_s[sfb]-t_s[sfb-1];
			}
			while (l<576) xr[ch][0][t_reorder[header->ID][sfreq][l++]]=0;
		}
	else {
	/* long blocks */
		int preflag=info->preflag[gr][ch];
		int scalefac=scalefac_l[gr][ch][0];

		sfb=0; l=0;
		a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
		while (l<non_zero[ch]) {
			xr[ch][0][l]=fras2(is[ch][l],a); 
			if (l==t_l[sfb]) {
				scalefac=scalefac_l[gr][ch][++sfb];
				a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
			}
			l++;
		}		
		while (l<576) xr[ch][0][l++]=0;
	}
}

/*
 * stereo stuff ****************************************************************
 */
static int find_isbound(int isbound[3],int gr,struct SIDE_INFO *info,struct AUDIO_HEADER *header)
{
int sfb,window,window_len,ms_flag,tmp,i;

	isbound[0]=isbound[1]=isbound[2]=-1;
	no_of_imdcts[0]=no_of_imdcts[1]=32;

   if (header->mode_extension==1 || header->mode_extension==3) {
	if (info->window_switching_flag[gr][0] && info->block_type[gr][0]==2) {

	/* find that isbound!
	 */
		tmp=non_zero[1];
		sfb=0; while ((3*t_s[sfb]+2) < tmp  && sfb < 12) sfb++;
		while ((isbound[0]<0 || isbound[1]<0 || isbound[2]<0) && !(info->mixed_block_flag[gr][0] && sfb<3) && sfb) {
			for (window=0;window<3;window++) {
				if (sfb==0) {
					window_len=t_s[0]+1;
					tmp=(window+1)*window_len - 1;
				} else {
					window_len=t_s[sfb]-t_s[sfb-1];
					tmp=(3*t_s[sfb-1]+2) + (window+1)*window_len;
				}
				if (isbound[window] < 0)
					for (i=0;i<window_len;i++)
						if (is[1][tmp--] != 0) {
							isbound[window]=t_s[sfb]+1; 
							break;
						}
			}
			sfb--;
		}
	
	/* mixed block magic now...
	 */
		if (sfb==2 && info->mixed_block_flag[gr][0]) 
		{
			if (isbound[0]<0 && isbound[1]<0 && isbound[2]<0) 
			{
				tmp=35;
				while (is[1][tmp] == 0) tmp--;
				sfb=0; while (t_l[sfb] < tmp  && sfb < 21) sfb++;
				isbound[0]=isbound[1]=isbound[2]=t_l[sfb]+1;
			} 
			else for (window=0;window<3;window++) 
				if (isbound[window]<0) isbound[window]=36;
		}
		if (header->ID==1) isbound[0]=isbound[1]=isbound[2]=MAX(isbound[0],MAX(isbound[1],isbound[2]));

	/* just how many imdcts?
	 */
		tmp=non_zero[0];
		sfb=0; while ((3*t_s[sfb]+2) < tmp && sfb < 12) sfb++;
		no_of_imdcts[0]=no_of_imdcts[1]=(t_s[sfb]-1)/6+1; 
	} else {

	/* long blocks now
	 */
                tmp=non_zero[1];
                while (is[1][tmp] == 0) tmp--;
                sfb=0; while (t_l[sfb] < tmp && sfb < 21) sfb++;
		isbound[0]=isbound[1]=isbound[2]=t_l[sfb]+1;
		no_of_imdcts[0]=no_of_imdcts[1]=(non_zero[0]-1)/18+1; 
	}
	if (header->mode_extension==1) ms_flag=0;
	else ms_flag=1;
   } else {

   /* intensity stereo is, regretably, turned off
    */
	ms_flag=1;

	/* i really put a lot of work in this, but it still looks like shit (works, though)
	 */ 
	if (!info->window_switching_flag[gr][0] || (info->window_switching_flag[gr][0] && info->block_type[gr][0]!=2)) 
		isbound[0]=isbound[1]=isbound[2]=(MAX(non_zero[0],non_zero[1]));
	else isbound[0]=isbound[1]=isbound[2]=576;

	if (info->window_switching_flag[gr][0] && info->block_type[gr][0]==2) {
		/* should do for mixed blocks too, though i havent tested... */
			tmp=(MAX(non_zero[0],non_zero[1]))/3;
			sfb=0; while (t_s[sfb]<tmp && sfb<12) sfb++;
			no_of_imdcts[0]=no_of_imdcts[1]=(t_s[sfb]-1)/6+1;
	}
	else no_of_imdcts[0]=no_of_imdcts[1]=(isbound[0]-1)/18+1;

   }

   return ms_flag;
}

static inline void stereo_s(int l,float a[2],int pos,int ms_flag,int is_pos,struct AUDIO_HEADER *header)
{
float ftmp,Mi,Si;

	if (l>=576) return; /* brrr... */

        if ((is_pos != IS_ILLEGAL) && (header->ID==1)) {
                ftmp=fras2(is[0][l],a[0]);
                xr[0][0][pos]=(1-t_is[is_pos])*ftmp;
                xr[1][0][pos]=t_is[is_pos]*ftmp;
		return;
	}

	if ((is_pos != IS_ILLEGAL) && (header->ID==0)) {
		ftmp=fras2(is[0][l],a[0]);
		if (is_pos&1) {
			xr[0][0][pos]= t_is2[intensity_scale][(is_pos+1)>>1] * ftmp;
			xr[1][0][pos]= ftmp;
		} else {
			xr[0][0][pos]= ftmp;
			xr[1][0][pos]= t_is2[intensity_scale][is_pos>>1] * ftmp;
		}
		return;
	}

        if (ms_flag) {
                Mi=fras2(is[0][l],a[0]);
                Si=fras2(is[1][l],a[1]);
                xr[0][0][pos]=(Mi+Si)*i_sq2;
                xr[1][0][pos]=(Mi-Si)*i_sq2;
        } else {
                xr[0][0][pos]=fras2(is[0][l],a[0]);
                xr[1][0][pos]=fras2(is[1][l],a[1]);
        }
}

static inline void stereo_l(int l,float a[2],int ms_flag,int is_pos,struct AUDIO_HEADER *header)
{
float ftmp,Mi,Si;
	if (l>=576) return;

/* new code by ???
*/
	if (is_pos != IS_ILLEGAL) {
		ftmp = fras2(is[0][l], a[0]);
		if (header -> ID ==1) {
			xr[0][0][l] = (1 - t_is[is_pos]) * ftmp;
			xr[1][0][l] = t_is[is_pos] * ftmp;
			return;
		} else if (is_pos & 1) {
			xr[0][0][l] = t_is2[intensity_scale][(is_pos + 1) >> 1] * ftmp;
			xr[1][0][l] = ftmp;
		} else {
			xr[0][0][l] = ftmp;
			xr[1][0][l] = t_is2[intensity_scale][is_pos >> 1] * ftmp;
		}
		return;
	}

	if (ms_flag) {
		Mi=fras2(is[0][l],a[0]);
		Si=fras2(is[1][l],a[1]);
		xr[0][0][l]=(Mi+Si)*i_sq2;
		xr[1][0][l]=(Mi-Si)*i_sq2;
	} else {
		xr[0][0][l]=fras2(is[0][l],a[0]);
		xr[1][0][l]=fras2(is[1][l],a[1]);
	}

}


/*
 * requantize_ms ***************************************************************
 */
void requantize_ms(int gr,struct SIDE_INFO *info,struct AUDIO_HEADER *header)
{
int l,sfb,ms_flag,is_pos,i,ch;
int *global_gain,subblock_gain[2],*scalefac_scale,scalefac[2],isbound[3];
int sfreq=header->sampling_frequency;
int id = header->ID;
float a[2];

memset(a, 0, sizeof(a));

global_gain=info->global_gain[gr];
scalefac_scale=info->scalefac_scale[gr];

        if (info->window_switching_flag[gr][0] && info->block_type[gr][0]==2)  
                if (info->mixed_block_flag[gr][0]) {
	/* 
	 * mixed blocks w/stereo processing - long block part ******************
	 */
                        int window,window_len;
                        int preflag[2]={0,0};

			ms_flag=find_isbound(isbound,gr,info,header);

			sfb=0; l=0;
			for (ch=0;ch<2;ch++) {
				scalefac[ch]=scalefac_l[gr][ch][0];
				a[ch]=fras_l(0,global_gain[ch],scalefac_scale[ch],scalefac[ch],preflag[ch]);
			}


			while (l<36) {
				int is_pos;
				if (l<isbound[0]) is_pos=IS_ILLEGAL;
				else {
					is_pos=scalefac[1];
					if (id==1) /* MPEG1 */
					{
						if (is_pos==7) is_pos=IS_ILLEGAL;
					}
					else /* MPEG2 */
					{
						if (is_pos==is_max[sfb]) is_pos=IS_ILLEGAL;
					}
				}

				stereo_l(l,a,ms_flag,is_pos,header);

				if (l==t_l[sfb]) {
					sfb++;
					for (ch=0;ch<2;ch++) {
						scalefac[ch]=scalefac_l[gr][ch][sfb];
						a[ch]=fras_l(sfb,global_gain[ch],scalefac_scale[ch],scalefac[ch],preflag[ch]);
					}
				}

				l++;
			}
	/*
	 * mixed blocks w/stereo processing - short block part *****************
	 */
                        sfb=3;
                        window_len=t_s[sfb]-t_s[sfb-1];

                        while (l<(MAX(non_zero[0],non_zero[1]))) {
                                for (window=0;window<3;window++) {
                                        subblock_gain[0]=info->subblock_gain[gr][0][window];
					subblock_gain[1]=info->subblock_gain[gr][1][window];
					scalefac[0]=scalefac_s[gr][0][sfb][window];
					scalefac[1]=scalefac_s[gr][1][sfb][window];

					if (t_s[sfb] < isbound[window]) {
						is_pos=IS_ILLEGAL;
						a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
						a[1]=fras_s(global_gain[1],subblock_gain[1],scalefac_scale[1],scalefac[1]);
					} else {
						is_pos=scalefac[1];
						if (id==1) /* MPEG1 */
						{
							if (is_pos==7) is_pos=IS_ILLEGAL;
						}
						else /* MPEG2 */
						{
							if (is_pos==is_max[sfb+6]) is_pos=IS_ILLEGAL;
						}
 
						a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
					}

                                        for (i=0;i<window_len;i++) {
                                                stereo_s(l,a,t_reorder[id][sfreq][l],ms_flag,is_pos,header);
                                                l++;
                                        }
                                }
                                sfb++;
                                window_len=t_s[sfb]-t_s[sfb-1];
                        }
                        while (l<576) {
			        int reorder = t_reorder[id][sfreq][l++];
			  
				xr[0][0][reorder]=xr[1][0][reorder]=0;
			}
                } else {                                                                       
        /*
	 * requantize_ms - short blocks w/stereo processing ********************
	 */
                        int window,window_len;

			ms_flag=find_isbound(isbound,gr,info,header);	
			sfb=0; l=0;
			window_len=t_s[0]+1;

                        while (l<(MAX(non_zero[0],non_zero[1]))) {
                                for (window=0;window<3;window++) {
					subblock_gain[0]=info->subblock_gain[gr][0][window];
					subblock_gain[1]=info->subblock_gain[gr][1][window];
					scalefac[0]=scalefac_s[gr][0][sfb][window];
					scalefac[1]=scalefac_s[gr][1][sfb][window];

                                        if (t_s[sfb] < isbound[window]) {
                                                is_pos=IS_ILLEGAL;
						a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
						a[1]=fras_s(global_gain[1],subblock_gain[1],scalefac_scale[1],scalefac[1]);
                                        } else {
                                                is_pos=scalefac[1];
                                                if (id==1) /* MPEG1 */
						{
                                                        if (is_pos==7) is_pos=IS_ILLEGAL;
						}
                                                else /* MPEG2 */
						{
                                                        if (is_pos==is_max[sfb+6]) is_pos=IS_ILLEGAL;
						}
                                                a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
                                        }

                                        for (i=0;i<window_len;i++) {
						stereo_s(l,a,t_reorder[id][sfreq][l],ms_flag,is_pos,header);
						l++;
					}
                                }
                                /* this won't do anymore
				 * window_len=-t_s[sfb]+t_s[++sfb];
				 */
				window_len  = -t_s[sfb++];
				window_len +=  t_s[sfb];

                        }
                        while (l<576) {
			        int reorder = t_reorder[id][sfreq][l++];
			  
				xr[0][0][reorder]=xr[1][0][reorder]=0;
			}
                }
        else {
        /*
	 * long blocks w/stereo processing *************************************
	 */
                int *preflag=info->preflag[gr];

		ms_flag=find_isbound(isbound,gr,info,header);

		sfb=0; l=0;
		scalefac[0]=scalefac_l[gr][0][sfb];
		a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
		scalefac[1]=scalefac_l[gr][1][sfb];
		a[1]=fras_l(sfb,global_gain[1],scalefac_scale[1],scalefac[1],preflag[1]);

		/* no intensity stereo part
		*/
	if (ms_flag)
                while (l< isbound[0]) {
#if defined(PENTIUM_RDTSC)

unsigned int cnt4, cnt3, cnt2, cnt1;
static int min_cycles = 99999999;

        __asm__(".byte 0x0f,0x31" : "=a" (cnt1), "=d" (cnt4));
#endif

			{
				register float Mi = fras2(is[0][l],a[0]);
				register float Si = fras2(is[1][l],a[1]);
				register float tmp = i_sq2;
				xr[0][0][l]=(Mi+Si)*tmp;
				xr[1][0][l]=(Mi-Si)*tmp;
			}

#if defined(PENTIUM_RDTSC)
                        __asm__(".byte 0x0f,0x31" : "=a" (cnt2), "=d" (cnt4));

                        if (cnt2-cnt1 < min_cycles) {
                          min_cycles = cnt2-cnt1;
                          //printf("%d cycles\n", min_cycles);
                        }

#endif
                        if (l==t_l[sfb]) {
#if defined(PENTIUM_RDTSC2)

unsigned int cnt4, cnt2, cnt1;
static int min_cycles = 99999999;

        __asm__(".byte 0x0f,0x31" : "=a" (cnt1), "=d" (cnt4));
#endif

                                sfb++;
				scalefac[0]=scalefac_l[gr][0][sfb];
				a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
				scalefac[1]=scalefac_l[gr][1][sfb];
				a[1]=fras_l(sfb,global_gain[1],scalefac_scale[1],scalefac[1],preflag[1]);
#if defined(PENTIUM_RDTSC2)
                        __asm__(".byte 0x0f,0x31" : "=a" (cnt2), "=d" (cnt4));

                        if (cnt2-cnt1 < min_cycles) {
                          min_cycles = cnt2-cnt1;
                          //printf("%d cycles, sfb %d\n", min_cycles, sfb);
                        }

#endif
                        }
                        l++;
                }
	else
                while (l< isbound[0]) {
                        xr[0][0][l]=fras2(is[0][l],a[0]);
                        xr[1][0][l]=fras2(is[1][l],a[1]);
                        if (l==t_l[sfb]) {
                                sfb++;
                                scalefac[0]=scalefac_l[gr][0][sfb];
                                a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
                                scalefac[1]=scalefac_l[gr][1][sfb];
                                a[1]=fras_l(sfb,global_gain[1],scalefac_scale[1],scalefac[1],preflag[1]);
                        }
                        l++;
                }


		/* intensity stereo part
		*/
		while (l<(MAX(non_zero[0],non_zero[1]))) {
			int is_pos=scalefac[1];
	
			if (id==1) /* MPEG1 */
			{
				if (is_pos==7) is_pos=IS_ILLEGAL;
			}
			else /* MPEG2 */
			{
				if (is_pos==is_max[sfb]) is_pos=IS_ILLEGAL;
			}
			stereo_l(l,a,ms_flag,is_pos,header);

			if (l==t_l[sfb]) {
				sfb++;
				scalefac[0]=scalefac_l[gr][0][sfb];
				scalefac[1]=scalefac_l[gr][1][sfb];
				a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
			}
			l++;
		}

                while (l<576) {
			xr[0][0][l]=0;
			xr[1][0][l]=0;
			l++;
		}
        }
}

/*
 * requantize_downmix **********************************************************
 */
void requantize_downmix(int gr,struct SIDE_INFO *info,struct AUDIO_HEADER *header)
{
int l,sfb,ms_flag,i;
int *global_gain,subblock_gain[2],*scalefac_scale,scalefac[2];
int sfreq=header->sampling_frequency;
int id = header->ID;
float a[2];

	/* first set some variables
	*/
	global_gain=info->global_gain[gr];
	scalefac_scale=info->scalefac_scale[gr];

	if (header->mode_extension==2 || header->mode_extension==3) ms_flag=1;
	else ms_flag=0;

	/* ... and then we're off for requantization
	*/
	if (info->window_switching_flag[gr][0] && info->block_type[gr][0]==2)
		if (info->mixed_block_flag[gr][0]) {
			ErrorLog("Internal error in MPEG decoder: mixed blocks encountered.");
		} else {
			int window,window_len;
			int isbound[3];
			int is_pos;

			memset(isbound, 0, sizeof(isbound));

			sfb=0; l=0; window_len=t_s[0]+1;
			
			while (l<(MAX(non_zero[0],non_zero[1]))) {
				for (window=0;window<3;window++) {
					subblock_gain[0]=info->subblock_gain[gr][0][window];
					subblock_gain[1]=info->subblock_gain[gr][1][window];
					scalefac[0]=scalefac_s[gr][0][sfb][window];
					is_pos=scalefac[1]=scalefac_s[gr][1][sfb][window];

					if (t_s[sfb] < isbound[window]) {
						a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
						if (ms_flag) {
							for (i=0;i<window_len;i++) {
								register float Mi=fras2(is[0][l],a[0]);
								xr[0][0][t_reorder[id][sfreq][l]]=Mi*i_sq2;
								l++;
							}
						} else {
							a[1]=fras_s(global_gain[1],subblock_gain[1],scalefac_scale[1],scalefac[1]);
							for (i=0;i<window_len;i++) {
								register float tmp1=fras2(is[0][l],a[0]);
								register float tmp2=fras2(is[1][l],a[1]);
								xr[0][0][t_reorder[id][sfreq][l]]=(tmp1+tmp2)*0.5f;
								l++;
							}
						}
					} else {
						a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
						for (i=0;i<window_len;i++) {
							register float ftmp = fras2(is[0][l], a[0]);
							if (id==0 && is_pos<is_max[sfb])
								ftmp*=t_downmix[intensity_scale][(is_pos+1)>>1];
							xr[0][0][t_reorder[id][sfreq][l]] = ftmp;
							l++;
						}						
					}
				}
				window_len  = -t_s[sfb++];
				window_len +=  t_s[sfb];
			}
			while (l<576) {
				xr[0][0][l]=0;
				l++;
			}
		}
	else {
		int *preflag=info->preflag[gr];
		int isbound;

		if (header->mode_extension==1 || header->mode_extension==3) {
			int tmp=non_zero[1];
			while (is[1][tmp] == 0) tmp--;
			sfb=0; while (t_l[sfb] < tmp && sfb < 21) sfb++;
			isbound=t_l[sfb]+1;
			no_of_imdcts[0]=no_of_imdcts[1]=(non_zero[0]-1)/18+1;
		} else {
			isbound=(MAX(non_zero[0],non_zero[1]));
			no_of_imdcts[0]=no_of_imdcts[1]=(isbound-1)/18+1;
		}

                sfb=0; l=0;
                scalefac[0]=scalefac_l[gr][0][sfb];
                a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
                scalefac[1]=scalefac_l[gr][1][sfb];
                a[1]=fras_l(sfb,global_gain[1],scalefac_scale[1],scalefac[1],preflag[1]);		

		/* no intensity stereo part
		 */
		if (ms_flag)
			while (l < isbound) {
				register float Mi = fras2(is[0][l],a[0]);
				register float tmp = i_sq2;
				xr[0][0][l]=Mi*tmp;

				if (l==t_l[sfb]) {
					sfb++;
                                	scalefac[0]=scalefac_l[gr][0][sfb];
                                	a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
					scalefac[1]=scalefac_l[gr][1][sfb];
				}
				l++;
			}
		else 
			while (l < isbound) {
				register float tmp1=fras2(is[0][l],a[0]);
				register float tmp2=fras2(is[1][l],a[1]);
				xr[0][0][l]=(tmp1+tmp2)*0.5f;
	                        if (l==t_l[sfb]) {
        	                        sfb++;
                	                scalefac[0]=scalefac_l[gr][0][sfb];
                        	        a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
                                	scalefac[1]=scalefac_l[gr][1][sfb];
                                	a[1]=fras_l(sfb,global_gain[1],scalefac_scale[1],scalefac[1],preflag[1]);
                        	}
                        	l++;
			}
		/* intensity stereo part
		*/
		while (l<(MAX(non_zero[0],non_zero[1]))) {
			int is_pos=scalefac[1];
			register float ftmp=fras2(is[0][l], a[0]);

			if (id==0  && is_pos<is_max[sfb]) {
				ftmp*=t_downmix[intensity_scale][(is_pos+1)>>1];
			}

			xr[0][0][l] = ftmp;

			if (l==t_l[sfb]) {
				sfb++;
				scalefac[0]=scalefac_l[gr][0][sfb];
				a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
				scalefac[1]=scalefac_l[gr][1][sfb];
			}
			l++;
		}

		/* _always_ zero out everything else
		*/
		while (l<576) {
			xr[0][0][l]=0;
			l++;
		}
	}
}

/* 
 * antialiasing butterflies ****************************************************
 * 
 */
void alias_reduction(int ch)
{
unsigned int sb;

        for (sb=1;sb<32;sb++) {
	        float *x = xr[ch][sb];
		register float a, b;

	        a = x[0];
		b = x[-1];
                x[-1] = b * Cs[0] - a * Ca[0];
                x[0]  = a * Cs[0] + b * Ca[0];

	        a = x[1];
		b = x[-2];
                x[-2] = b * Cs[1] - a * Ca[1];
                x[1]  = a * Cs[1] + b * Ca[1];

	        a = x[2];
		b = x[-3];
                x[-3] = b * Cs[2] - a * Ca[2];
                x[2]  = a * Cs[2] + b * Ca[2];

	        a = x[3];
		b = x[-4];
                x[-4] = b * Cs[3] - a * Ca[3];
                x[3]  = a * Cs[3] + b * Ca[3];

	        a = x[4];
		b = x[-5];
                x[-5] = b * Cs[4] - a * Ca[4];
                x[4]  = a * Cs[4] + b * Ca[4];

	        a = x[5];
		b = x[-6];
                x[-6] = b * Cs[5] - a * Ca[5];
                x[5]  = a * Cs[5] + b * Ca[5];

	        a = x[6];
		b = x[-7];
                x[-7] = b * Cs[6] - a * Ca[6];
                x[6]  = a * Cs[6] + b * Ca[6];

	        a = x[7];
		b = x[-8];
                x[-8] = b * Cs[7] - a * Ca[7];
                x[7]  = a * Cs[7] + b * Ca[7];
	}
}

/* calculating t_43 instead of having that big table in misc2.h
 */

void calculate_t43(void)
{
int i;
	for (i=0;i<8192;i++)
		t_43[i]=(float)pow((float)i,1.33333333333f);
}
