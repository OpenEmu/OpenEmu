/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "pce.h"

namespace MDFN_IEN_PCE
{

/*
  Startup:
	0x80 -> $1830
	0x40 -> $1823
	0x50 -> $1823
	0x02 -> $1821
	0x40 -> $1823

	Read from $1822 16 times, expecting (return value & 0xC0) == 0x80 each time

	0x50 -> $1823
	0x01 -> $1821
	0x40 -> $1823
	0x04 -> $1822

*/

DECLFR(PCE_TsushinRead)
{
 //printf("Read: %04x, %04x\n", A, HuCPU.PC);

 return(0x80);
}

DECLFW(PCE_TsushinWrite)
{
 //printf("Write: %04x %02x, %04x\n", A, V, HuCPU.PC);
}

};
