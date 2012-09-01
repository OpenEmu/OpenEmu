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
 * getdata.h
 * 
 * Amp library internal header file.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* getdata.h 
 *
 * tomislav uzelac  Apr 1996
 */
 
extern int decode_scalefactors(struct SIDE_INFO *info,struct AUDIO_HEADER *header,int gr,int ch); 

extern int is_max[21];
extern int intensity_scale;

#ifdef GETDATA

static char t_slen1[16]={0,0,0,0,3,1,1,1,2,2,2,3,3,3,4,4};
static char t_slen2[16]={0,1,2,3,0,1,2,3,1,2,3,1,2,3,2,3};

int is_max[21]; /* the maximum value of is_pos. for short blocks is_max[sfb=0] == is_max[6],
	   	 * it's sloppy but i'm sick of waisting storage. blaah...
		 */
int intensity_scale;

int decode_scalefactors(struct SIDE_INFO *info,struct AUDIO_HEADER *header,int gr,int ch);

/* my implementation of MPEG2 scalefactor decoding is, admitably, horrible
 * anyway, just take a look at pg.18 of MPEG2 specs, and you'll know what
 * this is all about
 */
static const char spooky_table[2][3][3][4]={
{
{ {6,5,5,5},   {9,9,9,9},   {6,9,9,9} },
{ {6,5,7,3},   {9,9,12,6},  {6,9,12,6}},
{ {11,10,0,0}, {18,18,0,0}, {15,18,0,0}} 
},
{
{ {7,7,7,0},   {12,12,12,0}, {6,15,12,0}},
{ {6,6,6,3},   {12,9,9,6},   {6,12,9,6}},
{ {8,8,5,0},   {15,12,9,0},  {6,18,9,0}}
}};

#endif /* GETDATA */
