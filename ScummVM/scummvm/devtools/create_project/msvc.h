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

#ifndef TOOLS_CREATE_PROJECT_MSVC_H
#define TOOLS_CREATE_PROJECT_MSVC_H

#include "create_project.h"

namespace CreateProjectTool {

class MSVCProvider : public ProjectProvider {
public:
	MSVCProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version);

protected:
	StringList _enableLanguageExtensions;
	StringList _disableEditAndContinue;

	void createWorkspace(const BuildSetup &setup);

	void createOtherBuildFiles(const BuildSetup &setup);

	/**
	 * Create the global project properties.
	 *
	 * @param setup Description of the desired build setup.
	 */
	void createGlobalProp(const BuildSetup &setup);

	/**
	 * Outputs a property file based on the input parameters.
	 *
	 * It can be easily used to create different global properties files
	 * for a 64 bit and a 32 bit version. It will also take care that the
	 * two platform configurations will output their files into different
	 * directories.
	 *
	 * @param properties File stream in which to write the property settings.
	 * @param bits Number of bits the platform supports.
	 * @param defines Defines the platform needs to have set.
	 * @param prefix File prefix, used to add additional include paths.
	 * @param runBuildEvents true if generating a revision number, false otherwise
	 */
	virtual void outputGlobalPropFile(const BuildSetup &setup, std::ofstream &properties, int bits, const StringList &defines, const std::string &prefix, bool runBuildEvents) = 0;

	/**
	 * Generates the project properties for debug and release settings.
	 *
	 * @param setup Description of the desired build setup.
	 * @param isRelease       Type of property file
	 * @param isWin32         Bitness of property file
	 * @param enableAnalysis  PREfast support
	 */
	virtual void createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32, bool enableAnalysis) = 0;

	/**
	 * Get the file extension for property files
	 */
	virtual const char *getPropertiesExtension() = 0;

	/**
	 * Get the Visual Studio version (used by the VS shell extension to launch the correct VS version)
	 */
	virtual int getVisualStudioVersion() = 0;

	/**
	 * Get the command line for the revision tool (shared between all Visual Studio based providers)
	 */
	std::string getPreBuildEvent() const;

	/**
	 * Get the command line for copying data files to the build directory.
	 *
	 * @param	isWin32		   	Bitness of property file.
	 * @param	createInstaller	true to NSIS create installer
	 *
	 * @return	The post build event.
	 */
	std::string getPostBuildEvent(bool isWin32, bool createInstaller) const;
};

} // End of CreateProjectTool namespace

#endif // TOOLS_CREATE_PROJECT_MSVC_H
