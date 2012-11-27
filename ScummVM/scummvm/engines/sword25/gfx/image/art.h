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

/* Simple macros to set up storage allocation and basic types for libart
   functions. */

#ifndef __ART_H__
#define __ART_H__

#include "common/scummsys.h"

namespace Sword25 {

/* These aren't, strictly speaking, configuration macros, but they're
   damn handy to have around, and may be worth playing with for
   debugging. */
#define art_new(type, n) ((type *)malloc ((n) * sizeof(type)))

#define art_renew(p, type, n) ((type *)realloc (p, (n) * sizeof(type)))

/* This one must be used carefully - in particular, p and max should
   be variables. They can also be pstruct->el lvalues. */
#define art_expand(p, type, max) \
			do { \
				if (max) {\
					type *tmp = art_renew(p, type, max <<= 1); \
					if (!tmp) error("Cannot reallocate memory for art data"); \
					p = tmp; \
				} else { \
					max = 1; \
					p = art_new(type, 1); \
					if (!p) error("Cannot allocate memory for art data"); \
				} \
			} while (0)

struct ArtDRect {
	/*< public >*/
	double x0, y0, x1, y1;
};

struct ArtPoint {
	/*< public >*/
	double x, y;
};

/* Basic data structures and constructors for sorted vector paths */

struct ArtSVPSeg {
	int n_points;
	int dir; /* == 0 for "up", 1 for "down" */
	ArtDRect bbox;
	ArtPoint *points;
};

struct ArtSVP {
	int n_segs;
	ArtSVPSeg segs[1];
};

void art_svp_free(ArtSVP *svp);

int art_svp_seg_compare(const void *s1, const void *s2);

/* Basic data structures and constructors for bezier paths */

enum ArtPathcode {
	ART_MOVETO,
	ART_MOVETO_OPEN,
	ART_CURVETO,
	ART_LINETO,
	ART_END
};

struct ArtBpath {
	/*< public >*/
	ArtPathcode code;
	double x1;
	double y1;
	double x2;
	double y2;
	double x3;
	double y3;
};

/* Basic data structures and constructors for simple vector paths */

/* CURVETO is not allowed! */
struct ArtVpath {
	ArtPathcode code;
	double x;
	double y;
};

/* Some of the functions need to go into their own modules */

void art_vpath_add_point(ArtVpath **p_vpath, int *pn_points, int *pn_points_max,
                    ArtPathcode code, double x, double y);

ArtVpath *art_bez_path_to_vec(const ArtBpath *bez, double flatness);

/* The funky new SVP intersector. */

#ifndef ART_WIND_RULE_DEFINED
#define ART_WIND_RULE_DEFINED
enum ArtWindRule {
	ART_WIND_RULE_NONZERO,
	ART_WIND_RULE_INTERSECT,
	ART_WIND_RULE_ODDEVEN,
	ART_WIND_RULE_POSITIVE
};
#endif

struct ArtSvpWriter {
	int (*add_segment)(ArtSvpWriter *self, int wind_left, int delta_wind,
	                   double x, double y);
	void (*add_point)(ArtSvpWriter *self, int seg_id, double x, double y);
	void (*close_segment)(ArtSvpWriter *self, int seg_id);
};

ArtSvpWriter *art_svp_writer_rewind_new(ArtWindRule rule);

ArtSVP *art_svp_writer_rewind_reap(ArtSvpWriter *self);

int art_svp_seg_compare(const void *s1, const void *s2);

void art_svp_intersector(const ArtSVP *in, ArtSvpWriter *out);


/* Sort vector paths into sorted vector paths. */

ArtSVP *art_svp_from_vpath(ArtVpath *vpath);

/* Sort vector paths into sorted vector paths. */

enum ArtPathStrokeJoinType {
	ART_PATH_STROKE_JOIN_MITER,
	ART_PATH_STROKE_JOIN_ROUND,
	ART_PATH_STROKE_JOIN_BEVEL
};

enum ArtPathStrokeCapType {
	ART_PATH_STROKE_CAP_BUTT,
	ART_PATH_STROKE_CAP_ROUND,
	ART_PATH_STROKE_CAP_SQUARE
};

ArtSVP *art_svp_vpath_stroke(ArtVpath *vpath,
                     ArtPathStrokeJoinType join,
                     ArtPathStrokeCapType cap,
                     double line_width,
                     double miter_limit,
                     double flatness);

/* This version may have winding numbers exceeding 1. */
ArtVpath *art_svp_vpath_stroke_raw(ArtVpath *vpath,
                         ArtPathStrokeJoinType join,
                         ArtPathStrokeCapType cap,
                         double line_width,
                         double miter_limit,
                         double flatness);


/* The spiffy antialiased renderer for sorted vector paths. */

struct ArtSVPRenderAAStep {
	int x;
	int delta; /* stored with 16 fractional bits */
};

struct ArtSVPRenderAAIter;

ArtSVPRenderAAIter *art_svp_render_aa_iter(const ArtSVP *svp,
                       int x0, int y0, int x1, int y1);

void art_svp_render_aa_iter_step(ArtSVPRenderAAIter *iter, int *p_start,
                            ArtSVPRenderAAStep **p_steps, int *p_n_steps);

void art_svp_render_aa_iter_done(ArtSVPRenderAAIter *iter);

void art_svp_render_aa(const ArtSVP *svp,
                  int x0, int y0, int x1, int y1,
                  void (*callback)(void *callback_data,
                                   int y,
                                   int start,
                                   ArtSVPRenderAAStep *steps, int n_steps),
                  void *callback_data);

} // End of namespace Sword25

#endif /* __ART_H__ */
