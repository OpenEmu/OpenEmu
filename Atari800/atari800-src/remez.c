/*
 * remez.c - Parks-McClellan algorithm for FIR filter design (C version)
 *
 * Copyright (C) 1995,1998 Jake Janovetz
 * Copyright (C) 1998-2005 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "remez.h"
#ifdef ASAP /* external project, see http://asap.sf.net */
#include "asap_internal.h"
#else
#include "log.h"
#include "util.h"
#endif

#define NEGATIVE       0
#define POSITIVE       1

#define Pi             3.1415926535897932
#define Pi2            6.2831853071795865

#define GRIDDENSITY    16
#define MAXITERATIONS  40

/*******************
 * CreateDenseGrid
 *=================
 * Creates the dense grid of frequencies from the specified bands.
 * Also creates the Desired Frequency Response function (D[]) and
 * the Weight function (W[]) on that dense grid
 *
 *
 * INPUT:
 * ------
 * int      r        - 1/2 the number of filter coefficients
 * int      numtaps  - Number of taps in the resulting filter
 * int      numband  - Number of bands in user specification
 * double   bands[]  - User-specified band edges [2*numband]
 * double   des[]    - Desired response per band [numband]
 * double   weight[] - Weight per band [numband]
 * int      symmetry - Symmetry of filter - used for grid check
 *
 * OUTPUT:
 * -------
 * int    gridsize   - Number of elements in the dense frequency grid
 * double Grid[]     - Frequencies (0 to 0.5) on the dense grid [gridsize]
 * double D[]        - Desired response on the dense grid [gridsize]
 * double W[]        - Weight function on the dense grid [gridsize]
 *******************/

static void CreateDenseGrid(int r, int numtaps, int numband, double bands[],
                            const double des[], const double weight[],
                            int *gridsize, double Grid[],
                            double D[], double W[], int symmetry)
{
	int i, j, k, band;
	double delf, lowf, highf;

	delf = 0.5 / (GRIDDENSITY * r);

	/* For differentiator, hilbert,
	 *   symmetry is odd and Grid[0] = max(delf, band[0]) */

	if (symmetry == NEGATIVE && delf > bands[0])
		bands[0] = delf;

	j = 0;
	for (band = 0; band < numband; band++) {
		Grid[j] = bands[2 * band];
		lowf = bands[2 * band];
		highf = bands[2 * band + 1];
		k = (int) ((highf - lowf) / delf + 0.5); /* .5 for rounding */
		for (i = 0; i < k; i++) {
			D[j] = des[band];
			W[j] = weight[band];
			Grid[j] = lowf;
			lowf += delf;
			j++;
		}
		Grid[j - 1] = highf;
	}

	/* Similar to above, if odd symmetry, last grid point can't be .5
	 *  - but, if there are even taps, leave the last grid point at .5 */
	if ((symmetry == NEGATIVE) &&
	   (Grid[*gridsize - 1] > (0.5 - delf)) &&
	   (numtaps % 2))
	{
		Grid[*gridsize - 1] = 0.5 - delf;
	}
}


/********************
 * InitialGuess
 *==============
 * Places Extremal Frequencies evenly throughout the dense grid.
 *
 *
 * INPUT:
 * ------
 * int r        - 1/2 the number of filter coefficients
 * int gridsize - Number of elements in the dense frequency grid
 *
 * OUTPUT:
 * -------
 * int Ext[]    - Extremal indexes to dense frequency grid [r+1]
 ********************/

static void InitialGuess(int r, int Ext[], int gridsize)
{
	int i;

	for (i = 0; i <= r; i++)
		Ext[i] = i * (gridsize - 1) / r;
}


/***********************
 * CalcParms
 *===========
 *
 *
 * INPUT:
 * ------
 * int    r      - 1/2 the number of filter coefficients
 * int    Ext[]  - Extremal indexes to dense frequency grid [r+1]
 * double Grid[] - Frequencies (0 to 0.5) on the dense grid [gridsize]
 * double D[]    - Desired response on the dense grid [gridsize]
 * double W[]    - Weight function on the dense grid [gridsize]
 *
 * OUTPUT:
 * -------
 * double ad[]   - 'b' in Oppenheim & Schafer [r+1]
 * double x[]    - [r+1]
 * double y[]    - 'C' in Oppenheim & Schafer [r+1]
 ***********************/

static void CalcParms(int r, const int Ext[], const double Grid[],
                      const double D[], const double W[],
                      double ad[], double x[], double y[])
{
	int i, j, k, ld;
	double sign, xi, delta, denom, numer;

	/* Find x[] */
	for (i = 0; i <= r; i++)
		x[i] = cos(Pi2 * Grid[Ext[i]]);

	/* Calculate ad[]  - Oppenheim & Schafer eq 7.132 */
	ld = (r - 1) / 15 + 1; /* Skips around to avoid round errors */
	for (i = 0; i <= r; i++) {
		denom = 1.0;
		xi = x[i];
		for (j = 0; j < ld; j++) {
			for (k = j; k <= r; k += ld)
				if (k != i)
					denom *= 2.0 * (xi - x[k]);
		}
		if (fabs(denom) < 0.00001)
			denom = 0.00001;
		ad[i] = 1.0 / denom;
	}

	/* Calculate delta  - Oppenheim & Schafer eq 7.131 */
	numer = denom = 0;
	sign = 1;
	for (i = 0; i <= r; i++) {
		numer += ad[i] * D[Ext[i]];
		denom += sign * ad[i] / W[Ext[i]];
		sign = -sign;
	}
	delta = numer / denom;
	sign = 1;

	/* Calculate y[]  - Oppenheim & Schafer eq 7.133b */
	for (i = 0; i <= r; i++) {
		y[i] = D[Ext[i]] - sign * delta / W[Ext[i]];
		sign = -sign;
	}
}


/*********************
 * ComputeA
 *==========
 * Using values calculated in CalcParms, ComputeA calculates the
 * actual filter response at a given frequency (freq).  Uses
 * eq 7.133a from Oppenheim & Schafer.
 *
 *
 * INPUT:
 * ------
 * double freq - Frequency (0 to 0.5) at which to calculate A
 * int    r    - 1/2 the number of filter coefficients
 * double ad[] - 'b' in Oppenheim & Schafer [r+1]
 * double x[]  - [r+1]
 * double y[]  - 'C' in Oppenheim & Schafer [r+1]
 *
 * OUTPUT:
 * -------
 * Returns double value of A[freq]
 *********************/

static double ComputeA(double freq, int r, const double ad[],
                       const double x[], const double y[])
{
	int i;
	double xc, c, denom, numer;

	denom = numer = 0;
	xc = cos(Pi2 * freq);
	for (i = 0; i <= r; i++) {
		c = xc - x[i];
		if (fabs(c) < 1.0e-7) {
			numer = y[i];
			denom = 1;
			break;
		}
		c = ad[i] / c;
		denom += c;
		numer += c * y[i];
	}
	return numer / denom;
}


/************************
 * CalcError
 *===========
 * Calculates the Error function from the desired frequency response
 * on the dense grid (D[]), the weight function on the dense grid (W[]),
 * and the present response calculation (A[])
 *
 *
 * INPUT:
 * ------
 * int    r      - 1/2 the number of filter coefficients
 * double ad[]   - [r+1]
 * double x[]    - [r+1]
 * double y[]    - [r+1]
 * int gridsize  - Number of elements in the dense frequency grid
 * double Grid[] - Frequencies on the dense grid [gridsize]
 * double D[]    - Desired response on the dense grid [gridsize]
 * double W[]    - Weight function on the desnse grid [gridsize]
 *
 * OUTPUT:
 * -------
 * double E[]    - Error function on dense grid [gridsize]
 ************************/

static void CalcError(int r, const double ad[],
                      const double x[], const double y[],
                      int gridsize, const double Grid[],
                      const double D[], const double W[], double E[])
{
	int i;
	double A;

	for (i = 0; i < gridsize; i++) {
		A = ComputeA(Grid[i], r, ad, x, y);
		E[i] = W[i] * (D[i] - A);
	}
}

/************************
 * Search
 *========
 * Searches for the maxima/minima of the error curve.  If more than
 * r+1 extrema are found, it uses the following heuristic (thanks
 * Chris Hanson):
 * 1) Adjacent non-alternating extrema deleted first.
 * 2) If there are more than one excess extrema, delete the
 *    one with the smallest error.  This will create a non-alternation
 *    condition that is fixed by 1).
 * 3) If there is exactly one excess extremum, delete the smaller
 *    of the first/last extremum
 *
 *
 * INPUT:
 * ------
 * int    r        - 1/2 the number of filter coefficients
 * int    Ext[]    - Indexes to Grid[] of extremal frequencies [r+1]
 * int    gridsize - Number of elements in the dense frequency grid
 * double E[]      - Array of error values.  [gridsize]
 * OUTPUT:
 * -------
 * int    Ext[]    - New indexes to extremal frequencies [r+1]
 ************************/

static void Search(int r, int Ext[], int gridsize, const double E[])
{
	int i, j, k, l, extra;     /* Counters */
	int up, alt;
	int *foundExt;             /* Array of found extremals */

	/* Allocate enough space for found extremals. */
	foundExt = (int *) Util_malloc((2 * r) * sizeof(int));
	k = 0;

	/* Check for extremum at 0. */
	if (((E[0] > 0.0) && (E[0] > E[1])) ||
	   ((E[0] < 0.0) && (E[0] < E[1])))
		foundExt[k++] = 0;

	/* Check for extrema inside dense grid */
	for (i = 1; i < gridsize - 1; i++) {
		if (((E[i] >= E[i - 1]) && (E[i] > E[i + 1]) && (E[i] > 0.0)) ||
		   ((E[i] <= E[i - 1]) && (E[i] < E[i + 1]) && (E[i] < 0.0)))
			foundExt[k++] = i;
	}

	/* Check for extremum at 0.5 */
	j = gridsize - 1;
	if (((E[j] > 0.0) && (E[j] > E[j - 1])) ||
	   ((E[j] < 0.0) && (E[j] < E[j - 1])))
		foundExt[k++] = j;

	/* Remove extra extremals */
	extra = k - (r + 1);

	while (extra > 0) {
		if (E[foundExt[0]] > 0.0)
			up = 1;                /* first one is a maxima */
		else
			up = 0;                /* first one is a minima */

		l = 0;
		alt = 1;
		for (j = 1; j < k; j++) {
			if (fabs(E[foundExt[j]]) < fabs(E[foundExt[l]]))
				l = j;              /* new smallest error. */
			if ((up) && (E[foundExt[j]] < 0.0))
				up = 0;             /* switch to a minima */
			else if ((!up) && (E[foundExt[j]] > 0.0))
				up = 1;             /* switch to a maxima */
			else {
				alt = 0;
				break;              /* Ooops, found two non-alternating */
			}                       /* extrema.  Delete smallest of them */
		}  /* if the loop finishes, all extrema are alternating */

		/* If there's only one extremal and all are alternating,
		 * delete the smallest of the first/last extremals. */
		if ((alt) && (extra == 1)) {
			if (fabs(E[foundExt[k - 1]]) < fabs(E[foundExt[0]]))
				l = foundExt[k - 1];   /* Delete last extremal */
			else
				l = foundExt[0];       /* Delete first extremal */
		}

		/* Loop that does the deletion */
		for (j = l; j < k; j++) {
			foundExt[j] = foundExt[j+1];
		}
		k--;
		extra--;
	}

	/* Copy found extremals to Ext[] */
	for (i = 0; i <= r; i++) {
		Ext[i] = foundExt[i];
	}

	free(foundExt);
}


/*********************
 * FreqSample
 *============
 * Simple frequency sampling algorithm to determine the impulse
 * response h[] from A's found in ComputeA
 *
 *
 * INPUT:
 * ------
 * int      N        - Number of filter coefficients
 * double   A[]      - Sample points of desired response [N/2]
 * int      symm     - Symmetry of desired filter
 *
 * OUTPUT:
 * -------
 * double h[] - Impulse Response of final filter [N]
 *********************/
static void FreqSample(int N, const double A[], double h[], int symm)
{
	int n, k;
	double x, val, M;

	M = (N - 1.0) / 2.0;
	if (symm == POSITIVE) {
		if (N % 2) {
			for (n = 0; n < N; n++) {
				val = A[0];
				x = Pi2 * (n - M) / N;
				for (k = 1; k <= M; k++)
					val += 2.0 * A[k] * cos(x * k);
				h[n] = val / N;
			}
		}
		else {
			for (n = 0; n < N; n++) {
				val = A[0];
				x = Pi2 * (n - M)/N;
				for (k = 1; k <= (N / 2 - 1); k++)
					val += 2.0 * A[k] * cos(x * k);
				h[n] = val / N;
			}
		}
	}
	else {
		if (N % 2) {
			for (n = 0; n < N; n++) {
				val = 0;
				x = Pi2 * (n - M) / N;
				for (k = 1; k <= M; k++)
					val += 2.0 * A[k] * sin(x * k);
				h[n] = val / N;
			}
		}
		else {
			for (n = 0; n < N; n++) {
				val = A[N / 2] * sin(Pi * (n - M));
				x = Pi2 * (n - M) / N;
				for (k = 1; k <= (N / 2 - 1); k++)
					val += 2.0 * A[k] * sin(x * k);
				h[n] = val / N;
			}
		}
	}
}

/*******************
 * isDone
 *========
 * Checks to see if the error function is small enough to consider
 * the result to have converged.
 *
 * INPUT:
 * ------
 * int    r     - 1/2 the number of filter coeffiecients
 * int    Ext[] - Indexes to extremal frequencies [r+1]
 * double E[]   - Error function on the dense grid [gridsize]
 *
 * OUTPUT:
 * -------
 * Returns 1 if the result converged
 * Returns 0 if the result has not converged
 ********************/

static int isDone(int r, const int Ext[], const double E[])
{
	int i;
	double min, max, current;

	min = max = fabs(E[Ext[0]]);
	for (i = 1; i <= r; i++) {
		current = fabs(E[Ext[i]]);
		if (current < min)
			min = current;
		if (current > max)
			max = current;
	}
	if (((max - min) / max) < 0.0001)
		return 1;
	return 0;
}

/********************
 * REMEZ_CreateFilter
 *=======
 * Calculates the optimal (in the Chebyshev/minimax sense)
 * FIR filter impulse response given a set of band edges,
 * the desired reponse on those bands, and the weight given to
 * the error in those bands.
 *
 * INPUT:
 * ------
 * int     numtaps     - Number of filter coefficients
 * int     numband     - Number of bands in filter specification
 * double  bands[]     - User-specified band edges [2 * numband]
 * double  des[]       - User-specified band responses [numband]
 * double  weight[]    - User-specified error weights [numband]
 * int     type        - Type of filter
 *
 * OUTPUT:
 * -------
 * double h[]      - Impulse response of final filter [numtaps]
 ********************/

void REMEZ_CreateFilter(double h[], int numtaps, int numband, double bands[],
           const double des[], const double weight[], int type)
{
	double *Grid, *W, *D, *E;
	int    i, iter, gridsize, r, *Ext;
	double *taps, c;
	double *x, *y, *ad;
	int    symmetry;

	if (type == REMEZ_BANDPASS)
		symmetry = POSITIVE;
	else
		symmetry = NEGATIVE;

	r = numtaps / 2;                  /* number of extrema */
	if ((numtaps % 2) && (symmetry == POSITIVE))
		r++;

	/* Predict dense grid size in advance for memory allocation
	 *   .5 is so we round up, not truncate */
	gridsize = 0;
	for (i = 0; i < numband; i++) {
		gridsize += (int) (2 * r * GRIDDENSITY *
		                   (bands[2 * i + 1] - bands[2 * i]) + .5);
	}
	if (symmetry == NEGATIVE) {
		gridsize--;
	}

	/* Dynamically allocate memory for arrays with proper sizes */
	Grid = (double *) Util_malloc(gridsize * sizeof(double));
	D = (double *) Util_malloc(gridsize * sizeof(double));
	W = (double *) Util_malloc(gridsize * sizeof(double));
	E = (double *) Util_malloc(gridsize * sizeof(double));
	Ext = (int *) Util_malloc((r + 1) * sizeof(int));
	taps = (double *) Util_malloc((r + 1) * sizeof(double));
	x = (double *) Util_malloc((r + 1) * sizeof(double));
	y = (double *) Util_malloc((r + 1) * sizeof(double));
	ad = (double *) Util_malloc((r + 1) * sizeof(double));

	/* Create dense frequency grid */
	CreateDenseGrid(r, numtaps, numband, bands, des, weight,
	                &gridsize, Grid, D, W, symmetry);
	InitialGuess(r, Ext, gridsize);

	/* For Differentiator: (fix grid) */
	if (type == REMEZ_DIFFERENTIATOR) {
		for (i = 0; i < gridsize; i++) {
			/* D[i] = D[i] * Grid[i]; */
			if (D[i] > 0.0001)
				W[i] = W[i] / Grid[i];
		}
	}

	/* For odd or Negative symmetry filters, alter the
	 * D[] and W[] according to Parks McClellan */
	if (symmetry == POSITIVE) {
		if (numtaps % 2 == 0) {
			for (i = 0; i < gridsize; i++) {
				c = cos(Pi * Grid[i]);
				D[i] /= c;
				W[i] *= c;
			}
		}
	}
	else {
		if (numtaps % 2) {
			for (i = 0; i < gridsize; i++) {
				c = sin(Pi2 * Grid[i]);
				D[i] /= c;
				W[i] *= c;
			}
		}
		else {
			for (i = 0; i < gridsize; i++) {
				c = sin(Pi * Grid[i]);
				D[i] /= c;
				W[i] *= c;
			}
		}
	}

	/* Perform the Remez Exchange algorithm */
	for (iter = 0; iter < MAXITERATIONS; iter++) {
		CalcParms(r, Ext, Grid, D, W, ad, x, y);
		CalcError(r, ad, x, y, gridsize, Grid, D, W, E);
		Search(r, Ext, gridsize, E);
		if (isDone(r, Ext, E))
			break;
	}
#ifndef ASAP
	if (iter == MAXITERATIONS) {
		Log_print("remez(): reached maximum iteration count. Results may be bad.");
	}
#endif

	CalcParms(r, Ext, Grid, D, W, ad, x, y);

	/* Find the 'taps' of the filter for use with Frequency
	 * Sampling.  If odd or Negative symmetry, fix the taps
	 * according to Parks McClellan */
	for (i = 0; i <= numtaps / 2; i++) {
		if (symmetry == POSITIVE) {
			if (numtaps % 2)
				c = 1;
			else
				c = cos(Pi * (double) i / numtaps);
		}
		else {
			if (numtaps % 2)
				c = sin(Pi2 * (double) i / numtaps);
			else
				c = sin(Pi * (double) i / numtaps);
		}
		taps[i] = ComputeA((double) i / numtaps, r, ad, x, y) * c;
	}

	/* Frequency sampling design with calculated taps */
	FreqSample(numtaps, taps, h, symmetry);

	/* Delete allocated memory */
	free(Grid);
	free(W);
	free(D);
	free(E);
	free(Ext);
	free(taps);
	free(x);
	free(y);
	free(ad);
}
