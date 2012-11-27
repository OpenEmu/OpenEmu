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

#ifndef COMMON_CONFIG_MANAGER_H
#define COMMON_CONFIG_MANAGER_H

#include "common/array.h"
//#include "common/config-file.h"
#include "common/hashmap.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/hash-str.h"

namespace Common {

class WriteStream;
class SeekableReadStream;

/**
 * The (singleton) configuration manager, used to query & set configuration
 * values using string keys.
 *
 * @todo Implement the callback based notification system (outlined below)
 *       which sends out notifications to interested parties whenever the value
 *       of some specific (or any) configuration key changes.
 */
class ConfigManager : public Singleton<ConfigManager> {

public:

	class Domain : public StringMap {
	private:
		StringMap _keyValueComments;
		String _domainComment;

	public:
		void setDomainComment(const String &comment);
		const String &getDomainComment() const;

		void setKVComment(const String &key, const String &comment);
		const String &getKVComment(const String &key) const;
		bool hasKVComment(const String &key) const;
	};

	typedef HashMap<String, Domain, IgnoreCase_Hash, IgnoreCase_EqualTo> DomainMap;

	/** The name of the application domain (normally 'scummvm'). */
	static char const *const kApplicationDomain;

	/** The transient (pseudo) domain. */
	static char const *const kTransientDomain;

#ifdef ENABLE_KEYMAPPER
	/** The name of keymapper domain used to store the key maps */
	static char const *const kKeymapperDomain;
#endif

	void				loadDefaultConfigFile();
	void				loadConfigFile(const String &filename);

	/**
	 * Retrieve the config domain with the given name.
	 * @param domName	the name of the domain to retrieve
	 * @return pointer to the domain, or 0 if the domain doesn't exist.
	 */
	Domain *			getDomain(const String &domName);
	const Domain *		getDomain(const String &domName) const;


	//
	// Generic access methods: No domain specified, use the values from the
	// various domains in the order of their priority.
	//

	bool				hasKey(const String &key) const;
	const String &		get(const String &key) const;
	void				set(const String &key, const String &value);

#if 1
	//
	// Domain specific access methods: Acces *one specific* domain and modify it.
	// TODO: I'd like to get rid of most of those if possible, or at least reduce
	// their usage, by using getDomain as often as possible. For example in the
	// options dialog code...
	//

	bool				hasKey(const String &key, const String &domName) const;
	const String &		get(const String &key, const String &domName) const;
	void				set(const String &key, const String &value, const String &domName);

	void				removeKey(const String &key, const String &domName);
#endif

	//
	// Some additional convenience accessors.
	//
	int					getInt(const String &key, const String &domName = String()) const;
	bool				getBool(const String &key, const String &domName = String()) const;
	void				setInt(const String &key, int value, const String &domName = String());
	void				setBool(const String &key, bool value, const String &domName = String());


	void				registerDefault(const String &key, const String &value);
	void				registerDefault(const String &key, const char *value);
	void				registerDefault(const String &key, int value);
	void				registerDefault(const String &key, bool value);

	void				flushToDisk();

	void				setActiveDomain(const String &domName);
	Domain *			getActiveDomain() { return _activeDomain; }
	const Domain *		getActiveDomain() const { return _activeDomain; }
	const String &		getActiveDomainName() const { return _activeDomainName; }

	void				addGameDomain(const String &domName);
	void				removeGameDomain(const String &domName);
	void				renameGameDomain(const String &oldName, const String &newName);

	void				addMiscDomain(const String &domName);
	void				removeMiscDomain(const String &domName);
	void				renameMiscDomain(const String &oldName, const String &newName);

	bool				hasGameDomain(const String &domName) const;
	bool				hasMiscDomain(const String &domName) const;

	const DomainMap &	getGameDomains() const { return _gameDomains; }
	DomainMap &			getGameDomains() { return _gameDomains; }

	static void			defragment();	// move in memory to reduce fragmentation
	void 				copyFrom(ConfigManager &source);

private:
	friend class Singleton<SingletonBaseType>;
	ConfigManager();

	void			loadFromStream(SeekableReadStream &stream);
	void			addDomain(const String &domainName, const Domain &domain);
	void			writeDomain(WriteStream &stream, const String &name, const Domain &domain);
	void			renameDomain(const String &oldName, const String &newName, DomainMap &map);

	Domain			_transientDomain;
	DomainMap		_gameDomains;
	DomainMap		_miscDomains;		// Any other domains
	Domain			_appDomain;
	Domain			_defaultsDomain;

#ifdef ENABLE_KEYMAPPER
	Domain			_keymapperDomain;
#endif

	Array<String>	_domainSaveOrder;

	String			_activeDomainName;
	Domain *		_activeDomain;

	String			_filename;
};

}	// End of namespace Common

/** Shortcut for accessing the configuration manager. */
#define ConfMan		Common::ConfigManager::instance()

#endif
