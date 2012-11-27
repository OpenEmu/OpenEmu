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

// AGOS debug functions

#include "common/file.h"
#include "common/textconsole.h"

#include "agos/debug.h"
#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/vga.h"

namespace AGOS {

const byte *AGOSEngine::dumpOpcode(const byte *p) {
	uint16 opcode;
	const char *s, *st;

	if (getGameType() == GType_ELVIRA1) {
		opcode = READ_BE_UINT16(p);
		p += 2;
		if (opcode == 10000)
			return NULL;
	} else {
		opcode = *p++;
		if (opcode == 255)
			return NULL;
	}

	if (getGameType() == GType_PP) {
		st = s = puzzlepack_opcodeNameTable[opcode];
	} else if (getGameType() == GType_FF) {
		st = s = feeblefiles_opcodeNameTable[opcode];
	} else if (getGameType() == GType_SIMON2 && getFeatures() & GF_TALKIE) {
		st = s = simon2talkie_opcodeNameTable[opcode];
	} else if (getGameType() == GType_SIMON2) {
		st = s = simon2dos_opcodeNameTable[opcode];
	} else if (getFeatures() & GF_TALKIE) {
		st = s = simon1talkie_opcodeNameTable[opcode];
	} else if (getGameType() == GType_SIMON1) {
		st = s = simon1dos_opcodeNameTable[opcode];
	} else if (getGameType() == GType_WW) {
		st = s = waxworks_opcodeNameTable[opcode];
	} else if (getGameType() == GType_ELVIRA2) {
		st = s = elvira2_opcodeNameTable[opcode];
	} else {
		st = s = elvira1_opcodeNameTable[opcode];
	}

	if (s == NULL) {
		error("dumpOpcode: INVALID OPCODE %d", opcode);
	}

	while (*st != '|')
		st++;
	debugN("%s ", st + 1);

	for (;;) {
		switch (*s++) {
		case 'x':
			debugN("\n");
			return NULL;
		case '|':
			debugN("\n");
			return p;
		case 'B':{
				byte b = *p++;
				if (b == 255)
					debugN("[%d] ", *p++);
				else
					debugN("%d ", b);
				break;
			}
		case 'V':{
				byte b = *p++;
				if (b == 255)
					debugN("[[%d]] ", *p++);
				else
					debugN("[%d] ", b);
				break;
			}

		case 'W':{
				int n = (int16)READ_BE_UINT16(p);
				p += 2;
				if (getGameType() == GType_PP) {
					if (n >= 60000 && n < 62048)
						debugN("[%d] ", n - 60000);
					else
						debugN("%d ", n);

				} else {
					if (n >= 30000 && n < 30512)
						debugN("[%d] ", n - 30000);
					else
						debugN("%d ", n);
				}
				break;
			}

		case 'w':{
				int n = (int16)READ_BE_UINT16(p);
				p += 2;
				debugN("%d ", n);
				break;
			}

		case 'I':{
				int n = (int16)READ_BE_UINT16(p);
				p += 2;
				if (n == -1)
					debugN("SUBJECT_ITEM ");
				else if (n == -3)
					debugN("OBJECT_ITEM ");
				else if (n == -5)
					debugN("ME_ITEM ");
				else if (n == -7)
					debugN("ACTOR_ITEM ");
				else if (n == -9)
					debugN("ITEM_A_PARENT ");
				else
					debugN("<%d> ", n);
				break;
			}

		case 'J':{
				debugN("-> ");
			}
			break;

		case 'T':{
				uint n = READ_BE_UINT16(p);
				p += 2;
				if (n != 0xFFFF)
					debugN("\"%s\"(%d) ", getStringPtrByID(n), n);
				else
					debugN("NULL_STRING ");
			}
			break;
		}
	}
}

void AGOSEngine::dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	const byte *p;

	debugN("; ****\n");

	p = (byte *)sl + SUBROUTINE_LINE_SMALL_SIZE;
	if (sub->id == 0) {
		debugN("; verb=%d, noun1=%d, noun2=%d\n", sl->verb, sl->noun1, sl->noun2);
		p = (byte *)sl + SUBROUTINE_LINE_BIG_SIZE;
	}

	for (;;) {
		p = dumpOpcode(p);
		if (p == NULL)
			break;
	}
}

void AGOSEngine::dumpSubroutine(Subroutine *sub) {
	SubroutineLine *sl;

	debugN("\n******************************************\n;Subroutine, ID=%d:\nSUB_%d:\n", sub->id, sub->id);
	sl = (SubroutineLine *)((byte *)sub + sub->first);
	for (; (byte *)sl != (byte *)sub; sl = (SubroutineLine *)((byte *)sub + sl->next)) {
		dumpSubroutineLine(sl, sub);
	}
	debugN("\nEND ******************************************\n");
}

void AGOSEngine::dumpSubroutines() {
	Subroutine *sub = _subroutineList;
	for (; sub; sub = sub->next) {
		dumpSubroutine(sub);
	}
}

void AGOSEngine::dumpAllSubroutines() {
	for (int i = 0; i < 65536; i++) {
		Subroutine *sub = getSubroutineByID(i);
		if (sub != NULL) {
			dumpSubroutine(sub);
		}
	}
}

void AGOSEngine::dumpVideoScript(const byte *src, bool singeOpcode) {
	uint16 opcode;
	const char *str, *strn;

	do {
		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
			opcode = *src++;
		} else {
			opcode = READ_BE_UINT16(src);
			src += 2;
		}

		if (opcode >= _numVideoOpcodes) {
			error("dumpVideoScript: Opcode %d out of range (%d)", opcode, _numVideoOpcodes);
		}

		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			strn = str = feeblefiles_videoOpcodeNameTable[opcode];
		} else if (getGameType() == GType_SIMON2) {
			strn = str = simon2_videoOpcodeNameTable[opcode];
		} else if (getGameType() == GType_SIMON1) {
			strn = str = simon1_videoOpcodeNameTable[opcode];
		} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			strn = str = ww_videoOpcodeNameTable[opcode];
		} else if (getGameType() == GType_ELVIRA1) {
			strn = str = elvira1_videoOpcodeNameTable[opcode];
		} else {
			strn = str = pn_videoOpcodeNameTable[opcode];
		}

		if (strn == NULL) {
			error("dumpVideoScript: Invalid Opcode %d", opcode);
		}

		while (*strn != '|')
			strn++;
		debugN("%.2d: %s ", opcode, strn + 1);

		int end = (getGameType() == GType_FF || getGameType() == GType_PP) ? 9999 : 999;
		for (; *str != '|'; str++) {
			switch (*str) {
			case 'x':
				debugN("\n");
				return;
			case 'b':
				debugN("%d ", *src++);
				break;
			case 'd':
				debugN("%d ", (int16)readUint16Wrapper(src));
				src += 2;
				break;
			case 'v':
				debugN("[%d] ", readUint16Wrapper(src));
				src += 2;
				break;
			case 'i':
				debugN("%d ", (int16)readUint16Wrapper(src));
				src += 2;
				break;
			case 'j':
				debugN("-> ");
				break;
			case 'q':
				while (readUint16Wrapper(src) != end) {
					debugN("(%d,%d) ", readUint16Wrapper(src),
									readUint16Wrapper(src + 2));
					src += 4;
				}
				src += 2;
				break;
			default:
				error("dumpVideoScript: Invalid fmt string '%c' in decompile VGA", *str);
			}
		}

		debugN("\n");
	} while (!singeOpcode);
}

void AGOSEngine::dumpVgaScript(const byte *ptr, uint16 res, uint16 id) {
	dumpVgaScriptAlways(ptr, res, id);
}

void AGOSEngine::dumpVgaScriptAlways(const byte *ptr, uint16 res, uint16 id) {
	debugN("; address=%x, vgafile=%d  vgasprite=%d\n",
					(unsigned int)(ptr - _vgaBufferPointers[res].vgaFile1), res, id);
	dumpVideoScript(ptr, false);
	debugN("; end\n");
}

void AGOSEngine::dumpAllVgaImageFiles() {
	const uint8 start = (getGameType() == GType_PN) ? 0 : 2;

	for (int z = start; z < _numZone; z++) {
		loadZone(z, false);
		dumpVgaBitmaps(z);
	}
}

void AGOSEngine::dumpAllVgaScriptFiles() {
	const uint8 start = (getGameType() == GType_PN) ? 0 : 2;

	for (int z = start; z < _numZone; z++) {
		uint16 zoneNum = (getGameType() == GType_PN) ? 0 : z;
		loadZone(z, false);

		VgaPointersEntry *vpe = &_vgaBufferPointers[zoneNum];
		if (vpe->vgaFile1 != NULL) {
			_curVgaFile1 = vpe->vgaFile1;
			dumpVgaFile(_curVgaFile1);
		}
	}
}

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::dumpVgaFile(const byte *vga) {
	const byte *pp;
	const byte *p;
	int16 count;

	pp = vga;
	p = pp + READ_LE_UINT16(pp + 2);
	count = READ_LE_UINT16(&((const VgaFile1Header_Feeble *) p)->animationCount);
	p = pp + READ_LE_UINT16(&((const VgaFile1Header_Feeble *) p)->animationTable);

	while (--count >= 0) {
		uint16 id = READ_LE_UINT16(&((const AnimationHeader_Feeble *) p)->id);

		dumpVgaScriptAlways(vga + READ_LE_UINT16(&((const AnimationHeader_Feeble *) p)->scriptOffs), id / 100, id);
		p += sizeof(AnimationHeader_Feeble);
	}

	pp = vga;
	p = pp + READ_LE_UINT16(pp + 2);
	count = READ_LE_UINT16(&((const VgaFile1Header_Feeble *) p)->imageCount);
	p = pp + READ_LE_UINT16(&((const VgaFile1Header_Feeble *) p)->imageTable);

	while (--count >= 0) {
		uint16 id = READ_LE_UINT16(&((const ImageHeader_Feeble *) p)->id);

		dumpVgaScriptAlways(vga + READ_LE_UINT16(&((const ImageHeader_Feeble *) p)->scriptOffs), id / 100, id);
		p += sizeof(ImageHeader_Feeble);
	}
}
#endif

void AGOSEngine_Simon1::dumpVgaFile(const byte *vga) {
	const byte *pp;
	const byte *p;
	int16 count;

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 4);
	count = READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->animationCount);
	p = pp + READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->animationTable);

	while (--count >= 0) {
		uint16 id = READ_BE_UINT16(&((const AnimationHeader_Simon *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const AnimationHeader_Simon *) p)->scriptOffs), id / 100, id);
		p += sizeof(AnimationHeader_Simon);
	}

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 4);
	count = READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->imageCount);
	p = pp + READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->imageTable);

	while (--count >= 0) {
		uint16 id = READ_BE_UINT16(&((const ImageHeader_Simon *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const ImageHeader_Simon *) p)->scriptOffs), id / 100, id);
		p += sizeof(ImageHeader_Simon);
	}
}

void AGOSEngine::dumpVgaFile(const byte *vga) {
	const byte *pp;
	const byte *p;
	int16 count;

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 10) + 20;
	count = READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->animationCount);
	p = pp + READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->animationTable);

	while (--count >= 0) {
		uint16 id = READ_BE_UINT16(&((const AnimationHeader_WW *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const AnimationHeader_WW *) p)->scriptOffs), id / 100, id);
		p += sizeof(AnimationHeader_WW);
	}

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 10) + 20;
	count = READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->imageCount);
	p = pp + READ_BE_UINT16(&((const VgaFile1Header_Common *) p)->imageTable);

	while (--count >= 0) {
		uint16 id = READ_BE_UINT16(&((const ImageHeader_WW *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const ImageHeader_WW *) p)->scriptOffs), id / 100, id);
		p += sizeof(ImageHeader_WW);
	}
}

static const byte bmp_hdr[] = {
	0x42, 0x4D,
	0x9E, 0x14, 0x00, 0x00,				/* offset 2, file size */
	0x00, 0x00, 0x00, 0x00,
	0x36, 0x04, 0x00, 0x00,
	0x28, 0x00, 0x00, 0x00,

	0x3C, 0x00, 0x00, 0x00,				/* image width */
	0x46, 0x00, 0x00, 0x00,				/* image height */
	0x01, 0x00, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,

	0x00, 0x01, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00,
};

void dumpBMP(const char *filename, int16 w, int16 h, const byte *bytes, const byte *palette) {
	Common::DumpFile out;
	byte my_hdr[sizeof(bmp_hdr)];
	int i;

	out.open(filename);
	if (!out.isOpen())
		return;

	memcpy(my_hdr, bmp_hdr, sizeof(bmp_hdr));

	*(uint32 *)(my_hdr + 2) = w * h + 1024 + sizeof(bmp_hdr);
	*(uint32 *)(my_hdr + 18) = w;
	*(uint32 *)(my_hdr + 22) = h;


	out.write(my_hdr, sizeof(my_hdr));

	for (i = 0; i != 256; i++, palette += 3) {
		byte color[4];
		color[0] = palette[2];
		color[1] = palette[1];
		color[2] = palette[0];
		color[3] = 0;
		out.write(color, 4);
	}

	while (--h >= 0) {
		out.write(bytes + h * ((w + 3) & ~3), ((w + 3) & ~3));
	}
}

void AGOSEngine::dumpBitmap(const char *filename, const byte *offs, uint16 w, uint16 h, int flags, const byte *palette,
								 byte base) {

	byte *imageBuffer = (byte *)malloc(w * h);
	assert(imageBuffer);

	VC10_state state;
	state.depack_cont = -0x80;
	state.srcPtr = offs;
	state.dh = h;
	state.height = h;
	state.width = w / 16;

	if (getFeatures() & GF_PLANAR) {
		state.srcPtr = convertImage(&state, (getGameType() == GType_PN || (flags & 0x80) != 0));
		flags &= ~0x80;
	}

	const byte *src = state.srcPtr;
	byte *dst = imageBuffer;
	int i, j;

	if (w > _screenWidth) {
		for (i = 0; i < w; i += 8) {
			decodeColumn(dst, src + readUint32Wrapper(src), h, w);
			dst += 8;
			src += 4;
		}
	} else if (h > _screenHeight) {
		for (i = 0; i < h; i += 8) {
			decodeRow(dst, src + readUint32Wrapper(src), w, w);
			dst += 8 * w;
			src += 4;
		}
	} else if (getGameType() == GType_FF || getGameType() == GType_PP) {
		if ((flags & 0x80)) {
			for (i = 0; i != w; i++) {
				byte *c = vc10_depackColumn(&state);
				for (j = 0; j != h; j++) {
					dst[j * w + i] = c[j];
				}
			}
		} else {
			for (j = 0; j != h; j++) {
				for (i = 0; i != w; i++) {
					dst[i] = src[i];
				}
			}
			dst += w;
			src += w;
		}
	} else if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) && w == 320 && (h == 134 || h == 135 || h == 200)) {
		for (j = 0; j != h; j++) {
			uint16 count = w / 8;

			byte *dstPtr = dst;
			do {
				uint32 bits = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3]);

				dstPtr[0] = (byte)((bits >> (32 - 5)) & 31);
				dstPtr[1] = (byte)((bits >> (32 - 10)) & 31);
				dstPtr[2] = (byte)((bits >> (32 - 15)) & 31);
				dstPtr[3] = (byte)((bits >> (32 - 20)) & 31);
				dstPtr[4] = (byte)((bits >> (32 - 25)) & 31);
				dstPtr[5] = (byte)((bits >> (32 - 30)) & 31);

				bits = (bits << 8) | src[4];

				dstPtr[6] = (byte)((bits >> (40 - 35)) & 31);
				dstPtr[7] = (byte)((bits) & 31);

				dstPtr += 8;
				src += 5;
			} while (--count);
			dst += w;
		}
	} else if (flags & 0x80) {
		for (i = 0; i != w; i += 2) {
			byte *c = vc10_depackColumn(&state);
			for (j = 0; j != h; j++) {
				byte col = c[j];
				dst[j * w + i] = (col >> 4) | base;
				dst[j * w + i + 1] = (col & 0xF) | base;
			}
		}
	} else {
		for (j = 0; j != h; j++) {
			for (i = 0; i != w / 2; i ++) {
				byte col = src[i];
				dst[i * 2] = (col >> 4) | base;
				dst[i * 2 + 1] = (col & 0xF) | base;
			}
			dst += w;
			src += w / 2;
		}
	}

	dumpBMP(filename, w, h, imageBuffer, palette);
	free(imageBuffer);
}

void AGOSEngine::dumpSingleBitmap(int file, int image, const byte *offs, int w, int h, byte base) {
	char buf[40];

	sprintf(buf, "dumps/File%d_Image%d.bmp", file, image);

	if (Common::File::exists(buf))
		return;

	dumpBitmap(buf, offs, w, h, 0, _displayPalette, base);
}

void AGOSEngine::palLoad(byte *pal, const byte *vga1, int a, int b) {
	const byte *src;
	uint16 num, palSize;
	byte *palptr = (byte *)&pal[0];

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		num = 256;
		palSize = 768;
	} else {
		num = 32;
		palSize = 96;
	}

	if (getGameType() == GType_PN && (getFeatures() & GF_EGA)) {
		memcpy(palptr, _displayPalette, 3 * 16);
	} else if (getGameType() == GType_PN || getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
		src = vga1 + READ_BE_UINT16(vga1 + 6) + b * 32;

		do {
			uint16 color = READ_BE_UINT16(src);
			palptr[0] = ((color & 0xf00) >> 8) * 32;
			palptr[1] = ((color & 0x0f0) >> 4) * 32;
			palptr[2] = ((color & 0x00f) >> 0) * 32;

			palptr += 3;
			src += 2;
		} while (--num);
	} else {
		src = vga1 + 6 + b * palSize;

		do {
			palptr[0] = src[0] << 2;
			palptr[1] = src[1] << 2;
			palptr[2] = src[2] << 2;

			palptr += 3;
			src += 3;
		} while (--num);
	}
}

void AGOSEngine::dumpVgaBitmaps(uint16 zoneNum) {
	uint16 width, height, flags;
	uint32 offs, offsEnd;
	const byte *p2;
	byte pal[768];

	uint16 zone = (getGameType() == GType_PN) ? 0 : zoneNum;
	VgaPointersEntry *vpe = &_vgaBufferPointers[zone];
	if (vpe->vgaFile1 == NULL || vpe->vgaFile2 == NULL)
		return;

	const byte *vga1 = vpe->vgaFile1;
	const byte *vga2 = vpe->vgaFile2;
	uint32 imageBlockSize = vpe->vgaFile2End - vpe->vgaFile2;

	memset(pal, 0, sizeof(pal));
	palLoad(pal, vga1, 0, 0);

	offsEnd = readUint32Wrapper(vga2 + 8);
	for (uint i = 1; ; i++) {
		if ((i * 8) >= offsEnd)
			break;

		p2 = vga2 + i * 8;
		offs = readUint32Wrapper(p2);

		width = readUint16Wrapper(p2 + 6);
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			height = READ_LE_UINT16(p2 + 4) & 0x7FFF;
			flags = p2[5];
		} else {
			height = p2[5];
			flags = p2[4];
		}

		debug(1, "Zone %d: Image %d. Offs= %d Width=%d, Height=%d, Flags=0x%X", zoneNum, i, offs, width, height, flags);
		if (offs >= imageBlockSize || width == 0 || height == 0)
			break;

		/* dump bitmap */
		char buf[40];
		sprintf(buf, "dumps/Res%d_Image%d.bmp", zoneNum, i);

		dumpBitmap(buf, vga2 + offs, width, height, flags, pal, 0);
	}
}

} // End of namespace AGOS
