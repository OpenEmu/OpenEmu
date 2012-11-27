/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

/*
 * This code is based on Libart_LGPL - library of basic graphic primitives
 *
 * Copyright (c) 1998 Raph Levien
 *
 * Licensed under GNU LGPL v2
 *
 */

/* Various utility functions RLL finds useful. */

#include "common/textconsole.h"

#include "sword25/gfx/image/art.h"

namespace Sword25 {

/**
 * art_svp_free: Free an #ArtSVP structure.
 * @svp: #ArtSVP to free.
 *
 * Frees an #ArtSVP structure and all the segments in it.
 **/
void art_svp_free(ArtSVP *svp) {
	int n_segs = svp->n_segs;
	int i;

	for (i = 0; i < n_segs; i++)
		free(svp->segs[i].points);
	free(svp);
}

#define EPSILON 0

/**
 * art_svp_seg_compare: Compare two segments of an svp.
 * @seg1: First segment to compare.
 * @seg2: Second segment to compare.
 *
 * Compares two segments of an svp. Return 1 if @seg2 is below or to the
 * right of @seg1, -1 otherwise.
 **/
int art_svp_seg_compare(const void *s1, const void *s2) {
	const ArtSVPSeg *seg1 = (const ArtSVPSeg *)s1;
	const ArtSVPSeg *seg2 = (const ArtSVPSeg *)s2;

	if (seg1->points[0].y - EPSILON > seg2->points[0].y) return 1;
	else if (seg1->points[0].y + EPSILON < seg2->points[0].y) return -1;
	else if (seg1->points[0].x - EPSILON > seg2->points[0].x) return 1;
	else if (seg1->points[0].x + EPSILON < seg2->points[0].x) return -1;
	else if ((seg1->points[1].x - seg1->points[0].x) *
	         (seg2->points[1].y - seg2->points[0].y) -
	         (seg1->points[1].y - seg1->points[0].y) *
	         (seg2->points[1].x - seg2->points[0].x) > 0) return 1;
	else return -1;
}

/**
 * art_vpath_add_point: Add point to vpath.
 * @p_vpath: Where the pointer to the #ArtVpath structure is stored.
 * @pn_points: Pointer to the number of points in *@p_vpath.
 * @pn_points_max: Pointer to the number of points allocated.
 * @code: The pathcode for the new point.
 * @x: The X coordinate of the new point.
 * @y: The Y coordinate of the new point.
 *
 * Adds a new point to *@p_vpath, reallocating and updating *@p_vpath
 * and *@pn_points_max as necessary. *@pn_points is incremented.
 *
 * This routine always adds the point after all points already in the
 * vpath. Thus, it should be called in the order the points are
 * desired.
 **/
void art_vpath_add_point(ArtVpath **p_vpath, int *pn_points, int *pn_points_max,
                    ArtPathcode code, double x, double y) {
	int i;

	i = (*pn_points)++;
	if (i == *pn_points_max)
		art_expand(*p_vpath, ArtVpath, *pn_points_max);
	(*p_vpath)[i].code = code;
	(*p_vpath)[i].x = x;
	(*p_vpath)[i].y = y;
}

/* Sort vector paths into sorted vector paths */

/* reverse a list of points in place */
static void reverse_points(ArtPoint *points, int n_points) {
	int i;
	ArtPoint tmp_p;

	for (i = 0; i < (n_points >> 1); i++) {
		tmp_p = points[i];
		points[i] = points[n_points - (i + 1)];
		points[n_points - (i + 1)] = tmp_p;
	}
}

/**
 * art_svp_from_vpath: Convert a vpath to a sorted vector path.
 * @vpath: #ArtVPath to convert.
 *
 * Converts a vector path into sorted vector path form. The svp form is
 * more efficient for rendering and other vector operations.
 *
 * Basically, the implementation is to traverse the vector path,
 * generating a new segment for each "run" of points in the vector
 * path with monotonically increasing Y values. All the resulting
 * values are then sorted.
 *
 * Note: I'm not sure that the sorting rule is correct with respect
 * to numerical stability issues.
 *
 * Return value: Resulting sorted vector path.
 **/
ArtSVP *art_svp_from_vpath(ArtVpath *vpath) {
	int n_segs, n_segs_max;
	ArtSVP *svp;
	int dir;
	int new_dir;
	int i;
	ArtPoint *points;
	int n_points, n_points_max;
	double x, y;
	double x_min, x_max;

	n_segs = 0;
	n_segs_max = 16;
	svp = (ArtSVP *)malloc(sizeof(ArtSVP) +
	                          (n_segs_max - 1) * sizeof(ArtSVPSeg));
	if (!svp)
		error("[art_svp_from_vpath] Cannot allocate memory");

	dir = 0;
	n_points = 0;
	n_points_max = 0;
	points = NULL;
	i = 0;

	x = y = 0; /* unnecessary, given "first code must not be LINETO" invariant,
        but it makes gcc -Wall -ansi -pedantic happier */
	x_min = x_max = 0; /* same */

	while (vpath[i].code != ART_END) {
		if (vpath[i].code == ART_MOVETO || vpath[i].code == ART_MOVETO_OPEN) {
			if (points != NULL && n_points >= 2) {
				if (n_segs == n_segs_max) {
					n_segs_max <<= 1;
					ArtSVP *tmp = (ArtSVP *)realloc(svp, sizeof(ArtSVP) +
					                                    (n_segs_max - 1) *
					                                    sizeof(ArtSVPSeg));

					if (!tmp)
						error("Cannot reallocate memory in art_svp_from_vpath()");

					svp = tmp;
				}
				svp->segs[n_segs].n_points = n_points;
				svp->segs[n_segs].dir = (dir > 0);
				if (dir < 0)
					reverse_points(points, n_points);
				svp->segs[n_segs].points = points;
				svp->segs[n_segs].bbox.x0 = x_min;
				svp->segs[n_segs].bbox.x1 = x_max;
				svp->segs[n_segs].bbox.y0 = points[0].y;
				svp->segs[n_segs].bbox.y1 = points[n_points - 1].y;
				n_segs++;
				points = NULL;
			}

			if (points == NULL) {
				n_points_max = 4;
				points = art_new(ArtPoint, n_points_max);
			}

			n_points = 1;
			points[0].x = x = vpath[i].x;
			points[0].y = y = vpath[i].y;
			x_min = x;
			x_max = x;
			dir = 0;
		} else { /* must be LINETO */
			new_dir = (vpath[i].y > y ||
			           (vpath[i].y == y && vpath[i].x > x)) ? 1 : -1;
			if (dir && dir != new_dir) {
				/* new segment */
				x = points[n_points - 1].x;
				y = points[n_points - 1].y;
				if (n_segs == n_segs_max) {
					n_segs_max <<= 1;
					ArtSVP *tmp = (ArtSVP *)realloc(svp, sizeof(ArtSVP) +
					                                     (n_segs_max - 1) *
					                                     sizeof(ArtSVPSeg));

					if (!tmp)
						error("Cannot reallocate memory in art_svp_from_vpath()");

					svp = tmp;
				}
				svp->segs[n_segs].n_points = n_points;
				svp->segs[n_segs].dir = (dir > 0);
				if (dir < 0)
					reverse_points(points, n_points);
				svp->segs[n_segs].points = points;
				svp->segs[n_segs].bbox.x0 = x_min;
				svp->segs[n_segs].bbox.x1 = x_max;
				svp->segs[n_segs].bbox.y0 = points[0].y;
				svp->segs[n_segs].bbox.y1 = points[n_points - 1].y;
				n_segs++;

				n_points = 1;
				n_points_max = 4;
				points = art_new(ArtPoint, n_points_max);
				points[0].x = x;
				points[0].y = y;
				x_min = x;
				x_max = x;
			}

			if (points != NULL) {
				if (n_points == n_points_max)
					art_expand(points, ArtPoint, n_points_max);
				points[n_points].x = x = vpath[i].x;
				points[n_points].y = y = vpath[i].y;
				if (x < x_min) x_min = x;
				else if (x > x_max) x_max = x;
				n_points++;
			}
			dir = new_dir;
		}
		i++;
	}

	if (points != NULL) {
		if (n_points >= 2) {
			if (n_segs == n_segs_max) {
				n_segs_max <<= 1;
				ArtSVP *tmp = (ArtSVP *)realloc(svp, sizeof(ArtSVP) +
				                                     (n_segs_max - 1) *
				                                      sizeof(ArtSVPSeg));

				if (!tmp)
					error("Cannot reallocate memory in art_svp_from_vpath()");

				svp = tmp;
			}
			svp->segs[n_segs].n_points = n_points;
			svp->segs[n_segs].dir = (dir > 0);
			if (dir < 0)
				reverse_points(points, n_points);
			svp->segs[n_segs].points = points;
			svp->segs[n_segs].bbox.x0 = x_min;
			svp->segs[n_segs].bbox.x1 = x_max;
			svp->segs[n_segs].bbox.y0 = points[0].y;
			svp->segs[n_segs].bbox.y1 = points[n_points - 1].y;
			n_segs++;
		} else
			free(points);
	}

	svp->n_segs = n_segs;

	qsort(&svp->segs, n_segs, sizeof(ArtSVPSeg), art_svp_seg_compare);

	return svp;
}


/* Basic constructors and operations for bezier paths */

#define RENDER_LEVEL 4
#define RENDER_SIZE (1 << (RENDER_LEVEL))

/**
 * art_vpath_render_bez: Render a bezier segment into the vpath.
 * @p_vpath: Where the pointer to the #ArtVpath structure is stored.
 * @pn_points: Pointer to the number of points in *@p_vpath.
 * @pn_points_max: Pointer to the number of points allocated.
 * @x0: X coordinate of starting bezier point.
 * @y0: Y coordinate of starting bezier point.
 * @x1: X coordinate of first bezier control point.
 * @y1: Y coordinate of first bezier control point.
 * @x2: X coordinate of second bezier control point.
 * @y2: Y coordinate of second bezier control point.
 * @x3: X coordinate of ending bezier point.
 * @y3: Y coordinate of ending bezier point.
 * @flatness: Flatness control.
 *
 * Renders a bezier segment into the vector path, reallocating and
 * updating *@p_vpath and *@pn_vpath_max as necessary. *@pn_vpath is
 * incremented by the number of vector points added.
 *
 * This step includes (@x0, @y0) but not (@x3, @y3).
 *
 * The @flatness argument guides the amount of subdivision. The Adobe
 * PostScript reference manual defines flatness as the maximum
 * deviation between the any point on the vpath approximation and the
 * corresponding point on the "true" curve, and we follow this
 * definition here. A value of 0.25 should ensure high quality for aa
 * rendering.
**/
static void art_vpath_render_bez(ArtVpath **p_vpath, int *pn, int *pn_max,
                     double x0, double y0,
                     double x1, double y1,
                     double x2, double y2,
                     double x3, double y3,
                     double flatness) {
	/* It's possible to optimize this routine a fair amount.

	   First, once the _dot conditions are met, they will also be met in
	   all further subdivisions. So we might recurse to a different
	   routine that only checks the _perp conditions.

	   Second, the distance _should_ decrease according to fairly
	   predictable rules (a factor of 4 with each subdivision). So it might
	   be possible to note that the distance is within a factor of 4 of
	   acceptable, and subdivide once. But proving this might be hard.

	   Third, at the last subdivision, x_m and y_m can be computed more
	   expeditiously (as in the routine above).

	   Finally, if we were able to subdivide by, say 2 or 3, this would
	   allow considerably finer-grain control, i.e. fewer points for the
	   same flatness tolerance. This would speed things up downstream.

	   In any case, this routine is unlikely to be the bottleneck. It's
	   just that I have this undying quest for more speed...

	*/

	bool subDivide = false;

	double x3_0 = x3 - x0;
	double y3_0 = y3 - y0;

	// z3_0_dot is dist z0-z3 squared
	double z3_0_dot = x3_0 * x3_0 + y3_0 * y3_0;

	if (z3_0_dot < 0.001) {
		/* if start and end point are almost identical, the flatness tests
		 * don't work properly, so fall back on testing whether both of
		 * the other two control points are the same as the start point,
		 * too.
		 */
		if (!(hypot(x1 - x0, y1 - y0) < 0.001
		        && hypot(x2 - x0, y2 - y0) < 0.001))
			subDivide = true;
	} else {
		/* we can avoid subdivision if:

			 z1 has distance no more than flatness from the z0-z3 line

			 z1 is no more z0'ward than flatness past z0-z3

			 z1 is more z0'ward than z3'ward on the line traversing z0-z3

			 and correspondingly for z2 */

		// perp is distance from line, multiplied by dist z0-z3
		double max_perp_sq = flatness * flatness * z3_0_dot;

		double z1_perp = (y1 - y0) * x3_0 - (x1 - x0) * y3_0;
		if (z1_perp * z1_perp > max_perp_sq) {
			subDivide = true;
		} else {
			double z2_perp = (y3 - y2) * x3_0 - (x3 - x2) * y3_0;
			if (z2_perp * z2_perp > max_perp_sq) {
				subDivide = true;
			} else {
				double z1_dot = (x1 - x0) * x3_0 + (y1 - y0) * y3_0;
				if (z1_dot < 0 && z1_dot * z1_dot > max_perp_sq) {
					subDivide = true;
				} else {
					double z2_dot = (x3 - x2) * x3_0 + (y3 - y2) * y3_0;
					if (z2_dot < 0 && z2_dot * z2_dot > max_perp_sq)
						subDivide = true;
					else if (z1_dot + z1_dot > z3_0_dot)
						subDivide = true;
					else if (z2_dot + z2_dot > z3_0_dot)
						subDivide = true;
				}
			}
		}
	}

	if (subDivide) {
		double xa1 = (x0 + x1) * 0.5;
		double ya1 = (y0 + y1) * 0.5;
		double xa2 = (x0 + 2 * x1 + x2) * 0.25;
		double ya2 = (y0 + 2 * y1 + y2) * 0.25;
		double xb1 = (x1 + 2 * x2 + x3) * 0.25;
		double yb1 = (y1 + 2 * y2 + y3) * 0.25;
		double xb2 = (x2 + x3) * 0.5;
		double yb2 = (y2 + y3) * 0.5;
		double x_m = (xa2 + xb1) * 0.5;
		double y_m = (ya2 + yb1) * 0.5;

		art_vpath_render_bez(p_vpath, pn, pn_max,
		                     x0, y0, xa1, ya1, xa2, ya2, x_m, y_m, flatness);
		art_vpath_render_bez(p_vpath, pn, pn_max,
		                     x_m, y_m, xb1, yb1, xb2, yb2, x3, y3, flatness);
	} else {
		// don't subdivide
		art_vpath_add_point(p_vpath, pn, pn_max,
	           	         ART_LINETO, x3, y3);
	}
}

/**
 * art_bez_path_to_vec: Create vpath from bezier path.
 * @bez: Bezier path.
 * @flatness: Flatness control.
 *
 * Creates a vector path closely approximating the bezier path defined by
 * @bez. The @flatness argument controls the amount of subdivision. In
 * general, the resulting vpath deviates by at most @flatness pixels
 * from the "ideal" path described by @bez.
 *
 * Return value: Newly allocated vpath.
 **/
ArtVpath *art_bez_path_to_vec(const ArtBpath *bez, double flatness) {
	ArtVpath *vec;
	int vec_n, vec_n_max;
	int bez_index;
	double x, y;

	vec_n = 0;
	vec_n_max = RENDER_SIZE;
	vec = art_new(ArtVpath, vec_n_max);

	/* Initialization is unnecessary because of the precondition that the
	   bezier path does not begin with LINETO or CURVETO, but is here
	   to make the code warning-free. */
	x = 0;
	y = 0;

	bez_index = 0;
	do {
		/* make sure space for at least one more code */
		if (vec_n >= vec_n_max)
			art_expand(vec, ArtVpath, vec_n_max);
		switch (bez[bez_index].code) {
		case ART_MOVETO_OPEN:
		case ART_MOVETO:
		case ART_LINETO:
			x = bez[bez_index].x3;
			y = bez[bez_index].y3;
			vec[vec_n].code = bez[bez_index].code;
			vec[vec_n].x = x;
			vec[vec_n].y = y;
			vec_n++;
			break;
		case ART_END:
			vec[vec_n].code = bez[bez_index].code;
			vec[vec_n].x = 0;
			vec[vec_n].y = 0;
			vec_n++;
			break;
		case ART_CURVETO:
			art_vpath_render_bez(&vec, &vec_n, &vec_n_max,
			                     x, y,
			                     bez[bez_index].x1, bez[bez_index].y1,
			                     bez[bez_index].x2, bez[bez_index].y2,
			                     bez[bez_index].x3, bez[bez_index].y3,
			                     flatness);
			x = bez[bez_index].x3;
			y = bez[bez_index].y3;
			break;
		}
	} while (bez[bez_index++].code != ART_END);
	return vec;
}


#define EPSILON_6 1e-6
#define EPSILON_2 1e-12

/* Render an arc segment starting at (xc + x0, yc + y0) to (xc + x1,
   yc + y1), centered at (xc, yc), and with given radius. Both x0^2 +
   y0^2 and x1^2 + y1^2 should be equal to radius^2.

   A positive value of radius means curve to the left, negative means
   curve to the right.
*/
static void art_svp_vpath_stroke_arc(ArtVpath **p_vpath, int *pn, int *pn_max,
                         double xc, double yc,
                         double x0, double y0,
                         double x1, double y1,
                         double radius,
                         double flatness) {
	double theta;
	double th_0, th_1;
	int n_pts;
	int i;
	double aradius;

	aradius = fabs(radius);
	theta = 2 * M_SQRT2 * sqrt(flatness / aradius);
	th_0 = atan2(y0, x0);
	th_1 = atan2(y1, x1);
	if (radius > 0) {
		/* curve to the left */
		if (th_0 < th_1) th_0 += M_PI * 2;
		n_pts = (int)ceil((th_0 - th_1) / theta);
	} else {
		/* curve to the right */
		if (th_1 < th_0) th_1 += M_PI * 2;
		n_pts = (int)ceil((th_1 - th_0) / theta);
	}
	art_vpath_add_point(p_vpath, pn, pn_max,
	                    ART_LINETO, xc + x0, yc + y0);
	for (i = 1; i < n_pts; i++) {
		theta = th_0 + (th_1 - th_0) * i / n_pts;
		art_vpath_add_point(p_vpath, pn, pn_max,
		                    ART_LINETO, xc + cos(theta) * aradius,
		                    yc + sin(theta) * aradius);
	}
	art_vpath_add_point(p_vpath, pn, pn_max,
	                    ART_LINETO, xc + x1, yc + y1);
}

/* Assume that forw and rev are at point i0. Bring them to i1,
   joining with the vector i1 - i2.

   This used to be true, but isn't now that the stroke_raw code is
   filtering out (near)zero length vectors: {It so happens that all
   invocations of this function maintain the precondition i1 = i0 + 1,
   so we could decrease the number of arguments by one. We haven't
   done that here, though.}

   forw is to the line's right and rev is to its left.

   Precondition: no zero-length vectors, otherwise a divide by
   zero will happen.  */
static void render_seg(ArtVpath **p_forw, int *pn_forw, int *pn_forw_max,
           ArtVpath **p_rev, int *pn_rev, int *pn_rev_max,
           ArtVpath *vpath, int i0, int i1, int i2,
           ArtPathStrokeJoinType join,
           double line_width, double miter_limit, double flatness) {
	double dx0, dy0;
	double dx1, dy1;
	double dlx0, dly0;
	double dlx1, dly1;
	double dmx, dmy;
	double dmr2;
	double scale;
	double cross;

	/* The vectors of the lines from i0 to i1 and i1 to i2. */
	dx0 = vpath[i1].x - vpath[i0].x;
	dy0 = vpath[i1].y - vpath[i0].y;

	dx1 = vpath[i2].x - vpath[i1].x;
	dy1 = vpath[i2].y - vpath[i1].y;

	/* Set dl[xy]0 to the vector from i0 to i1, rotated counterclockwise
	   90 degrees, and scaled to the length of line_width. */
	scale = line_width / sqrt(dx0 * dx0 + dy0 * dy0);
	dlx0 = dy0 * scale;
	dly0 = -dx0 * scale;

	/* Set dl[xy]1 to the vector from i1 to i2, rotated counterclockwise
	   90 degrees, and scaled to the length of line_width. */
	scale = line_width / sqrt(dx1 * dx1 + dy1 * dy1);
	dlx1 = dy1 * scale;
	dly1 = -dx1 * scale;

	/* now, forw's last point is expected to be colinear along d[xy]0
	   to point i0 - dl[xy]0, and rev with i0 + dl[xy]0. */

	/* positive for positive area (i.e. left turn) */
	cross = dx1 * dy0 - dx0 * dy1;

	dmx = (dlx0 + dlx1) * 0.5;
	dmy = (dly0 + dly1) * 0.5;
	dmr2 = dmx * dmx + dmy * dmy;

	if (join == ART_PATH_STROKE_JOIN_MITER &&
	        dmr2 * miter_limit * miter_limit < line_width * line_width)
		join = ART_PATH_STROKE_JOIN_BEVEL;

	/* the case when dmr2 is zero or very small bothers me
	   (i.e. near a 180 degree angle)
	   ALEX: So, we avoid the optimization when dmr2 is very small. This should
	   be safe since dmx/y is only used in optimization and in MITER case, and MITER
	   should be converted to BEVEL when dmr2 is very small. */
	if (dmr2 > EPSILON_2) {
		scale = line_width * line_width / dmr2;
		dmx *= scale;
		dmy *= scale;
	}

	if (cross *cross < EPSILON_2 && dx0 *dx1 + dy0 *dy1 >= 0) {
		/* going straight */
		art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
		                    ART_LINETO, vpath[i1].x - dlx0, vpath[i1].y - dly0);
		art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
		                    ART_LINETO, vpath[i1].x + dlx0, vpath[i1].y + dly0);
	} else if (cross > 0) {
		/* left turn, forw is outside and rev is inside */

		if (
		    (dmr2 > EPSILON_2) &&
		    /* check that i1 + dm[xy] is inside i0-i1 rectangle */
		    (dx0 + dmx) * dx0 + (dy0 + dmy) * dy0 > 0 &&
		    /* and that i1 + dm[xy] is inside i1-i2 rectangle */
		    ((dx1 - dmx) * dx1 + (dy1 - dmy) * dy1 > 0)
#ifdef PEDANTIC_INNER
		    &&
		    /* check that i1 + dl[xy]1 is inside i0-i1 rectangle */
		    (dx0 + dlx1) * dx0 + (dy0 + dly1) * dy0 > 0 &&
		    /* and that i1 + dl[xy]0 is inside i1-i2 rectangle */
		    ((dx1 - dlx0) * dx1 + (dy1 - dly0) * dy1 > 0)
#endif
		) {
			/* can safely add single intersection point */
			art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
			                    ART_LINETO, vpath[i1].x + dmx, vpath[i1].y + dmy);
		} else {
			/* need to loop-de-loop the inside */
			art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
			                    ART_LINETO, vpath[i1].x + dlx0, vpath[i1].y + dly0);
			art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
			                    ART_LINETO, vpath[i1].x, vpath[i1].y);
			art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
			                    ART_LINETO, vpath[i1].x + dlx1, vpath[i1].y + dly1);
		}

		if (join == ART_PATH_STROKE_JOIN_BEVEL) {
			/* bevel */
			art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
			                    ART_LINETO, vpath[i1].x - dlx0, vpath[i1].y - dly0);
			art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
			                    ART_LINETO, vpath[i1].x - dlx1, vpath[i1].y - dly1);
		} else if (join == ART_PATH_STROKE_JOIN_MITER) {
			art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
			                    ART_LINETO, vpath[i1].x - dmx, vpath[i1].y - dmy);
		} else if (join == ART_PATH_STROKE_JOIN_ROUND)
			art_svp_vpath_stroke_arc(p_forw, pn_forw, pn_forw_max,
			                         vpath[i1].x, vpath[i1].y,
			                         -dlx0, -dly0,
			                         -dlx1, -dly1,
			                         line_width,
			                         flatness);
	} else {
		/* right turn, rev is outside and forw is inside */

		if (
		    (dmr2 > EPSILON_2) &&
		    /* check that i1 - dm[xy] is inside i0-i1 rectangle */
		    (dx0 - dmx) * dx0 + (dy0 - dmy) * dy0 > 0 &&
		    /* and that i1 - dm[xy] is inside i1-i2 rectangle */
		    ((dx1 + dmx) * dx1 + (dy1 + dmy) * dy1 > 0)
#ifdef PEDANTIC_INNER
		    &&
		    /* check that i1 - dl[xy]1 is inside i0-i1 rectangle */
		    (dx0 - dlx1) * dx0 + (dy0 - dly1) * dy0 > 0 &&
		    /* and that i1 - dl[xy]0 is inside i1-i2 rectangle */
		    ((dx1 + dlx0) * dx1 + (dy1 + dly0) * dy1 > 0)
#endif
		) {
			/* can safely add single intersection point */
			art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
			                    ART_LINETO, vpath[i1].x - dmx, vpath[i1].y - dmy);
		} else {
			/* need to loop-de-loop the inside */
			art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
			                    ART_LINETO, vpath[i1].x - dlx0, vpath[i1].y - dly0);
			art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
			                    ART_LINETO, vpath[i1].x, vpath[i1].y);
			art_vpath_add_point(p_forw, pn_forw, pn_forw_max,
			                    ART_LINETO, vpath[i1].x - dlx1, vpath[i1].y - dly1);
		}

		if (join == ART_PATH_STROKE_JOIN_BEVEL) {
			/* bevel */
			art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
			                    ART_LINETO, vpath[i1].x + dlx0, vpath[i1].y + dly0);
			art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
			                    ART_LINETO, vpath[i1].x + dlx1, vpath[i1].y + dly1);
		} else if (join == ART_PATH_STROKE_JOIN_MITER) {
			art_vpath_add_point(p_rev, pn_rev, pn_rev_max,
			                    ART_LINETO, vpath[i1].x + dmx, vpath[i1].y + dmy);
		} else if (join == ART_PATH_STROKE_JOIN_ROUND)
			art_svp_vpath_stroke_arc(p_rev, pn_rev, pn_rev_max,
			                         vpath[i1].x, vpath[i1].y,
			                         dlx0, dly0,
			                         dlx1, dly1,
			                         -line_width,
			                         flatness);

	}
}

/* caps i1, under the assumption of a vector from i0 */
static void render_cap(ArtVpath **p_result, int *pn_result, int *pn_result_max,
           ArtVpath *vpath, int i0, int i1,
           ArtPathStrokeCapType cap, double line_width, double flatness) {
	double dx0, dy0;
	double dlx0, dly0;
	double scale;
	int n_pts;
	int i;

	dx0 = vpath[i1].x - vpath[i0].x;
	dy0 = vpath[i1].y - vpath[i0].y;

	/* Set dl[xy]0 to the vector from i0 to i1, rotated counterclockwise
	   90 degrees, and scaled to the length of line_width. */
	scale = line_width / sqrt(dx0 * dx0 + dy0 * dy0);
	dlx0 = dy0 * scale;
	dly0 = -dx0 * scale;

	switch (cap) {
	case ART_PATH_STROKE_CAP_BUTT:
		art_vpath_add_point(p_result, pn_result, pn_result_max,
		                    ART_LINETO, vpath[i1].x - dlx0, vpath[i1].y - dly0);
		art_vpath_add_point(p_result, pn_result, pn_result_max,
		                    ART_LINETO, vpath[i1].x + dlx0, vpath[i1].y + dly0);
		break;
	case ART_PATH_STROKE_CAP_ROUND:
		n_pts = (int)ceil(M_PI / (2.0 * M_SQRT2 * sqrt(flatness / line_width)));
		art_vpath_add_point(p_result, pn_result, pn_result_max,
		                    ART_LINETO, vpath[i1].x - dlx0, vpath[i1].y - dly0);
		for (i = 1; i < n_pts; i++) {
			double theta, c_th, s_th;

			theta = M_PI * i / n_pts;
			c_th = cos(theta);
			s_th = sin(theta);
			art_vpath_add_point(p_result, pn_result, pn_result_max,
			                    ART_LINETO,
			                    vpath[i1].x - dlx0 * c_th - dly0 * s_th,
			                    vpath[i1].y - dly0 * c_th + dlx0 * s_th);
		}
		art_vpath_add_point(p_result, pn_result, pn_result_max,
		                    ART_LINETO, vpath[i1].x + dlx0, vpath[i1].y + dly0);
		break;
	case ART_PATH_STROKE_CAP_SQUARE:
		art_vpath_add_point(p_result, pn_result, pn_result_max,
		                    ART_LINETO,
		                    vpath[i1].x - dlx0 - dly0,
		                    vpath[i1].y - dly0 + dlx0);
		art_vpath_add_point(p_result, pn_result, pn_result_max,
		                    ART_LINETO,
		                    vpath[i1].x + dlx0 - dly0,
		                    vpath[i1].y + dly0 + dlx0);
		break;
	}
}

/**
 * art_svp_from_vpath_raw: Stroke a vector path, raw version
 * @vpath: #ArtVPath to stroke.
 * @join: Join style.
 * @cap: Cap style.
 * @line_width: Width of stroke.
 * @miter_limit: Miter limit.
 * @flatness: Flatness.
 *
 * Exactly the same as art_svp_vpath_stroke(), except that the resulting
 * stroke outline may self-intersect and have regions of winding number
 * greater than 1.
 *
 * Return value: Resulting raw stroked outline in svp format.
 **/
ArtVpath *art_svp_vpath_stroke_raw(ArtVpath *vpath,
                         ArtPathStrokeJoinType join,
                         ArtPathStrokeCapType cap,
                         double line_width,
                         double miter_limit,
                         double flatness) {
	int begin_idx, end_idx;
	int i;
	ArtVpath *forw, *rev;
	int n_forw, n_rev;
	int n_forw_max, n_rev_max;
	ArtVpath *result;
	int n_result, n_result_max;
	double half_lw = 0.5 * line_width;
	int closed;
	int last, this_, next, second;
	double dx, dy;

	n_forw_max = 16;
	forw = art_new(ArtVpath, n_forw_max);

	n_rev_max = 16;
	rev = art_new(ArtVpath, n_rev_max);

	n_result = 0;
	n_result_max = 16;
	result = art_new(ArtVpath, n_result_max);

	for (begin_idx = 0; vpath[begin_idx].code != ART_END; begin_idx = end_idx) {
		n_forw = 0;
		n_rev = 0;

		closed = (vpath[begin_idx].code == ART_MOVETO);

		/* we don't know what the first point joins with until we get to the
		     last point and see if it's closed. So we start with the second
		     line in the path.

		     Note: this is not strictly true (we now know it's closed from
		     the opening pathcode), but why fix code that isn't broken?
		*/

		this_ = begin_idx;
		/* skip over identical points at the beginning of the subpath */
		for (i = this_ + 1; vpath[i].code == ART_LINETO; i++) {
			dx = vpath[i].x - vpath[this_].x;
			dy = vpath[i].y - vpath[this_].y;
			if (dx * dx + dy * dy > EPSILON_2)
				break;
		}
		next = i;
		second = next;

		/* invariant: this doesn't coincide with next */
		while (vpath[next].code == ART_LINETO) {
			last = this_;
			this_ = next;
			/* skip over identical points after the beginning of the subpath */
			for (i = this_ + 1; vpath[i].code == ART_LINETO; i++) {
				dx = vpath[i].x - vpath[this_].x;
				dy = vpath[i].y - vpath[this_].y;
				if (dx * dx + dy * dy > EPSILON_2)
					break;
			}
			next = i;
			if (vpath[next].code != ART_LINETO) {
				/* reached end of path */
				/* make "closed" detection conform to PostScript
				      semantics (i.e. explicit closepath code rather than
				      just the fact that end of the path is the beginning) */
				if (closed &&
				        vpath[this_].x == vpath[begin_idx].x &&
				        vpath[this_].y == vpath[begin_idx].y) {
					int j;

					/* path is closed, render join to beginning */
					render_seg(&forw, &n_forw, &n_forw_max,
					           &rev, &n_rev, &n_rev_max,
					           vpath, last, this_, second,
					           join, half_lw, miter_limit, flatness);

					/* do forward path */
					art_vpath_add_point(&result, &n_result, &n_result_max,
					                    ART_MOVETO, forw[n_forw - 1].x,
					                    forw[n_forw - 1].y);
					for (j = 0; j < n_forw; j++)
						art_vpath_add_point(&result, &n_result, &n_result_max,
						                    ART_LINETO, forw[j].x,
						                    forw[j].y);

					/* do reverse path, reversed */
					art_vpath_add_point(&result, &n_result, &n_result_max,
					                    ART_MOVETO, rev[0].x,
					                    rev[0].y);
					for (j = n_rev - 1; j >= 0; j--)
						art_vpath_add_point(&result, &n_result, &n_result_max,
						                    ART_LINETO, rev[j].x,
						                    rev[j].y);
				} else {
					/* path is open */
					int j;

					/* add to forw rather than result to ensure that
					   forw has at least one point. */
					render_cap(&forw, &n_forw, &n_forw_max,
					           vpath, last, this_,
					           cap, half_lw, flatness);
					art_vpath_add_point(&result, &n_result, &n_result_max,
					                    ART_MOVETO, forw[0].x,
					                    forw[0].y);
					for (j = 1; j < n_forw; j++)
						art_vpath_add_point(&result, &n_result, &n_result_max,
						                    ART_LINETO, forw[j].x,
						                    forw[j].y);
					for (j = n_rev - 1; j >= 0; j--)
						art_vpath_add_point(&result, &n_result, &n_result_max,
						                    ART_LINETO, rev[j].x,
						                    rev[j].y);
					render_cap(&result, &n_result, &n_result_max,
					           vpath, second, begin_idx,
					           cap, half_lw, flatness);
					art_vpath_add_point(&result, &n_result, &n_result_max,
					                    ART_LINETO, forw[0].x,
					                    forw[0].y);
				}
			} else
				render_seg(&forw, &n_forw, &n_forw_max,
				           &rev, &n_rev, &n_rev_max,
				           vpath, last, this_, next,
				           join, half_lw, miter_limit, flatness);
		}
		end_idx = next;
	}

	free(forw);
	free(rev);
	art_vpath_add_point(&result, &n_result, &n_result_max, ART_END, 0, 0);
	return result;
}


/* Render a vector path into a stroked outline.

   Status of this routine:

   Basic correctness: Only miter and bevel line joins are implemented,
   and only butt line caps. Otherwise, seems to be fine.

   Numerical stability: We cheat (adding random perturbation). Thus,
   it seems very likely that no numerical stability problems will be
   seen in practice.

   Speed: Should be pretty good.

   Precision: The perturbation fuzzes the coordinates slightly,
   but not enough to be visible.  */

/**
 * art_svp_vpath_stroke: Stroke a vector path.
 * @vpath: #ArtVPath to stroke.
 * @join: Join style.
 * @cap: Cap style.
 * @line_width: Width of stroke.
 * @miter_limit: Miter limit.
 * @flatness: Flatness.
 *
 * Computes an svp representing the stroked outline of @vpath. The
 * width of the stroked line is @line_width.
 *
 * Lines are joined according to the @join rule. Possible values are
 * ART_PATH_STROKE_JOIN_MITER (for mitered joins),
 * ART_PATH_STROKE_JOIN_ROUND (for round joins), and
 * ART_PATH_STROKE_JOIN_BEVEL (for bevelled joins). The mitered join
 * is converted to a bevelled join if the miter would extend to a
 * distance of more than @miter_limit * @line_width from the actual
 * join point.
 *
 * If there are open subpaths, the ends of these subpaths are capped
 * according to the @cap rule. Possible values are
 * ART_PATH_STROKE_CAP_BUTT (squared cap, extends exactly to end
 * point), ART_PATH_STROKE_CAP_ROUND (rounded half-circle centered at
 * the end point), and ART_PATH_STROKE_CAP_SQUARE (squared cap,
 * extending half @line_width past the end point).
 *
 * The @flatness parameter controls the accuracy of the rendering. It
 * is most important for determining the number of points to use to
 * approximate circular arcs for round lines and joins. In general, the
 * resulting vector path will be within @flatness pixels of the "ideal"
 * path containing actual circular arcs. I reserve the right to use
 * the @flatness parameter to convert bevelled joins to miters for very
 * small turn angles, as this would reduce the number of points in the
 * resulting outline path.
 *
 * The resulting path is "clean" with respect to self-intersections, i.e.
 * the winding number is 0 or 1 at each point.
 *
 * Return value: Resulting stroked outline in svp format.
 **/
ArtSVP *art_svp_vpath_stroke(ArtVpath *vpath,
                     ArtPathStrokeJoinType join,
                     ArtPathStrokeCapType cap,
                     double line_width,
                     double miter_limit,
                     double flatness) {
	ArtVpath *vpath_stroke;
	ArtSVP *svp, *svp2;
	ArtSvpWriter *swr;

	vpath_stroke = art_svp_vpath_stroke_raw(vpath, join, cap,
	                                        line_width, miter_limit, flatness);
	svp = art_svp_from_vpath(vpath_stroke);
	free(vpath_stroke);

	swr = art_svp_writer_rewind_new(ART_WIND_RULE_NONZERO);
	art_svp_intersector(svp, swr);

	svp2 = art_svp_writer_rewind_reap(swr);
	art_svp_free(svp);
	return svp2;
}


/* Testbed implementation of the new intersection code.
*/

typedef struct _ArtPriQ ArtPriQ;
typedef struct _ArtPriPoint ArtPriPoint;

struct _ArtPriQ {
	int n_items;
	int n_items_max;
	ArtPriPoint **items;
};

struct _ArtPriPoint {
	double x;
	double y;
	void *user_data;
};

static ArtPriQ *art_pri_new(void) {
	ArtPriQ *result = art_new(ArtPriQ, 1);
	if (!result)
		error("[art_pri_new] Cannot allocate memory");

	result->n_items = 0;
	result->n_items_max = 16;
	result->items = art_new(ArtPriPoint *, result->n_items_max);
	return result;
}

static void art_pri_free(ArtPriQ *pq) {
	free(pq->items);
	free(pq);
}

static bool art_pri_empty(ArtPriQ *pq) {
	return pq->n_items == 0;
}

/* This heap implementation is based on Vasek Chvatal's course notes:
   http://www.cs.rutgers.edu/~chvatal/notes/pq.html#heap */

static void art_pri_bubble_up(ArtPriQ *pq, int vacant, ArtPriPoint *missing) {
	ArtPriPoint **items = pq->items;
	int parent;

	parent = (vacant - 1) >> 1;
	while (vacant > 0 && (missing->y < items[parent]->y ||
	                      (missing->y == items[parent]->y &&
	                       missing->x < items[parent]->x))) {
		items[vacant] = items[parent];
		vacant = parent;
		parent = (vacant - 1) >> 1;
	}

	items[vacant] = missing;
}

static void art_pri_insert(ArtPriQ *pq, ArtPriPoint *point) {
	if (pq->n_items == pq->n_items_max)
		art_expand(pq->items, ArtPriPoint *, pq->n_items_max);

	art_pri_bubble_up(pq, pq->n_items++, point);
}

static void art_pri_sift_down_from_root(ArtPriQ *pq, ArtPriPoint *missing) {
	ArtPriPoint **items = pq->items;
	int vacant = 0, child = 2;
	int n = pq->n_items;

	while (child < n) {
		if (items[child - 1]->y < items[child]->y ||
		        (items[child - 1]->y == items[child]->y &&
		         items[child - 1]->x < items[child]->x))
			child--;
		items[vacant] = items[child];
		vacant = child;
		child = (vacant + 1) << 1;
	}
	if (child == n) {
		items[vacant] = items[n - 1];
		vacant = n - 1;
	}

	art_pri_bubble_up(pq, vacant, missing);
}

static ArtPriPoint *art_pri_choose(ArtPriQ *pq) {
	ArtPriPoint *result = pq->items[0];

	art_pri_sift_down_from_root(pq, pq->items[--pq->n_items]);
	return result;
}

/* A virtual class for an "svp writer". A client of this object creates an
   SVP by repeatedly calling "add segment" and "add point" methods on it.
*/

typedef struct _ArtSvpWriterRewind ArtSvpWriterRewind;

/* An implementation of the svp writer virtual class that applies the
   winding rule. */

struct _ArtSvpWriterRewind {
	ArtSvpWriter super;
	ArtWindRule rule;
	ArtSVP *svp;
	int n_segs_max;
	int *n_points_max;
};

static int art_svp_writer_rewind_add_segment(ArtSvpWriter *self, int wind_left,
                                  int delta_wind, double x, double y) {
	ArtSvpWriterRewind *swr = (ArtSvpWriterRewind *)self;
	ArtSVP *svp;
	ArtSVPSeg *seg;
	bool left_filled = 0, right_filled = 0;
	int wind_right = wind_left + delta_wind;
	int seg_num;
	const int init_n_points_max = 4;

	switch (swr->rule) {
	case ART_WIND_RULE_NONZERO:
		left_filled = (wind_left != 0);
		right_filled = (wind_right != 0);
		break;
	case ART_WIND_RULE_INTERSECT:
		left_filled = (wind_left > 1);
		right_filled = (wind_right > 1);
		break;
	case ART_WIND_RULE_ODDEVEN:
		left_filled = (wind_left & 1);
		right_filled = (wind_right & 1);
		break;
	case ART_WIND_RULE_POSITIVE:
		left_filled = (wind_left > 0);
		right_filled = (wind_right > 0);
		break;
	default:
		error("Unknown wind rule %d", swr->rule);
	}
	if (left_filled == right_filled) {
		/* discard segment now */
		return -1;
	}

	svp = swr->svp;
	seg_num = svp->n_segs++;
	if (swr->n_segs_max == seg_num) {
		swr->n_segs_max <<= 1;
		svp = (ArtSVP *)realloc(svp, sizeof(ArtSVP) +
		                            (swr->n_segs_max - 1) *
		                            sizeof(ArtSVPSeg));
		swr->svp = svp;
		int *tmp = art_renew(swr->n_points_max, int,
		                                        swr->n_segs_max);

		if (!tmp)
			error("Cannot reallocate memory in art_svp_writer_rewind_add_segment()");

		swr->n_points_max = tmp;
	}
	seg = &svp->segs[seg_num];
	seg->n_points = 1;
	seg->dir = right_filled;
	swr->n_points_max[seg_num] = init_n_points_max;
	seg->bbox.x0 = x;
	seg->bbox.y0 = y;
	seg->bbox.x1 = x;
	seg->bbox.y1 = y;
	seg->points = art_new(ArtPoint, init_n_points_max);
	if (!seg->points)
		error("[art_svp_writer_rewind_add_segment] Cannot allocate memory");

	seg->points[0].x = x;
	seg->points[0].y = y;
	return seg_num;
}

static void art_svp_writer_rewind_add_point(ArtSvpWriter *self, int seg_id,
                                double x, double y) {
	ArtSvpWriterRewind *swr = (ArtSvpWriterRewind *)self;
	ArtSVPSeg *seg;
	int n_points;

	if (seg_id < 0)
		/* omitted segment */
		return;

	seg = &swr->svp->segs[seg_id];
	n_points = seg->n_points++;
	if (swr->n_points_max[seg_id] == n_points)
		art_expand(seg->points, ArtPoint, swr->n_points_max[seg_id]);
	seg->points[n_points].x = x;
	seg->points[n_points].y = y;
	if (x < seg->bbox.x0)
		seg->bbox.x0 = x;
	if (x > seg->bbox.x1)
		seg->bbox.x1 = x;
	seg->bbox.y1 = y;
}

static void art_svp_writer_rewind_close_segment(ArtSvpWriter *self, int seg_id) {
	/* Not needed for this simple implementation. A potential future
	   optimization is to merge segments that can be merged safely. */
}

ArtSVP *art_svp_writer_rewind_reap(ArtSvpWriter *self) {
	ArtSvpWriterRewind *swr = (ArtSvpWriterRewind *)self;
	ArtSVP *result = swr->svp;

	free(swr->n_points_max);
	free(swr);
	return result;
}

ArtSvpWriter *art_svp_writer_rewind_new(ArtWindRule rule) {
	ArtSvpWriterRewind *result = art_new(ArtSvpWriterRewind, 1);
	if (!result)
		error("[art_svp_writer_rewind_new] Cannot allocate memory");

	result->super.add_segment = art_svp_writer_rewind_add_segment;
	result->super.add_point = art_svp_writer_rewind_add_point;
	result->super.close_segment = art_svp_writer_rewind_close_segment;

	result->rule = rule;
	result->n_segs_max = 16;
	result->svp = (ArtSVP *)malloc(sizeof(ArtSVP) +
	                                  (result->n_segs_max - 1) * sizeof(ArtSVPSeg));
	if (!result->svp)
		error("[art_svp_writer_rewind_new] Cannot allocate memory");

	result->svp->n_segs = 0;
	result->n_points_max = art_new(int, result->n_segs_max);

	return &result->super;
}

/* Now, data structures for the active list */

typedef struct _ArtActiveSeg ArtActiveSeg;

/* Note: BNEG is 1 for \ lines, and 0 for /. Thus,
   x[(flags & BNEG) ^ 1] <= x[flags & BNEG] */
#define ART_ACTIVE_FLAGS_BNEG 1

/* This flag is set if the segment has been inserted into the active
   list. */
#define ART_ACTIVE_FLAGS_IN_ACTIVE 2

/* This flag is set when the segment is to be deleted in the
   horiz commit process. */
#define ART_ACTIVE_FLAGS_DEL 4

/* This flag is set if the seg_id is a valid output segment. */
#define ART_ACTIVE_FLAGS_OUT 8

/* This flag is set if the segment is in the horiz list. */
#define ART_ACTIVE_FLAGS_IN_HORIZ 16

struct _ArtActiveSeg {
	int flags;
	int wind_left, delta_wind;
	ArtActiveSeg *left, *right; /* doubly linked list structure */

	const ArtSVPSeg *in_seg;
	int in_curs;

	double x[2];
	double y0, y1;
	double a, b, c; /* line equation; ax+by+c = 0 for the line, a^2 + b^2 = 1,
             and a>0 */

	/* bottom point and intersection point stack */
	int n_stack;
	int n_stack_max;
	ArtPoint *stack;

	/* horiz commit list */
	ArtActiveSeg *horiz_left, *horiz_right;
	double horiz_x;
	int horiz_delta_wind;
	int seg_id;
};

typedef struct _ArtIntersectCtx ArtIntersectCtx;

struct _ArtIntersectCtx {
	const ArtSVP *in;
	ArtSvpWriter *out;

	ArtPriQ *pq;

	ArtActiveSeg *active_head;

	double y;
	ArtActiveSeg *horiz_first;
	ArtActiveSeg *horiz_last;

	/* segment index of next input segment to be added to pri q */
	int in_curs;
};

#define EPSILON_A 1e-5 /* Threshold for breaking lines at point insertions */

/**
 * art_svp_intersect_setup_seg: Set up an active segment from input segment.
 * @seg: Active segment.
 * @pri_pt: Priority queue point to initialize.
 *
 * Sets the x[], a, b, c, flags, and stack fields according to the
 * line from the current cursor value. Sets the priority queue point
 * to the bottom point of this line. Also advances the input segment
 * cursor.
 **/
static void art_svp_intersect_setup_seg(ArtActiveSeg *seg, ArtPriPoint *pri_pt) {
	const ArtSVPSeg *in_seg = seg->in_seg;
	int in_curs = seg->in_curs++;
	double x0, y0, x1, y1;
	double dx, dy, s;
	double a, b, r2;

	x0 = in_seg->points[in_curs].x;
	y0 = in_seg->points[in_curs].y;
	x1 = in_seg->points[in_curs + 1].x;
	y1 = in_seg->points[in_curs + 1].y;
	pri_pt->x = x1;
	pri_pt->y = y1;
	dx = x1 - x0;
	dy = y1 - y0;
	r2 = dx * dx + dy * dy;
	s = r2 == 0 ? 1 : 1 / sqrt(r2);
	seg->a = a = dy * s;
	seg->b = b = -dx * s;
	seg->c = -(a * x0 + b * y0);
	seg->flags = (seg->flags & ~ART_ACTIVE_FLAGS_BNEG) | (dx > 0);
	seg->x[0] = x0;
	seg->x[1] = x1;
	seg->y0 = y0;
	seg->y1 = y1;
	seg->n_stack = 1;
	seg->stack[0].x = x1;
	seg->stack[0].y = y1;
}

/**
 * art_svp_intersect_add_horiz: Add point to horizontal list.
 * @ctx: Intersector context.
 * @seg: Segment with point to insert into horizontal list.
 *
 * Inserts @seg into horizontal list, keeping it in ascending horiz_x
 * order.
 *
 * Note: the horiz_commit routine processes "clusters" of segs in the
 * horiz list, all sharing the same horiz_x value. The cluster is
 * processed in active list order, rather than horiz list order. Thus,
 * the order of segs in the horiz list sharing the same horiz_x
 * _should_ be irrelevant. Even so, we use b as a secondary sorting key,
 * as a "belt and suspenders" defensive coding tactic.
 **/
static void art_svp_intersect_add_horiz(ArtIntersectCtx *ctx, ArtActiveSeg *seg) {
	ArtActiveSeg **pp = &ctx->horiz_last;
	ArtActiveSeg *place;
	ArtActiveSeg *place_right = NULL;

	if (seg->flags & ART_ACTIVE_FLAGS_IN_HORIZ) {
		warning("attempt to put segment in horiz list twice");
		return;
	}
	seg->flags |= ART_ACTIVE_FLAGS_IN_HORIZ;

	for (place = *pp; place != NULL && (place->horiz_x > seg->horiz_x ||
	                                    (place->horiz_x == seg->horiz_x &&
	                                     place->b < seg->b));
	        place = *pp) {
		place_right = place;
		pp = &place->horiz_left;
	}
	*pp = seg;
	seg->horiz_left = place;
	seg->horiz_right = place_right;
	if (place == NULL)
		ctx->horiz_first = seg;
	else
		place->horiz_right = seg;
}

static void art_svp_intersect_push_pt(ArtIntersectCtx *ctx, ArtActiveSeg *seg,
                          double x, double y) {
	ArtPriPoint *pri_pt;
	int n_stack = seg->n_stack;

	if (n_stack == seg->n_stack_max)
		art_expand(seg->stack, ArtPoint, seg->n_stack_max);
	seg->stack[n_stack].x = x;
	seg->stack[n_stack].y = y;
	seg->n_stack++;

	seg->x[1] = x;
	seg->y1 = y;

	pri_pt = art_new(ArtPriPoint, 1);
	if (!pri_pt)
		error("[art_svp_intersect_push_pt] Cannot allocate memory");

	pri_pt->x = x;
	pri_pt->y = y;
	pri_pt->user_data = seg;
	art_pri_insert(ctx->pq, pri_pt);
}

typedef enum {
	ART_BREAK_LEFT = 1,
	ART_BREAK_RIGHT = 2
} ArtBreakFlags;

/**
 * art_svp_intersect_break: Break an active segment.
 *
 * Note: y must be greater than the top point's y, and less than
 * the bottom's.
 *
 * Return value: x coordinate of break point.
 */
static double art_svp_intersect_break(ArtIntersectCtx *ctx, ArtActiveSeg *seg,
                        double x_ref, double y, ArtBreakFlags break_flags) {
	double x0, y0, x1, y1;
	const ArtSVPSeg *in_seg = seg->in_seg;
	int in_curs = seg->in_curs;
	double x;

	x0 = in_seg->points[in_curs - 1].x;
	y0 = in_seg->points[in_curs - 1].y;
	x1 = in_seg->points[in_curs].x;
	y1 = in_seg->points[in_curs].y;
	x = x0 + (x1 - x0) * ((y - y0) / (y1 - y0));
	if ((break_flags == ART_BREAK_LEFT && x > x_ref) ||
	        (break_flags == ART_BREAK_RIGHT && x < x_ref)) {
	}

	/* I think we can count on min(x0, x1) <= x <= max(x0, x1) with sane
	   arithmetic, but it might be worthwhile to check just in case. */

	if (y > ctx->y)
		art_svp_intersect_push_pt(ctx, seg, x, y);
	else {
		seg->x[0] = x;
		seg->y0 = y;
		seg->horiz_x = x;
		art_svp_intersect_add_horiz(ctx, seg);
	}

	return x;
}

/**
 * art_svp_intersect_add_point: Add a point, breaking nearby neighbors.
 * @ctx: Intersector context.
 * @x: X coordinate of point to add.
 * @y: Y coordinate of point to add.
 * @seg: "nearby" segment, or NULL if leftmost.
 *
 * Return value: Segment immediately to the left of the new point, or
 * NULL if the new point is leftmost.
 **/
static ArtActiveSeg *art_svp_intersect_add_point(ArtIntersectCtx *ctx, double x, double y,
                            ArtActiveSeg *seg, ArtBreakFlags break_flags) {
	ArtActiveSeg *left, *right;
	double x_min = x, x_max = x;
	bool left_live, right_live;
	double d;
	double new_x;
	ArtActiveSeg *test, *result = NULL;
	double x_test;

	left = seg;
	if (left == NULL)
		right = ctx->active_head;
	else
		right = left->right;
	left_live = (break_flags & ART_BREAK_LEFT) && (left != NULL);
	right_live = (break_flags & ART_BREAK_RIGHT) && (right != NULL);
	while (left_live || right_live) {
		if (left_live) {
			if (x <= left->x[left->flags & ART_ACTIVE_FLAGS_BNEG] &&
			        /* It may be that one of these conjuncts turns out to be always
			              true. We test both anyway, to be defensive. */
			        y != left->y0 && y < left->y1) {
				d = x_min * left->a + y * left->b + left->c;
				if (d < EPSILON_A) {
					new_x = art_svp_intersect_break(ctx, left, x_min, y,
					                                ART_BREAK_LEFT);
					if (new_x > x_max) {
						x_max = new_x;
						right_live = (right != NULL);
					} else if (new_x < x_min)
						x_min = new_x;
					left = left->left;
					left_live = (left != NULL);
				} else
					left_live = false;
			} else
				left_live = false;
		} else if (right_live) {
			if (x >= right->x[(right->flags & ART_ACTIVE_FLAGS_BNEG) ^ 1] &&
			        /* It may be that one of these conjuncts turns out to be always
			              true. We test both anyway, to be defensive. */
			        y != right->y0 && y < right->y1) {
				d = x_max * right->a + y * right->b + right->c;
				if (d > -EPSILON_A) {
					new_x = art_svp_intersect_break(ctx, right, x_max, y,
					                                ART_BREAK_RIGHT);
					if (new_x < x_min) {
						x_min = new_x;
						left_live = (left != NULL);
					} else if (new_x >= x_max)
						x_max = new_x;
					right = right->right;
					right_live = (right != NULL);
				} else
					right_live = false;
			} else
				right_live = false;
		}
	}

	/* Ascending order is guaranteed by break_flags. Thus, we don't need
	   to actually fix up non-ascending pairs. */

	/* Now, (left, right) defines an interval of segments broken. Sort
	   into ascending x order. */
	test = left == NULL ? ctx->active_head : left->right;
	result = left;
	if (test != NULL && test != right) {
		if (y == test->y0)
			x_test = test->x[0];
		else /* assert y == test->y1, I think */
			x_test = test->x[1];
		for (;;) {
			if (x_test <= x)
				result = test;
			test = test->right;
			if (test == right)
				break;
			new_x = x_test;
			if (new_x < x_test) {
				warning("art_svp_intersect_add_point: non-ascending x");
			}
			x_test = new_x;
		}
	}
	return result;
}

static void art_svp_intersect_swap_active(ArtIntersectCtx *ctx,
                              ArtActiveSeg *left_seg, ArtActiveSeg *right_seg) {
	right_seg->left = left_seg->left;
	if (right_seg->left != NULL)
		right_seg->left->right = right_seg;
	else
		ctx->active_head = right_seg;
	left_seg->right = right_seg->right;
	if (left_seg->right != NULL)
		left_seg->right->left = left_seg;
	left_seg->left = right_seg;
	right_seg->right = left_seg;
}

/**
 * art_svp_intersect_test_cross: Test crossing of a pair of active segments.
 * @ctx: Intersector context.
 * @left_seg: Left segment of the pair.
 * @right_seg: Right segment of the pair.
 * @break_flags: Flags indicating whether to break neighbors.
 *
 * Tests crossing of @left_seg and @right_seg. If there is a crossing,
 * inserts the intersection point into both segments.
 *
 * Return value: True if the intersection took place at the current
 * scan line, indicating further iteration is needed.
 **/
static bool art_svp_intersect_test_cross(ArtIntersectCtx *ctx,
                             ArtActiveSeg *left_seg, ArtActiveSeg *right_seg,
                             ArtBreakFlags break_flags) {
	double left_x0, left_y0, left_x1;
	double left_y1 = left_seg->y1;
	double right_y1 = right_seg->y1;
	double d;

	const ArtSVPSeg *in_seg;
	int in_curs;
	double d0, d1, t;
	double x, y; /* intersection point */

	if (left_seg->y0 == right_seg->y0 && left_seg->x[0] == right_seg->x[0]) {
		/* Top points of left and right segments coincide. This case
		     feels like a bit of duplication - we may want to merge it
		     with the cases below. However, this way, we're sure that this
		     logic makes only localized changes. */

		if (left_y1 < right_y1) {
			/* Test left (x1, y1) against right segment */
			left_x1 = left_seg->x[1];

			if (left_x1 <
			        right_seg->x[(right_seg->flags & ART_ACTIVE_FLAGS_BNEG) ^ 1] ||
			        left_y1 == right_seg->y0)
				return false;
			d = left_x1 * right_seg->a + left_y1 * right_seg->b + right_seg->c;
			if (d < -EPSILON_A)
				return false;
			else if (d < EPSILON_A) {
				/* I'm unsure about the break flags here. */
				double right_x1 = art_svp_intersect_break(ctx, right_seg,
				                  left_x1, left_y1,
				                  ART_BREAK_RIGHT);
				if (left_x1 <= right_x1)
					return false;
			}
		} else if (left_y1 > right_y1) {
			/* Test right (x1, y1) against left segment */
			double right_x1 = right_seg->x[1];

			if (right_x1 > left_seg->x[left_seg->flags & ART_ACTIVE_FLAGS_BNEG] ||
			        right_y1 == left_seg->y0)
				return false;
			d = right_x1 * left_seg->a + right_y1 * left_seg->b + left_seg->c;
			if (d > EPSILON_A)
				return false;
			else if (d > -EPSILON_A) {
				/* See above regarding break flags. */
				left_x1 = art_svp_intersect_break(ctx, left_seg,
				                 right_x1, right_y1,
				                 ART_BREAK_LEFT);
				if (left_x1 <= right_x1)
					return false;
			}
		} else { /* left_y1 == right_y1 */
			left_x1 = left_seg->x[1];
			double right_x1 = right_seg->x[1];

			if (left_x1 <= right_x1)
				return false;
		}
		art_svp_intersect_swap_active(ctx, left_seg, right_seg);
		return true;
	}

	if (left_y1 < right_y1) {
		/* Test left (x1, y1) against right segment */
		left_x1 = left_seg->x[1];

		if (left_x1 <
		        right_seg->x[(right_seg->flags & ART_ACTIVE_FLAGS_BNEG) ^ 1] ||
		        left_y1 == right_seg->y0)
			return false;
		d = left_x1 * right_seg->a + left_y1 * right_seg->b + right_seg->c;
		if (d < -EPSILON_A)
			return false;
		else if (d < EPSILON_A) {
			double right_x1 = art_svp_intersect_break(ctx, right_seg,
			                  left_x1, left_y1,
			                  ART_BREAK_RIGHT);
			if (left_x1 <= right_x1)
				return false;
		}
	} else if (left_y1 > right_y1) {
		/* Test right (x1, y1) against left segment */
		double right_x1 = right_seg->x[1];

		if (right_x1 > left_seg->x[left_seg->flags & ART_ACTIVE_FLAGS_BNEG] ||
		        right_y1 == left_seg->y0)
			return false;
		d = right_x1 * left_seg->a + right_y1 * left_seg->b + left_seg->c;
		if (d > EPSILON_A)
			return false;
		else if (d > -EPSILON_A) {
			left_x1 = art_svp_intersect_break(ctx, left_seg,
			                 right_x1, right_y1,
			                 ART_BREAK_LEFT);
			if (left_x1 <= right_x1)
				return false;
		}
	} else { /* left_y1 == right_y1 */
		left_x1 = left_seg->x[1];
		double right_x1 = right_seg->x[1];

		if (left_x1 <= right_x1)
			return false;
	}

	/* The segments cross. Find the intersection point. */

	in_seg = left_seg->in_seg;
	in_curs = left_seg->in_curs;
	left_x0 = in_seg->points[in_curs - 1].x;
	left_y0 = in_seg->points[in_curs - 1].y;
	left_x1 = in_seg->points[in_curs].x;
	left_y1 = in_seg->points[in_curs].y;
	d0 = left_x0 * right_seg->a + left_y0 * right_seg->b + right_seg->c;
	d1 = left_x1 * right_seg->a + left_y1 * right_seg->b + right_seg->c;
	if (d0 == d1) {
		x = left_x0;
		y = left_y0;
	} else {
		/* Is this division always safe? It could possibly overflow. */
		t = d0 / (d0 - d1);
		if (t <= 0) {
			x = left_x0;
			y = left_y0;
		} else if (t >= 1) {
			x = left_x1;
			y = left_y1;
		} else {
			x = left_x0 + t * (left_x1 - left_x0);
			y = left_y0 + t * (left_y1 - left_y0);
		}
	}

	/* Make sure intersection point is within bounds of right seg. */
	if (y < right_seg->y0) {
		x = right_seg->x[0];
		y = right_seg->y0;
	} else if (y > right_seg->y1) {
		x = right_seg->x[1];
		y = right_seg->y1;
	} else if (x < right_seg->x[(right_seg->flags & ART_ACTIVE_FLAGS_BNEG) ^ 1])
		x = right_seg->x[(right_seg->flags & ART_ACTIVE_FLAGS_BNEG) ^ 1];
	else if (x > right_seg->x[right_seg->flags & ART_ACTIVE_FLAGS_BNEG])
		x = right_seg->x[right_seg->flags & ART_ACTIVE_FLAGS_BNEG];

	if (y == left_seg->y0) {
		if (y != right_seg->y0) {
			art_svp_intersect_push_pt(ctx, right_seg, x, y);
			if ((break_flags & ART_BREAK_RIGHT) && right_seg->right != NULL)
				art_svp_intersect_add_point(ctx, x, y, right_seg->right,
				                            break_flags);
		} else {
			/* Intersection takes place at current scan line; process
			   immediately rather than queueing intersection point into
			   priq. */
			ArtActiveSeg *winner, *loser;

			/* Choose "most vertical" segement */
			if (left_seg->a > right_seg->a) {
				winner = left_seg;
				loser = right_seg;
			} else {
				winner = right_seg;
				loser = left_seg;
			}

			loser->x[0] = winner->x[0];
			loser->horiz_x = loser->x[0];
			loser->horiz_delta_wind += loser->delta_wind;
			winner->horiz_delta_wind -= loser->delta_wind;

			art_svp_intersect_swap_active(ctx, left_seg, right_seg);
			return true;
		}
	} else if (y == right_seg->y0) {
		art_svp_intersect_push_pt(ctx, left_seg, x, y);
		if ((break_flags & ART_BREAK_LEFT) && left_seg->left != NULL)
			art_svp_intersect_add_point(ctx, x, y, left_seg->left,
			                            break_flags);
	} else {
		/* Insert the intersection point into both segments. */
		art_svp_intersect_push_pt(ctx, left_seg, x, y);
		art_svp_intersect_push_pt(ctx, right_seg, x, y);
		if ((break_flags & ART_BREAK_LEFT) && left_seg->left != NULL)
			art_svp_intersect_add_point(ctx, x, y, left_seg->left, break_flags);
		if ((break_flags & ART_BREAK_RIGHT) && right_seg->right != NULL)
			art_svp_intersect_add_point(ctx, x, y, right_seg->right, break_flags);
	}
	return false;
}

/**
 * art_svp_intersect_active_delete: Delete segment from active list.
 * @ctx: Intersection context.
 * @seg: Segment to delete.
 *
 * Deletes @seg from the active list.
 **/
static void art_svp_intersect_active_delete(ArtIntersectCtx *ctx, ArtActiveSeg *seg) {
	ArtActiveSeg *left = seg->left, *right = seg->right;

	if (left != NULL)
		left->right = right;
	else
		ctx->active_head = right;
	if (right != NULL)
		right->left = left;
}

/**
 * art_svp_intersect_active_free: Free an active segment.
 * @seg: Segment to delete.
 *
 * Frees @seg.
 **/
static void art_svp_intersect_active_free(ArtActiveSeg *seg) {
	free(seg->stack);
	free(seg);
}

/**
 * art_svp_intersect_insert_cross: Test crossings of newly inserted line.
 *
 * Tests @seg against its left and right neighbors for intersections.
 * Precondition: the line in @seg is not purely horizontal.
 **/
static void art_svp_intersect_insert_cross(ArtIntersectCtx *ctx,
                               ArtActiveSeg *seg) {
	ArtActiveSeg *left = seg, *right = seg;

	for (;;) {
		if (left != NULL) {
			ArtActiveSeg *leftc;

			for (leftc = left->left; leftc != NULL; leftc = leftc->left)
				if (!(leftc->flags & ART_ACTIVE_FLAGS_DEL))
					break;
			if (leftc != NULL &&
			        art_svp_intersect_test_cross(ctx, leftc, left,
			                                     ART_BREAK_LEFT)) {
				if (left == right || right == NULL)
					right = left->right;
			} else {
				left = NULL;
			}
		} else if (right != NULL && right->right != NULL) {
			ArtActiveSeg *rightc;

			for (rightc = right->right; rightc != NULL; rightc = rightc->right)
				if (!(rightc->flags & ART_ACTIVE_FLAGS_DEL))
					break;
			if (rightc != NULL &&
			        art_svp_intersect_test_cross(ctx, right, rightc,
			                                     ART_BREAK_RIGHT)) {
				if (left == right || left == NULL)
					left = right->left;
			} else {
				right = NULL;
			}
		} else
			break;
	}
}

/**
 * art_svp_intersect_horiz: Add horizontal line segment.
 * @ctx: Intersector context.
 * @seg: Segment on which to add horizontal line.
 * @x0: Old x position.
 * @x1: New x position.
 *
 * Adds a horizontal line from @x0 to @x1, and updates the current
 * location of @seg to @x1.
 **/
static void art_svp_intersect_horiz(ArtIntersectCtx *ctx, ArtActiveSeg *seg,
                        double x0, double x1) {
	ArtActiveSeg *hs;

	if (x0 == x1)
		return;

	hs = art_new(ArtActiveSeg, 1);
	if (!hs)
		error("[art_svp_intersect_horiz] Cannot allocate memory");

	hs->flags = ART_ACTIVE_FLAGS_DEL | (seg->flags & ART_ACTIVE_FLAGS_OUT);
	if (seg->flags & ART_ACTIVE_FLAGS_OUT) {
		ArtSvpWriter *swr = ctx->out;

		swr->add_point(swr, seg->seg_id, x0, ctx->y);
	}
	hs->seg_id = seg->seg_id;
	hs->horiz_x = x0;
	hs->horiz_delta_wind = seg->delta_wind;
	hs->stack = NULL;

	/* Ideally, the (a, b, c) values will never be read. However, there
	   are probably some tests remaining that don't check for _DEL
	   before evaluating the line equation. For those, these
	   initializations will at least prevent a UMR of the values, which
	   can crash on some platforms. */
	hs->a = 0.0;
	hs->b = 0.0;
	hs->c = 0.0;

	seg->horiz_delta_wind -= seg->delta_wind;

	art_svp_intersect_add_horiz(ctx, hs);

	if (x0 > x1) {
		ArtActiveSeg *left;
		bool first = true;

		for (left = seg->left; left != NULL; left = seg->left) {
			int left_bneg = left->flags & ART_ACTIVE_FLAGS_BNEG;

			if (left->x[left_bneg] <= x1)
				break;
			if (left->x[left_bneg ^ 1] <= x1 &&
			        x1 *left->a + ctx->y *left->b + left->c >= 0)
				break;
			if (left->y0 != ctx->y && left->y1 != ctx->y) {
				art_svp_intersect_break(ctx, left, x1, ctx->y, ART_BREAK_LEFT);
			}
			art_svp_intersect_swap_active(ctx, left, seg);
			if (first && left->right != NULL) {
				art_svp_intersect_test_cross(ctx, left, left->right,
				                             ART_BREAK_RIGHT);
				first = false;
			}
		}
	} else {
		ArtActiveSeg *right;
		bool first = true;

		for (right = seg->right; right != NULL; right = seg->right) {
			int right_bneg = right->flags & ART_ACTIVE_FLAGS_BNEG;

			if (right->x[right_bneg ^ 1] >= x1)
				break;
			if (right->x[right_bneg] >= x1 &&
			        x1 *right->a + ctx->y *right->b + right->c <= 0)
				break;
			if (right->y0 != ctx->y && right->y1 != ctx->y) {
				art_svp_intersect_break(ctx, right, x1, ctx->y,
				                        ART_BREAK_LEFT);
			}
			art_svp_intersect_swap_active(ctx, seg, right);
			if (first && right->left != NULL) {
				art_svp_intersect_test_cross(ctx, right->left, right,
				                             ART_BREAK_RIGHT);
				first = false;
			}
		}
	}

	seg->x[0] = x1;
	seg->x[1] = x1;
	seg->horiz_x = x1;
	seg->flags &= ~ART_ACTIVE_FLAGS_OUT;
}

/**
 * art_svp_intersect_insert_line: Insert a line into the active list.
 * @ctx: Intersector context.
 * @seg: Segment containing line to insert.
 *
 * Inserts the line into the intersector context, taking care of any
 * intersections, and adding the appropriate horizontal points to the
 * active list.
 **/
static void art_svp_intersect_insert_line(ArtIntersectCtx *ctx, ArtActiveSeg *seg) {
	if (seg->y1 == seg->y0) {
		art_svp_intersect_horiz(ctx, seg, seg->x[0], seg->x[1]);
	} else {
		art_svp_intersect_insert_cross(ctx, seg);
		art_svp_intersect_add_horiz(ctx, seg);
	}
}

static void art_svp_intersect_process_intersection(ArtIntersectCtx *ctx,
                                       ArtActiveSeg *seg) {
	int n_stack = --seg->n_stack;
	seg->x[1] = seg->stack[n_stack - 1].x;
	seg->y1 = seg->stack[n_stack - 1].y;
	seg->x[0] = seg->stack[n_stack].x;
	seg->y0 = seg->stack[n_stack].y;
	seg->horiz_x = seg->x[0];
	art_svp_intersect_insert_line(ctx, seg);
}

static void art_svp_intersect_advance_cursor(ArtIntersectCtx *ctx, ArtActiveSeg *seg,
                                 ArtPriPoint *pri_pt) {
	const ArtSVPSeg *in_seg = seg->in_seg;
	int in_curs = seg->in_curs;
	ArtSvpWriter *swr = seg->flags & ART_ACTIVE_FLAGS_OUT ? ctx->out : NULL;

	if (swr != NULL)
		swr->add_point(swr, seg->seg_id, seg->x[1], seg->y1);
	if (in_curs + 1 == in_seg->n_points) {
		ArtActiveSeg *left = seg->left, *right = seg->right;

		seg->flags |= ART_ACTIVE_FLAGS_DEL;
		art_svp_intersect_add_horiz(ctx, seg);
		art_svp_intersect_active_delete(ctx, seg);
		if (left != NULL && right != NULL)
			art_svp_intersect_test_cross(ctx, left, right,
			                             (ArtBreakFlags)(ART_BREAK_LEFT | ART_BREAK_RIGHT));
		free(pri_pt);
	} else {
		seg->horiz_x = seg->x[1];

		art_svp_intersect_setup_seg(seg, pri_pt);
		art_pri_insert(ctx->pq, pri_pt);
		art_svp_intersect_insert_line(ctx, seg);
	}
}

static void art_svp_intersect_add_seg(ArtIntersectCtx *ctx, const ArtSVPSeg *in_seg) {
	ArtActiveSeg *seg = art_new(ArtActiveSeg, 1);
	ArtActiveSeg *test;
	double x0, y0;
	ArtActiveSeg *last = NULL;
	ArtActiveSeg *left, *right;
	ArtPriPoint *pri_pt = art_new(ArtPriPoint, 1);
	if (!pri_pt)
		error("[art_svp_intersect_add_seg] Cannot allocate memory");

	seg->flags = 0;
	seg->in_seg = in_seg;
	seg->in_curs = 0;

	seg->n_stack_max = 4;
	seg->stack = art_new(ArtPoint, seg->n_stack_max);

	seg->horiz_delta_wind = 0;

	seg->wind_left = 0;

	pri_pt->user_data = seg;
	art_svp_intersect_setup_seg(seg, pri_pt);
	art_pri_insert(ctx->pq, pri_pt);

	/* Find insertion place for new segment */
	/* This is currently a left-to-right scan, but should be replaced
	   with a binary search as soon as it's validated. */

	x0 = in_seg->points[0].x;
	y0 = in_seg->points[0].y;
	for (test = ctx->active_head; test != NULL; test = test->right) {
		double d;
		int test_bneg = test->flags & ART_ACTIVE_FLAGS_BNEG;

		if (x0 < test->x[test_bneg]) {
			if (x0 < test->x[test_bneg ^ 1])
				break;
			d = x0 * test->a + y0 * test->b + test->c;
			if (d < 0)
				break;
		}
		last = test;
	}

	left = art_svp_intersect_add_point(ctx, x0, y0, last, (ArtBreakFlags)(ART_BREAK_LEFT | ART_BREAK_RIGHT));
	seg->left = left;
	if (left == NULL) {
		right = ctx->active_head;
		ctx->active_head = seg;
	} else {
		right = left->right;
		left->right = seg;
	}
	seg->right = right;
	if (right != NULL)
		right->left = seg;

	seg->delta_wind = in_seg->dir ? 1 : -1;
	seg->horiz_x = x0;

	art_svp_intersect_insert_line(ctx, seg);
}

/**
 * art_svp_intersect_horiz_commit: Commit points in horiz list to output.
 * @ctx: Intersection context.
 *
 * The main function of the horizontal commit is to output new
 * points to the output writer.
 *
 * This "commit" pass is also where winding numbers are assigned,
 * because doing it here provides much greater tolerance for inputs
 * which are not in strict SVP order.
 *
 * Each cluster in the horiz_list contains both segments that are in
 * the active list (ART_ACTIVE_FLAGS_DEL is false) and that are not,
 * and are scheduled to be deleted (ART_ACTIVE_FLAGS_DEL is true). We
 * need to deal with both.
 **/
static void art_svp_intersect_horiz_commit(ArtIntersectCtx *ctx) {
	ArtActiveSeg *seg;
	int winding_number = 0; /* initialization just to avoid warning */
	int horiz_wind = 0;
	double last_x = 0; /* initialization just to avoid warning */

	/* Output points to svp writer. */
	for (seg = ctx->horiz_first; seg != NULL;) {
		/* Find a cluster with common horiz_x, */
		ArtActiveSeg *curs;
		double x = seg->horiz_x;

		/* Generate any horizontal segments. */
		if (horiz_wind != 0) {
			ArtSvpWriter *swr = ctx->out;
			int seg_id;

			seg_id = swr->add_segment(swr, winding_number, horiz_wind,
			                          last_x, ctx->y);
			swr->add_point(swr, seg_id, x, ctx->y);
			swr->close_segment(swr, seg_id);
		}

		/* Find first active segment in cluster. */

		for (curs = seg; curs != NULL && curs->horiz_x == x;
		        curs = curs->horiz_right)
			if (!(curs->flags & ART_ACTIVE_FLAGS_DEL))
				break;

		if (curs != NULL && curs->horiz_x == x) {
			/* There exists at least one active segment in this cluster. */

			/* Find beginning of cluster. */
			for (; curs->left != NULL; curs = curs->left)
				if (curs->left->horiz_x != x)
					break;

			if (curs->left != NULL)
				winding_number = curs->left->wind_left + curs->left->delta_wind;
			else
				winding_number = 0;

			do {
				if (!(curs->flags & ART_ACTIVE_FLAGS_OUT) ||
				        curs->wind_left != winding_number) {
					ArtSvpWriter *swr = ctx->out;

					if (curs->flags & ART_ACTIVE_FLAGS_OUT) {
						swr->add_point(swr, curs->seg_id,
						               curs->horiz_x, ctx->y);
						swr->close_segment(swr, curs->seg_id);
					}

					curs->seg_id = swr->add_segment(swr, winding_number,
					                                curs->delta_wind,
					                                x, ctx->y);
					curs->flags |= ART_ACTIVE_FLAGS_OUT;
				}
				curs->wind_left = winding_number;
				winding_number += curs->delta_wind;
				curs = curs->right;
			} while (curs != NULL && curs->horiz_x == x);
		}

		/* Skip past cluster. */
		do {
			ArtActiveSeg *next = seg->horiz_right;

			seg->flags &= ~ART_ACTIVE_FLAGS_IN_HORIZ;
			horiz_wind += seg->horiz_delta_wind;
			seg->horiz_delta_wind = 0;
			if (seg->flags & ART_ACTIVE_FLAGS_DEL) {
				if (seg->flags & ART_ACTIVE_FLAGS_OUT) {
					ArtSvpWriter *swr = ctx->out;
					swr->close_segment(swr, seg->seg_id);
				}
				art_svp_intersect_active_free(seg);
			}
			seg = next;
		} while (seg != NULL && seg->horiz_x == x);

		last_x = x;
	}
	ctx->horiz_first = NULL;
	ctx->horiz_last = NULL;
}

void art_svp_intersector(const ArtSVP *in, ArtSvpWriter *out) {
	ArtIntersectCtx *ctx;
	ArtPriQ *pq;
	ArtPriPoint *first_point;

	if (in->n_segs == 0)
		return;

	ctx = art_new(ArtIntersectCtx, 1);
	if (!ctx)
		error("[art_svp_intersector] Cannot allocate memory");

	ctx->in = in;
	ctx->out = out;
	pq = art_pri_new();
	ctx->pq = pq;

	ctx->active_head = NULL;

	ctx->horiz_first = NULL;
	ctx->horiz_last = NULL;

	ctx->in_curs = 0;
	first_point = art_new(ArtPriPoint, 1);
	if (!first_point)
		error("[art_svp_intersector] Cannot allocate memory");

	first_point->x = in->segs[0].points[0].x;
	first_point->y = in->segs[0].points[0].y;
	first_point->user_data = NULL;
	ctx->y = first_point->y;
	art_pri_insert(pq, first_point);

	while (!art_pri_empty(pq)) {
		ArtPriPoint *pri_point = art_pri_choose(pq);
		ArtActiveSeg *seg = (ArtActiveSeg *)pri_point->user_data;

		if (ctx->y != pri_point->y) {
			art_svp_intersect_horiz_commit(ctx);
			ctx->y = pri_point->y;
		}

		if (seg == NULL) {
			/* Insert new segment from input */
			const ArtSVPSeg *in_seg = &in->segs[ctx->in_curs++];
			art_svp_intersect_add_seg(ctx, in_seg);
			if (ctx->in_curs < in->n_segs) {
				const ArtSVPSeg *next_seg = &in->segs[ctx->in_curs];
				pri_point->x = next_seg->points[0].x;
				pri_point->y = next_seg->points[0].y;
				/* user_data is already NULL */
				art_pri_insert(pq, pri_point);
			} else
				free(pri_point);
		} else {
			int n_stack = seg->n_stack;

			if (n_stack > 1) {
				art_svp_intersect_process_intersection(ctx, seg);
				free(pri_point);
			} else {
				art_svp_intersect_advance_cursor(ctx, seg, pri_point);
			}
		}
	}

	art_svp_intersect_horiz_commit(ctx);

	art_pri_free(pq);
	free(ctx);
}


/* The spiffy antialiased renderer for sorted vector paths. */

typedef double artfloat;

struct ArtSVPRenderAAIter {
	const ArtSVP *svp;
	int x0, x1;
	int y;
	int seg_ix;

	int *active_segs;
	int n_active_segs;
	int *cursor;
	artfloat *seg_x;
	artfloat *seg_dx;

	ArtSVPRenderAAStep *steps;
};

static void art_svp_render_insert_active(int i, int *active_segs, int n_active_segs,
                             artfloat *seg_x, artfloat *seg_dx) {
	int j;
	artfloat x;
	int tmp1, tmp2;

	/* this is a cheap hack to get ^'s sorted correctly */
	x = seg_x[i] + 0.001 * seg_dx[i];
	for (j = 0; j < n_active_segs && seg_x[active_segs[j]] < x; j++)
		;

	tmp1 = i;
	while (j < n_active_segs) {
		tmp2 = active_segs[j];
		active_segs[j] = tmp1;
		tmp1 = tmp2;
		j++;
	}
	active_segs[j] = tmp1;
}

static void art_svp_render_delete_active(int *active_segs, int j, int n_active_segs) {
	int k;

	for (k = j; k < n_active_segs; k++)
		active_segs[k] = active_segs[k + 1];
}

/* Render the sorted vector path in the given rectangle, antialiased.

   This interface uses a callback for the actual pixel rendering. The
   callback is called y1 - y0 times (once for each scan line). The y
   coordinate is given as an argument for convenience (it could be
   stored in the callback's private data and incremented on each
   call).

   The rendered polygon is represented in a semi-runlength format: a
   start value and a sequence of "steps". Each step has an x
   coordinate and a value delta. The resulting value at position x is
   equal to the sum of the start value and all step delta values for
   which the step x coordinate is less than or equal to x. An
   efficient algorithm will traverse the steps left to right, keeping
   a running sum.

   All x coordinates in the steps are guaranteed to be x0 <= x < x1.
   (This guarantee is a change from the gfonted vpaar renderer, and is
   designed to simplify the callback).

   There is now a further guarantee that no two steps will have the
   same x value. This may allow for further speedup and simplification
   of renderers.

   The value 0x8000 represents 0% coverage by the polygon, while
   0xff8000 represents 100% coverage. This format is designed so that
   >> 16 results in a standard 0x00..0xff value range, with nice
   rounding.

   Status of this routine:

   Basic correctness: OK

   Numerical stability: pretty good, although probably not
   bulletproof.

   Speed: Needs more aggressive culling of bounding boxes.  Can
   probably speed up the [x0,x1) clipping of step values.  Can do more
   of the step calculation in fixed point.

   Precision: No known problems, although it should be tested
   thoroughly, especially for symmetry.

*/

ArtSVPRenderAAIter *art_svp_render_aa_iter(const ArtSVP *svp,
                       int x0, int y0, int x1, int y1) {
	ArtSVPRenderAAIter *iter = art_new(ArtSVPRenderAAIter, 1);
	if (!iter)
		error("[art_svp_render_aa_iter] Cannot allocate memory");

	iter->svp = svp;
	iter->y = y0;
	iter->x0 = x0;
	iter->x1 = x1;
	iter->seg_ix = 0;

	iter->active_segs = art_new(int, svp->n_segs);
	iter->cursor = art_new(int, svp->n_segs);
	iter->seg_x = art_new(artfloat, svp->n_segs);
	iter->seg_dx = art_new(artfloat, svp->n_segs);
	iter->steps = art_new(ArtSVPRenderAAStep, x1 - x0);
	iter->n_active_segs = 0;

	return iter;
}

#define ADD_STEP(xpos, xdelta)                            \
	/* stereotype code fragment for adding a step */      \
	if (n_steps == 0 || steps[n_steps - 1].x < xpos) {    \
		sx = n_steps;                                     \
		steps[sx].x = xpos;                               \
		steps[sx].delta = xdelta;                         \
		n_steps++;                                        \
	} else {                                              \
		for (sx = n_steps; sx > 0; sx--) {                \
			if (steps[sx - 1].x == xpos) {                \
				steps[sx - 1].delta += xdelta;            \
				sx = n_steps;                             \
				break;                                    \
			} else if (steps[sx - 1].x < xpos) {          \
				break;                                    \
			}                                             \
		}                                                 \
		if (sx < n_steps) {                               \
			memmove (&steps[sx + 1], &steps[sx],          \
			         (n_steps - sx) * sizeof(steps[0]));  \
			steps[sx].x = xpos;                           \
			steps[sx].delta = xdelta;                     \
			n_steps++;                                    \
		}                                                 \
	}

void art_svp_render_aa_iter_step(ArtSVPRenderAAIter *iter, int *p_start,
                            ArtSVPRenderAAStep **p_steps, int *p_n_steps) {
	const ArtSVP *svp = iter->svp;
	int *active_segs = iter->active_segs;
	int n_active_segs = iter->n_active_segs;
	int *cursor = iter->cursor;
	artfloat *seg_x = iter->seg_x;
	artfloat *seg_dx = iter->seg_dx;
	int i = iter->seg_ix;
	int j;
	int x0 = iter->x0;
	int x1 = iter->x1;
	int y = iter->y;
	int seg_index;

	int x;
	ArtSVPRenderAAStep *steps = iter->steps;
	int n_steps;
	artfloat y_top, y_bot;
	artfloat x_top, x_bot;
	artfloat x_min, x_max;
	int ix_min, ix_max;
	artfloat delta; /* delta should be int too? */
	int last, this_;
	int xdelta;
	artfloat rslope, drslope;
	int start;
	const ArtSVPSeg *seg;
	int curs;
	artfloat dy;

	int sx;

	/* insert new active segments */
	for (; i < svp->n_segs && svp->segs[i].bbox.y0 < y + 1; i++) {
		if (svp->segs[i].bbox.y1 > y &&
		        svp->segs[i].bbox.x0 < x1) {
			seg = &svp->segs[i];
			/* move cursor to topmost vector which overlaps [y,y+1) */
			for (curs = 0; seg->points[curs + 1].y < y; curs++)
				;
			cursor[i] = curs;
			dy = seg->points[curs + 1].y - seg->points[curs].y;
			if (fabs(dy) >= EPSILON_6)
				seg_dx[i] = (seg->points[curs + 1].x - seg->points[curs].x) /
				            dy;
			else
				seg_dx[i] = 1e12;
			seg_x[i] = seg->points[curs].x +
			           (y - seg->points[curs].y) * seg_dx[i];
			art_svp_render_insert_active(i, active_segs, n_active_segs++,
			                             seg_x, seg_dx);
		}
	}

	n_steps = 0;

	/* render the runlengths, advancing and deleting as we go */
	start = 0x8000;

	for (j = 0; j < n_active_segs; j++) {
		seg_index = active_segs[j];
		seg = &svp->segs[seg_index];
		curs = cursor[seg_index];
		while (curs != seg->n_points - 1 &&
		        seg->points[curs].y < y + 1) {
			y_top = y;
			if (y_top < seg->points[curs].y)
				y_top = seg->points[curs].y;
			y_bot = y + 1;
			if (y_bot > seg->points[curs + 1].y)
				y_bot = seg->points[curs + 1].y;
			if (y_top != y_bot) {
				delta = (seg->dir ? 16711680.0 : -16711680.0) *
				        (y_bot - y_top);
				x_top = seg_x[seg_index] + (y_top - y) * seg_dx[seg_index];
				x_bot = seg_x[seg_index] + (y_bot - y) * seg_dx[seg_index];
				if (x_top < x_bot) {
					x_min = x_top;
					x_max = x_bot;
				} else {
					x_min = x_bot;
					x_max = x_top;
				}
				ix_min = (int)floor(x_min);
				ix_max = (int)floor(x_max);
				if (ix_min >= x1) {
					/* skip; it starts to the right of the render region */
				} else if (ix_max < x0)
					/* it ends to the left of the render region */
					start += (int)delta;
				else if (ix_min == ix_max) {
					/* case 1, antialias a single pixel */
					xdelta = (int)((ix_min + 1 - (x_min + x_max) * 0.5) * delta);

					ADD_STEP(ix_min, xdelta)

					if (ix_min + 1 < x1) {
						xdelta = (int)(delta - xdelta);

						ADD_STEP(ix_min + 1, xdelta)
					}
				} else {
					/* case 2, antialias a run */
					rslope = 1.0 / fabs(seg_dx[seg_index]);
					drslope = delta * rslope;
					last =
					    (int)(drslope * 0.5 *
					    (ix_min + 1 - x_min) * (ix_min + 1 - x_min));
					xdelta = last;
					if (ix_min >= x0) {
						ADD_STEP(ix_min, xdelta)

						x = ix_min + 1;
					} else {
						start += last;
						x = x0;
					}
					if (ix_max > x1)
						ix_max = x1;
					for (; x < ix_max; x++) {
						this_ = (int)((seg->dir ? 16711680.0 : -16711680.0) * rslope *
						        (x + 0.5 - x_min));
						xdelta = this_ - last;
						last = this_;

						ADD_STEP(x, xdelta)
					}
					if (x < x1) {
						this_ =
						    (int)(delta * (1 - 0.5 *
						             (x_max - ix_max) * (x_max - ix_max) *
						             rslope));
						xdelta = this_ - last;
						last = this_;

						ADD_STEP(x, xdelta)

						if (x + 1 < x1) {
							xdelta = (int)(delta - last);

							ADD_STEP(x + 1, xdelta)
						}
					}
				}
			}
			curs++;
			if (curs != seg->n_points - 1 &&
			        seg->points[curs].y < y + 1) {
				dy = seg->points[curs + 1].y - seg->points[curs].y;
				if (fabs(dy) >= EPSILON_6)
					seg_dx[seg_index] = (seg->points[curs + 1].x -
					                     seg->points[curs].x) / dy;
				else
					seg_dx[seg_index] = 1e12;
				seg_x[seg_index] = seg->points[curs].x +
				                   (y - seg->points[curs].y) * seg_dx[seg_index];
			}
			/* break here, instead of duplicating predicate in while? */
		}
		if (seg->points[curs].y >= y + 1) {
			curs--;
			cursor[seg_index] = curs;
			seg_x[seg_index] += seg_dx[seg_index];
		} else {
			art_svp_render_delete_active(active_segs, j--,
			                             --n_active_segs);
		}
	}

	*p_start = start;
	*p_steps = steps;
	*p_n_steps = n_steps;

	iter->seg_ix = i;
	iter->n_active_segs = n_active_segs;
	iter->y++;
}

void art_svp_render_aa_iter_done(ArtSVPRenderAAIter *iter) {
	free(iter->steps);

	free(iter->seg_dx);
	free(iter->seg_x);
	free(iter->cursor);
	free(iter->active_segs);
	free(iter);
}

/**
 * art_svp_render_aa: Render SVP antialiased.
 * @svp: The #ArtSVP to render.
 * @x0: Left coordinate of destination rectangle.
 * @y0: Top coordinate of destination rectangle.
 * @x1: Right coordinate of destination rectangle.
 * @y1: Bottom coordinate of destination rectangle.
 * @callback: The callback which actually paints the pixels.
 * @callback_data: Private data for @callback.
 *
 * Renders the sorted vector path in the given rectangle, antialiased.
 *
 * This interface uses a callback for the actual pixel rendering. The
 * callback is called @y1 - @y0 times (once for each scan line). The y
 * coordinate is given as an argument for convenience (it could be
 * stored in the callback's private data and incremented on each
 * call).
 *
 * The rendered polygon is represented in a semi-runlength format: a
 * start value and a sequence of "steps". Each step has an x
 * coordinate and a value delta. The resulting value at position x is
 * equal to the sum of the start value and all step delta values for
 * which the step x coordinate is less than or equal to x. An
 * efficient algorithm will traverse the steps left to right, keeping
 * a running sum.
 *
 * All x coordinates in the steps are guaranteed to be @x0 <= x < @x1.
 * (This guarantee is a change from the gfonted vpaar renderer from
 * which this routine is derived, and is designed to simplify the
 * callback).
 *
 * The value 0x8000 represents 0% coverage by the polygon, while
 * 0xff8000 represents 100% coverage. This format is designed so that
 * >> 16 results in a standard 0x00..0xff value range, with nice
 * rounding.
 *
 **/
void art_svp_render_aa(const ArtSVP *svp,
                  int x0, int y0, int x1, int y1,
                  void (*callback)(void *callback_data,
                                   int y,
                                   int start,
                                   ArtSVPRenderAAStep *steps, int n_steps),
                  void *callback_data) {
	ArtSVPRenderAAIter *iter;
	int y;
	int start;
	ArtSVPRenderAAStep *steps;
	int n_steps;

	iter = art_svp_render_aa_iter(svp, x0, y0, x1, y1);


	for (y = y0; y < y1; y++) {
		art_svp_render_aa_iter_step(iter, &start, &steps, &n_steps);
		(*callback)(callback_data, y, start, steps, n_steps);
	}

	art_svp_render_aa_iter_done(iter);
}

} // End of namespace Sword25
