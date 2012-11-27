/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_PANORAMA_H
#define PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_PANORAMA_H

#include "pegasus/movie.h"

namespace Pegasus {

/*

	Panorama implements a wide image using a specially constructed movie file.
	The movie holds the image as a series of vertical strips, say 16 or 32 pixels wide.

	The panorama bounds defines the entire panorama. The view bounds represents the
	area on the panorama that is kept in memory.

	The panorama bounds is also stored in the movie file; it cannot be changed. The
	view bounds must always be a subset of the panorama bounds.

	In actuality, the area kept in memory is at least as wide as the view bounds (but
	may be wider to coincide with the width of the movies slices), and is as tall as
	the panorama bounds. The view bounds is used by the drawPanorama function to draw
	a piece of the panorama to the current screen.

	The panorama movie is built at a time scale of 1, with each strip lasting for one
	second, so that strip number corresponds exactly with the time value at which the
	strip is stored.

	TO USE:

	Call one initFromMovieFile to open the movie. Then set up a view rect by
	calling setViewBounds. Once these two functions have been called, drawPanorama
	will draw the panorama.

*/

class Panorama {
public:
	Panorama();
	virtual ~Panorama();

	void initFromMovieFile(const Common::String &);
	void releasePanorama();
	bool isPanoramaOpen() { return _panoramaMovie.isMovieValid(); }

	void setViewBounds(const Common::Rect &);
	void getViewBounds(Common::Rect &) const;

	void setMask(Surface *);

	void getPanoramaBounds(Common::Rect &) const;

	void drawPanorama(const Common::Rect &);

protected:
	void blankFields();
	void makeNewSurface(const Common::Rect &);
	void calcStripRange(const Common::Rect &, CoordType &, CoordType &);
	void loadStrips(CoordType, CoordType);
	void loadOneStrip(CoordType, CoordType);

	Movie _panoramaMovie;
	Surface _panoramaWorld, *_mask;
	Common::Rect _viewBounds;
	Common::Rect _drawBounds;
	CoordType _panoramaWidth, _panoramaHeight;
	CoordType _stripWidth; // Pixels per strip.
	CoordType _numStrips;
	CoordType _stripLeft, _stripRight;
};

} // End of namespace Pegasus

#endif
