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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_engine.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
static inline unsigned Sqr(int x) {
	return (x * x);
}

//////////////////////////////////////////////////////////////////////////////////
// Swap - swaps two integers
//////////////////////////////////////////////////////////////////////////////////
void BaseUtils::swap(int *a, int *b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}


//////////////////////////////////////////////////////////////////////////
float BaseUtils::normalizeAngle(float angle) {
	while (angle > 360) {
		angle -= 360;
	}
	while (angle < 0) {
		angle += 360;
	}

	return angle;
}


////////////////////////////////////////////////////////////////////////////////
void BaseUtils::createPath(const char *path, bool pathOnly) {
	/*  AnsiString pathStr;

	    if (!pathOnly) pathStr = PathUtil::getDirectoryName(path);
	    else pathStr = path;
	*/
//	try {
	warning("BaseUtils::CreatePath - not implemented: %s", path);
//		boost::filesystem::create_directories(path);
//	} catch (...) {
	return;
//	}
}


//////////////////////////////////////////////////////////////////////////
void BaseUtils::debugMessage(const char *text) {
	//MessageBox(hWnd, Text, "WME", MB_OK|MB_ICONINFORMATION);
}


//////////////////////////////////////////////////////////////////////////
char *BaseUtils::setString(char **string, const char *value) {
	delete[] *string;
	*string = new char[strlen(value) + 1];
	if (*string) {
		strcpy(*string, value);
	}
	return *string;
}

//////////////////////////////////////////////////////////////////////////
char *BaseUtils::strEntry(int entry, const char *str, const char delim) {
	int numEntries = 0;

	const char *start = NULL;
	int len = 0;

	for (uint32 i = 0; i <= strlen(str); i++) {
		if (numEntries == entry) {
			if (!start) {
				start = str + i;
			} else {
				len++;
			}
		}
		if (str[i] == delim || str[i] == '\0') {
			numEntries++;
			if (start) {
				char *ret = new char[len + 1];
				memset(ret, 0, len + 1);
				Common::strlcpy(ret, start, len + 1);
				return ret;
			}
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
int BaseUtils::randomInt(int from, int to) {
	if (to < from) {
		int i = to;
		to = from;
		from = i;
	}
	return BaseEngine::instance().randInt(from, to);
//	return (rand() % (to - from + 1)) + from;
}

//////////////////////////////////////////////////////////////////////////
float BaseUtils::randomFloat(float from, float to) {
	const uint32 randMax = RAND_MAX;
	float randNum = (float)BaseEngine::instance().randInt(0, randMax) / (float)randMax;
	return from + (to - from) * randNum;
}

//////////////////////////////////////////////////////////////////////////
float BaseUtils::randomAngle(float from, float to) {
	while (to < from) {
		to += 360;
	}
	return normalizeAngle(randomFloat(from, to));
}

//////////////////////////////////////////////////////////////////////////
void BaseUtils::RGBtoHSL(uint32 rgbColor, byte *outH, byte *outS, byte *outL) {
	float varR = (RGBCOLGetR(rgbColor) / 255.0f);
	float varG = (RGBCOLGetG(rgbColor) / 255.0f);
	float varB = (RGBCOLGetB(rgbColor) / 255.0f);

	//Min. value of RGB
	float varMin = MIN(varR, varG);
	varMin = MIN(varMin, varB);

	//Max. value of RGB
	float varMax = MAX(varR, varG);
	varMax = MAX(varMax, varB);

	//Delta RGB value
	float delMax = varMax - varMin;

	float H = 0.0f, S = 0.0f, L = 0.0f;

	L = (varMax + varMin) / 2.0f;

	//This is a gray, no chroma...
	if (delMax == 0) {
		H = 0;
		S = 0;
	}
	//Chromatic data...
	else {
		if (L < 0.5f) {
			S = delMax / (varMax + varMin);
		} else {
			S = delMax / (2.0f - varMax - varMin);
		}

		float delR = (((varMax - varR) / 6.0f) + (delMax / 2.0f)) / delMax;
		float delG = (((varMax - varG) / 6.0f) + (delMax / 2.0f)) / delMax;
		float delB = (((varMax - varB) / 6.0f) + (delMax / 2.0f)) / delMax;

		if (varR == varMax) {
			H = delB - delG;
		} else if (varG == varMax) {
			H = (1.0f / 3.0f) + delR - delB;
		} else if (varB == varMax) {
			H = (2.0f / 3.0f) + delG - delR;
		}

		if (H < 0) {
			H += 1;
		}
		if (H > 1) {
			H -= 1;
		}
	}

	*outH = (byte)(H * 255);
	*outS = (byte)(S * 255);
	*outL = (byte)(L * 255);
}


//////////////////////////////////////////////////////////////////////////
uint32 BaseUtils::HSLtoRGB(byte  InH, byte InS, byte InL) {
	float H = InH / 255.0f;
	float S = InS / 255.0f;
	float L = InL / 255.0f;

	byte R, G, B;


	if (S == 0) {
		R = (byte)(L * 255);
		G = (byte)(L * 255);
		B = (byte)(L * 255);
	} else {
		float var1, var2;

		if (L < 0.5) {
			var2 = L * (1.0 + S);
		} else {
			var2 = (L + S) - (S * L);
		}

		var1 = 2.0f * L - var2;

		R = (byte)(255 * Hue2RGB(var1, var2, H + (1.0f / 3.0f)));
		G = (byte)(255 * Hue2RGB(var1, var2, H));
		B = (byte)(255 * Hue2RGB(var1, var2, H - (1.0f / 3.0f)));
	}
	return BYTETORGBA(255, R, G, B);
}


//////////////////////////////////////////////////////////////////////////
float BaseUtils::Hue2RGB(float v1, float v2, float vH) {
	if (vH < 0.0f) {
		vH += 1.0f;
	}
	if (vH > 1.0f) {
		vH -= 1.0f;
	}
	if ((6.0f * vH) < 1.0f) {
		return (v1 + (v2 - v1) * 6.0f * vH);
	}
	if ((2.0f * vH) < 1.0f) {
		return (v2);
	}
	if ((3.0f * vH) < 2.0f) {
		return (v1 + (v2 - v1) * ((2.0f / 3.0f) - vH) * 6.0f);
	}
	return (v1);
}

} // end of namespace Wintermute
