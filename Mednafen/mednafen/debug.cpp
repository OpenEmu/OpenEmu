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

/* Support functions for the emulated system code-side of the debugger. */
#include "mednafen.h"

#ifdef WANT_DEBUGGER

static std::vector<AddressSpaceType> AddressSpaces;
static std::vector<RegGroupType*> RegGroups;

// Currently only called on emulator startup, not game load...
void MDFNDBG_Init(void)
{
 AddressSpaces.clear();
 RegGroups.clear();
}

void MDFNDBG_PostGameLoad(void)
{
 if(MDFNGameInfo->Debugger)
 {
  MDFNGameInfo->Debugger->AddressSpaces = &AddressSpaces;
  MDFNGameInfo->Debugger->RegGroups = &RegGroups;
 }
}

// Called on game close.
void MDFNDBG_Kill(void)
{
 for(unsigned int x = 0; x < AddressSpaces.size(); x++)
 {
  if(AddressSpaces[x].name)
   free(AddressSpaces[x].name);
  if(AddressSpaces[x].long_name)
   free(AddressSpaces[x].long_name);
 }
 AddressSpaces.clear();
 RegGroups.clear();
}

int ASpace_Add(void (*gasb)(const char *name, uint32 Address, uint32 Length, uint8 *Buffer),
        void (*pasb)(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer), const char *name, const char *long_name,
        uint32 TotalBits, uint32 NP2Size, bool IsSegmented, uint32 SegmentBits, uint32 OffsetBits, uint32 BitsOverlapped)
{
 AddressSpaceType newt;

 memset(&newt, 0, sizeof(newt));

 newt.GetAddressSpaceBytes = gasb;
 newt.PutAddressSpaceBytes = pasb;

 newt.name = strdup(name);
 newt.long_name = strdup(long_name);
 newt.TotalBits = TotalBits;
 newt.NP2Size = NP2Size;
 newt.IsSegmented = IsSegmented;
 newt.SegmentBits = SegmentBits;
 newt.OffsetBits = OffsetBits;
 newt.BitsOverlapped = BitsOverlapped;
 AddressSpaces.push_back(newt);

 return(AddressSpaces.size() - 1);
}

int ASpace_Add(const AddressSpaceType &newt)
{
 AddressSpaces.push_back(newt);
 return(AddressSpaces.size() - 1);
}

// Returns number of new bytes allocated.
static INLINE uint64 IncUsageMap(uint64 *****UsageMap, const uint32 address)
{
 uint64 ret = 0;

 if(!(*UsageMap))
 {
  (*UsageMap) = (uint64 ****)MDFN_calloc(sizeof(uint64 ***), 256, "Usage map");
  ret += sizeof(uint64 ***) * 256;
 }

 if(!(*UsageMap)[address >> 24])
 {
  (*UsageMap)[address >> 24] = (uint64 ***)MDFN_calloc(sizeof(uint64 **), 256, "Usage map");
  ret += sizeof(uint64 **) * 256;
 }

 if(!(*UsageMap)[address >> 24][(address >> 16) & 0xFF])
 {
  (*UsageMap)[address >> 24][(address >> 16) & 0xFF] = (uint64 **)MDFN_calloc(sizeof(uint64 *), 256, "Usage map");
  ret += sizeof(uint64 *) * 256;
 }

 if(!(*UsageMap)[address >> 24][(address >> 16) & 0xFF][(address >> 8) & 0xFF])
 {
  (*UsageMap)[address >> 24][(address >> 16) & 0xFF][(address >> 8) & 0xFF] = (uint64 *)MDFN_calloc(sizeof(uint64), 256, "Usage map");
  ret += sizeof(uint64) * 256;
 }

 (*UsageMap)[address >> 24][(address >> 16) & 0xFF][(address >> 8) & 0xFF][address & 0xFF]++;

 return(ret);
}

bool ASpace_Read(const int id, const uint32 address, const unsigned int size, const bool pre_bpoint)
{
 AddressSpaceType *as;

 assert(id > 0 && (const unsigned int)id < AddressSpaces.size());

 as = &AddressSpaces[id];

 if(pre_bpoint)
  return(FALSE);
 else
 {
  as->UsageReadMemUsed += IncUsageMap(&as->UsageMapRead, address);
 }
 return(FALSE);
}

bool ASpace_Write(const int id, const uint32 address, const uint32 value, const unsigned int size, const bool pre_bpoint)
{
 AddressSpaceType *as;

 assert(id > 0 && (const unsigned int)id < AddressSpaces.size());

 as = &AddressSpaces[id];

 if(pre_bpoint)
  return(FALSE);
 else
 {
  as->UsageWriteMemUsed += IncUsageMap(&as->UsageMapWrite, address);
  return(FALSE);
 }
}

static INLINE void ClearUsageMap(uint64 *****UsageMap)
{
 // Maybe we should use an alloced memory vector to speed this up...
 if((*UsageMap))
 {
  for(int a = 0; a < 256; a++)
  {
   if((*UsageMap)[a])
   {   
    for(int b = 0; b < 256; b++)
    {
     if((*UsageMap)[a][b])
     {
      for(int c = 0; c < 256; c++)
      {
       if((*UsageMap)[a][b][c])
       {
        MDFN_free((*UsageMap)[a][b][c]);
        (*UsageMap)[a][b][c] = NULL;
       }
      }
      MDFN_free((*UsageMap)[a][b]);
      (*UsageMap)[a][b] = NULL;
     }
    }

    MDFN_free((*UsageMap)[a]);
    (*UsageMap)[a] = NULL;
   }
  }

  MDFN_free((*UsageMap));
  (*UsageMap) = NULL;
 }
}

void ASpace_ClearReadMap(const int id)
{
 AddressSpaceType *as;

 assert(id > 0 && (const unsigned int)id < AddressSpaces.size());

 as = &AddressSpaces[id];

 ClearUsageMap(&as->UsageMapRead);

 as->UsageReadMemUsed = 0;
}

void ASpace_ClearWriteMap(const int id)
{
 AddressSpaceType *as;

 assert(id > 0 && (const unsigned int)id < AddressSpaces.size());

 as = &AddressSpaces[id];

 ClearUsageMap(&as->UsageMapWrite);

 as->UsageWriteMemUsed = 0;
}

void MDFNDBG_ResetRegGroupsInfo(void)
{
 RegGroups.clear();
}

void MDFNDBG_AddRegGroup(RegGroupType *groupie)
{
 RegGroups.push_back(groupie);
}

void ASpace_Reset(void)
{
 AddressSpaces.clear();
}


RegType::~RegType()
{

}

#endif
