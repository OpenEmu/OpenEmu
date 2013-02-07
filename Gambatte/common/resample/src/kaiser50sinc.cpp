/***************************************************************************
 *   Copyright (C) 2008-2009 by Sindre Aamås                               *
 *   sinamas@users.sourceforge.net                                         *
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
#include "kaiser50sinc.h"
#include "i0.h"
#include <cmath>

double kaiser50SincWin(const long n, const long M) {
	const double beta = 4.62;
	static const double i0beta_rec = 1.0 / i0(beta);

	double x = static_cast<double>(n * 2) / M - 1.0;
	x = x * x;
	x = beta * std::sqrt(1.0 - x);
	
	return i0(x) * i0beta_rec;
}
