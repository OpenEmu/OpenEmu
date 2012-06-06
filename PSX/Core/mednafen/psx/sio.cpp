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

#include "psx.h"
#include "sio.h"

namespace MDFN_IEN_PSX
{

// Dummy implementation.

static uint16 Status;
static uint16 Mode;
static uint16 Control;
static uint16 BaudRate;
static uint32 DataBuffer;

void SIO_Power(void)
{
 Status = 0;
 Mode = 0;
 Control = 0;
 BaudRate = 0;
 DataBuffer = 0;
}

uint32 SIO_Read(pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 ret = 0;

 switch(A & 0xE)
 {
  default:
	PSX_WARNING("[SIO] Unknown read: 0x%08x -- %d\n", A, timestamp);
	break;

  case 0x0:
  //case 0x2:
	ret = DataBuffer >> ((A & 2) * 8);
	break;

  case 0x4:
	ret = Status;
	break;

  case 0x8:
	ret = Mode;
	break;

  case 0xA:
	ret = Control;
	break;

  case 0xE:
	ret = BaudRate;
	break;
 }

 return(ret >> ((A & 1) * 8));
}

void SIO_Write(pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 V <<= (A & 1) * 8;

 switch(A & 0xE)
 {
  default:
	PSX_WARNING("[SIO] Unknown write: 0x%08x 0x%08x -- %d\n", A, V, timestamp);
	break;

  case 0x0:
  //case 0x2:
	V <<= (A & 2) * 8;
	DataBuffer = V;
        break;

  case 0x8:
        Mode = V;
        break;

  case 0xA:
        Control = V;
        break;

  case 0xE:
        BaudRate = V;
        break;
 }
}

}
