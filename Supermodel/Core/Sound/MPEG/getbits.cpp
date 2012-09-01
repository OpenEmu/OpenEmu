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
 * getbits.cpp
 * 
 * Amp library internal module.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* getbits.c  bit level routines, input buffer
 * 
 * Created by: tomislav uzelac  Apr 1996 
 * better synchronization, tomislav uzelac, Apr 23 1997
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amp.h"
#include "audio.h"
#include "formats.h"
#include "rtbuf.h"

#define	GETBITS
#include "getbits.h"

static const char *fstart, *lstart;
static int offset, end, eof, lend;

void MPEG_GetPlayPosition(int *playOffset, int *endOffset)
{
	if (in_file) 
		*playOffset = ftell(in_file);
	else
		*playOffset = offset;
	*endOffset = end;
}

void MPEG_SetPlayPosition(int playOffset, int endOffset)
{
	offset = playOffset;
	end = endOffset;
}

void m1setfile(const char *mstart, int mend)
{
	fstart = mstart;
	offset = 0;
	eof = 0;
	end = mend;
	lstart = NULL;
}

void MPEG_SetLoop(const char *loop, int loopend)
{
	lstart = loop;
	lend = loopend;
}

int m1fread(unsigned char *buf, int size1, int size2, void *f)
{
	int total = size1 * size2;

	if (in_file) return fread(buf, size1, size2, (FILE *) f);
	
	//printf("fstart=%X, total=%X, offset=%X, end=%X\n", fstart, total, offset, end);

	// if past EOF
	if ((total + offset) >= end)
	{
		if (lstart == NULL)
		{
			total = end - offset;
			eof = 1;
		}
		else
		{
			// if past the end, do the xfer in 2 pieces
			if ((total + offset) > end)
			{
				memcpy(buf, fstart + offset, end-offset);
				buf += (end-offset);
				total -= (end-offset);
			}

			fstart = lstart;
			offset = 0;
			end = lend;
		}
	}

	memcpy(buf, fstart + offset, total);

	offset += total;

	return total;
}

int m1feof(void *f)
{
	if (in_file) return feof((FILE *)f);
	return eof;
}

int m1fseek(void *f, int offs, int whence)
{
	if (in_file) return fseek((FILE *) f, offs, whence);

	switch (whence)
	{
		case SEEK_CUR:
			if ((offset + offs) < 0)
			{
				offset = 0;
				eof = 0;
				return -1;
			}
			if ((offset + offs) > end)
			{
				offset = end;
				eof = 1;
				return end;
			}
			offset += offs; 
			eof = 0;
			break;
	}
	return 0;
}
      
/* 
 * buffer and bit manipulation functions ***************************************
 */
static inline int _fillbfr(unsigned int size)
{
	_bptr=0;
        return get_input(_buffer, size);
}

static inline int readsync()
{
	_bptr=0;
	_buffer[0]=_buffer[1];
	_buffer[1]=_buffer[2];
	_buffer[2]=_buffer[3];
	return get_input(&_buffer[3],1);
}

static inline unsigned int _getbits(int n)
{
unsigned int pos,ret_value;

        pos = _bptr >> 3;
	ret_value = _buffer[pos] << 24 |
		    _buffer[pos+1] << 16 |
		    _buffer[pos+2] << 8 |
		    _buffer[pos+3];
        ret_value <<= _bptr & 7;
        ret_value >>= 32 - n;
        _bptr += n;
        return ret_value;
}       

int fillbfr(unsigned int advance)
{
int overflow,retval;

        retval=get_input(&buffer[append], advance);
	
	if ( append + advance >= BUFFER_SIZE ) {
		overflow = append + advance - BUFFER_SIZE;
		memcpy (buffer,&buffer[BUFFER_SIZE], overflow);
		if (overflow < 4) memcpy(&buffer[BUFFER_SIZE],buffer,4);
		append = overflow;
	} else {
		if (append==0) memcpy(&buffer[BUFFER_SIZE],buffer,4);
		append+=advance;
	}
	return retval;
}

unsigned int getbits(int n)
{
        if (n) {
        unsigned int pos,ret_value;

                pos = data >> 3;
                ret_value = buffer[pos] << 24 |
                        buffer[pos+1] << 16 |
                        buffer[pos+2] << 8 |
                        buffer[pos+3];
                ret_value <<= data & 7;
                ret_value >>= 32 - n;

                data += n;
                data &= (8*BUFFER_SIZE)-1;

                return ret_value;
        } else
                return 0;
}

/*
 * header and side info parsing stuff ******************************************
 */
static inline void parse_header(struct AUDIO_HEADER *header) 
{
        header->ID=_getbits(1);
        header->layer=_getbits(2);
        header->protection_bit=_getbits(1);
        header->bitrate_index=_getbits(4);
        header->sampling_frequency=_getbits(2);
        header->padding_bit=_getbits(1);
        header->private_bit=_getbits(1);
        header->mode=_getbits(2);
        header->mode_extension=_getbits(2);
        if (!header->mode) header->mode_extension=0;
        header->copyright=_getbits(1);
        header->original=_getbits(1);
        header->emphasis=_getbits(2);
}

static inline int header_sanity_check(struct AUDIO_HEADER *header)
{
	if ( 	header->layer==0 ||
		header->bitrate_index==15 ||
		header->sampling_frequency==3) return -1;

	/* an additional check to make shure that stuffing never gets mistaken
 	 * for a syncword. This rules out some legal layer1 streams, but who
 	 * cares about layer1 anyway :-). I must get this right sometime.
	 */
	if ( header->ID==1 && header->layer==3 && header->protection_bit==1) return -1;
	return 0;
}


int gethdr(struct AUDIO_HEADER *header)
{
int s,retval;
struct AUDIO_HEADER tmp;

	/* TODO: add a simple byte counter to check only first, say, 1024
	 * bytes for a new header and then return GETHDR_SYN
	 */
	if ((retval=_fillbfr(4))!=0) return retval;

	for(;;) {
		while ((s=_getbits(12)) != 0xfff) {
			if (s==0xffe) {
				parse_header(&tmp);
				if (header_sanity_check(&tmp)==0) return GETHDR_NS;
			}
			if ((retval=readsync())!=0) return retval;
		}
	
		parse_header(&tmp);
		if (header_sanity_check(&tmp)!=0) {
			if ((retval=readsync())!=0) return retval;
		} else break;
	}

	if (tmp.layer==3) return GETHDR_FL1;
	/* if (tmp.layer==2) return GETHDR_FL2; */
	if (tmp.bitrate_index==0) return GETHDR_FF;
		
	//printf("layer: %d\n", tmp.layer);
	//printf("sampling frequency: %d\n", tmp.sampling_frequency);

	memcpy(header,&tmp,sizeof(tmp));
	return 0;
}

/* dummy function, to get crc out of the way
*/
void getcrc()
{
	_fillbfr(2);
	_getbits(16);
}

/* sizes of side_info:
 * MPEG1   1ch 17    2ch 32
 * MPEG2   1ch  9    2ch 17
 */
void getinfo(struct AUDIO_HEADER *header,struct SIDE_INFO *info)
{
int gr,ch,scfsi_band,region,window;
int nch;	
	if (header->mode==3) {
		nch=1;
		if (header->ID) {
			_fillbfr(17);
			info->main_data_begin=_getbits(9);
			_getbits(5);
		} else {
			_fillbfr(9);
			info->main_data_begin=_getbits(8);
			_getbits(1);
		}
	} else {
		nch=2;
                if (header->ID) {
			_fillbfr(32);
                        info->main_data_begin=_getbits(9);
                        _getbits(3);
                } else {
			_fillbfr(17);
                        info->main_data_begin=_getbits(8);
                        _getbits(2);
                }
	}

	if (header->ID) for (ch=0;ch<nch;ch++)
		for (scfsi_band=0;scfsi_band<4;scfsi_band++)
			info->scfsi[ch][scfsi_band]=_getbits(1);

	for (gr=0;gr<(header->ID ? 2:1);gr++)
		for (ch=0;ch<nch;ch++) {
			info->part2_3_length[gr][ch]=_getbits(12);
			info->big_values[gr][ch]=_getbits(9);
			info->global_gain[gr][ch]=_getbits(8);
			if (header->ID) info->scalefac_compress[gr][ch]=_getbits(4);
			else info->scalefac_compress[gr][ch]=_getbits(9);
			info->window_switching_flag[gr][ch]=_getbits(1);

			if (info->window_switching_flag[gr][ch]) {
				info->block_type[gr][ch]=_getbits(2);
				info->mixed_block_flag[gr][ch]=_getbits(1);

				for (region=0;region<2;region++)
					info->table_select[gr][ch][region]=_getbits(5);
				info->table_select[gr][ch][2]=0;

				for (window=0;window<3;window++)
					info->subblock_gain[gr][ch][window]=_getbits(3);
			} else {
				for (region=0;region<3;region++)
					info->table_select[gr][ch][region]=_getbits(5);

				info->region0_count[gr][ch]=_getbits(4);
				info->region1_count[gr][ch]=_getbits(3);
				info->block_type[gr][ch]=0;
			}

			if (header->ID) info->preflag[gr][ch]=_getbits(1);
			info->scalefac_scale[gr][ch]=_getbits(1);
			info->count1table_select[gr][ch]=_getbits(1);
		}
	return;
}

int dummy_getinfo(int n)
{
	n-=4;
        if ( m1fseek(in_file,n,SEEK_CUR) != 0)
	{
                if (m1feof(in_file)) return GETHDR_EOF;
                else return GETHDR_ERR;
	}
	return 0;
}

int rewind_stream(int nbytes)
{
	nbytes+=5;
	if (m1fseek(in_file, -nbytes, SEEK_CUR) != 0) {
		/* what if we need to be at the very beginning? */
		nbytes--;
		if (m1fseek(in_file, -nbytes, SEEK_CUR) != 0) return GETHDR_ERR;
	}
	return 0;
}

static inline int get_input(unsigned char* bp, unsigned int size)
{
#ifdef LINUX_REALTIME
        return prefetch_get_input(bp,size);
#else /* LINUX_REALTIME */
	if ( m1fread( bp , 1, size, in_file) != size) 
	{
                if (m1feof(in_file)) return GETHDR_EOF;
                else return GETHDR_ERR;
	}
	return 0;
#endif /* LINUX_REALTIME */
}
