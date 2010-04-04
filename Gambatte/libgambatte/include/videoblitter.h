/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef GAMBATTE_VIDEOBLITTER_H
#define GAMBATTE_VIDEOBLITTER_H

namespace Gambatte {

struct PixelBuffer {
	enum Format { RGB32, RGB16, UYVY };
	
	void *pixels;
	Format format;
	unsigned pitch;
	
	PixelBuffer() : pixels(0), format(RGB32), pitch(0) {}
};

class VideoBlitter {
public:
	virtual void setBufferDimensions(unsigned width, unsigned height) = 0;
	virtual const PixelBuffer inBuffer() = 0;
	virtual void blit() = 0;
	virtual ~VideoBlitter() {}
};

}

#endif
