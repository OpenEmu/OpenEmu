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
#include "visualstudio.h"

#include <fstream>
#include <algorithm>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// Visual Studio Provider (Visual Studio 2005 & 2008)
//////////////////////////////////////////////////////////////////////////

VisualStudioProvider::VisualStudioProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: MSVCProvider(global_warnings, project_warnings, version) {
}

const char *VisualStudioProvider::getProjectExtension() {
	return ".vcproj";
}

const char *VisualStudioProvider::getPropertiesExtension() {
	return ".vsprops";
}

int VisualStudioProvider::getVisualStudioVersion() {
	if (_version == 9)
		return 2008;

	if (_version == 8)
		return 2005;

	error("Unsupported version passed to getVisualStudioVersion");
}

void VisualStudioProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                                             const StringList &includeList, const StringList &excludeList) {
	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	project << "<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
	           "<VisualStudioProject\n"
	           "\tProjectType=\"Visual C++\"\n"
	           "\tVersion=\"" << _version << ".00\"\n"
	           "\tName=\"" << name << "\"\n"
	           "\tProjectGUID=\"{" << uuid << "}\"\n"
	           "\tRootNamespace=\"" << name << "\"\n"
	           "\tKeyword=\"Win32Proj\"\n";

	if (_version >= 9)
		project << "\tTargetFrameworkVersion=\"131072\"\n";

	project << "\t>\n"
	           "\t<Platforms>\n"
	           "\t\t<Platform Name=\"Win32\" />\n"
	           "\t\t<Platform Name=\"x64\" />\n"
	           "\t</Platforms>\n"
	           "\t<Configurations>\n";

	// Check for project-specific warnings:
	std::map< std::string, std::list<std::string> >::iterator warningsIterator = _projectWarnings.find(name);

	if (setup.devTools || name == setup.projectName) {
		std::string libraries;

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			libraries += ' ' + *i + ".lib";

		// Win32
		outputConfiguration(project, setup, libraries, "Debug", "Win32", "", true);
		outputConfiguration(project, setup, libraries, "Analysis", "Win32", "", true);
		outputConfiguration(project, setup, libraries, "Release", "Win32", "", true);

		// x64
		// For 'x64' we must disable NASM support. Usually we would need to disable the "nasm" feature for that and
		// re-create the library list, BUT since NASM doesn't link any additional libraries, we can just use the
		// libraries list created for IA-32. If that changes in the future, we need to adjust this part!
		outputConfiguration(project, setup, libraries, "Debug", "x64", "64", false);
		outputConfiguration(project, setup, libraries, "Analysis", "x64", "64", false);
		outputConfiguration(project, setup, libraries, "Release", "x64", "64", false);

	} else {
		bool enableLanguageExtensions = find(_enableLanguageExtensions.begin(), _enableLanguageExtensions.end(), name) != _enableLanguageExtensions.end();
		bool disableEditAndContinue = find(_disableEditAndContinue.begin(), _disableEditAndContinue.end(), name) != _disableEditAndContinue.end();

		std::string warnings = "";
		if (warningsIterator != _projectWarnings.end())
			for (StringList::const_iterator i = warningsIterator->second.begin(); i != warningsIterator->second.end(); ++i)
				warnings +=  *i + ';';

		std::string toolConfig;
		toolConfig  = (!warnings.empty() ? "DisableSpecificWarnings=\"" + warnings + "\"" : "");
		toolConfig += (disableEditAndContinue   ? "DebugInformationFormat=\"3\" " : "");
		toolConfig += (enableLanguageExtensions ? "DisableLanguageExtensions=\"false\" " : "");

		// Win32
		outputConfiguration(setup, project, toolConfig, "Debug", "Win32", "");
		outputConfiguration(setup, project, toolConfig, "Analysis", "Win32", "");
		outputConfiguration(setup, project, toolConfig, "Release", "Win32", "");
		outputConfiguration(setup, project, toolConfig, "Debug", "x64", "64");
		outputConfiguration(setup, project, toolConfig, "Analysis", "x64", "64");
		outputConfiguration(setup, project, toolConfig, "Release", "x64", "64");
	}

	project << "\t</Configurations>\n"
	           "\t<Files>\n";

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

	project << "\t</Files>\n"
	           "</VisualStudioProject>\n";
}

void VisualStudioProvider::outputConfiguration(std::ostream &project, const BuildSetup &setup, const std::string &libraries, const std::string &config, const std::string &platform, const std::string &props, const bool isWin32) {
	project << "\t\t<Configuration Name=\"" << config << "|" << platform << "\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\" << setup.projectDescription << "_" << config << props << ".vsprops\">\n"
	           "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" DebugInformationFormat=\"3\" />\n"
	           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/" << setup.projectName << ".exe\"\n"
	           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
	           "\t\t\t/>\n";
	outputBuildEvents(project, setup, isWin32);
	project << "\t\t</Configuration>\n";
}

void VisualStudioProvider::outputConfiguration(const BuildSetup &setup, std::ostream &project, const std::string &toolConfig, const std::string &config, const std::string &platform, const std::string &props) {
	project << "\t\t<Configuration Name=\"" << config << "|" << platform << "\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\" << setup.projectDescription << "_" << config << props << ".vsprops\">\n"
	           "\t\t\t<Tool Name=\"VCCLCompilerTool\" "<< toolConfig << "/>\n"
	           "\t\t</Configuration>\n";
}

void VisualStudioProvider::outputBuildEvents(std::ostream &project, const BuildSetup &setup, const bool isWin32) {
	if (!setup.devTools && setup.runBuildEvents) {
		project << "\t\t\t<Tool\tName=\"VCPreBuildEventTool\"\n"
		           "\t\t\t\tCommandLine=\"" << getPreBuildEvent() << "\"\n"
		           "\t\t\t/>\n"
		           "\t\t\t<Tool\tName=\"VCPostBuildEventTool\"\n"
		           "\t\t\t\tCommandLine=\"" << getPostBuildEvent(isWin32, setup.createInstaller) << "\"\n"
		           "\t\t\t/>\n";
	}
}

void VisualStudioProvider::writeReferences(const BuildSetup &setup, std::ofstream &output) {
	output << "\tProjectSection(ProjectDependencies) = postProject\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == setup.projectName)
			continue;

		output << "\t\t{" << i->second << "} = {" << i->second << "}\n";
	}

	output << "\tEndProjectSection\n";
}

void VisualStudioProvider::outputGlobalPropFile(const BuildSetup &setup, std::ofstream &properties, int bits, const StringList &defines, const std::string &prefix, bool runBuildEvents) {
	std::string warnings;
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i)
		warnings +=  *i + ';';

	std::string definesList;
	for (StringList::const_iterator i = defines.begin(); i != defines.end(); ++i) {
		if (i != defines.begin())
			definesList += ';';
		definesList += *i;
	}

	// Add define to include revision header
	if (runBuildEvents)
		definesList += REVISION_DEFINE ";";

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"" << setup.projectDescription << "_Global\"\n"
	              "\tOutputDirectory=\"$(ConfigurationName)" << bits << "\"\n"
	              "\tIntermediateDirectory=\"$(ConfigurationName)" << bits << "/$(ProjectName)\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n"
	              "\t\tDisableLanguageExtensions=\"" << (setup.devTools ? "false" : "true") << "\"\n"
	              "\t\tDisableSpecificWarnings=\"" << warnings << "\"\n"
	              "\t\tAdditionalIncludeDirectories=\"" << prefix << ";" << prefix << "\\engines;$(" << LIBS_DEFINE << ")\\include;$(TargetDir)\"\n"
	              "\t\tPreprocessorDefinitions=\"" << definesList << "\"\n"
	              "\t\tExceptionHandling=\"" << (setup.devTools ? "1" : "0") << "\"\n";

#if NEEDS_RTTI
	properties << "\t\tRuntimeTypeInfo=\"true\"\n";
#else
	properties << "\t\tRuntimeTypeInfo=\"false\"\n";
#endif

	properties << "\t\tWarningLevel=\"4\"\n"
	              "\t\tWarnAsError=\"false\"\n"
	              "\t\tCompileAs=\"0\"\n"
	              "\t\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLibrarianTool\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLinkerTool\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	              "\t\tSubSystem=\"1\"\n";

	if (!setup.devTools)
		properties << "\t\tEntryPointSymbol=\"WinMainCRTStartup\"\n";

	properties << "\t\tAdditionalLibraryDirectories=\"$(" << LIBS_DEFINE << ")\\lib\\" << ((bits == 32) ? "x86" : "x64") << "\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCResourceCompilerTool\"\n"
	              "\t\tAdditionalIncludeDirectories=\"" << prefix << "\"\n"
	              "\t/>\n"
	              "</VisualStudioPropertySheet>\n";

	properties.flush();
}

void VisualStudioProvider::createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32, bool enableAnalysis) {
	const std::string outputType = (enableAnalysis ? "Analysis" : (isRelease ? "Release" : "Debug"));
	const std::string outputBitness = (isWin32 ? "32" : "64");

	std::ofstream properties((setup.outputDir + '/' + setup.projectDescription + "_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension() + "\" for writing");

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"" << setup.projectDescription << "_" << outputType << outputBitness << "\"\n"
	              "\tInheritedPropertySheets=\".\\" << setup.projectDescription << "_Global" << (isWin32 ? "" : "64") << ".vsprops\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n";

	if (isRelease) {
		properties << "\t\tEnableIntrinsicFunctions=\"true\"\n"
		              "\t\tWholeProgramOptimization=\"true\"\n"
		              "\t\tPreprocessorDefinitions=\"WIN32;RELEASE_BUILD\"\n"
		              "\t\tStringPooling=\"true\"\n"
		              "\t\tBufferSecurityCheck=\"false\"\n"
		              "\t\tDebugInformationFormat=\"0\"\n"
		              "\t\tRuntimeLibrary=\"0\"\n"
		              "\t\tAdditionalOption=\"" << (enableAnalysis ? "/analyze" : "") << "\"\n"
		              "\t/>\n"
		              "\t<Tool\n"
		              "\t\tName=\"VCLinkerTool\"\n"
		              "\t\tLinkIncremental=\"1\"\n"
		              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
		              "\t\tSetChecksum=\"true\"\n";
	} else {
		properties << "\t\tOptimization=\"0\"\n"
		              "\t\tPreprocessorDefinitions=\"WIN32\"\n"
		              "\t\tMinimalRebuild=\"true\"\n"
		              "\t\tBasicRuntimeChecks=\"3\"\n"
		              "\t\tRuntimeLibrary=\"1\"\n"
		              "\t\tEnableFunctionLevelLinking=\"true\"\n"
		              "\t\tWarnAsError=\"false\"\n"
		              "\t\tDebugInformationFormat=\"" << (isWin32 ? "4" : "3") << "\"\n" // For x64 format "4" (Edit and continue) is not supported, thus we default to "3"
		              "\t\tAdditionalOption=\"" << (enableAnalysis ? "/analyze" : "") << "\"\n"
		              "\t/>\n"
		              "\t<Tool\n"
		              "\t\tName=\"VCLinkerTool\"\n"
		              "\t\tLinkIncremental=\"2\"\n"
		              "\t\tGenerateDebugInformation=\"true\"\n"
		              "\t\tIgnoreDefaultLibraryNames=\"libcmt.lib\"\n";
	}

	properties << "\t/>\n"
	              "</VisualStudioPropertySheet>\n";

	properties.flush();
	properties.close();
}

void VisualStudioProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                                  const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {
	const std::string indentString = getIndent(indentation + 2);

	if (indentation)
		projectFile << getIndent(indentation + 1) << "<Filter\tName=\"" << dir.name << "\">\n";

	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');
		} else {
			if (producesObjectFile(node->name)) {
				std::string name, ext;
				splitFilename(node->name, name, ext);
				const bool isDuplicate = (std::find(duplicate.begin(), duplicate.end(), name + ".o") != duplicate.end());

				if (ext == "asm") {
					std::string objFileName = "$(IntDir)\\";
					if (isDuplicate)
						objFileName += objPrefix;
					objFileName += "$(InputName).obj";

					const std::string toolLine = indentString + "\t\t<Tool Name=\"VCCustomBuildTool\" CommandLine=\"nasm.exe -f win32 -g -o &quot;" + objFileName + "&quot; &quot;$(InputPath)&quot;&#x0D;&#x0A;\" Outputs=\"" + objFileName + "\" />\n";

					// NASM is not supported for x64, thus we do not need to add additional entries here :-).
					projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
					            << indentString << "\t<FileConfiguration Name=\"Debug|Win32\">\n"
					            << toolLine
					            << indentString << "\t</FileConfiguration>\n"
								<< indentString << "\t<FileConfiguration Name=\"Analysis|Win32\">\n"
								<< toolLine
								<< indentString << "\t</FileConfiguration>\n"
					            << indentString << "\t<FileConfiguration Name=\"Release|Win32\">\n"
					            << toolLine
					            << indentString << "\t</FileConfiguration>\n"
					            << indentString << "</File>\n";
				} else {
					if (isDuplicate) {
						const std::string toolLine = indentString + "\t\t<Tool Name=\"VCCLCompilerTool\" ObjectFile=\"$(IntDir)\\" + objPrefix + "$(InputName).obj\" XMLDocumentationFileName=\"$(IntDir)\\" + objPrefix + "$(InputName).xdc\" />\n";

						projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
						            << indentString << "\t<FileConfiguration Name=\"Debug|Win32\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
									<< indentString << "\t<FileConfiguration Name=\"Analysis|Win32\">\n"
									<< toolLine
									<< indentString << "\t</FileConfiguration>\n"
						            << indentString << "\t<FileConfiguration Name=\"Release|Win32\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
									<< indentString << "\t<FileConfiguration Name=\"Debug|x64\">\n"
									<< toolLine
									<< indentString << "\t</FileConfiguration>\n"
									<< indentString << "\t<FileConfiguration Name=\"Analysis|x64\">\n"
									<< toolLine
									<< indentString << "\t</FileConfiguration>\n"
						            << indentString << "\t<FileConfiguration Name=\"Release|x64\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
						            << indentString << "</File>\n";
					} else {
						projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\" />\n";
					}
				}
			} else {
				projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\" />\n";
			}
		}
	}

	if (indentation)
		projectFile << getIndent(indentation + 1) << "</Filter>\n";
}

} // End of CreateProjectTool namespace
