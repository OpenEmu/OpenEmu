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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_GAME_H
#define HUGO_GAME_H

#include "common/keyboard.h"

namespace Common {
class WriteStream;
class SeekableReadStream;
}

namespace Hugo {

// Game specific equates
#define TAKE_TEXT   "Picked up the %s ok."

enum {LOOK_NAME = 1, TAKE_NAME};                    // Index of name used in showing takeables and in confirming take

// Definitions of 'generic' commands: Max # depends on size of gencmd in
// the Object record since each requires 1 bit.  Currently up to 16
enum {LOOK = 1, TAKE = 2, DROP = 4, LOOK_S = 8};

enum TEXTCOLORS {
	_TBLACK,    _TBLUE,         _TGREEN,       _TCYAN,
	_TRED,      _TMAGENTA,      _TBROWN,       _TWHITE,
	_TGRAY,     _TLIGHTBLUE,    _TLIGHTGREEN,  _TLIGHTCYAN,
	_TLIGHTRED, _TLIGHTMAGENTA, _TLIGHTYELLOW, _TBRIGHTWHITE
};

enum Uif {U_FONT5, U_FONT6, U_FONT8, UIF_IMAGES, NUM_UIF_ITEMS};
static const int kFirstFont = U_FONT5;

/**
 * Enumerate ways of cycling a sequence of frames
 */
enum Cycle {kCycleInvisible, kCycleAlmostInvisible, kCycleNotCycling, kCycleForward, kCycleBackward};

/**
 * Enumerate sequence index matching direction of travel
 */
enum {SEQ_RIGHT, SEQ_LEFT, SEQ_DOWN, SEQ_UP};

enum Font {LARGE_ROMAN, MED_ROMAN, NUM_GDI_FONTS, INIT_FONTS, DEL_FONTS};

/**
 * Enumerate the different path types for an object
 */
enum Path {
	kPathUser,                                      // User has control of object via cursor keys
	kPathAuto,                                      // Computer has control, controlled by action lists
	kPathQuiet,                                     // Computer has control and no commands allowed
	kPathChase,                                     // Computer has control, object is chasing hero
	kPathChase2,                                    // Same as CHASE, except keeps cycling when stationary
	kPathWander,                                    // Computer has control, object is wandering randomly
	kPathWander2                                    // Same as WANDER, except keeps cycling when stationary
};

struct hugoBoot {                                   // Common HUGO boot file
	char _checksum;                                 // Checksum for boot structure (not exit text)
	char _registered;                               // TRUE if registered version, else FALSE
	char _pbswitch[8];                              // Playback switch string
	char _distrib[32];                              // Distributor branding string
	uint16 _exitLen;                                // Length of exit text (next in file)
} PACKED_STRUCT;

/**
 * Game specific type definitions
 */
typedef byte *ImagePtr;                             // ptr to an object image (sprite)
typedef byte *SoundPtr;                             // ptr to sound (or music) data

/**
 * Structure for initializing maze processing
 */
struct Maze {
	bool _enabledFl;                                // TRUE when maze processing enabled
	byte _size;                                     // Size of (square) maze matrix
	int  _x1, _y1, _x2, _y2;                        // maze hotspot bounding box
	int  _x3, _x4;                                  // north, south x entry coordinates
	byte _firstScreenIndex;                         // index of first screen in maze
};

/**
 * The following is a linked list of images in an animation sequence
 * The image data is in 8-bit DIB format, i.e. 1 byte = 1 pixel
 */
struct Seq {                                        // Linked list of images
	byte   *_imagePtr;                              // ptr to image
	uint16  _bytesPerLine8;                         // bytes per line (8bits)
	uint16  _lines;                                 // lines
	uint16  _x1, _x2, _y1, _y2;                     // Offsets from x,y: data bounding box
	Seq  *_nextSeqPtr;                              // ptr to next record
};

/**
 * The following is an array of structures of above sequences
 */
struct SeqList {
	uint16 _imageNbr;                               // Number of images in sequence
	Seq *_seqPtr;                                   // Ptr to sequence structure
};

#include "common/pack-start.h"                      // START STRUCT PACKING
struct SoundHdr {                                   // Sound file lookup entry
	uint16 _size;                                   // Size of sound data in bytes
	uint32 _offset;                                 // Offset of sound data in file
} PACKED_STRUCT;
#include "common/pack-end.h"                        // END STRUCT PACKING

static const int kMaxSeqNumb = 4;                   // Number of sequences of images in object

/**
 * Following is definition of object attributes
 */
struct Object {
	uint16     _nounIndex;                           // String identifying object
	uint16     _dataIndex;                           // String describing the object
	uint16    *_stateDataIndex;                      // Added by Strangerke to handle the LOOK_S state-dependant descriptions
	Path       _pathType;                            // Describe path object follows
	int        _vxPath, _vyPath;                     // Delta velocities (e.g. for CHASE)
	uint16     _actIndex;                            // Action list to do on collision with hero
	byte       _seqNumb;                             // Number of sequences in list
	Seq       *_currImagePtr;                        // Sequence image currently in use
	SeqList    _seqList[kMaxSeqNumb];                // Array of sequence structure ptrs and lengths
	Cycle      _cycling;                             // Whether cycling, forward or backward
	byte       _cycleNumb;                           // No. of times to cycle
	byte       _frameInterval;                       // Interval (in ticks) between frames
	byte       _frameTimer;                          // Decrementing timer for above
	int8       _radius;                              // Defines sphere of influence by hero
	byte       _screenIndex;                         // Screen in which object resides
	int        _x, _y;                               // Current coordinates of object
	int        _oldx, _oldy;                         // Previous coordinates of object
	int8       _vx, _vy;                             // Velocity
	byte       _objValue;                            // Value of object
	int        _genericCmd;                          // Bit mask of 'generic' commands for object
	uint16     _cmdIndex;                            // ptr to list of cmd structures for verbs
	bool       _carriedFl;                           // TRUE if object being carried
	byte       _state;                               // state referenced in cmd list
	bool       _verbOnlyFl;                          // TRUE if verb-only cmds allowed e.g. sit,look
	byte       _priority;                            // Whether object fore, background or floating
	int16      _viewx, _viewy;                       // Position to view object from (or 0 or -1)
	int16      _direction;                           // Direction to view object from
	byte       _curSeqNum;                           // Save which seq number currently in use
	byte       _curImageNum;                         // Save which image of sequence currently in use
	int8       _oldvx;                               // Previous vx (used in wandering)
	int8       _oldvy;                               // Previous vy
};
} // End of namespace Hugo

#endif
