/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
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
#ifndef STATESAVER_H
#define STATESAVER_H

class SaveState;

class StateSaver {
	StateSaver();
	
public:
	enum { SS_SHIFT = 2 };
	enum { SS_DIV = 1 << 2 };
	enum { SS_WIDTH = 160 >> SS_SHIFT };
	enum { SS_HEIGHT = 144 >> SS_SHIFT };
	
	static void saveState(const SaveState &state, const char *filename);
	static bool loadState(SaveState &state, const char *filename);
};

#endif
