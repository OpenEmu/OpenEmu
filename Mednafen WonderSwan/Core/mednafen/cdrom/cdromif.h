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

#ifndef __MDFN_CDROM_CDROMIF_H
#define __MDFN_CDROM_CDROMIF_H

#include "CDUtility.h"

#include <queue>

typedef CDUtility::TOC CD_TOC;

enum
{
 // Status/Error messages
 CDIF_MSG_DONE = 0,		// Read -> emu. args: No args.
 CDIF_MSG_INFO,			// Read -> emu. args: str_message
 CDIF_MSG_FATAL_ERROR,		// Read -> emu. args: *TODO ARGS*

 //
 // Command messages.
 //
 CDIF_MSG_DIEDIEDIE,		// Emu -> read

 CDIF_MSG_READ_SECTOR,		/* Emu -> read
					args[0] = lba
				*/

 CDIF_MSG_EJECT,		// Emu -> read, args[0]; 0=insert, 1=eject
};

class CDIF_Message
{
 public:

 CDIF_Message();
 CDIF_Message(unsigned int message_, uint32 arg0 = 0, uint32 arg1 = 0, uint32 arg2 = 0, uint32 arg3 = 0);
 CDIF_Message(unsigned int message_, const std::string &str);
 ~CDIF_Message();

 unsigned int message;
 uint32 args[4];
 void *parg;
 std::string str_message;
};

class CDIF_Queue
{
 public:

 CDIF_Queue();
 ~CDIF_Queue();

 bool Read(CDIF_Message *message, bool blocking = TRUE);

 void Write(const CDIF_Message &message);

 private:
 std::queue<CDIF_Message> ze_queue;
 MDFN_Mutex *ze_mutex;
};


typedef struct
{
 bool valid;
 bool error;
 uint32 lba;
 uint8 data[2352 + 96];
} CDIF_Sector_Buffer;

class CDAccess;

// TODO: prohibit copy constructor
class CDIF
{
 public:

 CDIF(const char *device_name);
 ~CDIF();

 void ReadTOC(CDUtility::TOC *read_target);

 void HintReadSector(uint32 lba);
 bool ReadRawSector(uint8 *buf, uint32 lba);

 // Call for mode 1 or mode 2 form 1 only.
 bool ValidateRawSector(uint8 *buf);

 // Utility/Wrapped functions
 // Reads mode 1 and mode2 form 1 sectors(2048 bytes per sector returned)
 // Will return the type(1, 2) of the first sector read to the buffer supplied, 0 on error
 int ReadSector(uint8* pBuf, uint32 lba, uint32 nSectors);

 // Return true if operation succeeded or it was a NOP(either due to not being implemented, or the current status matches eject_status).
 // Returns false on failure(usually drive error of some kind; not completely fatal, can try again).
 bool Eject(bool eject_status);

 inline bool IsPhysical(void) { return is_phys_cache; }

 // FIXME: Semi-private:
 int ReadThreadStart(const char *device_name);

 private:

 bool is_phys_cache;
 CDUtility::TOC disc_toc;
 CDAccess *disc_cdaccess;
 MDFN_Thread *CDReadThread;

 // Queue for messages to the read thread.
 CDIF_Queue ReadThreadQueue;

 // Queue for messages to the emu thread.
 CDIF_Queue EmuThreadQueue;


 enum { SBSize = 256 };
 CDIF_Sector_Buffer SectorBuffers[SBSize];

 uint32 SBWritePos;
 
 MDFN_Mutex *SBMutex;
 bool UnrecoverableError;


 //
 // Read-thread-only:
 //
 void RT_EjectDisc(bool eject_status, bool skip_actual_eject = false);

 uint32 ra_lba;
 int ra_count;
 uint32 last_read_lba;
 bool DiscEjected;
};

#endif
