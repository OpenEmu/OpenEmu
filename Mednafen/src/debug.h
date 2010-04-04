#ifndef __MDFN_DEBUG_H
#define __MDFN_DEBUG_H

#ifdef WANT_DEBUGGER
void MDFNDBG_AddASpace(void (*gasb)(const char *name, uint32 Address, uint32 Length, uint8 *Buffer),
        void (*pasb)(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer), const char *name, const char *long_name,
        uint32 TotalBits, bool IsSegmented = 0, uint32 SegmentBits = 0, uint32 OffsetBits = 0, uint32 BitsOverlapped = 0);

void MDFNDBG_ResetRegGroupsInfo(void);
void MDFNDBG_AddRegGroup(RegGroupType *groupie);
void MDFNDBG_ResetASpaceInfo(void);


void MDFNDBG_Init(void);
void MDFNDBG_PostGameLoad(void);
void MDFNDBG_Kill(void);

#endif

#endif
