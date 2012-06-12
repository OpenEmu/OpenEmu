/**************************************************************************
 * Parks-McClellan algorithm for FIR filter design (C version)
 *-------------------------------------------------
 *  Copyright (c) 1995,1998  Jake Janovetz (janovetz@uiuc.edu)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.

 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *************************************************************************/
#ifndef REMEZ_H_
#define REMEZ_H_

#define REMEZ_BANDPASS       1
#define REMEZ_DIFFERENTIATOR 2
#define REMEZ_HILBERT        3

/* Function prototype for REMEZ_CreateFilter() - the only function that should need be
 * called from external code */
void REMEZ_CreateFilter(double h[], int numtaps,
           int numband, double bands[], const double des[], const double weight[],
           int type);

#endif /* REMEZ_H_ */
