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
#include "msvc.h"

#include <fstream>
#include <algorithm>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// MSVC Provider (Base class)
//////////////////////////////////////////////////////////////////////////
MSVCProvider::MSVCProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {

	_enableLanguageExtensions = tokenize(ENABLE_LANGUAGE_EXTENSIONS, ',');
	_disableEditAndContinue   = tokenize(DISABLE_EDIT_AND_CONTINUE, ',');
}

void MSVCProvider::createWorkspace(const BuildSetup &setup) {
	UUIDMap::const_iterator svmUUID = _uuidMap.find(setup.projectName);
	if (svmUUID == _uuidMap.end())
		error("No UUID for \"" + setup.projectName + "\" project created");

	const std::string svmProjectUUID = svmUUID->second;
	assert(!svmProjectUUID.empty());

	std::string solutionUUID = createUUID();

	std::ofstream solution((setup.outputDir + '/' + setup.projectName + ".sln").c_str());
	if (!solution)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectName + ".sln\" for writing");

	solution << "Microsoft Visual Studio Solution File, Format Version " << _version + 1 << ".00\n";
	solution << "# Visual Studio " << getVisualStudioVersion() << "\n";

	// Write main project
	if (!setup.devTools) {
		solution << "Project(\"{" << solutionUUID << "}\") = \"" << setup.projectName << "\", \"" << setup.projectName << getProjectExtension() << "\", \"{" << svmProjectUUID << "}\"\n";

		// Project dependencies are moved to vcxproj files in Visual Studio 2010
		if (_version < 10)
			writeReferences(setup, solution);

		solution << "EndProject\n";
	}

	// Note we assume that the UUID map only includes UUIDs for enabled engines!
	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == setup.projectName)
			continue;

		solution << "Project(\"{" << solutionUUID << "}\") = \"" << i->first << "\", \"" << i->first << getProjectExtension() << "\", \"{" << i->second << "}\"\n"
		         << "EndProject\n";
	}

	solution << "Global\n"
	            "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
	            "\t\tDebug|Win32 = Debug|Win32\n"
	            "\t\tAnalysis|Win32 = Analysis|Win32\n"
	            "\t\tRelease|Win32 = Release|Win32\n"
	            "\t\tDebug|x64 = Debug|x64\n"
	            "\t\tAnalysis|x64 = Analysis|x64\n"
	            "\t\tRelease|x64 = Release|x64\n"
	            "\tEndGlobalSection\n"
	            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		solution << "\t\t{" << i->second << "}.Debug|Win32.ActiveCfg = Debug|Win32\n"
		            "\t\t{" << i->second << "}.Debug|Win32.Build.0 = Debug|Win32\n"
		            "\t\t{" << i->second << "}.Analysis|Win32.ActiveCfg = Analysis|Win32\n"
		            "\t\t{" << i->second << "}.Analysis|Win32.Build.0 = Analysis|Win32\n"
		            "\t\t{" << i->second << "}.Release|Win32.ActiveCfg = Release|Win32\n"
		            "\t\t{" << i->second << "}.Release|Win32.Build.0 = Release|Win32\n"
		            "\t\t{" << i->second << "}.Debug|x64.ActiveCfg = Debug|x64\n"
		            "\t\t{" << i->second << "}.Debug|x64.Build.0 = Debug|x64\n"
		            "\t\t{" << i->second << "}.Analysis|x64.ActiveCfg = Analysis|x64\n"
		            "\t\t{" << i->second << "}.Analysis|x64.Build.0 = Analysis|x64\n"
		            "\t\t{" << i->second << "}.Release|x64.ActiveCfg = Release|x64\n"
		            "\t\t{" << i->second << "}.Release|x64.Build.0 = Release|x64\n";
	}

	solution << "\tEndGlobalSection\n"
	            "\tGlobalSection(SolutionProperties) = preSolution\n"
	            "\t\tHideSolutionNode = FALSE\n"
	            "\tEndGlobalSection\n"
	            "EndGlobal\n";
}

void MSVCProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// Create the global property file
	createGlobalProp(setup);

	// Create the configuration property files (for Debug and Release with 32 and 64bits versions)
	// Note: we use the debug properties for the analysis configuration
	createBuildProp(setup, true, false, false);
	createBuildProp(setup, true, true, false);
	createBuildProp(setup, false, false, false);
	createBuildProp(setup, false, false, true);
	createBuildProp(setup, false, true, false);
	createBuildProp(setup, false, true, true);
}

void MSVCProvider::createGlobalProp(const BuildSetup &setup) {
	std::ofstream properties((setup.outputDir + '/' + setup.projectDescription + "_Global" + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_Global" + getPropertiesExtension() + "\" for writing");

	outputGlobalPropFile(setup, properties, 32, setup.defines, convertPathToWin(setup.filePrefix), setup.runBuildEvents);
	properties.close();

	properties.open((setup.outputDir + '/' + setup.projectDescription + "_Global64" + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_Global64" + getPropertiesExtension() + "\" for writing");

	// HACK: We must disable the "nasm" feature for x64. To achieve that we must duplicate the feature list and
	// recreate a define list.
	FeatureList x64Features = setup.features;
	setFeatureBuildState("nasm", x64Features, false);
	StringList x64Defines = getFeatureDefines(x64Features);
	StringList x64EngineDefines = getEngineDefines(setup.engines);
	x64Defines.splice(x64Defines.end(), x64EngineDefines);

	// HACK: This definitely should not be here, but otherwise we would not define SDL_BACKEND for x64.
	x64Defines.push_back("WIN32");
	x64Defines.push_back("SDL_BACKEND");

	outputGlobalPropFile(setup, properties, 64, x64Defines, convertPathToWin(setup.filePrefix), setup.runBuildEvents);
}

std::string MSVCProvider::getPreBuildEvent() const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Pre-Build script...\n"
			  "echo.\n"
			  "@call &quot;$(SolutionDir)../../devtools/create_project/scripts/prebuild.cmd&quot; &quot;$(SolutionDir)/../..&quot;  &quot;$(TargetDir)&quot;\n"
	          "EXIT /B0";

	return cmdLine;
}

std::string MSVCProvider::getPostBuildEvent(bool isWin32, bool createInstaller) const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Post-Build script...\n"
	          "echo.\n"
	          "@call &quot;$(SolutionDir)../../devtools/create_project/scripts/postbuild.cmd&quot; &quot;$(SolutionDir)/../..&quot; &quot;$(OutDir)&quot; ";

	cmdLine += (isWin32) ? "x86" : "x64";

	cmdLine += " %" LIBS_DEFINE "% ";

	// Specify if installer needs to be built or not
	cmdLine += (createInstaller ? "1" : "0");

	cmdLine += "\n"
	           "EXIT /B0";

	return cmdLine;
}

} // End of CreateProjectTool namespace
