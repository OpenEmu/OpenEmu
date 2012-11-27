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

#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/bomp.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

#include "common/pack-start.h"	// START STRUCT PACKING

struct AkosHeader {
	byte unk_1[2];
	byte flags;
	byte unk_2;
	uint16 num_anims;
	uint16 unk_3;
	uint16 codec;
} PACKED_STRUCT;

struct AkosOffset {
	uint32 akcd;	// offset into the akcd data
	uint16 akci;	// offset into the akci data
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


enum AkosOpcodes {
	AKC_Return = 0xC001,
	AKC_SetVar = 0xC010,
	AKC_CmdQue3 = 0xC015,
	AKC_C016 = 0xC016,
	AKC_C017 = 0xC017,
	AKC_C018 = 0xC018,
	AKC_C019 = 0xC019,
	AKC_ComplexChan = 0xC020,
	AKC_C021 = 0xC021,
	AKC_C022 = 0xC022,
	AKC_ComplexChan2 = 0xC025,
	AKC_Jump = 0xC030,
	AKC_JumpIfSet = 0xC031,
	AKC_AddVar = 0xC040,
	AKC_C042 = 0xC042,
	AKC_C044 = 0xC044,
	AKC_C045 = 0xC045,
	AKC_C046 = 0xC046,
	AKC_C047 = 0xC047,
	AKC_C048 = 0xC048,
	AKC_Ignore = 0xC050,
	AKC_IncVar = 0xC060,
	AKC_CmdQue3Quick = 0xC061,
	AKC_JumpStart = 0xC070,
	AKC_JumpE = 0xC070,
	AKC_JumpNE = 0xC071,
	AKC_JumpL = 0xC072,
	AKC_JumpLE = 0xC073,
	AKC_JumpG = 0xC074,
	AKC_JumpGE = 0xC075,
	AKC_StartAnim = 0xC080,
	AKC_StartVarAnim = 0xC081,
	AKC_Random = 0xC082,
	AKC_SetActorClip = 0xC083,
	AKC_StartAnimInActor = 0xC084,
	AKC_SetVarInActor = 0xC085,
	AKC_HideActor = 0xC086,
	AKC_SetDrawOffs = 0xC087,
	AKC_JumpTable = 0xC088,
	AKC_SoundStuff = 0xC089,
	AKC_Flip = 0xC08A,
	AKC_Cmd3 = 0xC08B,
	AKC_Ignore3 = 0xC08C,
	AKC_Ignore2 = 0xC08D,
	AKC_C08E = 0xC08E,
	AKC_SkipStart = 0xC090,
	AKC_SkipE = 0xC090,
	AKC_SkipNE = 0xC091,
	AKC_SkipL = 0xC092,
	AKC_SkipLE = 0xC093,
	AKC_SkipG = 0xC094,
	AKC_SkipGE = 0xC095,
	AKC_ClearFlag = 0xC09F,
	AKC_C0A0 = 0xC0A0,
	AKC_C0A1 = 0xC0A1,
	AKC_C0A2 = 0xC0A2,
	AKC_C0A3 = 0xC0A3,
	AKC_C0A4 = 0xC0A4,
	AKC_C0A5 = 0xC0A5,
	AKC_C0A6 = 0xC0A6,
	AKC_C0A7 = 0xC0A7,
	AKC_EndSeq = 0xC0FF
};

static bool akos_compare(int a, int b, byte cmd) {
	switch (cmd) {
	case 0:
		return a == b;
	case 1:
		return a != b;
	case 2:
		return a < b;
	case 3:
		return a <= b;
	case 4:
		return a > b;
	default:
		return a >= b;
	}
}

void AkosCostumeLoader::loadCostume(int id) {
	_akos = _vm->getResourceAddress(rtCostume, id);
	assert(_akos);
}

bool AkosCostumeLoader::hasManyDirections() {
	const AkosHeader *akhd;

	akhd = (const AkosHeader *)_vm->findResourceData(MKTAG('A','K','H','D'), _akos);
	return (akhd->flags & 2) != 0;
}

void AkosCostumeLoader::costumeDecodeData(Actor *a, int frame, uint usemask) {
	uint anim;
	const byte *r;
	const AkosHeader *akhd;
	uint offs;
	int i;
	byte code;
	uint16 start, len;
	uint16 mask;

	if (a->_costume == 0)
		return;

	loadCostume(a->_costume);

	if (_vm->_game.version >= 7 && hasManyDirections())
		anim = toSimpleDir(1, a->getFacing()) + frame * 8;
	else
		anim = newDirToOldDir(a->getFacing()) + frame * 4;

	akhd = (const AkosHeader *)_vm->findResourceData(MKTAG('A','K','H','D'), _akos);

	if (anim >= READ_LE_UINT16(&akhd->num_anims))
		return;

	r = _vm->findResourceData(MKTAG('A','K','C','H'), _akos);
	assert(r);

	offs = READ_LE_UINT16(r + anim * sizeof(uint16));
	if (offs == 0)
		return;
	r += offs;

	const uint8 *akstPtr = _vm->findResourceData(MKTAG('A','K','S','T'), _akos);
	const uint8 *aksfPtr = _vm->findResourceData(MKTAG('A','K','S','F'), _akos);

	i = 0;
	mask = READ_LE_UINT16(r); r += 2;
	do {
		if (mask & 0x8000) {
			const uint8 *akst = akstPtr;
			const uint8 *aksf = aksfPtr;

			code = *r++;
			if (usemask & 0x8000) {
				switch (code) {
				case 1:
					a->_cost.active[i] = 0;
					a->_cost.frame[i] = frame;
					a->_cost.end[i] = 0;
					a->_cost.start[i] = 0;
					a->_cost.curpos[i] = 0;
					a->_cost.heCondMaskTable[i] = 0;

					if (akst) {
						int size = _vm->getResourceDataSize(akst) / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if (READ_LE_UINT32(akst) == 0) {
									a->_cost.heCondMaskTable[i] = READ_LE_UINT32(akst + 4);
									found = true;
									break;
								}
								akst += 8;
							}
							if (!found) {
								error("Sequence not found in actor %p costume %d", (void *)a, a->_costume);
							}
						}
					}
					break;
				case 4:
					a->_cost.stopped |= 1 << i;
					break;
				case 5:
					a->_cost.stopped &= ~(1 << i);
					break;
				default:
					start = READ_LE_UINT16(r); r += 2;
					len = READ_LE_UINT16(r); r += 2;

					a->_cost.heJumpOffsetTable[i] = 0;
					a->_cost.heJumpCountTable[i] = 0;
					if (aksf) {
						int size = _vm->getResourceDataSize(aksf) / 6;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if (READ_LE_UINT16(aksf) == start) {
									a->_cost.heJumpOffsetTable[i] = READ_LE_UINT16(aksf + 2);
									a->_cost.heJumpCountTable[i] = READ_LE_UINT16(aksf + 4);
									found = true;
									break;
								}
								aksf += 6;
							}
							if (!found) {
								error("Sequence not found in actor %p costume %d", (void *)a, a->_costume);
							}
						}
					}

					a->_cost.active[i] = code;
					a->_cost.frame[i] = frame;
					a->_cost.end[i] = start + len;
					a->_cost.start[i] = start;
					a->_cost.curpos[i] = start;
					a->_cost.heCondMaskTable[i] = 0;
					if (akst) {
						int size = _vm->getResourceDataSize(akst) / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if (READ_LE_UINT32(akst) == start) {
									a->_cost.heCondMaskTable[i] = READ_LE_UINT32(akst + 4);
									found = true;
									break;
								}
								akst += 8;
							}
							if (!found) {
								error("Sequence not found in actor %p costume %d", (void *)a, a->_costume);
							}
						}
					}
					break;
				}
			} else {
				if (code != 1 && code != 4 && code != 5)
					r += sizeof(uint16) * 2;
			}
		}
		i++;
		mask <<= 1;
		usemask <<= 1;
	} while ((uint16)mask);
}

void AkosRenderer::setPalette(uint16 *new_palette) {
	uint size, i;

	size = _vm->getResourceDataSize(akpl);
	if (size == 0)
		return;

	if (size > 256)
		error("akos_setPalette: %d is too many colors", size);

	if (_vm->_game.features & GF_16BIT_COLOR) {
		if (_paletteNum) {
			for (i = 0; i < size; i++)
				_palette[i] = READ_LE_UINT16(_vm->_hePalettes + _paletteNum * _vm->_hePaletteSlot + 768 + akpl[i] * 2);
		} else if (rgbs) {
			for (i = 0; i < size; i++) {
				if (new_palette[i] == 0xFF) {
					uint8 col = akpl[i];
					_palette[i] = _vm->get16BitColor(rgbs[col * 3 + 0], rgbs[col * 3 + 1], rgbs[col * 3 + 2]);
				} else {
					_palette[i] = new_palette[i];
				}
			}
		}
	} else if (_vm->_game.heversion >= 99 && _paletteNum) {
		for (i = 0; i < size; i++)
			_palette[i] = (byte)_vm->_hePalettes[_paletteNum * _vm->_hePaletteSlot + 768 + akpl[i]];
	} else {
		for (i = 0; i < size; i++) {
			_palette[i] = new_palette[i] != 0xFF ? new_palette[i] : akpl[i];
		}
	}

	if (_vm->_game.heversion == 70) {
		for (i = 0; i < size; i++)
			_palette[i] = _vm->_HEV7ActorPalette[_palette[i]];
	}

	if (size == 256) {
		byte color = new_palette[0];
		if (color == 255) {
			_palette[0] = color;
		} else {
			_useBompPalette = true;
		}
	}
}

void AkosRenderer::setCostume(int costume, int shadow) {
	const byte *akos = _vm->getResourceAddress(rtCostume, costume);
	assert(akos);

	akhd = (const AkosHeader *) _vm->findResourceData(MKTAG('A','K','H','D'), akos);
	akof = (const AkosOffset *) _vm->findResourceData(MKTAG('A','K','O','F'), akos);
	akci = _vm->findResourceData(MKTAG('A','K','C','I'), akos);
	aksq = _vm->findResourceData(MKTAG('A','K','S','Q'), akos);
	akcd = _vm->findResourceData(MKTAG('A','K','C','D'), akos);
	akpl = _vm->findResourceData(MKTAG('A','K','P','L'), akos);
	_codec = READ_LE_UINT16(&akhd->codec);
	akct = _vm->findResourceData(MKTAG('A','K','C','T'), akos);
	rgbs = _vm->findResourceData(MKTAG('R','G','B','S'), akos);

	xmap = 0;
	if (shadow) {
		const uint8 *xmapPtr = _vm->getResourceAddress(rtImage, shadow);
		assert(xmapPtr);
		xmap = _vm->findResourceData(MKTAG('X','M','A','P'), xmapPtr);
		assert(xmap);
	}
}

void AkosRenderer::setFacing(const Actor *a) {
	_mirror = (newDirToOldDir(a->getFacing()) != 0 || akhd->flags & 1);
	if (a->_flip)
		_mirror = !_mirror;
}

byte AkosRenderer::drawLimb(const Actor *a, int limb) {
	uint code;
	const byte *p;
	const AkosOffset *off;
	const CostumeData &cost = a->_cost;
	const CostumeInfo *costumeInfo;
	uint i, extra;
	byte result = 0;
	int xmoveCur, ymoveCur;
	uint32 heCondMaskIndex[32];
	bool useCondMask;
	int lastDx, lastDy;

	lastDx = lastDy = 0;
	for (i = 0; i < 32; ++i) {
		heCondMaskIndex[i] = i;
	}

	if (_skipLimbs)
		return 0;

	if (_vm->_game.heversion >= 70 && cost.active[limb] == 8)
		return 0;

	if (!cost.active[limb] || cost.stopped & (1 << limb))
		return 0;

	useCondMask = false;
	p = aksq + cost.curpos[limb];

	code = p[0];
	if (code & 0x80)
		code = READ_BE_UINT16(p);

	if (_vm->_game.heversion >= 90)
		_shadow_mode = 0;

	if (code == AKC_C021 || code == AKC_C022) {
		uint16 s = cost.curpos[limb] + 4;
		uint j = 0;
		extra = p[3];
		uint8 n = extra;
		assert(n <= ARRAYSIZE(heCondMaskIndex));
		while (n--) {
			heCondMaskIndex[j++] = aksq[s++];
		}
		useCondMask = true;
		p += extra + 2;
		code = (code == AKC_C021) ? AKC_ComplexChan : AKC_ComplexChan2;
	}

	if (code == AKC_Return || code == AKC_EndSeq)
		return 0;

	if (code != AKC_ComplexChan && code != AKC_ComplexChan2) {
		off = akof + (code & 0xFFF);

		assert((code & 0xFFF) * 6 < READ_BE_UINT32((const byte *)akof - 4) - 8);
		assert((code & 0x7000) == 0);

		_srcptr = akcd + READ_LE_UINT32(&off->akcd);
		costumeInfo = (const CostumeInfo *) (akci + READ_LE_UINT16(&off->akci));

		_width = READ_LE_UINT16(&costumeInfo->width);
		_height = READ_LE_UINT16(&costumeInfo->height);
		xmoveCur = _xmove + (int16)READ_LE_UINT16(&costumeInfo->rel_x);
		ymoveCur = _ymove + (int16)READ_LE_UINT16(&costumeInfo->rel_y);
		_xmove += (int16)READ_LE_UINT16(&costumeInfo->move_x);
		_ymove -= (int16)READ_LE_UINT16(&costumeInfo->move_y);

		switch (_codec) {
		case 1:
			result |= codec1(xmoveCur, ymoveCur);
			break;
		case 5:
			result |= codec5(xmoveCur, ymoveCur);
			break;
		case 16:
			result |= codec16(xmoveCur, ymoveCur);
			break;
		default:
			error("akos_drawLimb: invalid _codec %d", _codec);
		}
	} else {
		if (code == AKC_ComplexChan2)  {
			lastDx = (int16)READ_LE_UINT16(p + 2);
			lastDy = (int16)READ_LE_UINT16(p + 4);
			p += 4;
		}

		extra = p[2];
		p += 3;
		uint32 decflag = heCondMaskIndex[0];

		for (i = 0; i != extra; i++) {
			code = p[4];
			if (code & 0x80)
				code = READ_BE_UINT16(p + 4);
			off = akof + (code & 0xFFF);

			_srcptr = akcd + READ_LE_UINT32(&off->akcd);
			costumeInfo = (const CostumeInfo *) (akci + READ_LE_UINT16(&off->akci));

			_width = READ_LE_UINT16(&costumeInfo->width);
			_height = READ_LE_UINT16(&costumeInfo->height);

			xmoveCur = _xmove + (int16)READ_LE_UINT16(p + 0);
			ymoveCur = _ymove + (int16)READ_LE_UINT16(p + 2);

			if (i == extra - 1) {
				_xmove += lastDx;
				_ymove -= lastDy;
			}

			uint16 shadowMask = 0;

			if (!useCondMask || !akct) {
				decflag = 1;
			} else {
				uint32 cond = READ_LE_UINT32(akct + cost.heCondMaskTable[limb] + heCondMaskIndex[i] * 4);
				if (cond == 0) {
					decflag = 1;
				} else {
					uint32 type = cond & ~0x3FFFFFFF;
					cond &= 0x3FFFFFFF;
					if (_vm->_game.heversion >= 90) {
						shadowMask = cond & 0xE000;
						cond &= ~0xE000;
					}
					if (_vm->_game.heversion >= 90 && cond == 0) {
						decflag = 1;
					} else if (type == 0x40000000) { // restored_bit
						decflag = (a->_heCondMask & cond) == cond ? 1 : 0;
					} else if (type == 0x80000000) { // dirty_bit
						decflag = (a->_heCondMask & cond) ? 0 : 1;
					} else {
						decflag = (a->_heCondMask & cond) ? 1 : 0;
					}
				}
			}

			p += (p[4] & 0x80) ? 6 : 5;

			if (decflag == 0)
				continue;

			if (_vm->_game.heversion >= 90) {
				if (_vm->_game.heversion >= 99)
					_shadow_mode = 0;
				if (xmap && (shadowMask & 0x8000))
					_shadow_mode = 3;
			}

			switch (_codec) {
			case 1:
				result |= codec1(xmoveCur, ymoveCur);
				break;
			case 5:
				result |= codec5(xmoveCur, ymoveCur);
				break;
			case 16:
				result |= codec16(xmoveCur, ymoveCur);
				break;
			case 32:
				result |= codec32(xmoveCur, ymoveCur);
				break;
			default:
				error("akos_drawLimb: invalid _codec %d", _codec);
			}
		}
	}

	return result;
}

void AkosRenderer::codec1_genericDecode(Codec1 &v1) {
	const byte *mask, *src;
	byte *dst;
	byte len, maskbit;
	int y;
	uint16 color, height, pcolor;
	const byte *scaleytab;
	bool masked;
	bool skip_column = false;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[v1.scaleYindex];
	maskbit = revBitMask(v1.x & 7);
	mask = _vm->getMaskBuffer(v1.x - (_vm->_virtscr[kMainVirtScreen].xstart & 7), v1.y, _zbuf);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (_scaleY == 255 || *scaleytab++ < _scaleY) {
				if (_actorHitMode) {
					if (color && y == _actorHitY && v1.x == _actorHitX) {
						_actorHitResult = true;
						return;
					}
				} else {
					masked = (y < v1.boundsRect.top || y >= v1.boundsRect.bottom) || (v1.x < 0 || v1.x >= v1.boundsRect.right) || (*mask & maskbit);

					if (color && !masked && !skip_column) {
						pcolor = _palette[color];
						if (_shadow_mode == 1) {
							if (pcolor == 13)
								pcolor = _shadow_table[*dst];
						} else if (_shadow_mode == 2) {
							error("codec1_spec2"); // TODO
						} else if (_shadow_mode == 3) {
							if (_vm->_game.features & GF_16BIT_COLOR) {
								uint16 srcColor = (pcolor >> 1) & 0x7DEF;
								uint16 dstColor = (READ_UINT16(dst) >> 1) & 0x7DEF;
								pcolor = srcColor + dstColor;
							} else if (_vm->_game.heversion >= 90) {
								pcolor = (pcolor << 8) + *dst;
								pcolor = xmap[pcolor];
							} else if (pcolor < 8) {
								pcolor = (pcolor << 8) + *dst;
								pcolor = _shadow_table[pcolor];
							}
						}
						if (_vm->_bytesPerPixel == 2) {
							WRITE_UINT16(dst, pcolor);
						} else {
							*dst = pcolor;
						}
					}
				}
				dst += _out.pitch;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[v1.scaleYindex];

				if (_scaleX == 255 || v1.scaletable[v1.scaleXindex] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= v1.boundsRect.right)
						return;
					maskbit = revBitMask(v1.x & 7);
					v1.destptr += v1.scaleXstep * _vm->_bytesPerPixel;
					skip_column = false;
				} else
					skip_column = true;
				v1.scaleXindex += v1.scaleXstep;
				dst = v1.destptr;
				mask = _vm->getMaskBuffer(v1.x - (_vm->_virtscr[kMainVirtScreen].xstart & 7), v1.y, _zbuf);
			}
		StartPos:;
		} while (--len);
	} while (1);
}

// This is exact duplicate of smallCostumeScaleTable[] in costume.cpp
// See FIXME below for explanation
const byte smallCostumeScaleTableAKOS[256] = {
	0xFF, 0xFD, 0x7D, 0xBD, 0x3D, 0xDD, 0x5D, 0x9D,
	0x1D, 0xED, 0x6D, 0xAD, 0x2D, 0xCD, 0x4D, 0x8D,
	0x0D, 0xF5, 0x75, 0xB5, 0x35, 0xD5, 0x55, 0x95,
	0x15, 0xE5, 0x65, 0xA5, 0x25, 0xC5, 0x45, 0x85,
	0x05, 0xF9, 0x79, 0xB9, 0x39, 0xD9, 0x59, 0x99,
	0x19, 0xE9, 0x69, 0xA9, 0x29, 0xC9, 0x49, 0x89,
	0x09, 0xF1, 0x71, 0xB1, 0x31, 0xD1, 0x51, 0x91,
	0x11, 0xE1, 0x61, 0xA1, 0x21, 0xC1, 0x41, 0x81,
	0x01, 0xFB, 0x7B, 0xBB, 0x3B, 0xDB, 0x5B, 0x9B,
	0x1B, 0xEB, 0x6B, 0xAB, 0x2B, 0xCB, 0x4B, 0x8B,
	0x0B, 0xF3, 0x73, 0xB3, 0x33, 0xD3, 0x53, 0x93,
	0x13, 0xE3, 0x63, 0xA3, 0x23, 0xC3, 0x43, 0x83,
	0x03, 0xF7, 0x77, 0xB7, 0x37, 0xD7, 0x57, 0x97,
	0x17, 0xE7, 0x67, 0xA7, 0x27, 0xC7, 0x47, 0x87,
	0x07, 0xEF, 0x6F, 0xAF, 0x2F, 0xCF, 0x4F, 0x8F,
	0x0F, 0xDF, 0x5F, 0x9F, 0x1F, 0xBF, 0x3F, 0x7F,
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE
};
const byte bigCostumeScaleTable[768] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

byte AkosRenderer::codec1(int xmoveCur, int ymoveCur) {
	int num_colors;
	bool use_scaling;
	int i, j;
	int skip = 0, startScaleIndexX, startScaleIndexY;
	Common::Rect rect;
	int step;
	byte drawFlag = 1;
	Codec1 v1;

	const int scaletableSize = (_vm->_game.heversion >= 61) ? 128 : 384;

	/* implement custom scale table */

	// FIXME. HACK
	// For some illogical reason gcc 3.4.x produces wrong code if
	// smallCostumeScaleTable from costume.cpp is used here
	// So I had to put copy of it back here as it was before 1.227 revision
	// of this file.
	v1.scaletable = (_vm->_game.heversion >= 61) ? smallCostumeScaleTableAKOS : bigCostumeScaleTable;
	if (_vm->VAR_CUSTOMSCALETABLE != 0xFF && _vm->_res->isResourceLoaded(rtString, _vm->VAR(_vm->VAR_CUSTOMSCALETABLE))) {
		v1.scaletable = _vm->getStringAddressVar(_vm->VAR_CUSTOMSCALETABLE);
	}

	// Setup color decoding variables
	num_colors = _vm->getResourceDataSize(akpl);
	if (num_colors == 32) {
		v1.mask = 7;
		v1.shr = 3;
	} else if (num_colors == 64) {
		v1.mask = 3;
		v1.shr = 2;
	} else {
		v1.mask = 15;
		v1.shr = 4;
	}

	use_scaling = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	v1.x = _actorX;
	v1.y = _actorY;

	v1.boundsRect.left = 0;
	v1.boundsRect.top = 0;
	v1.boundsRect.right = _out.w;
	v1.boundsRect.bottom = _out.h;

	if (use_scaling) {

		/* Scale direction */
		v1.scaleXstep = -1;
		if (xmoveCur < 0) {
			xmoveCur = -xmoveCur;
			v1.scaleXstep = 1;
		}

		if (_mirror) {
			/* Adjust X position */
			startScaleIndexX = j = scaletableSize - xmoveCur;
			for (i = 0; i < xmoveCur; i++) {
				if (v1.scaletable[j++] < _scaleX)
					v1.x -= v1.scaleXstep;
			}

			rect.left = rect.right = v1.x;

			j = startScaleIndexX;
			for (i = 0, skip = 0; i < _width; i++) {
				if (rect.right < 0) {
					skip++;
					startScaleIndexX = j;
				}
				if (v1.scaletable[j++] < _scaleX)
					rect.right++;
			}
		} else {
			/* No mirror */
			/* Adjust X position */
			startScaleIndexX = j = scaletableSize + xmoveCur;
			for (i = 0; i < xmoveCur; i++) {
				if (v1.scaletable[j--] < _scaleX)
					v1.x += v1.scaleXstep;
			}

			rect.left = rect.right = v1.x;

			j = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (rect.left >= v1.boundsRect.right) {
					startScaleIndexX = j;
					skip++;
				}
				if (v1.scaletable[j--] < _scaleX)
					rect.left--;
			}
		}

		if (skip)
			skip--;

		step = -1;
		if (ymoveCur < 0) {
			ymoveCur = -ymoveCur;
			step = -step;
		}

		startScaleIndexY = scaletableSize - ymoveCur;
		for (i = 0; i < ymoveCur; i++) {
			if (v1.scaletable[startScaleIndexY++] < _scaleY)
				v1.y -= step;
		}

		rect.top = rect.bottom = v1.y;
		startScaleIndexY = scaletableSize - ymoveCur;
		for (i = 0; i < _height; i++) {
			if (v1.scaletable[startScaleIndexY++] < _scaleY)
				rect.bottom++;
		}

		startScaleIndexY = scaletableSize - ymoveCur;
	} else {
		if (!_mirror)
			xmoveCur = -xmoveCur;

		v1.x += xmoveCur;
		v1.y += ymoveCur;

		if (_mirror) {
			rect.left = v1.x;
			rect.right = v1.x + _width;
		} else {
			rect.left = v1.x - _width;
			rect.right = v1.x;
		}

		rect.top = v1.y;
		rect.bottom = rect.top + _height;

		startScaleIndexX = scaletableSize;
		startScaleIndexY = scaletableSize;
	}

	v1.scaleXindex = startScaleIndexX;
	v1.scaleYindex = startScaleIndexY;
	v1.skip_width = _width;
	v1.scaleXstep = _mirror ? 1 : -1;

	if (_vm->_game.heversion >= 71 && !use_scaling) {
		if (_clipOverride.right > _clipOverride.left && _clipOverride.bottom > _clipOverride.top) {
			v1.boundsRect = _clipOverride;
		}
	}

	if (_actorHitMode) {
		if (_actorHitX < rect.left || _actorHitX >= rect.right || _actorHitY < rect.top || _actorHitY >= rect.bottom)
			return 0;
	} else
		markRectAsDirty(rect);

	if (rect.top >= v1.boundsRect.bottom || rect.bottom <= v1.boundsRect.top)
		return 0;

	if (rect.left >= v1.boundsRect.right || rect.right <= v1.boundsRect.left)
		return 0;

	v1.replen = 0;

	if (_mirror) {
		if (!use_scaling)
			skip = v1.boundsRect.left - v1.x;

		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(v1, skip);
			v1.x = v1.boundsRect.left;
		} else {
			skip = rect.right - v1.boundsRect.right;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				v1.skip_width -= skip;
			}
		}
	} else {
		if (!use_scaling)
			skip = rect.right - v1.boundsRect.right + 1;
		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(v1, skip)	;
			v1.x = v1.boundsRect.right - 1;
		} else {
			skip = (v1.boundsRect.left -1) - rect.left;

			if (skip <= 0)
				drawFlag = 2;
			else
				v1.skip_width -= skip;
		}
	}

	if (v1.skip_width <= 0 || _height <= 0)
		return 0;

	if (rect.left < v1.boundsRect.left)
		rect.left = v1.boundsRect.left;

	if (rect.top < v1.boundsRect.top)
		rect.top = v1.boundsRect.top;

	if (rect.top > v1.boundsRect.bottom)
		rect.top = v1.boundsRect.bottom;

	if (rect.bottom > v1.boundsRect.bottom)
		rect.bottom = v1.boundsRect.bottom;

	if (_draw_top > rect.top)
		_draw_top = rect.top;
	if (_draw_bottom < rect.bottom)
		_draw_bottom = rect.bottom;

	v1.destptr = (byte *)_out.pixels + v1.y * _out.pitch + v1.x * _vm->_bytesPerPixel;

	codec1_genericDecode(v1);

	return drawFlag;
}

void AkosRenderer::markRectAsDirty(Common::Rect rect) {
	rect.left -= _vm->_virtscr[kMainVirtScreen].xstart & 7;
	rect.right -= _vm->_virtscr[kMainVirtScreen].xstart & 7;
	_vm->markRectAsDirty(kMainVirtScreen, rect, _actorID);
}

byte AkosRenderer::codec5(int xmoveCur, int ymoveCur) {
	Common::Rect clip;
	int32 maxw, maxh;

	if (_actorHitMode) {
		error("codec5: _actorHitMode not yet implemented");
		return 0;
	}

	if (!_mirror) {
		clip.left = (_actorX - xmoveCur - _width) + 1;
	} else {
		clip.left = _actorX + xmoveCur - 1;
	}

	clip.top = _actorY + ymoveCur;
	clip.right = clip.left + _width;
	clip.bottom = clip.top + _height;
	maxw = _out.w;
	maxh = _out.h;

	markRectAsDirty(clip);

	clip.clip(maxw, maxh);

	if ((clip.left >= clip.right) || (clip.top >= clip.bottom))
		return 0;

	if (_draw_top > clip.top)
		_draw_top = clip.top;
	if (_draw_bottom < clip.bottom)
		_draw_bottom = clip.bottom;

	BompDrawData bdd;

	bdd.dst = _out;
	if (!_mirror) {
		bdd.x = (_actorX - xmoveCur - _width) + 1;
	} else {
		bdd.x = _actorX + xmoveCur;
	}
	bdd.y = _actorY + ymoveCur;

	bdd.src = _srcptr;
	bdd.srcwidth = _width;
	bdd.srcheight = _height;

	bdd.scale_x = 255;
	bdd.scale_y = 255;

	bdd.maskPtr = _vm->getMaskBuffer(0, 0, _zbuf);
	bdd.numStrips = _numStrips;

	bdd.shadowMode = _shadow_mode;
	bdd.shadowPalette = _vm->_shadowPalette;

	bdd.actorPalette = _useBompPalette ? _palette : 0;

	bdd.mirror = !_mirror;

	drawBomp(bdd);

	_useBompPalette = false;

	return 0;
}

void AkosRenderer::akos16SetupBitReader(const byte *src) {
	_akos16.repeatMode = false;
	_akos16.numbits = 16;
	_akos16.mask = (1 << *src) - 1;
	_akos16.shift = *(src);
	_akos16.color = *(src + 1);
	_akos16.bits = (*(src + 2) | *(src + 3) << 8);
	_akos16.dataptr = src + 4;
}

#define AKOS16_FILL_BITS()                                        \
		if (_akos16.numbits <= 8) {                                \
		  _akos16.bits |= (*_akos16.dataptr++) << _akos16.numbits;   \
		  _akos16.numbits += 8;                                    \
		}

#define AKOS16_EAT_BITS(n)                                        \
		_akos16.numbits -= (n);                                    \
		_akos16.bits >>= (n);


void AkosRenderer::akos16SkipData(int32 numbytes) {
	akos16DecodeLine(0, numbytes, 0);
}

void AkosRenderer::akos16DecodeLine(byte *buf, int32 numbytes, int32 dir) {
	uint16 bits, tmp_bits;

	while (numbytes != 0) {
		if (buf) {
			*buf = _akos16.color;
			buf += dir;
		}

		if (!_akos16.repeatMode) {
			AKOS16_FILL_BITS()
			bits = _akos16.bits & 3;
			if (bits & 1) {
				AKOS16_EAT_BITS(2)
				if (bits & 2) {
					tmp_bits = _akos16.bits & 7;
					AKOS16_EAT_BITS(3)
					if (tmp_bits != 4) {
						// A color change
						_akos16.color += (tmp_bits - 4);
					} else {
						// Color does not change, but rather identical pixels get repeated
						_akos16.repeatMode = true;
						AKOS16_FILL_BITS()
						_akos16.repeatCount = (_akos16.bits & 0xff) - 1;
						AKOS16_EAT_BITS(8)
						AKOS16_FILL_BITS()
					}
				} else {
					AKOS16_FILL_BITS()
					_akos16.color = ((byte)_akos16.bits) & _akos16.mask;
					AKOS16_EAT_BITS(_akos16.shift)
					AKOS16_FILL_BITS()
				}
			} else {
				AKOS16_EAT_BITS(1);
			}
		} else {
			if (--_akos16.repeatCount == 0) {
				_akos16.repeatMode = false;
			}
		}
		numbytes--;
	}
}

void AkosRenderer::akos16Decompress(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir,
		int32 numskip_before, int32 numskip_after, byte transparency, int maskLeft, int maskTop, int zBuf) {
	byte *tmp_buf = _akos16.buffer;
	int maskpitch;
	byte *maskptr;
	const byte maskbit = revBitMask(maskLeft & 7);

	if (dir < 0) {
		dest -= (t_width - 1);
		tmp_buf += (t_width - 1);
	}

	akos16SetupBitReader(src);

	if (numskip_before != 0) {
		akos16SkipData(numskip_before);
	}

	maskpitch = _numStrips;

	maskptr = _vm->getMaskBuffer(maskLeft, maskTop, zBuf);

	assert(t_height > 0);
	assert(t_width > 0);
	while (t_height--) {
		akos16DecodeLine(tmp_buf, t_width, dir);
		bompApplyMask(_akos16.buffer, maskptr, maskbit, t_width, transparency);
		bool HE7Check = (_vm->_game.heversion == 70);
		bompApplyShadow(_shadow_mode, _shadow_table, _akos16.buffer, dest, t_width, transparency, HE7Check);

		if (numskip_after != 0)	{
			akos16SkipData(numskip_after);
		}
		dest += pitch;
		maskptr += maskpitch;
	}
}

byte AkosRenderer::codec16(int xmoveCur, int ymoveCur) {
	assert(_vm->_bytesPerPixel == 1);

	Common::Rect clip;
	int32 minx, miny, maxw, maxh;
	int32 skip_x, skip_y, cur_x, cur_y;
	byte transparency = (_vm->_game.heversion >= 61) ? _palette[0] : 255;

	if (_actorHitMode) {
		error("codec16: _actorHitMode not yet implemented");
		return 0;
	}

	if (!_mirror) {
		clip.left = (_actorX - xmoveCur - _width) + 1;
	} else {
		clip.left = _actorX + xmoveCur;
	}

	clip.top = _actorY + ymoveCur;
	clip.right = clip.left + _width;
	clip.bottom = clip.top + _height;

	minx = miny = 0;
	maxw = _out.w;
	maxh = _out.h;

	if (_vm->_game.heversion >= 71) {
		if (_clipOverride.right > _clipOverride.left && _clipOverride.bottom > _clipOverride.top) {
			minx = _clipOverride.left;
			miny = _clipOverride.top;
			maxw = _clipOverride.right;
			maxh = _clipOverride.bottom;
		}
	}

	markRectAsDirty(clip);

	skip_x = 0;
	skip_y = 0;
	cur_x = _width - 1;
	cur_y = _height - 1;

	if (clip.left < minx) {
		skip_x = -clip.left;
		clip.left = 0;
	}

	if (clip.right > maxw) {
		cur_x -= clip.right - maxw;
		clip.right = maxw;
	}

	if (clip.top < miny) {
		skip_y -= clip.top;
		clip.top = 0;
	}

	if (clip.bottom > maxh) {
		cur_y -= clip.bottom - maxh;
		clip.bottom = maxh;
	}

	if ((clip.left >= clip.right) || (clip.top >= clip.bottom))
		return 0;

	if (_draw_top > clip.top)
		_draw_top = clip.top;
	if (_draw_bottom < clip.bottom)
		_draw_bottom = clip.bottom;

	int32 width_unk, height_unk;

	height_unk = clip.top;
	int32 dir;

	if (!_mirror) {
		dir = -1;

		int tmp_skip_x = skip_x;
		skip_x = _width - 1 - cur_x;
		cur_x = _width - 1 - tmp_skip_x;
		width_unk = clip.right - 1;
	} else {
		dir = 1;
		width_unk = clip.left;
	}

	int32 out_height;

	out_height = cur_y - skip_y;
	if (out_height < 0) {
		out_height = -out_height;
	}
	out_height++;

	cur_x -= skip_x;
	if (cur_x < 0) {
		cur_x = -cur_x;
	}
	cur_x++;

	int32 numskip_before = skip_x + (skip_y * _width);
	int32 numskip_after = _width - cur_x;

	byte *dst = (byte *)_out.pixels + height_unk * _out.pitch + width_unk * _vm->_bytesPerPixel;

	akos16Decompress(dst, _out.pitch, _srcptr, cur_x, out_height, dir, numskip_before, numskip_after, transparency, clip.left, clip.top, _zbuf);
	return 0;
}

byte AkosRenderer::codec32(int xmoveCur, int ymoveCur) {
#ifdef ENABLE_HE
	Common::Rect src, dst;

	if (!_mirror) {
		dst.left = (_actorX - xmoveCur - _width) + 1;
	} else {
		dst.left = _actorX + xmoveCur;
	}

	src.top = src.left = 0;
	src.right = _width;
	src.bottom = _height;

	dst.top = _actorY + ymoveCur;
	dst.right = dst.left + _width;
	dst.bottom = dst.top + _height;

	int diff;
	diff = dst.left - _clipOverride.left;
	if (diff < 0) {
		src.left -= diff;
		dst.left -= diff;
	}
	diff = dst.right - _clipOverride.right;
	if (diff > 0) {
		src.right -= diff;
		dst.right -= diff;
	}
	diff = dst.top - _clipOverride.top;
	if (diff < 0) {
		src.top -= diff;
		dst.top -= diff;
	}
	diff = dst.bottom - _clipOverride.bottom;
	if (diff > 0) {
		src.bottom -= diff;
		dst.bottom -= diff;
	}

	if (dst.isValidRect() == false)
		return 0;

	markRectAsDirty(dst);

	if (_draw_top > dst.top)
		_draw_top = dst.top;
	if (_draw_bottom < dst.bottom)
		_draw_bottom = dst.bottom;

	const uint8 *palPtr = NULL;
	if (_vm->_game.features & GF_16BIT_COLOR) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot + 768;
		if (_paletteNum) {
			palPtr = _vm->_hePalettes + _paletteNum * _vm->_hePaletteSlot + 768;
		} else if (rgbs) {
			for (uint i = 0; i < 256; i++)
				WRITE_LE_UINT16(_palette + i, _vm->get16BitColor(rgbs[i * 3 + 0], rgbs[i * 3 + 1], rgbs[i * 3 + 2]));
			palPtr = (uint8 *)_palette;
		}
	} else if (_vm->_game.heversion >= 99) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot + 768;
	}

	byte *dstPtr = (byte *)_out.pixels + dst.top * _out.pitch + dst.left * _vm->_bytesPerPixel;
	if (_shadow_mode == 3) {
		Wiz::decompressWizImage<kWizXMap>(dstPtr, _out.pitch, kDstScreen, _srcptr, src, 0, palPtr, xmap, _vm->_bytesPerPixel);
	} else {
		if (palPtr != NULL) {
			Wiz::decompressWizImage<kWizRMap>(dstPtr, _out.pitch, kDstScreen, _srcptr, src, 0, palPtr, NULL, _vm->_bytesPerPixel);
		} else {
			Wiz::decompressWizImage<kWizCopy>(dstPtr, _out.pitch, kDstScreen, _srcptr, src, 0, NULL, NULL, _vm->_bytesPerPixel);
		}
	}
#endif
	return 0;
}

byte AkosCostumeLoader::increaseAnims(Actor *a) {
	return ((ScummEngine_v6 *)_vm)->akos_increaseAnims(_akos, a);
}

bool ScummEngine_v6::akos_increaseAnims(const byte *akos, Actor *a) {
	const byte *aksq, *akfo;
	int i;
	uint size;
	bool result;

	aksq = findResourceData(MKTAG('A','K','S','Q'), akos);
	akfo = findResourceData(MKTAG('A','K','F','O'), akos);

	size = getResourceDataSize(akfo) / 2;

	result = false;
	for (i = 0; i < 16; i++) {
		if (a->_cost.active[i] != 0)
			result |= akos_increaseAnim(a, i, aksq, (const uint16 *)akfo, size);
	}
	return result;
}

#define GW(o) ((int16)READ_LE_UINT16(aksq+curpos+(o)))
#define GUW(o) READ_LE_UINT16(aksq+curpos+(o))
#define GB(o) aksq[curpos+(o)]

bool ScummEngine_v6::akos_increaseAnim(Actor *a, int chan, const byte *aksq, const uint16 *akfo, int numakfo) {
	byte active;
	uint old_curpos, curpos, end;
	uint code;
	bool flag_value, needRedraw;
	int tmp, tmp2;

	active = a->_cost.active[chan];
	end = a->_cost.end[chan];
	old_curpos = curpos = a->_cost.curpos[chan];
	flag_value = false;
	needRedraw = false;

	do {

		code = aksq[curpos];
		if (code & 0x80)
			code = READ_BE_UINT16(aksq + curpos);

		switch (active) {
		case 6:
		case 8:
			switch (code) {
			case AKC_JumpIfSet:
			case AKC_AddVar:
			case AKC_SetVar:
			case AKC_SkipGE:
			case AKC_SkipG:
			case AKC_SkipLE:
			case AKC_SkipL:

			case AKC_SkipNE:
			case AKC_SkipE:
			case AKC_C016:
			case AKC_C017:
			case AKC_C018:
			case AKC_C019:
				curpos += 5;
				break;
			case AKC_JumpTable:
			case AKC_SetActorClip:
			case AKC_Ignore3:
			case AKC_Ignore2:
			case AKC_Ignore:
			case AKC_StartAnim:
			case AKC_StartVarAnim:
			case AKC_CmdQue3:
			case AKC_C042:
			case AKC_C044:
			case AKC_C0A3:
				curpos += 3;
				break;
			case AKC_SoundStuff:
				if (_game.heversion >= 61)
					curpos += 6;
				else
					curpos += 8;
				break;
			case AKC_Cmd3:
			case AKC_SetVarInActor:
			case AKC_SetDrawOffs:
				curpos += 6;
				break;
			case AKC_ClearFlag:
			case AKC_HideActor:
			case AKC_IncVar:
			case AKC_CmdQue3Quick:
			case AKC_Return:
			case AKC_EndSeq:
				curpos += 2;
				break;
			case AKC_JumpGE:
			case AKC_JumpG:
			case AKC_JumpLE:
			case AKC_JumpL:
			case AKC_JumpNE:
			case AKC_JumpE:
			case AKC_Random:
				curpos += 7;
				break;
			case AKC_Flip:
			case AKC_Jump:
			case AKC_StartAnimInActor:
			case AKC_C0A0:
			case AKC_C0A1:
			case AKC_C0A2:
				curpos += 4;
				break;
			case AKC_ComplexChan2:
				curpos += 4;
				// Fall through
			case AKC_ComplexChan:
				curpos += 3;
				tmp = aksq[curpos - 1];
				while (--tmp >= 0) {
					curpos += 4;
					curpos += (aksq[curpos] & 0x80) ? 2 : 1;
				}
				break;
			case AKC_C021:
			case AKC_C022:
			case AKC_C045:
			case AKC_C046:
			case AKC_C047:
			case AKC_C048:
				needRedraw = 1;
				curpos += aksq[curpos + 2];
				break;
			case AKC_C08E:
				akos_queCommand(7, a, GW(2), 0);
				curpos += 4;
				break;
			default:
				curpos += (code & 0x8000) ? 2 : 1;
				break;
			}
			break;
		case 2:
			curpos += (code & 0x8000) ? 2 : 1;
			if (curpos > end)
				curpos = a->_cost.start[chan];
			break;
		case 3:
			if (curpos != end)
				curpos += (code & 0x8000) ? 2 : 1;
			break;
		}

		code = aksq[curpos];
		if (code & 0x80)
			code = READ_BE_UINT16(aksq + curpos);

		if (flag_value && code != AKC_ClearFlag)
			continue;

		switch (code) {
		case AKC_StartAnimInActor:
			akos_queCommand(4, derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:29"), a->getAnimVar(GB(3)), 0);
			continue;

		case AKC_Random:
			a->setAnimVar(GB(6), _rnd.getRandomNumberRng(GW(2), GW(4)));
			continue;
		case AKC_JumpGE:
		case AKC_JumpG:
		case AKC_JumpLE:
		case AKC_JumpL:
		case AKC_JumpNE:
		case AKC_JumpE:
			if (akos_compare(a->getAnimVar(GB(4)), GW(5), code - AKC_JumpStart) != 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_IncVar:
			a->setAnimVar(0, a->getAnimVar(0) + 1);
			continue;
		case AKC_SetVar:
			a->setAnimVar(GB(4), GW(2));
			continue;
		case AKC_AddVar:
			a->setAnimVar(GB(4), a->getAnimVar(GB(4)) + GW(2));
			continue;
		case AKC_Flip:
			a->_flip = GW(2) != 0;
			continue;
		case AKC_CmdQue3:
			if (_game.heversion >= 61)
				tmp = GB(2);
			else
				tmp = GB(2) - 1;
			if ((uint) tmp < 24)
				akos_queCommand(3, a, a->_sound[tmp], 0);
			continue;
		case AKC_CmdQue3Quick:
			akos_queCommand(3, a, a->_sound[0], 0);
			continue;
		case AKC_StartAnim:
			akos_queCommand(4, a, GB(2), 0);
			continue;
		case AKC_StartVarAnim:
			akos_queCommand(4, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_SetVarInActor:
			derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:9")->setAnimVar(GB(3), GW(4));
			continue;
		case AKC_HideActor:
			akos_queCommand(1, a, 0, 0);
			continue;
		case AKC_SetActorClip:
			akos_queCommand(5, a, GB(2), 0);
			continue;
		case AKC_SoundStuff:
			if (_game.heversion >= 61)
				continue;
			tmp = GB(2) - 1;
			if (tmp >= 8)
				continue;
			tmp2 = GB(4);
			if (tmp2 < 1 || tmp2 > 3)
				error("akos_increaseAnim:8 invalid code %d", tmp2);
			akos_queCommand(tmp2 + 6, a, a->_sound[tmp], GB(6));
			continue;
		case AKC_SetDrawOffs:
			akos_queCommand(6, a, GW(2), GW(4));
			continue;
		case AKC_JumpTable:
			if (akfo == NULL)
				error("akos_increaseAnim: no AKFO table");
			tmp = a->getAnimVar(GB(2)) - 1;
			if (_game.heversion >= 80) {
				if (tmp < 0 || tmp > a->_cost.heJumpCountTable[chan] - 1)
					error("akos_increaseAnim: invalid jump value %d", tmp);
				curpos = READ_LE_UINT16(akfo + a->_cost.heJumpOffsetTable[chan] + tmp * 2);
			} else {
				if (tmp < 0 || tmp > numakfo - 1)
					error("akos_increaseAnim: invalid jump value %d", tmp);
				curpos = READ_LE_UINT16(&akfo[tmp]);
			}
			break;
		case AKC_JumpIfSet:
			if (!a->getAnimVar(GB(4)))
				continue;
			a->setAnimVar(GB(4), 0);
			curpos = GUW(2);
			break;

		case AKC_ClearFlag:
			flag_value = false;
			continue;

		case AKC_Jump:
			curpos = GUW(2);
			break;

		case AKC_Return:
		case AKC_EndSeq:
		case AKC_ComplexChan:
		case AKC_C08E:
		case AKC_ComplexChan2:
			break;

		case AKC_C021:
		case AKC_C022:
			needRedraw = 1;
			break;

		case AKC_Cmd3:
		case AKC_Ignore:
		case AKC_Ignore3:
			continue;

		case AKC_Ignore2:
			if (_game.heversion >= 71)
				akos_queCommand(3, a, a->_sound[a->getAnimVar(GB(2))], 0);
			continue;

		case AKC_SkipE:
		case AKC_SkipNE:
		case AKC_SkipL:
		case AKC_SkipLE:
		case AKC_SkipG:
		case AKC_SkipGE:
			if (akos_compare(a->getAnimVar(GB(4)), GW(2), code - AKC_SkipStart) == 0)
				flag_value = true;
			continue;
		case AKC_C016:
			if (_sound->isSoundRunning( a->_sound[a->getAnimVar(GB(4))]))  {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C017:
			if (!_sound->isSoundRunning(a->_sound[a->getAnimVar(GB(4))])) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C018:
			if (_sound->isSoundRunning(a->_sound[GB(4)])) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C019:
			if (!_sound->isSoundRunning(a->_sound[GB(4)])) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C042:
			akos_queCommand(9, a, a->_sound[GB(2)], 0);
			continue;
		case AKC_C044:
			akos_queCommand(9, a, a->_sound[a->getAnimVar(GB(2))], 0);
			continue;
		case AKC_C045:
			((ActorHE *)a)->setUserCondition(GB(3), a->getAnimVar(GB(4)));
			continue;
		case AKC_C046:
			a->setAnimVar(GB(4), ((ActorHE *)a)->isUserConditionSet(GB(3)));
			continue;
		case AKC_C047:
			((ActorHE *)a)->setTalkCondition(GB(3));
			continue;
		case AKC_C048:
			a->setAnimVar(GB(4), ((ActorHE *)a)->isTalkConditionSet(GB(3)));
			continue;
		case AKC_C0A0:
			akos_queCommand(8, a, GB(2), 0);
			continue;
		case AKC_C0A1:
			if (((ActorHE *)a)->_heTalking != 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C0A2:
			if (((ActorHE *)a)->_heTalking == 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C0A3:
			akos_queCommand(8, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_C0A4:
			if (VAR(VAR_TALK_ACTOR) != 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C0A5:
			if (VAR(VAR_TALK_ACTOR) == 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		default:
			if ((code & 0xC000) == 0xC000)
				error("Undefined uSweat token %X", code);
		}
		break;
	} while (1);

	int code2 = aksq[curpos];
	if (code2 & 0x80)
		code2 = READ_BE_UINT16(aksq + curpos);

	if ((code2 & 0xC000) == 0xC000 && code2 != AKC_ComplexChan && code2 != AKC_Return && code2 != AKC_EndSeq && code2 != AKC_C08E && code2 != AKC_ComplexChan2 && code2 != AKC_C021 && code2 != AKC_C022)
		error("Ending with undefined uSweat token %X", code2);

	a->_cost.curpos[chan] = curpos;

	if (needRedraw)
		return 1;
	else
		return curpos != old_curpos;
}

void ScummEngine_v6::akos_queCommand(byte cmd, Actor *a, int param_1, int param_2) {
	_akosQueuePos++;
	assertRange(0, _akosQueuePos, 31, "akos_queCommand: _akosQueuePos");

	_akosQueue[_akosQueuePos].cmd = cmd;
	_akosQueue[_akosQueuePos].actor = a->_number;
	_akosQueue[_akosQueuePos].param1 = param_1;
	_akosQueue[_akosQueuePos].param2 = param_2;
}

void ScummEngine_v6::akos_processQueue() {
	byte cmd;
	int actor, param_1, param_2;

	while (_akosQueuePos) {
		cmd = _akosQueue[_akosQueuePos].cmd;
		actor = _akosQueue[_akosQueuePos].actor;
		param_1 = _akosQueue[_akosQueuePos].param1;
		param_2 = _akosQueue[_akosQueuePos].param2;
		_akosQueuePos--;

		Actor *a = derefActor(actor, "akos_processQueue");

		switch (cmd) {
		case 1:
			a->putActor(0, 0, 0);
			break;
		case 3:
			_sound->addSoundToQueue(param_1, 0, -1, 0);
			break;
		case 4:
			a->startAnimActor(param_1);
			break;
		case 5:
			a->_forceClip = param_1;
			break;
		case 6:
			a->_heOffsX = param_1;
			a->_heOffsY = param_2;
			break;
		case 7:
#ifdef ENABLE_HE
			assert(_game.heversion >= 71);
			((ScummEngine_v71he *)this)->queueAuxEntry(a->_number, param_1);
#endif
			break;
		case 8:
			_actorToPrintStrFor = a->_number;

			a->_talkPosX = ((ActorHE *)a)->_heTalkQueue[param_1].posX;
			a->_talkPosY = ((ActorHE *)a)->_heTalkQueue[param_1].posY;
			a->_talkColor = ((ActorHE *)a)->_heTalkQueue[param_1].color;

			_string[0].loadDefault();
			_string[0].color = a->_talkColor;
			actorTalk(((ActorHE *)a)->_heTalkQueue[param_1].sentence);

			break;
		case 9:
			_sound->addSoundToQueue(param_1, 0, -1, 4);
			break;
		default:
			error("akos_queCommand(%d,%d,%d,%d)", cmd, a->_number, param_1, param_2);
		}
	}
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::akos_processQueue() {
	byte cmd;
	int actor, param_1, param_2;

	while (_akosQueuePos) {
		cmd = _akosQueue[_akosQueuePos].cmd;
		actor = _akosQueue[_akosQueuePos].actor;
		param_1 = _akosQueue[_akosQueuePos].param1;
		param_2 = _akosQueue[_akosQueuePos].param2;
		_akosQueuePos--;

		Actor *a = derefActor(actor, "akos_processQueue");

		switch (cmd) {
		case 1:
			a->putActor(0, 0, 0);
			break;
		case 3:
			if (param_1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->startSfx(param_1, 63);
				}
			}
			break;
		case 4:
			a->startAnimActor(param_1);
			break;
		case 5:
			a->_forceClip = param_1;
			break;
		case 6:
			a->_heOffsX = param_1;
			a->_heOffsY = param_2;
			break;
		case 7:
			if (param_1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->setVolume(param_1, param_2);
				}
			}
			break;
		case 8:
			if (param_1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->setPan(param_1, param_2);
				}
			}
			break;
		case 9:
			if (param_1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->setPriority(param_1, param_2);
				}
			}
			break;
		default:
			error("akos_queCommand(%d,%d,%d,%d)", cmd, a->_number, param_1, param_2);
		}
	}
}
#endif

} // End of namespace Scumm
