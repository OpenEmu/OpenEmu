/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MOHAWK_VIDEO_H
#define MOHAWK_VIDEO_H

#include "common/array.h"
#include "graphics/pixelformat.h"
#include "video/video_decoder.h"

namespace Mohawk {

class MohawkEngine;

struct MLSTRecord {
	uint16 index;
	uint16 movieID;
	uint16 code;
	uint16 left;
	uint16 top;
	uint16 u0[3];
	uint16 loop;
	uint16 volume;
	uint16 u1;
};

struct VideoEntry {
	// Playback variables
	Video::VideoDecoder *video;
	uint16 x;
	uint16 y;
	bool loop;
	bool enabled;
	Audio::Timestamp start;

	// Identification
	Common::String filename; // External video files
	int id;                  // Internal Mohawk files

	// Helper functions
	Video::VideoDecoder *operator->() const { assert(video); return video; } // TODO: Remove this eventually
	void clear();
	bool endOfVideo();
};

typedef int32 VideoHandle;

enum {
	NULL_VID_HANDLE = -1
};

class VideoManager {
public:
	VideoManager(MohawkEngine *vm);
	~VideoManager();

	// Generic movie functions
	void playMovieBlocking(const Common::String &filename, uint16 x = 0, uint16 y = 0, bool clearScreen = false);
	void playMovieBlockingCentered(const Common::String &filename, bool clearScreen = true);
	VideoHandle playMovie(const Common::String &filename, int16 x = -1, int16 y = -1, bool loop = false);
	VideoHandle playMovie(uint16 id, int16 x = -1, int16 y = -1, bool loop = false);
	bool updateMovies();
	void pauseVideos();
	void resumeVideos();
	void stopVideos();
	bool isVideoPlaying();

	// Riven-related functions
	void activateMLST(uint16 mlstId, uint16 card);
	void clearMLST();
	void enableMovieRiven(uint16 id);
	void disableMovieRiven(uint16 id);
	void disableAllMovies();
	VideoHandle playMovieRiven(uint16 id);
	void stopMovieRiven(uint16 id);
	void playMovieBlockingRiven(uint16 id);
	VideoHandle findVideoHandleRiven(uint16 id);

	// Handle functions
	VideoHandle findVideoHandle(uint16 id);
	VideoHandle findVideoHandle(const Common::String &filename);
	int getCurFrame(VideoHandle handle);
	uint32 getFrameCount(VideoHandle handle);
	uint32 getTime(VideoHandle handle);
	uint32 getDuration(VideoHandle videoHandle);
	bool endOfVideo(VideoHandle handle);
	void setVideoBounds(VideoHandle handle, Audio::Timestamp start, Audio::Timestamp end);
	void drawVideoFrame(VideoHandle handle, Audio::Timestamp time);
	void seekToTime(VideoHandle handle, Audio::Timestamp time);
	void setVideoLooping(VideoHandle handle, bool loop);
	void waitUntilMovieEnds(VideoHandle videoHandle);
	void delayUntilMovieEnds(VideoHandle videoHandle);
	void pauseMovie(VideoHandle videoHandle, bool pause);

private:
	MohawkEngine *_vm;

	// Riven-related variables
	Common::Array<MLSTRecord> _mlstRecords;

	// Keep tabs on any videos playing
	Common::Array<VideoEntry> _videoStreams;

	VideoHandle createVideoHandle(uint16 id, uint16 x, uint16 y, bool loop, byte volume = 0xff);
	VideoHandle createVideoHandle(const Common::String &filename, uint16 x, uint16 y, bool loop, byte volume = 0xff);
};

} // End of namespace Mohawk

#endif
