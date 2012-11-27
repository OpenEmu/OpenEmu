/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef	SWORD2_OBJECT_H
#define	SWORD2_OBJECT_H

#include "common/endian.h"

namespace Sword2 {

// these structures represent the broken up compact components
// these here declared to the system must be the same as those declared to
// LINC (or it wont work)

// mouse structure - defines mouse detection area, detection priority &
// 'type' flag

struct ObjectMouse {
	int32 x1;			// Top-left and bottom-right of mouse
	int32 y1;			// area. (These coords are inclusive.)
	int32 x2;
	int32 y2;
	int32 priority;
	int32 pointer;			// type (or resource id?) of pointer used over this area

	static int size() {
		return 24;
	}

	void read(byte *addr);
	void write(byte *addr);
};

// logic structure - contains fields used in logic script processing

class ObjectLogic {
	// int32 looping;		// 0 when first calling fn<function>;
					// 1 when calling subsequent times in
					// same loop
	// int32 pause;			// pause count, used by fnPause()

private:
	byte *_addr;

public:
	ObjectLogic(byte *addr) {
		_addr = addr;
	}

	static int size() {
		return 8;
	}

	byte *data() {
		return _addr;
	}

	int32 getLooping()       { return READ_LE_UINT32(_addr);     }
	int32 getPause()         { return READ_LE_UINT32(_addr + 4); }

	void setLooping(int32 x) { WRITE_LE_UINT32(_addr, x);        }
	void setPause(int32 x)   { WRITE_LE_UINT32(_addr + 4, x);    }
};

// status bits for 'type' field of ObjectGraphic)

// in low word:

#define	NO_SPRITE	0x00000000	// don't print
#define	BGP0_SPRITE	0x00000001	// fixed to background parallax[0]
#define	BGP1_SPRITE	0x00000002	// fixed to background parallax[1]
#define	BACK_SPRITE	0x00000004	// 'background' sprite, fixed to main background
#define	SORT_SPRITE	0x00000008	// 'sorted' sprite, fixed to main background
#define	FORE_SPRITE	0x00000010	// 'foreground' sprite, fixed to main background
#define	FGP0_SPRITE	0x00000020	// fixed to foreground parallax[0]
#define	FGP1_SPRITE	0x00000040	// fixed to foreground parallax[0]

// in high word:

#define UNSHADED_SPRITE	0x00000000	// not to be shaded
#define SHADED_SPRITE	0x00010000	// to be shaded, based on shading mask

// graphic structure - contains fields appropriate to sprite output

class ObjectGraphic {
	// int32 type;			// see above
	// int32 anim_resource;		// resource id of animation file
	// int32 anim_pc;		// current frame number of animation

private:
	byte *_addr;

public:
	ObjectGraphic(byte *addr) {
		_addr = addr;
	}

	static int size() {
		return 12;
	}

	byte *data() {
		return _addr;
	}

	int32 getType()               { return READ_LE_UINT32(_addr);     }
	int32 getAnimResource()       { return READ_LE_UINT32(_addr + 4); }
	int32 getAnimPc()             { return READ_LE_UINT32(_addr + 8); }

	void setType(int32 x)         { WRITE_LE_UINT32(_addr, x);        }
	void setAnimResource(int32 x) { WRITE_LE_UINT32(_addr + 4, x);    }
	void setAnimPc(int32 x)       { WRITE_LE_UINT32(_addr + 8, x);    }
};

// speech structure - contains fields used by speech scripts & text output

class ObjectSpeech {
	// int32 pen;			// color to use for body of characters
	// int32 width;			// max width of text sprite
	// int32 command;		// speech script command id
	// int32 ins1;			// speech script instruction parameters
	// int32 ins2;
	// int32 ins3;
	// int32 ins4;
	// int32 ins5;
	// int32 wait_state;		// 0 not waiting,
					// 1 waiting for next speech command

private:
	byte *_addr;

public:
	ObjectSpeech(byte *addr) {
		_addr = addr;
	}

	static int size() {
		return 36;
	}

	byte *data() {
		return _addr;
	}

	int32 getPen()             { return READ_LE_UINT32(_addr);      }
	int32 getWidth()           { return READ_LE_UINT32(_addr + 4);  }
	int32 getCommand()         { return READ_LE_UINT32(_addr + 8);  }
	int32 getIns1()            { return READ_LE_UINT32(_addr + 12); }
	int32 getIns2()            { return READ_LE_UINT32(_addr + 16); }
	int32 getIns3()            { return READ_LE_UINT32(_addr + 20); }
	int32 getIns4()            { return READ_LE_UINT32(_addr + 24); }
	int32 getIns5()            { return READ_LE_UINT32(_addr + 28); }
	int32 getWaitState()       { return READ_LE_UINT32(_addr + 32); }

	void setPen(int32 x)       { WRITE_LE_UINT32(_addr, x);         }
	void setWidth(int32 x)     { WRITE_LE_UINT32(_addr + 4, x);     }
	void setCommand(int32 x)   { WRITE_LE_UINT32(_addr + 8, x);     }
	void setIns1(int32 x)      { WRITE_LE_UINT32(_addr + 12, x);    }
	void setIns2(int32 x)      { WRITE_LE_UINT32(_addr + 16, x);    }
	void setIns3(int32 x)      { WRITE_LE_UINT32(_addr + 20, x);    }
	void setIns4(int32 x)      { WRITE_LE_UINT32(_addr + 24, x);    }
	void setIns5(int32 x)      { WRITE_LE_UINT32(_addr + 28, x);    }
	void setWaitState(int32 x) { WRITE_LE_UINT32(_addr + 32, x);    }
};

// mega structure - contains fields used for mega-character & mega-set
// processing

class ObjectMega {
	// int32 NOT_USED_1;		// only free roaming megas need to
					// check this before registering their
					// graphics for drawing
	// int32 NOT_USED_2;		// id of floor on which we are standing
	// int32 NOT_USED_3;		// id of object which we are getting to
	// int32 NOT_USED_4;		// pixel distance to stand from player
					// character when in conversation
	// int32 currently_walking;	// number given us by the auto router
	// int32 walk_pc;		// current frame number of walk-anim
	// int32 scale_a;		// current scale factors, taken from
	// int32 scale_b;		// floor data
	// int32 feet_x;		// mega feet coords - frame-offsets are
	// int32 feet_y;		// added to these position mega frames
	// int32 current_dir;		// current dirction faced by mega; used
					// by autorouter to determine turns
					// required
	// int32 NOT_USED_5;		// means were currently avoiding a
					// collision (see fnWalk)
	// int32 megaset_res;		// resource id of mega-set file
	// int32 NOT_USED_6;		// NOT USED

private:
	byte *_addr;

public:
	ObjectMega(byte *addr) {
		_addr = addr;
	}

	static int size() {
		return 56;
	}

	byte *data() {
		return _addr;
	}

	int32 getIsWalking()         { return READ_LE_UINT32(_addr + 16); }
	int32 getWalkPc()            { return READ_LE_UINT32(_addr + 20); }
	int32 getScaleA()            { return READ_LE_UINT32(_addr + 24); }
	int32 getScaleB()            { return READ_LE_UINT32(_addr + 28); }
	int32 getFeetX()             { return READ_LE_UINT32(_addr + 32); }
	int32 getFeetY()             { return READ_LE_UINT32(_addr + 36); }
	int32 getCurDir()            { return READ_LE_UINT32(_addr + 40); }
	int32 getMegasetRes()        { return READ_LE_UINT32(_addr + 48); }

	void setIsWalking(int32 x)   { WRITE_LE_UINT32(_addr + 16, x);    }
	void setWalkPc(int32 x)      { WRITE_LE_UINT32(_addr + 20, x);    }
	void setScaleA(int32 x)      { WRITE_LE_UINT32(_addr + 24, x);    }
	void setScaleB(int32 x)      { WRITE_LE_UINT32(_addr + 28, x);    }
	void setFeetX(int32 x)       { WRITE_LE_UINT32(_addr + 32, x);    }
	void setFeetY(int32 x)       { WRITE_LE_UINT32(_addr + 36, x);    }
	void setCurDir(int32 x)      { WRITE_LE_UINT32(_addr + 40, x);    }
	void setMegasetRes(int32 x)  { WRITE_LE_UINT32(_addr + 48, x);    }

	int32 calcScale() {
		// Calc scale at which to print the sprite, based on feet
		// y-coord & scaling constants (NB. 'scale' is actually
		// 256 * true_scale, to maintain accuracy)

		// Ay+B gives 256 * scale ie. 256 * 256 * true_scale for even
		// better accuracy, ie. scale = (Ay + B) / 256
		return (getScaleA() * getFeetY() + getScaleB()) / 256;
	}
};

// walk-data structure - contains details of layout of frames in the
// mega-set, and how they are to be used

struct ObjectWalkdata {
	int32 nWalkFrames;		// no. of frames per walk-cycle
	int32 usingStandingTurnFrames;	// 0 = no  1 = yes
	int32 usingWalkingTurnFrames;	// 0 = no  1 = yes
	int32 usingSlowInFrames;	// 0 = no  1 = yes
	int32 usingSlowOutFrames;	// 0 = no  !0 = number of slow-out frames in each direction
	int32 nSlowInFrames[8];		// no. of slow-in frames in each direction
	int32 leadingLeg[8];		// leading leg for walk	in each direction  (0 = left  1 = right)
	int32 dx[8 * (12 + 1)];		// walk step distances in x direction
	int32 dy[8 * (12 + 1)];		// walk step distances in y direction

	static int size() {
		return 916;
	}

	void read(byte *addr);
	void write(byte *addr);
};

} // End of namespace Sword2

#endif
