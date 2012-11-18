/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
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
#ifndef GAMBATTE_INPUTGETTER_H
#define GAMBATTE_INPUTGETTER_H

namespace gambatte {
class InputGetter {
public:
	enum { A = 0x01, B = 0x02, SELECT = 0x04, START = 0x08, RIGHT = 0x10, LEFT = 0x20, UP = 0x40, DOWN = 0x80 };
	virtual ~InputGetter() {};
	
	/** @return A|B|SELECT|START|RIGHT|LEFT|UP|DOWN if those buttons are pressed. */
	virtual unsigned operator()() = 0;
};
}

#endif
