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

#ifndef __MDFN_PCE_MCGENJIN_H
#define __MDFN_PCE_MCGENJIN_H

#include <vector>
#include <blip/Blip_Buffer.h>

class MCGenjin_CS_Device
{
 public:

 MCGenjin_CS_Device();
 virtual ~MCGenjin_CS_Device();

 virtual void Power(void);
 virtual void EndFrame(int32 timestamp);

 virtual int StateAction(StateMem *sm, int load, int data_only, const char *sname);

 virtual uint8 Read(int32 timestamp, uint32 A);
 virtual void Write(int32 timestamp, uint32 A, uint8 V);

 virtual uint32 GetNVSize(void);
 virtual void ReadNV(uint8 *buffer, uint32 offset, uint32 count);
 virtual void WriteNV(const uint8 *buffer, uint32 offset, uint32 count);
};

class MCGenjin
{
 public:

 MCGenjin(Blip_Buffer *bb, const uint8 *rr, uint32 rr_size);
 ~MCGenjin();

 void Power(void);
 void EndFrame(int32 timestamp);
 int StateAction(StateMem *sm, int load, int data_only);

 INLINE unsigned GetNVPDC(void) { return 2; }
 uint32 GetNVSize(const unsigned di);
 void ReadNV(const unsigned di, uint8 *buffer, uint32 offset, uint32 count);
 void WriteNV(const unsigned di, const uint8 *buffer, uint32 offset, uint32 count);

 INLINE uint8 combobble(uint8 v)
 {
  if(dlr)
   return ((((v * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL) >> 32);
  else
   return v;
 }

 template<unsigned ar>
 INLINE uint8 ReadTP(int32 timestamp, uint32 A)
 {
  uint8 ret = 0xFF;

  switch(ar)
  {
   case 0: ret = combobble(rom[A & 0x3FFFF & (rom.size() - 1)]);
   case 1: ret = combobble(rom[((A & 0x3FFFF) | (bank_select << 18)) & (rom.size() - 1)]);
   case 2: ret = cs[0]->Read(timestamp, A & 0x3FFFF);
   case 3: ret = cs[1]->Read(timestamp, A & 0x3FFFF);
  }

  return ret;
 }

 template<unsigned ar>
 INLINE void WriteTP(int32 timestamp, uint32 A, uint8 V)
 {
  switch(ar)
  {
   case 0:
   case 1:
	if(A & 1)
	 bank_select = V;
	else
	 dlr = V & 0x01;
	break;

   case 2: return cs[0]->Write(timestamp, A & 0x3FFFF, V);
   case 3: return cs[1]->Write(timestamp, A & 0x3FFFF, V);
  }
 }


 INLINE uint8 Read(int32 timestamp, uint32 A)
 {
  switch((A >> 18) & 0x3)
  {
   default: return 0xFF;
   case 0: return ReadTP<0>(timestamp, A);
   case 1: return ReadTP<1>(timestamp, A);
   case 2: return ReadTP<2>(timestamp, A);
   case 3: return ReadTP<3>(timestamp, A);
  }
 }

 INLINE void Write(int32 timestamp, uint32 A, uint8 V)
 {
  switch((A >> 18) & 0x3)
  {
   case 0: WriteTP<0>(timestamp, A, V);
   case 1: WriteTP<1>(timestamp, A, V);
   case 2: WriteTP<2>(timestamp, A, V);
   case 3: WriteTP<3>(timestamp, A, V);
  }
 }

 private:

 std::vector<uint8> rom;

 MCGenjin_CS_Device *cs[2];

 uint8 bank_select;
 uint8 dlr;
};

#endif
