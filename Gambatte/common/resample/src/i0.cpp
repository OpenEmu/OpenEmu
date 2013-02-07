/***************************************************************************
 *   Copyright (C) 2009 by Sindre Aamås                                    *
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
#include "i0.h"

double i0(double x) {
	double sum = 1.0;
	double xpm_dmfac = 1.0;
	double m = 1.0;
	unsigned n = 16;
	
	x = 0.25 * x * x;
	
	do {
		xpm_dmfac *= x / (m*m);
		sum += xpm_dmfac;
		m += 1.0;
	} while (--n);
	
	return sum;
}
