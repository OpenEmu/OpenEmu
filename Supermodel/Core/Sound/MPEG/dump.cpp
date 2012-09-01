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
 * dump.cpp
 * 
 * Amp library internal module.
 */
 

/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* dump.c  binary/hex dump from buffer
 * 
 * Created by: tomislav uzelac  May 1996
 * Last modified by: tomislav May 31 1997
 */
//#include <unistd.h>
#include <string.h>

#include "audio.h"
#include "getbits.h"

#define DUMP
#include "dump.h"

/* no hex dump, sorry
 */
void dump(int *length)   /* in fact int length[4] */
{
int i,j;
int _data,space=0;
	printf(" *********** binary dump\n");
	_data=data;
	for (i=0;i<4;i++) {
		for (j=0;j<space;j++) printf(" ");
		for (j=0;j<length[i];j++) {
			printf("%1d",(buffer[_data/8] >> (7-(_data&7)) )&1 );
			space++;
			_data++;
			_data&=8*BUFFER_SIZE-1;
			if (!(_data & 7)) {
				printf(" ");
				space++;
				if (space>70) {
					printf("\n");
					space=0;
				}
			}
		}
		printf("~\n");
	}
}

void show_header(struct AUDIO_HEADER *header)
{
int bitrate=t_bitrate[header->ID][3-header->layer][header->bitrate_index];
int fs=t_sampling_frequency[header->ID][header->sampling_frequency];
int mpg,layer;
char stm[8];
	if (A_QUIET) return;
	layer=4-header->layer;
	if (header->ID==1) mpg=1;
	else mpg=2;
	if (header->mode==3) strcpy(stm,"mono");
	else strcpy(stm,"stereo");

	printf("\n\
Properties:    %s %dHz\n\
Coding Method: MPEG%1d.0 layer%1d\n\
Bitrate:       %dkbit/s\n"\
		,stm,fs,mpg,layer,bitrate);
}
