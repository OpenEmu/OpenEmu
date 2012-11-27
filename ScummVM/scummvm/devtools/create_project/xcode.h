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
 *
 */

#ifndef TOOLS_CREATE_PROJECT_XCODE_H
#define TOOLS_CREATE_PROJECT_XCODE_H

#include "create_project.h"

#include <algorithm>
#include <vector>

namespace CreateProjectTool {

class XCodeProvider : public ProjectProvider {
public:
	XCodeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version = 0);

protected:

	void createWorkspace(const BuildSetup &setup);

	void createOtherBuildFiles(const BuildSetup &setup);

	void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                       const StringList &includeList, const StringList &excludeList);

	void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
	                            const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix);

private:
	enum {
		SettingsAsList        = 0x01,
		SettingsSingleItem    = 0x02,
		SettingsNoQuote       = 0x04,
		SettingsQuoteVariable = 0x08,
		SettingsNoValue       = 0x10
	};

	// File properties
	struct FileProperty {
		std::string fileEncoding;
		std::string lastKnownFileType;
		std::string fileName;
		std::string filePath;
		std::string sourceTree;

		FileProperty(std::string fileType = "", std::string name = "", std::string path = "", std::string source = "") :
			fileEncoding(""), lastKnownFileType(fileType), fileName(name), filePath(path), sourceTree(source)
		{
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// XCObject and children
	typedef std::vector<std::string> ValueList;

	struct Entry {
		std::string value;
		std::string comment;

		Entry(std::string val, std::string cmt) : value(val), comment(cmt) {}
	};

	typedef std::vector<Entry> EntryList;

	struct Setting {
		EntryList entries;
		int flags;
		int indent;
		int order;

		explicit Setting(std::string value = "", std::string comment = "", int flgs = 0, int idt = 0, int ord = -1) : flags(flgs), indent(idt), order(ord) {
			entries.push_back(Entry(value, comment));
		}

		explicit Setting(ValueList values, int flgs = 0, int idt = 0, int ord = -1) : flags(flgs), indent(idt), order(ord) {
			for (unsigned int i = 0; i < values.size(); i++)
				entries.push_back(Entry(values[i], ""));
		}

		explicit Setting(EntryList ents, int flgs = 0, int idt = 0, int ord = -1) : entries(ents), flags(flgs), indent(idt), order(ord) {}

		void addEntry(std::string value, std::string comment = "") {
			entries.push_back(Entry(value, comment));
		}
	};

	typedef std::map<std::string, Setting> SettingList;
	typedef std::pair<std::string, Setting> SettingPair;
	typedef std::vector<SettingPair> OrderedSettingList;

	static bool OrderSortPredicate(const SettingPair& s1, const SettingPair& s2) {
		return s1.second.order < s2.second.order;
	}

	struct Property {
	public:
		SettingList settings;
		int flags;
		bool hasOrder;

		Property() : flags(0), hasOrder(false) {}

		// Constructs a simple Property
		explicit Property(std::string name, std::string value = "", std::string comment = "", int flgs = 0, int indent = 0, bool order = false) : flags(flgs), hasOrder(order) {
			Setting setting(value, comment, flags, indent);

			settings[name] = setting;
		}

		Property(std::string name, ValueList values, int flgs = 0, int indent = 0, bool order = false) : flags(flgs), hasOrder(order) {
			Setting setting(values, flags, indent);

			settings[name] = setting;
		}

		// Copy constructor
		Property(const Property &rhs) {
			settings = rhs.settings;
			flags = rhs.flags;
		}

		OrderedSettingList getOrderedSettingList() {
			OrderedSettingList list;

			// Prepare vector to sort
			for (SettingList::const_iterator setting = settings.begin(); setting != settings.end(); ++setting)
				list.push_back(SettingPair(setting->first, setting->second));

			// Sort vector using setting order
			if (hasOrder)
				std::sort(list.begin(), list.end(), OrderSortPredicate);

			return list;
		}
	};

	typedef std::map<std::string, Property> PropertyList;

	// Main object struct
	// This is all a big hack unfortunately, but making everything all properly abstracted would
	// be overkill since we only have to generate a single project
	struct Object {
	public:
		std::string id;					// Unique identifier for this object
 		std::string name;				// Name	(may not be unique - for ex. configuration entries)
		std::string refType;			// Type of object this references (if any)
		std::string comment;			// Main comment (empty for no comment)

		PropertyList properties;		// List of object properties, including output configuration

		// Constructs an object and add a default type property
		Object(XCodeProvider *objectParent, std::string objectId, std::string objectName, std::string objectType, std::string objectRefType = "", std::string objectComment = "")
		    : id(objectId), name(objectName), refType(objectRefType), comment(objectComment), parent(objectParent) {
			assert(objectParent);
			assert(!objectId.empty());
			assert(!objectName.empty());
			assert(!objectType.empty());

			addProperty("isa", objectType, "", SettingsNoQuote|SettingsNoValue);
		}

		// Add a simple Property with just a name and a value
		void addProperty(std::string propName, std::string propValue, std::string propComment = "", int propFlags = 0, int propIndent = 0) {
			properties[propName] = Property(propValue, "", propComment, propFlags, propIndent);
		}

		std::string toString(int flags = 0) {
			std::string output;
			output = "\t\t" + parent->getHash(id) + (comment.empty() ? "" : " /* " + comment + " */") + " = {";

			if (flags & SettingsAsList)
				output += "\n";

			// Special case: always output the isa property first
			output += parent->writeProperty("isa", properties["isa"], flags);

			// Write each property
			for (PropertyList::iterator property = properties.begin(); property != properties.end(); ++property) {
				if ((*property).first == "isa")
					continue;

				output += parent->writeProperty((*property).first, (*property).second, flags);
			}

			if (flags & SettingsAsList)
				output += "\t\t";

			output += "};\n";

			return output;
		}

	private:
		XCodeProvider *parent;

		// Returns the type property (should always be the first in the properties map)
		std::string getType() {
			assert(!properties.empty());
			assert(!properties["isa"].settings.empty());

			SettingList::iterator it = properties["isa"].settings.begin();

			return (*it).first;
		}
	};

	struct ObjectList {
	private:
		std::map<std::string, bool> objectMap;

	public:
		std::vector<Object *> objects;
		std::string comment;
		int flags;

		void add(Object *obj) {
			std::map<std::string, bool>::iterator it = objectMap.find(obj->id);
			if (it != objectMap.end() && it->second == true)
				return;

			objects.push_back(obj);
			objectMap[obj->id] = true;
		}

		std::string toString() {
			std::string output;

			if (!comment.empty())
				output = "\n/* Begin " + comment + " section */\n";

			for (std::vector<Object *>::iterator object = objects.begin(); object != objects.end(); ++object)
				output += (*object)->toString(flags);

			if (!comment.empty())
				output += "/* End " + comment + " section */\n";

			return output;
		}
	};

	// All objects
	std::map<std::string, std::string> _hashDictionnary;
	ValueList _defines;

	// Targets
	ValueList _targets;

	// Lists of objects
	ObjectList _buildFile;
	ObjectList _copyFilesBuildPhase;
	ObjectList _fileReference;
	ObjectList _frameworksBuildPhase;
	ObjectList _groups;
	ObjectList _nativeTarget;
	ObjectList _project;
	ObjectList _resourcesBuildPhase;
	ObjectList _sourcesBuildPhase;
	ObjectList _buildConfiguration;
	ObjectList _configurationList;

	void ouputMainProjectFile(const BuildSetup &setup);

	// Setup objects
	void setupCopyFilesBuildPhase();
	void setupFrameworksBuildPhase();
	void setupNativeTarget();
	void setupProject();
	void setupResourcesBuildPhase();
	void setupSourcesBuildPhase();
	void setupBuildConfiguration();

	// Misc
	void setupDefines(const BuildSetup &setup); // Setup the list of defines to be used on build configurations

	// Hash generation
	std::string getHash(std::string key);
	std::string newHash() const;

	// Output
	std::string writeProperty(const std::string &variable, Property &property, int flags = 0) const;
	std::string writeSetting(const std::string &variable, std::string name, std::string comment = "", int flags = 0, int indent = 0) const;
	std::string writeSetting(const std::string &variable, const Setting &setting) const;
};

} // End of CreateProjectTool namespace

#endif // TOOLS_CREATE_PROJECT_XCODE_H
