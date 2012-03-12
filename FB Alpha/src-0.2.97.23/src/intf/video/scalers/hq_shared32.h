// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#define abs32(value) (value & 0x7FFFFFFF)
#define abs16(value) (value & 0x7FFF)

const  int	Ymask = 0x00FF0000;
const  int	Umask = 0x0000FF00;
const  int	Vmask = 0x000000FF;
const  int	trY   = 0x00300000;
const  int	trU   = 0x00000700;
const  int	trV   = 0x00000006;

void Interp1(unsigned char * pc, unsigned int c1, unsigned int c2);
void Interp2(unsigned char * pc, unsigned int c1, unsigned int c2, unsigned int c3);
void Interp3(unsigned char * pc, unsigned int c1, unsigned int c2);
void Interp4(unsigned char * pc, unsigned int c1, unsigned int c2, unsigned int c3);
void Interp5(unsigned char * pc, unsigned int c1, unsigned int c2);
void Interp1_16(unsigned char * pc, unsigned short c1, unsigned short c2);
void Interp2_16(unsigned char * pc, unsigned short c1, unsigned short c2, unsigned short c3);
void Interp3_16(unsigned char * pc, unsigned short c1, unsigned short c2);
void Interp4_16(unsigned char * pc, unsigned short c1, unsigned short c2, unsigned short c3);
void Interp5_16(unsigned char * pc, unsigned short c1, unsigned short c2);
bool Diff(unsigned int c1, unsigned int c2);
unsigned int RGBtoYUV(unsigned int c);
