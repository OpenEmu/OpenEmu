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

#include "pcfx.h"
#include "fxscsi.h"

bool FXSCSI_Init(void)
{


}

uint8 FXSCSI_CtrlRead(uint32 A)
{
 uint8 ret = 0; //rand();
 //printf("FXSCSI: %08x(ret=%02x)\n", A, ret);
 return(ret);
}


void FXSCSI_CtrlWrite(uint32 A, uint8 V)
{
 printf("FXSCSI Write: %08x %02x\n", A, V);
}
