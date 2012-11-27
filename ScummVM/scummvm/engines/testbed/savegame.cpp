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
 */

#include "common/savefile.h"

#include "testbed/savegame.h"

namespace Testbed {

/**
 * This test creates a savefile for the given testbed-state and could be reloaded using the saveFile API.
 * It is intended to test saving and loading from savefiles.
 */
bool SaveGametests::writeDataToFile(const char *fileName, const char *msg) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::OutSaveFile *saveFile = saveFileMan->openForSaving(fileName);

	if (!saveFile) {
		Testsuite::logDetailedPrintf("Can't open saveFile %s\n", fileName);
		return false;
	}

	saveFile->writeString(msg);
	saveFile->finalize();
	delete saveFile;

	return true;
}

bool SaveGametests::readAndVerifyData(const char *fileName, const char *expected) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::InSaveFile *loadFile = saveFileMan->openForLoading(fileName);

	if (!loadFile) {
		Testsuite::logDetailedPrintf("Can't open save File to load\n");
		return false;
	}

	Common::String lineToRead = loadFile->readLine();
	delete loadFile;

	if (lineToRead.equals(expected)) {
		return true;
	}

	return false;
}

TestExitStatus SaveGametests::testSaveLoadState() {
	// create a savefile with "ScummVM Rocks!" written on it
	if (!writeDataToFile("tBedSavefile.0", "ScummVM Rocks!")) {
		Testsuite::logDetailedPrintf("Writing data to savefile failed\n");
		return kTestFailed;
	}

	// Verify if it contains the same data
	if (!readAndVerifyData("tBedSavefile.0", "ScummVM Rocks!")) {
		Testsuite::logDetailedPrintf("Reading data from savefile failed\n");
		return kTestFailed;
	}

	return kTestPassed;
}

TestExitStatus SaveGametests::testRemovingSavefile() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	// Create a dummy savefile
	if (!writeDataToFile("tBedSavefileToRemove.0", "Dummy Savefile!")) {
		Testsuite::logDetailedPrintf("Writing data to savefile failed\n");
		return kTestFailed;
	}

	// Remove it
	saveFileMan->removeSavefile("tBedSavefileToRemove.0");

	// Try opening it Now
	Common::InSaveFile *loadFile = saveFileMan->openForLoading("saveFile.0");
	if (loadFile) {
		// Removing failed
		Testsuite::logDetailedPrintf("Removing savefile failed\n");
		return kTestFailed;
	}

	return kTestPassed;
}

TestExitStatus SaveGametests::testRenamingSavefile() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	// Open a file for renaming
	if (!writeDataToFile("tBedSomeWeirdName.0", "Rename me!")) {
		Testsuite::logDetailedPrintf("Writing data to savefile failed\n");
		return kTestFailed;
	}

	// Rename it
	saveFileMan->renameSavefile("tBedSomeWeirdName.0", "tBedSomeCoolName.0");

	// Verify if it contains the same data
	if (!readAndVerifyData("tBedSomeCoolName.0", "Rename me!")) {
		Testsuite::logDetailedPrintf("Renaming savefile failed\n");
		return kTestFailed;
	}

	return kTestPassed;
}

TestExitStatus SaveGametests::testListingSavefile() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	saveFileMan->clearError();

	// create some savefiles
	const char *savefileName[] = {"tBedSavefileToList.0", "tBedSavefileToList.1", "tBedSavefileToList.2"};
	writeDataToFile("tBedSavefileToList.0", "Save me!");
	writeDataToFile("tBedSavefileToList.1", "Save me!");
	writeDataToFile("tBedSavefileToList.2", "Save me!");

	Common::Error err = saveFileMan->getError();

	if (err.getCode() != Common::kNoError) {
		// Abort. Some Error in writing files
		Testsuite::logDetailedPrintf("Error while creating savefiles: %s\n", err.getDesc().c_str());
		return kTestFailed;
	}

	Common::StringArray savefileList = saveFileMan->listSavefiles("tBedSavefileToList.?");
	if (savefileList.size() == ARRAYSIZE(savefileName)) {
		// Match them exactly
		// As the order of savefileList may be platform specific, match them exhaustively
		for (uint i = 0; i < ARRAYSIZE(savefileName); i++) {
			for (uint j = 0; j < savefileList.size(); j++) {
				if (savefileList[j].equals(savefileName[i])) {
					break;
				}
				if (savefileList.size() == j) {
					// A match for this name not found
					Testsuite::logDetailedPrintf("Listed Names don't match\n");
					return kTestFailed;
				}
			}
		}
		return kTestPassed;
	}

	Testsuite::logDetailedPrintf("listing Savefiles failed!\n");
	return kTestFailed;
}

TestExitStatus SaveGametests::testErrorMessages() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	saveFileMan->clearError();

	// Try opening a non existing file
	readAndVerifyData("tBedSomeNonExistentSaveFile.0", "File doesn't exists!");

	Common::Error err = saveFileMan->getError();
	if (err.getCode() == Common::kNoError) {
		// blunder! how come?
		Testsuite::logDetailedPrintf("SaveFileMan.getError() failed\n");
		return kTestFailed;
	}
	// Can't actually predict whether which error, kInvalidPath or kPathDoesNotExist or some other?
	// So just return kTestPassed if some error
	Testsuite::logDetailedPrintf("getError returned : %s\n", saveFileMan->getErrorDesc().c_str());
	return kTestPassed;
}

SaveGameTestSuite::SaveGameTestSuite() {
	addTest("OpeningSaveFile", &SaveGametests::testSaveLoadState, false);
	addTest("RemovingSaveFile", &SaveGametests::testRemovingSavefile, false);
	addTest("RenamingSaveFile", &SaveGametests::testRenamingSavefile, false);
	addTest("ListingSaveFile", &SaveGametests::testListingSavefile, false);
	addTest("VerifyErrorMessages", &SaveGametests::testErrorMessages, false);
}

} // End of namespace Testbed
