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
#include "mcgenjin.h"

MCGenjin_CS_Device::MCGenjin_CS_Device()
{

}

MCGenjin_CS_Device::~MCGenjin_CS_Device()
{

}

void MCGenjin_CS_Device::Power(void)
{

}

void MCGenjin_CS_Device::EndFrame(int32 timestamp)
{

}

int MCGenjin_CS_Device::StateAction(StateMem *sm, int load, int data_only, const char *sname)
{
 return 1;
}


uint8 MCGenjin_CS_Device::Read(int32 timestamp, uint32 A)
{
 return 0xFF;
}

void MCGenjin_CS_Device::Write(int32 timestamp, uint32 A, uint8 V)
{

}

uint32 MCGenjin_CS_Device::GetNVSize(void)
{
 return 0;
}

void MCGenjin_CS_Device::ReadNV(uint8 *buffer, uint32 offset, uint32 count)
{
 memset(buffer, 0, count);
}

void MCGenjin_CS_Device::WriteNV(const uint8 *buffer, uint32 offset, uint32 count)
{

}

class MCGenjin_CS_Device_RAM : public MCGenjin_CS_Device
{
 public:

 MCGenjin_CS_Device_RAM(uint32 size, bool nv)
 {
  assert(round_up_pow2(size) == size);

  ram.resize(size);
  nonvolatile = nv;
 }

 virtual ~MCGenjin_CS_Device_RAM()
 {

 }

 virtual void Power(void)
 {
  if(!nonvolatile)
   ram.assign(ram.size(), 0xFF);

  bank_select = 0;
 }

 virtual int StateAction(StateMem *sm, int load, int data_only, const char *sname)
 {
  SFORMAT StateRegs[] = 
  {
   SFARRAY(&ram[0], ram.size()),
   SFVAR(bank_select),
   SFEND
  };
  int ret = 1;

  ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, sname);

  return ret;
 }


 virtual uint8 Read(int32 timestamp, uint32 A)
 {
  return ram[(A | (bank_select << 18)) & (ram.size() - 1)];
 }

 virtual void Write(int32 timestamp, uint32 A, uint8 V)
 {
  if(!A)
   bank_select = V;

  ram[(A | (bank_select << 18)) & (ram.size() - 1)] = V;
 }

 virtual uint32 GetNVSize(void)
 {
  return nonvolatile ? ram.size() : 0;
 }

 virtual void ReadNV(uint8 *buffer, uint32 offset, uint32 count)
 {
  while(count)
  {
   *buffer = ram[offset % ram.size()];
   buffer++;
   offset++;
   count--;
  }
 }

 virtual void WriteNV(const uint8 *buffer, uint32 offset, uint32 count)
 {
  while(count)
  {
   ram[offset % ram.size()] = *buffer;
   buffer++;
   offset++;
   count--;
  }
 }

 private:
 std::vector<uint8> ram;
 bool nonvolatile;
 uint8 bank_select;
};

#include "mcgenjin_ym2413.inc"

void MCGenjin::Power(void)
{
 bank_select = 0;
 dlr = 0;

 for(unsigned i = 0; i < 2; i++)
  cs[i]->Power();
}

void MCGenjin::EndFrame(int32 timestamp)
{
 for(unsigned i = 0; i < 2; i++)
  cs[i]->EndFrame(timestamp);
}

uint32 MCGenjin::GetNVSize(const unsigned di)
{
 return cs[di]->GetNVSize();
}


void MCGenjin::ReadNV(const unsigned di, uint8 *buffer, uint32 offset, uint32 count)
{
 cs[di]->ReadNV(buffer, offset, count);
}

void MCGenjin::WriteNV(const unsigned di, const uint8 *buffer, uint32 offset, uint32 count)
{
 cs[di]->WriteNV(buffer, offset, count);
}

MCGenjin::MCGenjin(Blip_Buffer *bb, const uint8 *rr, uint32 rr_size)
{
 uint8 revision, num256_pages, region, cs_di[2];

 if(rr_size < 8192)
  throw MDFN_Error(0, _("MCGenjin ROM size is too small!"));

 if(memcmp(rr + 0x1FD0, "MCGENJIN", 8))
  throw MDFN_Error(0, _("MC Genjin header magic missing!"));

 rom.resize(round_up_pow2(rr_size));

 memcpy(&rom[0], rr, rr_size);

 revision = rom[0x1FD8];
 num256_pages = rom[0x1FD9];
 region = rom[0x1FDA];
 cs_di[0] = rom[0x1FDB];
 cs_di[1] = rom[0x1FDC];

 for(unsigned i = 0; i < 2; i++)
 {
  switch(cs_di[i])
  {
   default:
	for(unsigned si = 0; si < i; si++) // FIXME: auto ptr to make this not necessary
         delete cs[si];

	throw MDFN_Error(0, _("Unsupported MCGENJIN device on CS%d: 0x%02x"), i, cs_di[i]);
	break;

   case 0x00:
	MDFN_printf(_("CS%d: Unused\n"), i);
	cs[i] = new MCGenjin_CS_Device();
	break;

   case 0x10 ... 0x18:
   case 0x20 ... 0x28:
	MDFN_printf(_("CS%d: %uKiB %sRAM\n"), i, 8 << (cs_di[i] & 0xF), (cs_di[i] & 0x20) ? "Nonvolatile " : "");
	cs[i] = new MCGenjin_CS_Device_RAM(8192 << (cs_di[i] & 0xF), (bool)(cs_di[i] & 0x20));
	break;
  }
 }
}

MCGenjin::~MCGenjin()
{
 for(unsigned i = 0; i < 2; i++)
  delete cs[i];
}

int MCGenjin::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(bank_select),
  SFVAR(dlr),
  SFEND
 };
 int ret = 1;

 ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, "MCGENJIN");

 for(unsigned i = 0; i < 2; i++)
  ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, i ? "MCGENJIN_CS1" : "MCGENJIN_CS0");

 return ret;
}

