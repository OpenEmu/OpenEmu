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

#include "../mednafen.h"
#include "font-data.h"

const font4x5 FontData4x5[] =
{
	#include "font4x5.h"
};

const font5x7 FontData5x7[] =
{
        #include "font5x7.h"
};

const font6x13 FontData6x13[] =
{
	#include "font6x13.h"
};

const font9x18 FontData9x18[]=
{
        #include "font9x18.h"
};


const int FontData4x5_Size = sizeof(FontData4x5);
const int FontData5x7_Size = sizeof(FontData5x7);
const int FontData6x13_Size = sizeof(FontData6x13);
const int FontData9x18_Size = sizeof(FontData9x18);

