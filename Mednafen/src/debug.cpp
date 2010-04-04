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

void MDFNDBG_AddASpace(void (*gasb)(const char *name, uint32 Address, uint32 Length, uint8 *Buffer),
        void (*pasb)(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer), const char *name, const char *long_name,
        uint32 TotalBits, bool IsSegmented, uint32 SegmentBits, uint32 OffsetBits, uint32 BitsOverlapped)
{
 AddressSpaceType newt;

 memset(&newt, 0, sizeof(newt));

 newt.GetAddressSpaceBytes = gasb;
 newt.PutAddressSpaceBytes = pasb;

 newt.name = strdup(name);
 newt.long_name = strdup(long_name);
 newt.TotalBits = TotalBits;
 newt.IsSegmented = IsSegmented;
 newt.SegmentBits = SegmentBits;
 newt.OffsetBits = OffsetBits;
 newt.BitsOverlapped = BitsOverlapped;
 AddressSpaces.push_back(newt);
}

void MDFNDBG_ResetRegGroupsInfo(void)
{
 RegGroups.clear();
}

void MDFNDBG_AddRegGroup(RegGroupType *groupie)
{
 RegGroups.push_back(groupie);
}

void MDFNDBG_ResetASpaceInfo(void)
{
 AddressSpaces.clear();
}

#endif
