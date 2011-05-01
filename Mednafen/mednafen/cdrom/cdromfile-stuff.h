#ifndef __MDFN_CDROMFILE_STUFF_H
#define __MDFN_CDROMFILE_STUFF_H

// Track formats(more abstract and simple)
typedef enum
{
 CD_TRACK_FORMAT_AUDIO = 0x00,
 CD_TRACK_FORMAT_DATA = 0x01,
 //CDRF_FORMAT_CDI = 0x02
} CD_Track_Format_t;

enum
{
 ADR_NOQINFO = 0x00,
 ADR_CURPOS  = 0x01,
 ADR_MCN     = 0x02,
 ADR_ISRC    = 0x03
};

typedef struct
{
 uint8 adr;
 uint8 control;
 uint32 lba;
} CD_TOC_Entry;

enum
{
 DISC_TYPE_CDDA_OR_M1 = 0x00,
 DISC_TYPE_CD_I       = 0x10,
 DISC_TYPE_CD_XA      = 0x20
};

typedef struct
{
 uint8 first_track;
 uint8 last_track;
 uint8 disc_type;
 CD_TOC_Entry tracks[100 + 1];  // [0] is unused, [100] is for the leadout track.
				// Also, for convenience, tracks[last_track + 1] will always refer
				// to the leadout track(even if last_track < 99, IE the leadout track details are duplicated).
} CD_TOC;

#endif
