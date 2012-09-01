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
 * audio.cpp
 *
 * Main Amp module.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/

/* audio.c	main amp source file 
 *
 * Created by: tomislav uzelac	Apr 1996 
 * Karl Anders Oygard added the IRIX code, 10 Mar 1997.
 * Ilkka Karvinen fixed /dev/dsp initialization, 11 Mar 1997.
 * Lutz Vieweg added the HP/UX code, 14 Mar 1997.
 * Dan Nelson added FreeBSD modifications, 23 Mar 1997.
 * Andrew Richards complete reorganisation, new features, 25 Mar 1997
 * Edouard Lafargue added sajber jukebox support, 12 May 1997
 */ 


#include "amp.h"

#define AUDIO
#include "audio.h"
#include "formats.h"
#include "getbits.h"
#include "huffman.h"
#include "layer2.h"
#include "layer3.h"
#include "position.h"
#include "rtbuf.h"
#include "transform.h"

//#ifndef __BEOS__
//typedef int bool;
//#endif

#include <new>
#include <cstring>
#include "Supermodel.h"
#include "MPEG.h"

//#include "m1snd.h"
//#include "oss.h"
//#include "mpeg.h"

#define BUF_SIZE (1152 * sizeof(short) * 2)

void calculate_t43(void);

static int decoder_init = 0;
static int cnt = 0;
static int stream = -1;
static char *buf0;
static int playing = 0;
static int outpos = 0;
static int mpeg_eof = 0;
static char *dst, *readbuf;
static struct AUDIO_HEADER m1hdr;;

void statusDisplay(struct AUDIO_HEADER *header, int frameNo)
{
	int minutes,seconds;

	if ((A_SHOW_CNT || A_SHOW_TIME) && !(frameNo%10))
		msg("\r");
	if (A_SHOW_CNT && !(frameNo%10) ) {
		msg("{ %d } ",frameNo);
	}
	if (A_SHOW_TIME && !(frameNo%10)) {
		seconds=frameNo*1152/t_sampling_frequency[header->ID][header->sampling_frequency];
		minutes=seconds/60;
		seconds=seconds % 60;
		msg("[%d:%02d]",minutes,seconds);
	}
	if (A_SHOW_CNT || A_SHOW_TIME)
		fflush(stderr);
}

// one mpeg frame is 576 samples.
int decodeMPEGOneFrame(struct AUDIO_HEADER *header)
{
	int snd_eof = 0, g;

	if ((g=gethdr(header))!=0) {
		report_header_error(g);
		snd_eof=1;
		return snd_eof;
	}

	if (header->protection_bit==0) getcrc();

	statusDisplay(header,0);

	if (header->layer==1) {
		if (layer3_frame(header,cnt)) {
			ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
			return -1;
		}
	} else if (header->layer==2)
		if (layer2_frame(header,cnt)) {
			ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
			return -1;
		}

	cnt++;

	return snd_eof;
}

int decodeMPEG(void)
{
struct AUDIO_HEADER header;
int g,snd_eof=0;

	initialise_globals();
	
	cnt = 0;

	if ((g=gethdr(&header))!=0) {
		report_header_error(g);
		return -1;
	}

	if (header.protection_bit==0) getcrc();

	//printf("%d Hz, layer %d\n", t_sampling_frequency[header.ID][header.sampling_frequency], header.layer);

	if (setup_audio(&header)!=0) {	// will never fail (setup_audio() does nothing)
		ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
		return -1;
	}
	
	if (header.layer==1) {
		if (layer3_frame(&header,cnt)) {
			ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
			return -1;
		}
	} else if (header.layer==2)
		if (layer2_frame(&header,cnt)) {
			ErrorLog("Internal error in MPEG decoder (%s:%d).", __FILE__, __LINE__);
			return -1;
		}

	/*
	 * decoder loop **********************************
	 */
	snd_eof=0;
	while (!snd_eof) {
		while (!snd_eof && ready_audio()) {
			snd_eof = decodeMPEGOneFrame(&header);
		}
	}
	return 0;
}

/* call this once at the beginning
 */
void initialise_decoder(void)
{
	premultiply();
	imdct_init();
	calculate_t43();
}

/* call this before each file is played
 */
void initialise_globals(void)
{
	append=data=nch=0; 
        f_bdirty=TRUE;
        bclean_bytes=0;

	memset(s,0,sizeof s);
	memset(res,0,sizeof res);
}

void report_header_error(int err)
{
	switch (err) {
		case GETHDR_ERR:
					ErrorLog("Internal error in MPEG decoder: unable to read bit stream.");
					break;
		case GETHDR_NS : 
					ErrorLog("Internal error in MPEG decoder: invalid MPEG format encountered.");
					break;
		case GETHDR_FL1: 
					ErrorLog("Internal error in MPEG decoder: unsupported MPEG format encountered.");
					break;
		case GETHDR_FF : 
					ErrorLog("Internal error in MPEG decoder: unsupported bit stream encountered.");
					break;	
		case GETHDR_SYN: 
					ErrorLog("Internal error in MPEG decoder: out of sync!");
					break;
		case GETHDR_EOF: 
		default: 		; /* some stupid compilers need the semicolon */
	}	
}

int setup_audio(struct AUDIO_HEADER *header)
{
	return 0;
}

void close_audio(void)
{
}

int ready_audio(void)
{
	return 1;
}

// callback: called by the engine to output a frame of audio
void printout(void)
{
	int j;

        if (nch==2)
	{
                j=32 * 18 * 2;
	}
        else
	{
                j=32 * 18;
	}

//	printf("printout: %x, %d\n", (unsigned int), j*2);
	memcpy(dst, sample_buffer, j*2);

	dst += j*2;
	outpos += j/2;
}

void MPEG_Decode(INT16 **outputs, int length) 
{
	int i, remaining, bias;
	INT16 *get;

	remaining = length;

//	printf("%d: %x %x\n", length, (unsigned int)outputs[0], (unsigned int)outputs[1]);

	if (!playing)
	{
		memset(&outputs[0][0], 0, length * sizeof(INT16));
		memset(&outputs[1][0], 0, length * sizeof(INT16));
		return;
	}

	bias = 0;

	// will we need more data from the decoder?
	if (outpos < length)
	{
		// if there's anything left in the current buffer, drain it first
		if (outpos != 0)
		{
			get = (INT16 *)readbuf;

			for (i = 0; i < outpos; i++)
			{
				outputs[1][i] = *get++;
				outputs[0][i] = *get++;
			}

			remaining -= outpos;
			bias = outpos;
			readbuf += (outpos * 4);
		}

		outpos = 0;
		dst = buf0;
		while ((outpos < remaining) && (playing))
		{
			mpeg_eof = decodeMPEGOneFrame(&m1hdr);
			if (mpeg_eof)
			{
				MPEG_StopPlaying();
			}
		}

		// reset read pointer
		readbuf = buf0;
	}

	get = (INT16 *)readbuf;

	for (i = 0; i < remaining; i++)
	{
		outputs[1][i+bias] = *get++;
		outputs[0][i+bias] = *get++;
	}

	outpos -= remaining;
	readbuf += (remaining * 4);
}

void MPEG_PlayFile(char *filename)
{
	memset(buf0, 0, BUF_SIZE);

	in_file = fopen(filename, "rb");

	initialise_globals();

	cnt = 0;
	mpeg_eof = 0;
	outpos = 0;
	dst = buf0;
	readbuf = buf0;

	gethdr(&m1hdr);
	if (m1hdr.protection_bit == 0) getcrc();

//	printf("%d Hz, layer %d\n", t_sampling_frequency[m1hdr.ID][m1hdr.sampling_frequency], m1hdr.layer);

//	stream_set_srate(stream, t_sampling_frequency[m1hdr.ID][m1hdr.sampling_frequency]);

	// prime the stream
	if (m1hdr.layer == 1) 
	{
		layer3_frame(&m1hdr, cnt);
	}
	else if (m1hdr.layer == 2) 
	{
		layer2_frame(&m1hdr, cnt);
	}

	playing = 1;
}

extern void m1setfile(const char *mstart, int mend);
void MPEG_PlayMemory(const char *sa, int length)
{
	memset(buf0, 0, BUF_SIZE);
	
	m1setfile(sa, length);

	initialise_globals();

	cnt = 0;
	mpeg_eof = 0;
	outpos = 0;
	dst = buf0;
	readbuf = buf0;

	gethdr(&m1hdr);
	if (m1hdr.protection_bit == 0) getcrc();

//	printf("%d Hz, layer %d\n", t_sampling_frequency[m1hdr.ID][m1hdr.sampling_frequency], m1hdr.layer);

//	stream_set_srate(stream, t_sampling_frequency[m1hdr.ID][m1hdr.sampling_frequency]);

	// prime the stream
	if (m1hdr.layer == 1) 
	{
		layer3_frame(&m1hdr, cnt);
	}
	else if (m1hdr.layer == 2) 
	{
		layer2_frame(&m1hdr, cnt);
	}

	in_file = NULL;

	playing = 1;
}

void MPEG_StopPlaying(void)
{
	if (playing)
	{
		playing = 0;
		if (in_file)
			fclose(in_file);
	}
}

bool MPEG_IsPlaying(void)
{
	return playing ? TRUE : false;
}

bool MPEG_Init(void)
{
	if (!decoder_init)
	{
		initialise_decoder();	/* initialise decoder */
		decoder_init = 1;
			buf0 = new(std::nothrow) char[BUF_SIZE];
		if (NULL == buf0)
			return FAIL;
		memset(buf0, 0, BUF_SIZE);
		playing = 0;
	}

	return OKAY;
}

void MPEG_Shutdown( void )
{
	decoder_init = 0;
	if (buf0 != NULL)
		delete [] buf0;
	buf0 = NULL;
}

