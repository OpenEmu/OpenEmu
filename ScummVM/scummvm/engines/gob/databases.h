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

#ifndef GOB_DATABASES_H
#define GOB_DATABASES_H

#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/language.h"

#include "gob/dbase.h"

namespace Gob {

class Databases {
public:
	Databases();
	~Databases();

	void setLanguage(Common::Language language);

	bool open(const Common::String &id, const Common::String &file);
	bool close(const Common::String &id);

	bool getString(const Common::String &id, Common::String group,
			Common::String section, Common::String keyword, Common::String &result) const;

private:
	typedef Common::HashMap<Common::String, Common::StringMap, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DBMap;

	DBMap _databases;

	Common::String _language;

	int findField(const dBase &db, const Common::String &field, dBase::Type type) const;
	bool buildMap(const dBase &db, Common::StringMap &map) const;
};

} // End of namespace Gob

#endif // GOB_DATABASES_H
