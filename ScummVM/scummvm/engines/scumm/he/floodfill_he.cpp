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

#ifdef ENABLE_HE

#include "scumm/he/floodfill_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"

namespace Scumm {

static bool floodFillPixelCheck(int x, int y, const FloodFillState *ffs) {
	int diffColor = ffs->color1 - ffs->color2;
	if (x >= 0 && x < ffs->dst_w && y >= 0 && y < ffs->dst_h) {
		uint8 color = *(ffs->dst + y * ffs->dst_w + x);
		diffColor = color - ffs->color1;
	}
	return diffColor == 0;
}

static void floodFillProcessRect(FloodFillState *ffs, const Common::Rect *r) {
	Common::Rect *dr = &ffs->dstBox;
	if (dr->right >= dr->left && dr->top <= dr->bottom) {
		int rw = r->right - r->left + 1;
		int rh = r->bottom - r->top + 1;
		assert(r->top + rh <= ffs->dst_h);
		assert(r->left + rw <= ffs->dst_w);
		uint8 *dst = ffs->dst + r->top * ffs->dst_w + r->left;
		if (rw <= 1) {
			--rh;
			while (rh >= 0) {
				*dst = ffs->color2;
				dst += ffs->dst_w;
				--rh;
			}
		} else {
			--rh;
			while (rh >= 0) {
				memset(dst, ffs->color2, rw);
				dst += ffs->dst_w;
				--rh;
			}
		}
		dr->extend(*r);
	} else {
		*dr = *r;
	}
}

static void floodFillAddLine(FloodFillLine **ffl, int y, int x1, int x2, int dy) {
	(*ffl)->y = y;
	(*ffl)->x1 = x1;
	(*ffl)->x2 = x2;
	(*ffl)->inc = dy;
	(*ffl)++;
}

static void floodFillProcess(int x, int y, FloodFillState *ffs, FloodFillPixelCheckCallback pixelCheckCallback) {
	ffs->dstBox.left = ffs->dstBox.top = 12345;
	ffs->dstBox.right = ffs->dstBox.bottom = -12345;

	FloodFillLine **fillLineCur = &ffs->fillLineTableCur;
	FloodFillLine **fillLineEnd = &ffs->fillLineTableEnd;

	assert(*fillLineCur < *fillLineEnd);
	if (ffs->srcBox.top <= y + 1 && ffs->srcBox.bottom >= y + 1) {
		(*fillLineCur)->y = y;
		(*fillLineCur)->x1 = x;
		(*fillLineCur)->x2 = x;
		(*fillLineCur)->inc = 1;
		(*fillLineCur)++;
	}

	assert(*fillLineCur < *fillLineEnd);
	if (ffs->srcBox.top <= y && ffs->srcBox.bottom >= y) {
		(*fillLineCur)->y = y + 1;
		(*fillLineCur)->x1 = x;
		(*fillLineCur)->x2 = x;
		(*fillLineCur)->inc = -1;
		(*fillLineCur)++;
	}

	assert(ffs->fillLineTable <= *fillLineCur);
	FloodFillLine **fillLineStart = fillLineCur;

	while (ffs->fillLineTable < *fillLineStart) {
		Common::Rect r;
		int x_start;
		FloodFillLine *fflCur = --(*fillLineCur);
		int dy = fflCur->inc;
		int x_end = fflCur->x2;
		int x1 = fflCur->x1;
		int x2 = fflCur->x1 + 1;
		r.bottom = r.top = y = fflCur->y + fflCur->inc;
		r.left = x2;
		r.right = x1;
		x = x1;
		while (ffs->srcBox.left <= x) {
			if (!(*pixelCheckCallback)(x, y, ffs)) {
				break;
			}
			r.left = x;
			--x;
		}
		if (r.right >= r.left && r.top <= r.bottom) {
			floodFillProcessRect(ffs, &r);
		}
		if (x >= x1) goto skip;
		x_start = x + 1;
		if (x1 > x_start) {
			assert(*fillLineEnd > *fillLineCur);
			if (ffs->srcBox.top <= y - dy && ffs->srcBox.bottom >= y - dy) {
				--x1;
				floodFillAddLine(fillLineCur, y, x_start, x1, -dy);
			}
		}
		x = x2;
		while (x_start <= x_end) {
			r.left = x;
			r.top = y;
			r.right = x - 1;
			r.bottom = y;
			while (ffs->srcBox.right >= x) {
				if (!(*pixelCheckCallback)(x, y, ffs)) {
					break;
				}
				r.right = x;
				++x;
			}
			if (r.right >= r.left && r.top <= r.bottom) {
				floodFillProcessRect(ffs, &r);
			}
			assert(ffs->fillLineTableCur < ffs->fillLineTableEnd);
			if (ffs->srcBox.top <= y + dy && ffs->srcBox.bottom >= y + dy) {
				floodFillAddLine(&ffs->fillLineTableCur, y, x_start, x - 1, dy);
			}
			x_start = x_end + 1;
			if (x > x_start) {
				assert(ffs->fillLineTableCur < ffs->fillLineTableEnd);
				if (ffs->srcBox.top <= y - dy && ffs->srcBox.bottom >= y - dy) {
					floodFillAddLine(&ffs->fillLineTableCur, y, x_start, x - 1, -dy);
				}
			}
skip:
			++x;
			while (x <= x_end) {
				if ((*pixelCheckCallback)(x, y, ffs)) {
					break;
				}
				++x;
			}
			x_start = x;
		}
	}
}

void floodFill(FloodFillParameters *ffp, ScummEngine_v90he *vm) {
	uint8 *dst;
	VirtScreen *vs = &vm->_virtscr[kMainVirtScreen];
	if (ffp->flags & 0x8000) {
		dst = vs->getBackPixels(0, vs->topline);
	} else {
		dst = vs->getPixels(0, vs->topline);
	}
	uint8 color = ffp->flags & 0xFF;

	Common::Rect r;
	r.left = r.top = 12345;
	r.right = r.bottom = -12345;

	FloodFillState *ffs = new FloodFillState;
	ffs->fillLineTableCount = vs->h * 2;
	ffs->fillLineTable = new FloodFillLine[ffs->fillLineTableCount];
	ffs->color2 = color;
	ffs->dst = dst;
	ffs->dst_w = vs->w;
	ffs->dst_h = vs->h;
	ffs->srcBox = ffp->box;
	ffs->fillLineTableCur = &ffs->fillLineTable[0];
	ffs->fillLineTableEnd = &ffs->fillLineTable[ffs->fillLineTableCount];

	if (ffp->x < 0 || ffp->y < 0 || ffp->x >= vs->w || ffp->y >= vs->h) {
		ffs->color1 = color;
	} else {
		ffs->color1 = *(dst + ffp->y * vs->w + ffp->x);
	}

	debug(5, "floodFill() x=%d y=%d color1=%d ffp->flags=0x%X", ffp->x, ffp->y, ffs->color1, ffp->flags);
	if (ffs->color1 != color) {
		floodFillProcess(ffp->x, ffp->y, ffs, floodFillPixelCheck);
		r = ffs->dstBox;
	}
	r.debugPrint(5, "floodFill() dirty_rect");

	delete[] ffs->fillLineTable;
	delete ffs;

	vm->VAR(119) = 1;

	if (r.left <= r.right && r.top <= r.bottom) {
		if (ffp->flags & 0x8000) {
			vm->restoreBackgroundHE(r);
		} else {
			++r.bottom;
			vm->markRectAsDirty(kMainVirtScreen, r);
		}
	}
}

void Wiz::fillWizFlood(const WizParameters *params) {
	if (params->processFlags & kWPFClipBox2) {
		int px = params->box2.left;
		int py = params->box2.top;
		uint8 *dataPtr = _vm->getResourceAddress(rtImage, params->img.resNum);
		if (dataPtr) {
			int state = 0;
			if (params->processFlags & kWPFNewState) {
				state = params->img.state;
			}
			uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), dataPtr, state, 0);
			assert(wizh);
			int c = READ_LE_UINT32(wizh + 0x0);
			int w = READ_LE_UINT32(wizh + 0x4);
			int h = READ_LE_UINT32(wizh + 0x8);
			assert(c == 0);
			Common::Rect imageRect(w, h);
			if (params->processFlags & kWPFClipBox) {
				if (!imageRect.intersects(params->box)) {
					return;
				}
				imageRect.clip(params->box);
			}
			uint8 color = _vm->VAR(93);
			if (params->processFlags & kWPFFillColor) {
				color = params->fillColor;
			}
			if (imageRect.contains(px, py)) {
				uint8 *wizd = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), dataPtr, state, 0);
				assert(wizd);

				FloodFillState *ffs = new FloodFillState;
				ffs->fillLineTableCount = h * 2;
				ffs->fillLineTable = new FloodFillLine[ffs->fillLineTableCount];
				ffs->color2 = color;
				ffs->dst = wizd;
				ffs->dst_w = w;
				ffs->dst_h = h;
				ffs->srcBox = imageRect;
				ffs->fillLineTableCur = &ffs->fillLineTable[0];
				ffs->fillLineTableEnd = &ffs->fillLineTable[ffs->fillLineTableCount];

				if (px < 0 || py < 0 || px >= w || py >= h) {
					ffs->color1 = color;
				} else {
					ffs->color1 = *(wizd + py * w + px);
				}

				debug(0, "floodFill() x=%d y=%d color1=%d", px, py, ffs->color1);

				if (ffs->color1 != color) {
					floodFillProcess(px, py, ffs, floodFillPixelCheck);
				}

				delete[] ffs->fillLineTable;
				delete ffs;
			}
		}
	}
	_vm->_res->setModified(rtImage, params->img.resNum);
}

} // End of namespace Scumm

#endif // ENABLE_HE
