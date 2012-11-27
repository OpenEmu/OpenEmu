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

#include "common/file.h"
#include "common/textconsole.h"

#include "gob/databases.h"

namespace Gob {

Databases::Databases() {
}

Databases::~Databases() {
}

void Databases::setLanguage(Common::Language language) {
	Common::String lang;

	if      (language == Common::UNK_LANG)
		lang = "";
	else if (language == Common::EN_ANY)
		lang = "E";
	else if (language == Common::EN_GRB)
		lang = "E";
	else if (language == Common::EN_USA)
		lang = "E";
	else if (language == Common::DE_DEU)
		lang = "G";
	else if (language == Common::FR_FRA)
		lang = "F";
	else
		warning("Databases::setLanguage(): Language \"%s\" not supported",
				Common::getLanguageDescription(language));

	if (!_databases.empty() && (lang != _language))
		warning("Databases::setLanguage(): \"%s\" != \"%s\" and there's still databases open!",
				_language.c_str(), lang.c_str());

	_language = lang;
}

bool Databases::open(const Common::String &id, const Common::String &file) {
	if (_databases.contains(id)) {
		warning("Databases::open(): A database with the ID \"%s\" already exists", id.c_str());
		return false;
	}

	Common::File dbFile;
	if (!dbFile.open(file)) {
		warning("Databases::open(): No such file \"%s\"", file.c_str());
		return false;
	}

	dBase db;
	if (!db.load(dbFile)) {
		warning("Databases::open(): Failed loading database file \"%s\"", file.c_str());
		return false;
	}

	_databases.setVal(id, Common::StringMap());
	DBMap::iterator map = _databases.find(id);
	assert(map != _databases.end());

	if (!buildMap(db, map->_value)) {
		warning("Databases::open(): Failed building a map for database \"%s\"", file.c_str());
		_databases.erase(map);
		return false;
	}

	return true;
}

bool Databases::close(const Common::String &id) {
	DBMap::iterator db = _databases.find(id);
	if (db == _databases.end()) {
		warning("Databases::open(): A database with the ID \"%s\" does not exist", id.c_str());
		return false;
	}

	_databases.erase(db);
	return true;
}

bool Databases::getString(const Common::String &id, Common::String group,
		Common::String section, Common::String keyword, Common::String &result) const {

	DBMap::iterator db = _databases.find(id);
	if (db == _databases.end()) {
		warning("Databases::getString(): A database with the ID \"%s\" does not exist", id.c_str());
		return false;
	}

	if (_language.empty()) {
		warning("Databases::getString(): No language set");
		return false;
	}

	Common::String key = _language + ":" + group + ":" + section + ":" + keyword;

	Common::StringMap::const_iterator entry = db->_value.find(key);
	if (entry == db->_value.end())
		return false;

	result = entry->_value;
	return true;
}

int Databases::findField(const dBase &db, const Common::String &field,
		dBase::Type type) const {

	const Common::Array<dBase::Field> &fields = db.getFields();

	for (uint i = 0; i < fields.size(); i++) {
		if (!fields[i].name.equalsIgnoreCase(field))
			continue;

		if (fields[i].type != type)
			return -1;

		return i;
	}

	return -1;
}

bool Databases::buildMap(const dBase &db, Common::StringMap &map) const {
	int fLanguage = findField(db, "Langage", dBase::kTypeString);
	int fGroup    = findField(db, "Nom"    , dBase::kTypeString);
	int fSection  = findField(db, "Section", dBase::kTypeString);
	int fKeyword  = findField(db, "Motcle" , dBase::kTypeString);
	int fText     = findField(db, "Texte"  , dBase::kTypeString);

	if ((fLanguage < 0) || (fGroup < 0) || (fSection < 0) || (fKeyword < 0) || (fText < 0))
		return false;

	const Common::Array<dBase::Record> &records = db.getRecords();

	Common::Array<dBase::Record>::const_iterator record;
	for (record = records.begin(); record != records.end(); ++record) {
		Common::String key;

		key += db.getString(*record, fLanguage) + ":";
		key += db.getString(*record, fGroup   ) + ":";
		key += db.getString(*record, fSection ) + ":";
		key += db.getString(*record, fKeyword );

		map.setVal(key, db.getString(*record, fText));
	}

	return true;
}

} // End of namespace Gob
