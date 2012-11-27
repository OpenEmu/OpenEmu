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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "common/localization.h"
#include "common/translation.h"

namespace Common {

void getLanguageYesNo(Language id, KeyCode &keyYes, KeyCode &keyNo) {
	// If all else fails, use English as fallback.
	keyYes = KEYCODE_y;
	keyNo = KEYCODE_n;

	switch (id) {
	case Common::RU_RUS:
		break;
	case Common::PL_POL:
		keyYes = Common::KEYCODE_t;
		break;
	case Common::HE_ISR:
		keyYes = Common::KEYCODE_f;
		break;
	case Common::ES_ESP:
		keyYes = Common::KEYCODE_s;
		break;
	case Common::IT_ITA:
		keyYes = Common::KEYCODE_s;
		break;
	case Common::FR_FRA:
		keyYes = Common::KEYCODE_o;
		break;
	case Common::DE_DEU:
		keyYes = Common::KEYCODE_j;
		break;
	default:
		break;
	}
}

void getLanguageYesNo(KeyCode &keyYes, KeyCode &keyNo) {
#ifdef USE_TRANSLATION
	getLanguageYesNo(Common::parseLanguageFromLocale(TransMan.getCurrentLanguage().c_str()), keyYes, keyNo);
#else
	getLanguageYesNo(Common::EN_ANY, keyYes, keyNo);
#endif
}

} // End of namespace Common
