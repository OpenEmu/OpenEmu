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

#ifndef TOOLS_CREATE_PROJECT_MSBUILD_H
#define TOOLS_CREATE_PROJECT_MSBUILD_H

#include "msvc.h"

namespace CreateProjectTool {

class MSBuildProvider : public MSVCProvider {
public:
	MSBuildProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version);

protected:
	void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                       const StringList &includeList, const StringList &excludeList);

	void outputProjectSettings(std::ofstream &project, const std::string &name, const BuildSetup &setup, bool isRelease, bool isWin32, bool enableAnalysis);

	void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
	                            const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix);

	void writeReferences(const BuildSetup &setup, std::ofstream &output);

	void outputGlobalPropFile(const BuildSetup &setup, std::ofstream &properties, int bits, const StringList &defines, const std::string &prefix, bool runBuildEvents);

	void createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32, bool enableAnalysis);

	const char *getProjectExtension();
	const char *getPropertiesExtension();
	int getVisualStudioVersion();

private:
	struct FileEntry {
		std::string name;
		std::string path;
		std::string filter;
		std::string prefix;

		bool operator<(const FileEntry& rhs) const {
			return path.compare(rhs.path) == -1;   // Not exactly right for alphabetical order, but good enough
		}
	};
	typedef std::list<FileEntry> FileEntries;

	std::list<std::string> _filters; // list of filters (we need to create a GUID for each filter id)
	FileEntries _compileFiles;
	FileEntries _includeFiles;
	FileEntries _otherFiles;
	FileEntries _asmFiles;
	FileEntries _resourceFiles;

	void computeFileList(const FileNode &dir, const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix);
	void createFiltersFile(const BuildSetup &setup, const std::string &name);

	void outputFilter(std::ostream &filters, const FileEntries &files, const std::string &action);
	void outputFiles(std::ostream &projectFile, const FileEntries &files, const std::string &action);
};

} // End of CreateProjectTool namespace

#endif // TOOLS_CREATE_PROJECT_MSBUILD_H
