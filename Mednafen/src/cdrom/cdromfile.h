#ifndef __MDFN_CDROMFILE_H
#define __MDFN_CDROMFILE_H

#include "../tremor/ivorbisfile.h"
#include <mpcdec/mpcdec.h>
#include <sndfile.h>
#include <stdio.h>

#define CACHE_START_SECTORS	2

enum
{
 CDRF_SUBM_NONE = 0,
 CDRF_SUBM_RW = 1,
 CDRF_SUBM_RW_RAW = 2
};

typedef struct __CDRFILE_TRACK_INFO
{
        lsn_t LSN;
        track_format_t Format;
	bool IsData2352;
        lba_t pregap;
	lba_t index;
	lba_t sectors;	// Not including pregap sectors!
        FILE *fp;
	bool FirstFileInstance;
	bool RawAudioMSBFirst;
	long FileOffset;
	unsigned int SubchannelMode;

        SNDFILE *sf;
	SF_INFO sfinfo;

        OggVorbis_File *ovfile;

	mpc_decoder *MPCDecoder;
	mpc_streaminfo *MPCStreamInfo;
	mpc_reader_file *MPCReaderFile;


	MPC_SAMPLE_FORMAT *MPCBuffer; 
	uint32 MPCBufferIn;
	uint32 MPCBufferOffs;
	uint32 LastSamplePos;

	uint8 StartCache[2352 * CACHE_START_SECTORS];
} CDRFILE_TRACK_INFO;

typedef struct __CDRFile
{
        track_t NumTracks;
        track_t FirstTrack;
	lba_t total_sectors;
        CDRFILE_TRACK_INFO Tracks[100]; // Track #0(HMM?) through 99
	CdIo *p_cdio;
} CDRFile;

CDRFile *cdrfile_open(const char *path);
void cdrfile_destroy(CDRFile *p_cdrfile);
lba_t cdrfile_get_track_lsn(const CDRFile *p_cdrfile, track_t i_track);
int cdrfile_read_audio_sector(CDRFile *p_cdrfile, void *buf, uint8 *SubPWBuf, lsn_t lsn);
track_t cdrfile_get_num_tracks (const CDRFile *p_cdrfile);
track_format_t cdrfile_get_track_format(const CDRFile *p_cdrfile, track_t i_track);
unsigned int cdrfile_get_track_sec_count(const CDRFile *p_cdrfile, track_t i_track);


track_t cdrfile_get_first_track_num(const CDRFile *p_cdrfile);
int cdrfile_read_mode1_sectors (const CDRFile *p_cdrfile, void *buf, uint8 *SubPWBuf, lsn_t lsn, bool b_form2, unsigned int i_sectors);
uint32_t cdrfile_stat_size (const CDRFile *p_cdrfile);

bool cdrfile_check_subq_checksum(uint8 *SubQBuf);

#endif
