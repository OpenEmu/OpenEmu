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
/* Generates okiadpcm-deltatable.h for okiadpcm.cpp */
/* WARNING: Not compiled/updated automatically. */

#define OKIADPCM_GENERATE_DELTATABLE
#include "okiadpcm.cpp" // EVIL EVIL BE GONE YE EVIL

static void BuildDeltaTable(int32 DeltaTable[49][16])
{
 int32 extra_accuracy_mul;
 int32 extra_accuracy_div;

 extra_accuracy_mul = 1;
 extra_accuracy_div = 1;

 for(int ssi = 0; ssi < 49; ssi++)
 {
  for(int nibbles = 0; nibbles < 16; nibbles++)
  {
   int32 ssn = OKIADPCM_StepSizes[ssi];
   int32 delta = 0;

   if(nibbles & 0x4)
    delta += ssn * extra_accuracy_mul;

   if(nibbles & 0x2)
    delta += ssn * extra_accuracy_mul / 2;

   if(nibbles & 0x1)
    delta += ssn * extra_accuracy_mul / 4;

   delta += ssn * extra_accuracy_mul / 8;

   // Divide(or shift right in the future!) before inverting the sign so there won't be any asymmetric  rounding issues
   // for negative numbers.
   delta /= extra_accuracy_div;

   if(nibbles & 8)
    delta = -delta;
 
   DeltaTable[ssi][nibbles] = delta;
  }
 }
}

int main(int argc, char *argv[])
{
 int32 DeltaTable[49][16];

 BuildDeltaTable(DeltaTable);

 for(int ssi = 0; ssi < 49; ssi++)
 {
  puts("{");
  for(int nibbles = 0; nibbles < 16; nibbles++)
  {
   printf((nibbles == 15) ? "%d ":"%d, ", DeltaTable[ssi][nibbles]);
  }
  puts("},");
 }

 return(0);
}
