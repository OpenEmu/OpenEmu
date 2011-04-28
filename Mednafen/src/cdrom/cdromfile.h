#ifndef __MDFN_CDROMFILE_H
#define __MDFN_CDROMFILE_H

#include <stdio.h>

#include "cdromfile-stuff.h"

enum
{
 CDRF_SUBM_NONE = 0,
 CDRF_SUBM_RW = 1,
 CDRF_SUBM_RW_RAW = 2
};

// Disk-image(rip) track/sector formats
enum
{
 DI_FORMAT_AUDIO       = 0x00,
 DI_FORMAT_MODE1       = 0x01,
 DI_FORMAT_MODE1_RAW   = 0x02,
 DI_FORMAT_MODE2       = 0x03,
 DI_FORMAT_MODE2_FORM1 = 0x04,
 DI_FORMAT_MODE2_FORM2 = 0x05,
 DI_FORMAT_MODE2_RAW   = 0x06,
 _DI_FORMAT_COUNT
};

static const int32 DI_Size_Table[7] =
{
 2352, // Audio
 2048, // MODE1
 2352, // MODE1 RAW
 2336, // MODE2
 2048, // MODE2 Form 1
 2324, // Mode 2 Form 2
 2352
};

static const char *DI_CDRDAO_Strings[7] = 
{
 "AUDIO",
 "MODE1",
 "MODE1_RAW",
 "MODE2",
 "MODE2_FORM1",
 "MODE2_FORM2",
 "MODE2_RAW"
};

static const char *DI_CUE_Strings[7] = 
{
 "AUDIO",
 "MODE1/2048",
 "MODE1/2352",

 // FIXME: These are just guesses:
 "MODE2/2336",
 "MODE2/2048",
 "MODE2/2324",
 "MODE2/2352"
};


struct CDRFile;

CDRFile *cdrfile_open(const char *path);
void cdrfile_destroy(CDRFile *p_cdrfile);

int32 cdrfile_get_track_lba(const CDRFile *p_cdrfile, int32 i_track);
int32 cdrfile_get_num_tracks (const CDRFile *p_cdrfile);

CD_Track_Format_t cdrfile_get_track_format(const CDRFile *p_cdrfile, int32 i_track);

uint32 cdrfile_get_track_sec_count(const CDRFile *p_cdrfile, int32 i_track);


int32 cdrfile_get_first_track_num(const CDRFile *p_cdrfile);

int cdrfile_read_raw_sector(CDRFile *p_cdrfile, uint8 *buf, int32 lba);

uint32_t cdrfile_stat_size (const CDRFile *p_cdrfile);

bool cdrfile_check_subq_checksum(uint8 *SubQBuf);

bool cdrfile_read_toc(const CDRFile *p_cdrfile, CD_TOC *toc);

bool cdrfile_is_physical(const CDRFile *p_cdrfile);

#endif
