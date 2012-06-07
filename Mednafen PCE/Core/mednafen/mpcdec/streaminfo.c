/*
  Copyright (c) 2005-2009, The Musepack Development Team
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.

  * Neither the name of the The Musepack Development Team nor the
  names of its contributors may be used to endorse or promote
  products derived from this software without specific prior
  written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/// \file streaminfo.c
/// Implementation of streaminfo reading functions.

#include <math.h>
#include "mpcdec.h"
#include "streaminfo.h"
#include <stdio.h>
#include "internal.h"
#include "huffman.h"
#include "mpc_bits_reader.h"

unsigned long mpc_crc32(unsigned char *buf, int len);

static const char na[] = "n.a.";
static char const * const versionNames[] = {
    na, "'Unstable/Experimental'", na, na, na, "'quality 0'", "'quality 1'",
    "'Telephone'", "'Thumb'", "'Radio'", "'Standard'", "'Extreme'", "'Insane'",
    "'BrainDead'", "'quality 9'", "'quality 10'"
};
static const mpc_int32_t samplefreqs[8] = { 44100, 48000, 37800, 32000 };

static const char *
mpc_get_version_string(float profile) // profile is 0...15, where 7...13 is used
{
	return profile >= sizeof versionNames / sizeof *versionNames ? na : versionNames[(int)profile];
}

static void
mpc_get_encoder_string(mpc_streaminfo* si)
{
	int ver = si->encoder_version;
	if (si->stream_version >= 8)
		ver = (si->encoder_version >> 24) * 100 + ((si->encoder_version >> 16) & 0xFF);
	if (ver <= 116) {
		if (ver == 0) {
			sprintf(si->encoder, "Buschmann 1.7.0...9, Klemm 0.90...1.05");
		} else {
			switch (ver % 10) {
				case 0:
					sprintf(si->encoder, "Release %u.%u", ver / 100,
							ver / 10 % 10);
					break;
				case 2: case 4: case 6: case 8:
					sprintf(si->encoder, "Beta %u.%02u", ver / 100,
							ver % 100);
					break;
				default:
					sprintf(si->encoder, "--Alpha-- %u.%02u",
							ver / 100, ver % 100);
					break;
			}
		}
	} else {
		int major = si->encoder_version >> 24;
		int minor = (si->encoder_version >> 16) & 0xFF;
		int build = (si->encoder_version >> 8) & 0xFF;
		char * tmp = "--Stable--";

		if (minor & 1)
			tmp = "--Unstable--";

		sprintf(si->encoder, "%s %u.%u.%u", tmp, major, minor, build);
	}
}

static mpc_status check_streaminfo(mpc_streaminfo * si)
{
	if (si->max_band == 0 || si->max_band >= 32
	    || si->channels > 2 || si->channels == 0 || si->sample_freq == 0)
		return MPC_STATUS_FAIL;
	return MPC_STATUS_OK;
}

/// Reads streaminfo from SV7 header.
mpc_status
streaminfo_read_header_sv7(mpc_streaminfo* si, mpc_bits_reader * r)
{
    mpc_uint16_t Estimatedpeak_title = 0;
	mpc_uint32_t frames, last_frame_samples;

	si->bitrate            = 0;
	frames                 = (mpc_bits_read(r, 16) << 16) | mpc_bits_read(r, 16);
	mpc_bits_read(r, 1); // intensity stereo : should be 0
	si->ms                 = mpc_bits_read(r, 1);
	si->max_band           = mpc_bits_read(r, 6);
	si->profile            = mpc_bits_read(r, 4);
	si->profile_name       = mpc_get_version_string(si->profile);
	mpc_bits_read(r, 2); // Link ?
	si->sample_freq        = samplefreqs[mpc_bits_read(r, 2)];
	Estimatedpeak_title    = (mpc_uint16_t) mpc_bits_read(r, 16);   // read the ReplayGain data
	si->gain_title         = (mpc_uint16_t) mpc_bits_read(r, 16);
	si->peak_title         = (mpc_uint16_t) mpc_bits_read(r, 16);
	si->gain_album         = (mpc_uint16_t) mpc_bits_read(r, 16);
	si->peak_album         = (mpc_uint16_t) mpc_bits_read(r, 16);
	si->is_true_gapless    = mpc_bits_read(r, 1); // true gapless: used?
	last_frame_samples     = mpc_bits_read(r, 11); // true gapless: valid samples for last frame
	si->fast_seek          = mpc_bits_read(r, 1); // fast seeking
	mpc_bits_read(r, 19); // unused
	si->encoder_version    = mpc_bits_read(r, 8);
    si->channels           = 2;
	si->block_pwr          = 0;

	// convert gain info
	if (si->gain_title != 0) {
		int tmp = (int)((MPC_OLD_GAIN_REF - (mpc_int16_t)si->gain_title / 100.) * 256. + .5);
		if (tmp >= (1 << 16) || tmp < 0) tmp = 0;
		si->gain_title = (mpc_int16_t) tmp;
	}

	if (si->gain_album != 0) {
		int tmp = (int)((MPC_OLD_GAIN_REF - (mpc_int16_t)si->gain_album / 100.) * 256. + .5);
		if (tmp >= (1 << 16) || tmp < 0) tmp = 0;
		si->gain_album = (mpc_int16_t) tmp;
	}

	if (si->peak_title != 0)
 		si->peak_title = (mpc_uint16_t) (log10(si->peak_title) * 20 * 256 + .5);

	if (si->peak_album != 0)
		si->peak_album = (mpc_uint16_t) (log10(si->peak_album) * 20 * 256 + .5);

	mpc_get_encoder_string(si);

	if (last_frame_samples == 0) last_frame_samples = MPC_FRAME_LENGTH;
	else if (last_frame_samples > MPC_FRAME_LENGTH) return MPC_STATUS_FAIL;
	si->samples = (mpc_int64_t) frames * MPC_FRAME_LENGTH;
	if (si->is_true_gapless)
		si->samples -= (MPC_FRAME_LENGTH - last_frame_samples);
	else
		si->samples -= MPC_DECODER_SYNTH_DELAY;

	si->average_bitrate = (si->tag_offset  - si->header_position) * 8.0
			*  si->sample_freq / si->samples;

	return check_streaminfo(si);
}

/// Reads replay gain datas
void  streaminfo_gain(mpc_streaminfo* si, const mpc_bits_reader * r_in)
{
	mpc_bits_reader r = *r_in;

	int version = mpc_bits_read(&r, 8); // gain version
	if (version != 1) // we only know ver 1
		return;
	si->gain_title         = (mpc_uint16_t) mpc_bits_read(&r, 16);
	si->peak_title         = (mpc_uint16_t) mpc_bits_read(&r, 16);
	si->gain_album         = (mpc_uint16_t) mpc_bits_read(&r, 16);
	si->peak_album         = (mpc_uint16_t) mpc_bits_read(&r, 16);
}

/// Reads streaminfo from SV8 header.
mpc_status
streaminfo_read_header_sv8(mpc_streaminfo* si, const mpc_bits_reader * r_in,
						   mpc_size_t block_size)
{
	mpc_uint32_t CRC;
	mpc_bits_reader r = *r_in;

	CRC = (mpc_bits_read(&r, 16) << 16) | mpc_bits_read(&r, 16);
	if (CRC != mpc_crc32(r.buff + 1 - (r.count >> 3), (int)block_size - 4))
		return MPC_STATUS_FAIL;

	si->stream_version = mpc_bits_read(&r, 8);
	if (si->stream_version != 8)
		return MPC_STATUS_FAIL;

	mpc_bits_get_size(&r, &si->samples);
	mpc_bits_get_size(&r, &si->beg_silence);

	si->is_true_gapless = 1;
	si->sample_freq = samplefreqs[mpc_bits_read(&r, 3)];
	si->max_band = mpc_bits_read(&r, 5) + 1;
	si->channels = mpc_bits_read(&r, 4) + 1;
	si->ms = mpc_bits_read(&r, 1);
	si->block_pwr = mpc_bits_read(&r, 3) * 2;

	si->bitrate = 0;

	if ((si->samples - si->beg_silence) != 0)
		si->average_bitrate = (si->tag_offset - si->header_position) * 8.0
				*  si->sample_freq / (si->samples - si->beg_silence);

	return check_streaminfo(si);
}

/// Reads encoder informations
void  streaminfo_encoder_info(mpc_streaminfo* si, const mpc_bits_reader * r_in)
{
	mpc_bits_reader r = *r_in;

	si->profile            = mpc_bits_read(&r, 7) / 8.;
	si->profile_name       = mpc_get_version_string(si->profile);
	si->pns                = mpc_bits_read(&r, 1);
	si->encoder_version = mpc_bits_read(&r, 8) << 24; // major
	si->encoder_version |= mpc_bits_read(&r, 8) << 16; // minor
	si->encoder_version |= mpc_bits_read(&r, 8) << 8; // build


	mpc_get_encoder_string(si);
}

double
mpc_streaminfo_get_length(mpc_streaminfo * si)
{
	return (double) (si->samples - si->beg_silence) / si->sample_freq;
}

mpc_int64_t mpc_streaminfo_get_length_samples(mpc_streaminfo *si)
{
	return si->samples - si->beg_silence;
}
