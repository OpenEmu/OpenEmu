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
 * position.cpp
 * 
 * Amp library internal module.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
/* position.c 	ffwd/rew within a stream
 *
 * Creted by: Tomislav Uzelac, May 10 1997
 */
#include <stdlib.h>
#include <string.h>
#include "amp.h"
#include "audio.h"
#include "getbits.h"

#define POSITION
#include "position.h"

/* Returns the number of frames actually skipped, -1 on error.
 *
 * Values in header are not changed if retval!=nframes.
 * This is not necessary because gethdr() doesn't clobber
 * the contents of header, but I don't want to rely on that. 
 */
int ffwd(struct AUDIO_HEADER *header, int nframes)
{
int cnt=0,g;
int hsize,bitrate,fs,mean_frame_size;
struct AUDIO_HEADER tmp;
	memcpy(&tmp,header,sizeof(tmp));

	while (cnt < nframes) {
	        if (tmp.ID)
        	        if (tmp.mode==3) hsize=21;
                	else hsize=36;
        	else
                	if (tmp.mode==3) hsize=13;
               	 	else  hsize=21;
                if (tmp.protection_bit==0) hsize+=2;
		if ((g=dummy_getinfo(hsize)))  /* dummy_getinfo: reads hsize-4 bytes */
			switch (g) {
                        case GETHDR_EOF: return cnt;
			case GETHDR_ERR:
			default:	return -1;
                        }

	        bitrate=t_bitrate[tmp.ID][3-tmp.layer][tmp.bitrate_index];
        	fs=t_sampling_frequency[tmp.ID][tmp.sampling_frequency];		
	        if (tmp.ID) mean_frame_size=144000*bitrate/fs;
	        else mean_frame_size=72000*bitrate/fs;
		fillbfr(mean_frame_size + tmp.padding_bit - hsize);

		if ((g=gethdr(&tmp))) 
			switch (g) {
			case GETHDR_EOF: return cnt;
			case GETHDR_ERR:
			default:	return -1;
			}
		cnt++;
	}	

	memcpy(header,&tmp,sizeof(tmp));		
	return cnt;
}

/* Mostly the same as ffwd. Some streams might be 'tough', i.e.
 * the ones switching bitrates.
 */
int rew(struct AUDIO_HEADER *header, int nframes)
{
int cnt=0;
int bitrate,fs,mean_frame_size;
struct AUDIO_HEADER tmp;
	memcpy(&tmp,header,sizeof(tmp));

	while (cnt < nframes) {
		/* ffwd/rew functions are to be called right after the header has been parsed
		 * so we have to go back one frame + 4 bytes + 1 byte (in case padding was used).
		 */
	        bitrate=t_bitrate[tmp.ID][3-tmp.layer][tmp.bitrate_index];
        	fs=t_sampling_frequency[tmp.ID][tmp.sampling_frequency];		
	        if (tmp.ID) mean_frame_size=144000*bitrate/fs;
	        else mean_frame_size=72000*bitrate/fs;

		if (rewind_stream(mean_frame_size) !=0) {
			memcpy(header,&tmp,sizeof(tmp));
			return cnt;
		}
		if ((gethdr(&tmp))) return -1; 
		cnt++;
	}
	/* We have to make sure that the bit reservoir contains enough data.
	 * Hopefully, layer3_frame will take care of that.
	 */
	f_bdirty=TRUE;
	bclean_bytes=0;

	memcpy(header,&tmp,sizeof(tmp));		
	return cnt;
}

/* TODO: after the gethdr function is enhanced with the counter to count
 * the number of bytes to search for the next syncword, make the call to
 * gethdr() from rew() have that counter something like (frame_size-1) so
 * that we don't go back again and again to the same header. (not very important)
 */
