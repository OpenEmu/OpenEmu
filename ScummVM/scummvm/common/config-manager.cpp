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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/textconsole.h"

static bool isValidDomainName(const Common::String &domName) {
	const char *p = domName.c_str();
	while (*p && (Common::isAlnum(*p) || *p == '-' || *p == '_'))
		p++;
	return *p == 0;
}

namespace Common {

DECLARE_SINGLETON(ConfigManager);

char const *const ConfigManager::kApplicationDomain = "scummvm";
char const *const ConfigManager::kTransientDomain = "__TRANSIENT";

#ifdef ENABLE_KEYMAPPER
char const *const ConfigManager::kKeymapperDomain = "keymapper";
#endif

#pragma mark -


ConfigManager::ConfigManager() : _activeDomain(0) {
}

void ConfigManager::defragment() {
	ConfigManager *newInstance = new ConfigManager();
	newInstance->copyFrom(*_singleton);
	delete _singleton;
	_singleton = newInstance;
}

void ConfigManager::copyFrom(ConfigManager &source) {
	_transientDomain = source._transientDomain;
	_gameDomains = source._gameDomains;
	_miscDomains = source._miscDomains;
	_appDomain = source._appDomain;
	_defaultsDomain = source._defaultsDomain;
#ifdef ENABLE_KEYMAPPER
	_keymapperDomain = source._keymapperDomain;
#endif
	_domainSaveOrder = source._domainSaveOrder;
	_activeDomainName = source._activeDomainName;
	_activeDomain = &_gameDomains[_activeDomainName];
	_filename = source._filename;
}


void ConfigManager::loadDefaultConfigFile() {
	// Open the default config file
	assert(g_system);
	SeekableReadStream *stream = g_system->createConfigReadStream();
	_filename.clear();  // clear the filename to indicate that we are using the default config file

	// ... load it, if available ...
	if (stream) {
		loadFromStream(*stream);

		// ... and close it again.
		delete stream;

	} else {
		// No config file -> create new one!
		debug("Default configuration file missing, creating a new one");

		flushToDisk();
	}
}

void ConfigManager::loadConfigFile(const String &filename) {
	_filename = filename;

	FSNode node(filename);
	File cfg_file;
	if (!cfg_file.open(node)) {
		debug("Creating configuration file: %s", filename.c_str());
	} else {
		debug("Using configuration file: %s", _filename.c_str());
		loadFromStream(cfg_file);
	}
}

/**
 * Add a ready-made domain based on its name and contents
 * The domain name should not already exist in the ConfigManager.
 **/
void ConfigManager::addDomain(const String &domainName, const ConfigManager::Domain &domain) {
	if (domainName.empty())
		return;
	if (domainName == kApplicationDomain) {
		_appDomain = domain;
#ifdef ENABLE_KEYMAPPER
	} else if (domainName == kKeymapperDomain) {
		_keymapperDomain = domain;
#endif
	} else if (domain.contains("gameid")) {
		// If the domain contains "gameid" we assume it's a game domain
		if (_gameDomains.contains(domainName))
			warning("Game domain %s already exists in ConfigManager", domainName.c_str());

		_gameDomains[domainName] = domain;

		_domainSaveOrder.push_back(domainName);

		// Check if we have the same misc domain. For older config files
		// we could have 'ghost' domains with the same name, so delete
		// the ghost domain
		if (_miscDomains.contains(domainName))
			_miscDomains.erase(domainName);
	} else {
		// Otherwise it's a miscellaneous domain
		if (_miscDomains.contains(domainName))
			warning("Misc domain %s already exists in ConfigManager", domainName.c_str());

		_miscDomains[domainName] = domain;
	}
}


void ConfigManager::loadFromStream(SeekableReadStream &stream) {
	String domainName;
	String comment;
	Domain domain;
	int lineno = 0;

	_appDomain.clear();
	_gameDomains.clear();
	_miscDomains.clear();
	_transientDomain.clear();
	_domainSaveOrder.clear();

#ifdef ENABLE_KEYMAPPER
	_keymapperDomain.clear();
#endif

	// TODO: Detect if a domain occurs multiple times (or likewise, if
	// a key occurs multiple times inside one domain).

	while (!stream.eos() && !stream.err()) {
		lineno++;

		// Read a line
		String line = stream.readLine();

		if (line.size() == 0) {
			// Do nothing
		} else if (line[0] == '#') {
			// Accumulate comments here. Once we encounter either the start
			// of a new domain, or a key-value-pair, we associate the value
			// of the 'comment' variable with that entity.
			comment += line;
			comment += "\n";
		} else if (line[0] == '[') {
			// It's a new domain which begins here.
			// Determine where the previously accumulated domain goes, if we accumulated anything.
			addDomain(domainName, domain);
			domain.clear();
			const char *p = line.c_str() + 1;
			// Get the domain name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// dashes and underscores).
			while (*p && (isAlnum(*p) || *p == '-' || *p == '_'))
				p++;

			if (*p == '\0')
				error("Config file buggy: missing ] in line %d", lineno);
			else if (*p != ']')
				error("Config file buggy: Invalid character '%c' occurred in section name in line %d", *p, lineno);

			domainName = String(line.c_str() + 1, p);

			domain.setDomainComment(comment);
			comment.clear();

		} else {
			// This line should be a line with a 'key=value' pair, or an empty one.

			// Skip leading whitespaces
			const char *t = line.c_str();
			while (isSpace(*t))
				t++;

			// Skip empty lines / lines with only whitespace
			if (*t == 0)
				continue;

			// If no domain has been set, this config file is invalid!
			if (domainName.empty()) {
				error("Config file buggy: Key/value pair found outside a domain in line %d", lineno);
			}

			// Split string at '=' into 'key' and 'value'. First, find the "=" delimeter.
			const char *p = strchr(t, '=');
			if (!p)
				error("Config file buggy: Junk found in line line %d: '%s'", lineno, t);

			// Extract the key/value pair
			String key(t, p);
			String value(p + 1);

			// Trim of spaces
			key.trim();
			value.trim();

			// Finally, store the key/value pair in the active domain
			domain[key] = value;

			// Store comment
			domain.setKVComment(key, comment);
			comment.clear();
		}
	}

	addDomain(domainName, domain); // Add the last domain found
}

void ConfigManager::flushToDisk() {
#ifndef __DC__
	WriteStream *stream;

	if (_filename.empty()) {
		// Write to the default config file
		assert(g_system);
		stream = g_system->createConfigWriteStream();
		if (!stream)    // If writing to the config file is not possible, do nothing
			return;
	} else {
		DumpFile *dump = new DumpFile();
		assert(dump);

		if (!dump->open(_filename)) {
			warning("Unable to write configuration file: %s", _filename.c_str());
			delete dump;
			return;
		}

		stream = dump;
	}

	// Write the application domain
	writeDomain(*stream, kApplicationDomain, _appDomain);

#ifdef ENABLE_KEYMAPPER
	// Write the keymapper domain
	writeDomain(*stream, kKeymapperDomain, _keymapperDomain);
#endif

	DomainMap::const_iterator d;

	// Write the miscellaneous domains next
	for (d = _miscDomains.begin(); d != _miscDomains.end(); ++d) {
		writeDomain(*stream, d->_key, d->_value);
	}

	// First write the domains in _domainSaveOrder, in that order.
	// Note: It's possible for _domainSaveOrder to list domains which
	// are not present anymore, so we validate each name.
	Array<String>::const_iterator i;
	for (i = _domainSaveOrder.begin(); i != _domainSaveOrder.end(); ++i) {
		if (_gameDomains.contains(*i)) {
			writeDomain(*stream, *i, _gameDomains[*i]);
		}
	}

	// Now write the domains which haven't been written yet
	for (d = _gameDomains.begin(); d != _gameDomains.end(); ++d) {
		if (find(_domainSaveOrder.begin(), _domainSaveOrder.end(), d->_key) == _domainSaveOrder.end())
			writeDomain(*stream, d->_key, d->_value);
	}

	delete stream;

#endif // !__DC__
}

void ConfigManager::writeDomain(WriteStream &stream, const String &name, const Domain &domain) {
	if (domain.empty())
		return;     // Don't bother writing empty domains.

	// WORKAROUND: Fix for bug #1972625 "ALL: On-the-fly targets are
	// written to the config file": Do not save domains that came from
	// the command line
	if (domain.contains("id_came_from_command_line"))
		return;

	String comment;

	// Write domain comment (if any)
	comment = domain.getDomainComment();
	if (!comment.empty())
		stream.writeString(comment);

	// Write domain start
	stream.writeByte('[');
	stream.writeString(name);
	stream.writeByte(']');
	stream.writeByte('\n');

	// Write all key/value pairs in this domain, including comments
	Domain::const_iterator x;
	for (x = domain.begin(); x != domain.end(); ++x) {
		if (!x->_value.empty()) {
			// Write comment (if any)
			if (domain.hasKVComment(x->_key)) {
				comment = domain.getKVComment(x->_key);
				stream.writeString(comment);
			}
			// Write the key/value pair
			stream.writeString(x->_key);
			stream.writeByte('=');
			stream.writeString(x->_value);
			stream.writeByte('\n');
		}
	}
	stream.writeByte('\n');
}


#pragma mark -


const ConfigManager::Domain *ConfigManager::getDomain(const String &domName) const {
	assert(!domName.empty());
	assert(isValidDomainName(domName));

	if (domName == kTransientDomain)
		return &_transientDomain;
	if (domName == kApplicationDomain)
		return &_appDomain;
#ifdef ENABLE_KEYMAPPER
	if (domName == kKeymapperDomain)
		return &_keymapperDomain;
#endif
	if (_gameDomains.contains(domName))
		return &_gameDomains[domName];
	if (_miscDomains.contains(domName))
		return &_miscDomains[domName];

	return 0;
}

ConfigManager::Domain *ConfigManager::getDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));

	if (domName == kTransientDomain)
		return &_transientDomain;
	if (domName == kApplicationDomain)
		return &_appDomain;
#ifdef ENABLE_KEYMAPPER
	if (domName == kKeymapperDomain)
		return &_keymapperDomain;
#endif
	if (_gameDomains.contains(domName))
		return &_gameDomains[domName];
	if (_miscDomains.contains(domName))
		return &_miscDomains[domName];

	return 0;
}


#pragma mark -


bool ConfigManager::hasKey(const String &key) const {
	// Search the domains in the following order:
	// 1) the transient domain,
	// 2) the active game domain (if any),
	// 3) the application domain.
	// The defaults domain is explicitly *not* checked.

	if (_transientDomain.contains(key))
		return true;

	if (_activeDomain && _activeDomain->contains(key))
		return true;

	if (_appDomain.contains(key))
		return true;

	return false;
}

bool ConfigManager::hasKey(const String &key, const String &domName) const {
	// FIXME: For now we continue to allow empty domName to indicate
	// "use 'default' domain". This is mainly needed for the SCUMM ConfigDialog
	// and should be removed ASAP.
	if (domName.empty())
		return hasKey(key);

	const Domain *domain = getDomain(domName);

	if (!domain)
		return false;
	return domain->contains(key);
}

void ConfigManager::removeKey(const String &key, const String &domName) {
	Domain *domain = getDomain(domName);

	if (!domain)
		error("ConfigManager::removeKey(%s, %s) called on non-existent domain",
		      key.c_str(), domName.c_str());

	domain->erase(key);
}


#pragma mark -


const String &ConfigManager::get(const String &key) const {
	if (_transientDomain.contains(key))
		return _transientDomain[key];
	else if (_activeDomain && _activeDomain->contains(key))
		return (*_activeDomain)[key];
	else if (_appDomain.contains(key))
		return _appDomain[key];

	return _defaultsDomain.getVal(key);
}

const String &ConfigManager::get(const String &key, const String &domName) const {
	// FIXME: For now we continue to allow empty domName to indicate
	// "use 'default' domain". This is mainly needed for the SCUMM ConfigDialog
	// and should be removed ASAP.
	if (domName.empty())
		return get(key);

	const Domain *domain = getDomain(domName);

	if (!domain)
		error("ConfigManager::get(%s,%s) called on non-existent domain",
		      key.c_str(), domName.c_str());

	if (domain->contains(key))
		return (*domain)[key];

	return _defaultsDomain.getVal(key);
}

int ConfigManager::getInt(const String &key, const String &domName) const {
	String value(get(key, domName));
	char *errpos;

	// For now, be tolerant against missing config keys. Strictly spoken, it is
	// a bug in the calling code to retrieve an int for a key which isn't even
	// present... and a default value of 0 seems rather arbitrary.
	if (value.empty())
		return 0;

	// We use the special value '0' for the base passed to strtol. Doing that
	// makes it possible to enter hex values as "0x1234", but also decimal
	// values ("123") are still valid.
	int ivalue = (int)strtol(value.c_str(), &errpos, 0);
	if (value.c_str() == errpos)
		error("ConfigManager::getInt(%s,%s): '%s' is not a valid integer",
		      key.c_str(), domName.c_str(), errpos);

	return ivalue;
}

bool ConfigManager::getBool(const String &key, const String &domName) const {
	String value(get(key, domName));
	bool val;
	if (parseBool(value, val))
		return val;

	error("ConfigManager::getBool(%s,%s): '%s' is not a valid bool",
	      key.c_str(), domName.c_str(), value.c_str());
}


#pragma mark -


void ConfigManager::set(const String &key, const String &value) {
	// Remove the transient domain value, if any.
	_transientDomain.erase(key);

	// Write the new key/value pair into the active domain, resp. into
	// the application domain if no game domain is active.
	if (_activeDomain)
		(*_activeDomain)[key] = value;
	else
		_appDomain[key] = value;
}

void ConfigManager::set(const String &key, const String &value, const String &domName) {
	// FIXME: For now we continue to allow empty domName to indicate
	// "use 'default' domain". This is mainly needed for the SCUMM ConfigDialog
	// and should be removed ASAP.
	if (domName.empty()) {
		set(key, value);
		return;
	}

	Domain *domain = getDomain(domName);

	if (!domain)
		error("ConfigManager::set(%s,%s,%s) called on non-existent domain",
		      key.c_str(), value.c_str(), domName.c_str());

	(*domain)[key] = value;

	// TODO/FIXME: We used to erase the given key from the transient domain
	// here. Do we still want to do that?
	// It was probably there to simplify the options dialogs code:
	// Imagine you are editing the current options (via the SCUMM ConfigDialog,
	// for example). If you edit the game domain for that, but a matching
	// entry in the transient domain is present, than your changes may not take
	// effect. So you want to remove the key from the transient domain before
	// adding it to the active domain.
	// But doing this here seems rather evil... need to comb the options dialog
	// code to find out if it's still necessary, and if that's the case, how
	// to replace it in a clean fashion...
#if 0
	if (domName == kTransientDomain)
		_transientDomain[key] = value;
	else {
		if (domName == kApplicationDomain) {
			_appDomain[key] = value;
			if (_activeDomainName.empty() || !_gameDomains[_activeDomainName].contains(key))
				_transientDomain.erase(key);
		} else {
			_gameDomains[domName][key] = value;
			if (domName == _activeDomainName)
				_transientDomain.erase(key);
		}
	}
#endif
}

void ConfigManager::setInt(const String &key, int value, const String &domName) {
	set(key, String::format("%i", value), domName);
}

void ConfigManager::setBool(const String &key, bool value, const String &domName) {
	set(key, String(value ? "true" : "false"), domName);
}


#pragma mark -


void ConfigManager::registerDefault(const String &key, const String &value) {
	_defaultsDomain[key] = value;
}

void ConfigManager::registerDefault(const String &key, const char *value) {
	registerDefault(key, String(value));
}

void ConfigManager::registerDefault(const String &key, int value) {
	registerDefault(key, String::format("%i", value));
}

void ConfigManager::registerDefault(const String &key, bool value) {
	registerDefault(key, value ? "true" : "false");
}


#pragma mark -


void ConfigManager::setActiveDomain(const String &domName) {
	if (domName.empty()) {
		_activeDomain = 0;
	} else {
		assert(isValidDomainName(domName));
		_activeDomain = & _gameDomains[domName];
	}
	_activeDomainName = domName;
}

void ConfigManager::addGameDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));

	// TODO: Do we want to generate an error/warning if a domain with
	// the given name already exists?

	_gameDomains[domName];

	// Add it to the _domainSaveOrder, if it's not already in there
	if (find(_domainSaveOrder.begin(), _domainSaveOrder.end(), domName) == _domainSaveOrder.end())
		_domainSaveOrder.push_back(domName);
}

void ConfigManager::addMiscDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));

	_miscDomains[domName];
}

void ConfigManager::removeGameDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));
	_gameDomains.erase(domName);
}

void ConfigManager::removeMiscDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));
	_miscDomains.erase(domName);
}


void ConfigManager::renameGameDomain(const String &oldName, const String &newName) {
	renameDomain(oldName, newName, _gameDomains);
}

void ConfigManager::renameMiscDomain(const String &oldName, const String &newName) {
	renameDomain(oldName, newName, _miscDomains);
}

/**
 * Common private function to rename both game and misc domains
 **/
void ConfigManager::renameDomain(const String &oldName, const String &newName, DomainMap &map) {
	if (oldName == newName)
		return;

	assert(!oldName.empty());
	assert(!newName.empty());
	assert(isValidDomainName(oldName));
	assert(isValidDomainName(newName));

//	_gameDomains[newName].merge(_gameDomains[oldName]);
	Domain &oldDom = map[oldName];
	Domain &newDom = map[newName];
	Domain::const_iterator iter;
	for (iter = oldDom.begin(); iter != oldDom.end(); ++iter)
		newDom[iter->_key] = iter->_value;

	map.erase(oldName);
}

bool ConfigManager::hasGameDomain(const String &domName) const {
	assert(!domName.empty());
	return isValidDomainName(domName) && _gameDomains.contains(domName);
}

bool ConfigManager::hasMiscDomain(const String &domName) const {
	assert(!domName.empty());
	return isValidDomainName(domName) && _miscDomains.contains(domName);
}

#pragma mark -

void ConfigManager::Domain::setDomainComment(const String &comment) {
	_domainComment = comment;
}
const String &ConfigManager::Domain::getDomainComment() const {
	return _domainComment;
}

void ConfigManager::Domain::setKVComment(const String &key, const String &comment) {
	_keyValueComments[key] = comment;
}
const String &ConfigManager::Domain::getKVComment(const String &key) const {
	return _keyValueComments[key];
}
bool ConfigManager::Domain::hasKVComment(const String &key) const {
	return _keyValueComments.contains(key);
}

} // End of namespace Common
