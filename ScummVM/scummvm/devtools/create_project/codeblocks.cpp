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

#include "config.h"
#include "codeblocks.h"

#include <fstream>

namespace CreateProjectTool {

CodeBlocksProvider::CodeBlocksProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {
}

void CodeBlocksProvider::createWorkspace(const BuildSetup &setup) {
	std::ofstream workspace((setup.outputDir + '/' + setup.projectName + ".workspace").c_str());
	if (!workspace)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectName + ".workspace\" for writing");

	workspace << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n"
	             "<CodeBlocks_workspace_file>\n";

	workspace << "\t<Workspace title=\"" << setup.projectDescription << "\">\n";

	writeReferences(setup, workspace);

	// Note we assume that the UUID map only includes UUIDs for enabled engines!
	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == setup.projectName)
			continue;

		workspace << "\t\t<Project filename=\"" << i->first << ".cbp\" />\n";
	}

	workspace << "\t</Workspace>\n"
	             "</CodeBlocks_workspace_file>";
}

// HACK We need to pre-process library names
//      since the MSVC and mingw precompiled
//      libraries have different names :(
std::string processLibraryName(std::string name) {
	// Remove "_static" in lib name
	size_t pos = name.find("_static");
	if (pos != std::string::npos)
		return name.replace(pos, 7, "");

	// Replace "zlib" by "libz"
	if (name == "zlib")
		return "libz";

	return name;
}

void CodeBlocksProvider::createProjectFile(const std::string &name, const std::string &, const BuildSetup &setup, const std::string &moduleDir,
                                           const StringList &includeList, const StringList &excludeList) {

	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	project << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n"
	           "<CodeBlocks_project_file>\n"
	           "\t<FileVersion major=\"1\" minor=\"6\" />\n"
	           "\t<Project>\n"
	           "\t\t<Option title=\"" << name << "\" />\n"
	           "\t\t<Option pch_mode=\"2\" />\n"
	           "\t\t<Option compiler=\"gcc\" />\n"
	           "\t\t<Build>\n";

	if (name == setup.projectName) {
		std::string libraries;

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			libraries += processLibraryName(*i) + ".a;";

		project << "\t\t\t<Target title=\"default\">\n"
		           "\t\t\t\t<Option output=\"" << setup.projectName << "\\" << setup.projectName << "\" prefix_auto=\"1\" extension_auto=\"1\" />\n"
		           "\t\t\t\t<Option object_output=\"" << setup.projectName << "\" />\n"
		           "\t\t\t\t<Option external_deps=\"" << libraries /* + list of engines engines\name\name.a */ << "\" />\n"
		           "\t\t\t\t<Option type=\"1\" />\n"
		           "\t\t\t\t<Option compiler=\"gcc\" />\n"
		           "\t\t\t\t<Option parameters=\"-d 8 --debugflags=parser\" />\n"
		           "\t\t\t\t<Option projectIncludeDirsRelation=\"2\" />\n";

		//////////////////////////////////////////////////////////////////////////
		// Compiler
		project << "\t\t\t\t<Compiler>\n";

		writeWarnings(name, project);
		writeDefines(setup.defines, project);

		project << "\t\t\t\t\t<Add directory=\"$(" << LIBS_DEFINE << ")include\" />\n"
		           "\t\t\t\t\t<Add directory=\"$(" << LIBS_DEFINE << ")include\\SDL\" />\n"
		           "\t\t\t\t\t<Add directory=\"..\\..\\engines\" />\n"
		           "\t\t\t\t\t<Add directory=\"..\\..\\common\" />\n"
		           "\t\t\t\t\t<Add directory=\"..\\..\" />\n"
		           "\t\t\t\t</Compiler>\n";

		//////////////////////////////////////////////////////////////////////////
		// Linker
		project << "\t\t\t\t<Linker>\n";

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			project << "\t\t\t\t\t<Add library=\"" << processLibraryName(*i) << "\" />\n";

		for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
			if (i->first == setup.projectName)
				continue;

			project << "\t\t\t\t\t<Add library=\"" << setup.projectName << "\\engines\\" << i->first << "\\lib" << i->first << ".a\" />\n";
		}

		project << "\t\t\t\t\t<Add directory=\"$(" << LIBS_DEFINE << ")lib\\mingw\" />\n"
		           "\t\t\t\t\t<Add directory=\"$(" << LIBS_DEFINE << ")lib\" />\n"
		           "\t\t\t\t</Linker>\n";

		//////////////////////////////////////////////////////////////////////////
		// Resource compiler
		project << "\t\t\t\t<ResourceCompiler>\n"
		           "\t\t\t\t\t<Add directory=\"..\\..\\dists\" />\n"
		           "\t\t\t\t\t<Add directory=\"..\\..\\..\\" << setup.projectName << "\" />\n"
		           "\t\t\t\t</ResourceCompiler>\n"
		           "\t\t\t</Target>\n"
		           "\t\t</Build>\n";



	} else {
		project << "\t\t\t<Target title=\"default\">\n"
		           "\t\t\t\t<Option output=\"" << setup.projectName << "\\engines\\" << name << "\\lib" << name << "\" prefix_auto=\"1\" extension_auto=\"1\" />\n"
		           "\t\t\t\t<Option working_dir=\"\" />\n"
		           "\t\t\t\t<Option object_output=\"" << setup.projectName << "\" />\n"
		           "\t\t\t\t<Option type=\"2\" />\n"
		           "\t\t\t\t<Option compiler=\"gcc\" />\n"
		           "\t\t\t\t<Option createDefFile=\"1\" />\n"
		           "\t\t\t\t<Compiler>\n";

		writeWarnings(name, project);
		writeDefines(setup.defines, project);

		project << "\t\t\t\t\t<Add option=\"-g\" />\n"
		           "\t\t\t\t\t<Add directory=\"..\\..\\engines\" />\n"
		           "\t\t\t\t\t<Add directory=\"..\\..\\..\\" << setup.projectName << "\" />\n";

		// Sword2.5 engine needs theora and vorbis includes
		if (name == "sword25")
			project << "\t\t\t\t\t<Add directory=\"$(" << LIBS_DEFINE << ")include\" />\n";

		project << "\t\t\t\t</Compiler>\n"
		           "\t\t\t</Target>\n"
		           "\t\t</Build>\n";
	}

	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	if (modulePath.size())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);


	project << "\t\t<Extensions>\n"
	           "\t\t\t<code_completion />\n"
	           "\t\t\t<debugger />\n"
	           "\t\t</Extensions>\n"
	           "\t</Project>\n"
	           "</CodeBlocks_project_file>";

}

void CodeBlocksProvider::writeWarnings(const std::string &name, std::ofstream &output) const {

	// Global warnings
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i)
		output << "\t\t\t\t\t<Add option=\"" << *i << "\" />\n";

	// Check for project-specific warnings:
	std::map<std::string, StringList>::iterator warningsIterator = _projectWarnings.find(name);
	if (warningsIterator != _projectWarnings.end())
		for (StringList::const_iterator i = warningsIterator->second.begin(); i != warningsIterator->second.end(); ++i)
			output << "\t\t\t\t\t<Add option=\"" << *i << "\" />\n";

}

void CodeBlocksProvider::writeDefines(const StringList &defines, std::ofstream &output) const {
	for (StringList::const_iterator i = defines.begin(); i != defines.end(); ++i)
		output << "\t\t\t\t\t<Add option=\"-D" << *i << "\" />\n";
}

void CodeBlocksProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                                const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {

	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');
		} else {
			std::string name, ext;
			splitFilename(node->name, name, ext);

			if (ext == "rc") {
				projectFile << "\t\t<Unit filename=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
				               "\t\t\t<Option compilerVar=\"WINDRES\" />\n"
				               "\t\t</Unit>\n";
			} else if (ext == "asm") {
				projectFile << "\t\t<Unit filename=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
				               "\t\t\t<Option compiler=\"gcc\" use=\"1\" buildCommand=\"$(" << LIBS_DEFINE << ")bin/nasm.exe -f win32 -g $file -o $object\" />"
				               "\t\t</Unit>\n";
			} else {
				projectFile << "\t\t<Unit filename=\"" << convertPathToWin(filePrefix + node->name) << "\" />\n";
			}
		}
	}
}

void CodeBlocksProvider::writeReferences(const BuildSetup &setup, std::ofstream &output) {
	output << "\t\t<Project filename=\"" << setup.projectName << ".cbp\" active=\"1\">\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == " << PROJECT_NAME << ")
			continue;

		output << "\t\t\t<Depends filename=\"" << i->first << ".cbp\" />\n";
	}

	output << "\t\t</Project>\n";
}

const char *CodeBlocksProvider::getProjectExtension() {
	return ".cbp";
}


} // End of CreateProjectTool namespace
