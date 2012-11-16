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
#ifndef GAMBATTE_INT_H
#define GAMBATTE_INT_H

#ifdef HAVE_CSTDINT

#include <cstdint>

namespace gambatte {
using std::uint_least32_t;
using std::uint_least16_t;
}

#elif defined(HAVE_STDINT_H)

#include <stdint.h>

namespace gambatte {
using ::uint_least32_t;
using ::uint_least16_t;
}

#else

namespace gambatte {
#ifdef CHAR_LEAST_32
typedef unsigned char uint_least32_t;
#elif defined(SHORT_LEAST_32)
typedef unsigned short uint_least32_t;
#elif defined(INT_LEAST_32)
typedef unsigned uint_least32_t;
#else
typedef unsigned long uint_least32_t;
#endif

#ifdef CHAR_LEAST_16
typedef unsigned char uint_least16_t;
#else
typedef unsigned short uint_least16_t;
#endif
}

#endif

#endif
