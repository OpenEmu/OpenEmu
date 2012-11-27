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

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"

namespace Cruise {

uint8 *PAL_ptr = NULL;

int16 numLoadedPal;
int16 fileData2;

char currentBaseName[15] = "";

void loadPal(volumeDataStruct *entry) {
	// This code isn't currently being used
#if 0
	char name[20];

	if (_vm->_PAL_file.isOpen())
		_vm->_PAL_file.close();

	removeExtention(entry->ident, name);
	strcat(name, ".PAL");

	if (!_vm->_PAL_file.open(name))
		return;

	numLoadedPal = _vm->_PAL_file.readSint16BE();
	fileData2 = _vm->_PAL_file.readSint16BE();

	PAL_ptr = (uint8 *)MemAlloc(numLoadedPal * fileData2);
#endif
}

void closePal() {
	if (_vm->_PAL_file.isOpen()) {
		_vm->_PAL_file.close();

		MemFree(PAL_ptr);
		PAL_ptr = NULL;

		numLoadedPal = 0;
		fileData2 = 0;
	}
}

int closeBase() {
	if (_vm->_currentVolumeFile.isOpen()) {
		_vm->_currentVolumeFile.close();

		MemFree(volumePtrToFileDescriptor);

		strcpy(currentBaseName, "");
	}

	if (_vm->_PAL_file.isOpen()) {
		closePal();
	}

	return 0;
}

int getVolumeDataEntry(volumeDataStruct *entry) {
	char buffer[256];
	int i;

	volumeNumEntry = 0;
	volumeNumberOfEntry = 0;

	if (_vm->_currentVolumeFile.isOpen()) {
		freeDisk();
	}

	askDisk(-1);

	strcpy(buffer, entry->ident);

	_vm->_currentVolumeFile.open(buffer);

	if (!_vm->_currentVolumeFile.isOpen()) {
		return (-14);
	}

	changeCursor(CURSOR_DISK);

	volumeNumberOfEntry = _vm->_currentVolumeFile.readSint16BE();
	volumeSizeOfEntry = _vm->_currentVolumeFile.readSint16BE();

	volumeNumEntry = volumeNumberOfEntry;

	assert(volumeSizeOfEntry == 14 + 4 + 4 + 4 + 4);

	volumePtrToFileDescriptor = (fileEntry *) mallocAndZero(sizeof(fileEntry) * volumeNumEntry);

	for (i = 0; i < volumeNumEntry; i++) {
		volumePtrToFileDescriptor[i].name[0] = 0;
		volumePtrToFileDescriptor[i].offset = 0;
		volumePtrToFileDescriptor[i].size = 0;
		volumePtrToFileDescriptor[i].extSize = 0;
		volumePtrToFileDescriptor[i].unk3 = 0;
	}

	for (i = 0; i < volumeNumEntry; i++) {
		_vm->_currentVolumeFile.read(&volumePtrToFileDescriptor[i].name, 14);
		volumePtrToFileDescriptor[i].offset = _vm->_currentVolumeFile.readSint32BE();
		volumePtrToFileDescriptor[i].size = _vm->_currentVolumeFile.readSint32BE();
		volumePtrToFileDescriptor[i].extSize = _vm->_currentVolumeFile.readSint32BE();
		volumePtrToFileDescriptor[i].unk3 = _vm->_currentVolumeFile.readSint32BE();
	}

	strcpy(currentBaseName, entry->ident);

	loadPal(entry);

	return 0;
}

int searchFileInVolCnf(const char *fileName, int32 diskNumber) {
	int foundDisk = -1;
	int i;

	for (i = 0; i < numOfDisks; i++) {
		if (volumeData[i].diskNumber == diskNumber) {
			int j;
			int numOfEntry = volumeData[i].size / 13;

			for (j = 0; j < numOfEntry; j++) {
				if (!strcmp(volumeData[i].ptr[j].name, fileName)) {
					return (i);
				}
			}
		}
	}

	return (foundDisk);
}

int32 findFileInDisksSub1(const char *fileName) {
	int foundDisk = -1;
	int i;

	for (i = 0; i < numOfDisks; i++) {
		int j;
		int numOfEntry = volumeData[i].size / 13;

		for (j = 0; j < numOfEntry; j++) {
			if (!strcmp(volumeData[i].ptr[j].name, fileName)) {
				return (i);
			}
		}
	}

	return (foundDisk);
}

void freeDisk() {
	if (_vm->_currentVolumeFile.isOpen()) {
		_vm->_currentVolumeFile.close();
		MemFree(volumePtrToFileDescriptor);
	}

	/* TODO
	 * if (PAL_fileHandle)
	 * {
	 * freeAllDataPtr();
	 * }
	 */
}

int16 findFileInList(char *fileName) {
	int i;

	if (!_vm->_currentVolumeFile.isOpen()) {
		return (-1);
	}

	strToUpper(fileName);

	if (volumeNumEntry <= 0) {
		return (-1);
	}

	for (i = 0; i < volumeNumEntry; i++) {
		if (!strcmp(volumePtrToFileDescriptor[i].name, fileName)) {
			return (i);
		}
	}

	return (-1);
}

void askDisk(int16 discNumber) {
	char fileName[256];
	char string[256];
	char messageDrawn = 0;

	if (discNumber != -1) {
		currentDiskNumber = discNumber;
	}
	// skip drive selection stuff

	sprintf(fileName, "VOL.%d", currentDiskNumber);

	sprintf(string, "INSERER LE DISQUE %d EN ", currentDiskNumber);

	//while (Common::File::exists((const char*)fileName))
	{
		if (!messageDrawn) {
			drawMsgString(string);
			messageDrawn = 1;
		}
	}

	changeCursor(currentCursor);
}

int16 findFileInDisks(const char *name) {
	char fileName[50];
	int disk;
	int fileIdx;

	strcpy(fileName, name);
	strToUpper(fileName);

	if (!volumeDataLoaded) {
		debug(1, "CNF wasn't loaded, reading now...");
		if (_vm->_currentVolumeFile.isOpen()) {
			askDisk(-1);
			freeDisk();
		}

		askDisk(1);
		readVolCnf();
	}

	if (_vm->_currentVolumeFile.isOpen()) {
		askDisk(-1);
	}

	fileIdx = findFileInList(fileName);

	if (fileIdx >= 0) {
		return (fileIdx);
	}

	disk = searchFileInVolCnf(fileName, currentDiskNumber);

	if (disk >= 0) {
		int temp;

		debug(1, "File found on disk %d", disk);

		if (_vm->_currentVolumeFile.isOpen()) {
			askDisk(-1);
		}

		freeDisk();

		askDisk(volumeData[disk].diskNumber);

		getVolumeDataEntry(&volumeData[disk]);

		temp = findFileInList(fileName);

		if (temp >= 0)
			return (temp);

		return (-1);

	} else {
		int temp;

		temp = findFileInDisksSub1(fileName);

		if (temp >= 0) {
			int temp2;

			askDisk(volumeData[temp].diskNumber);

			getVolumeDataEntry(&volumeData[temp]);

			temp2 = findFileInList(fileName);

			if (temp2 >= 0)
				return (temp2);
		}

		return (-1);
	}
}

int closeCnf() {
	for (long int i = 0; i < numOfDisks; i++) {
		if (volumeData[i].ptr) {
			MemFree(volumeData[i].ptr);
			volumeData[i].ptr = NULL;
		}
	}

	volumeDataLoaded = 0;

	return 0;
}

int16 readVolCnf() {
	int i;
	Common::File fileHandle;
	//short int sizeHEntry;

	volumeDataLoaded = 0;

	for (i = 0; i < 20; i++) {
		volumeData[i].ident[0] = 0;
		volumeData[i].ptr = NULL;
		volumeData[i].diskNumber = i + 1;
		volumeData[i].size = 0;
	}

	fileHandle.open("VOL.CNF");

	if (!fileHandle.isOpen()) {
		return (0);
	}

	numOfDisks = fileHandle.readSint16BE();
	/*sizeHEntry =*/ fileHandle.readSint16BE();		// size of one header entry - 20 bytes

	for (i = 0; i < numOfDisks; i++) {
		//      fread(&volumeData[i],20,1,fileHandle);
		fileHandle.read(&volumeData[i].ident, 10);
		fileHandle.read(&volumeData[i].ptr, 4);
		volumeData[i].diskNumber = fileHandle.readSint16BE();
		volumeData[i].size = fileHandle.readSint32BE();

		debug(1, "Disk number: %d", volumeData[i].diskNumber);
	}

	for (i = 0; i < numOfDisks; i++) {
		dataFileName *ptr;

		volumeData[i].size = fileHandle.readSint32BE();

		ptr = (dataFileName *) mallocAndZero(volumeData[i].size);

		volumeData[i].ptr = ptr;

		if (!ptr) {
			fileHandle.close();
			return (-2);
		}

		fileHandle.read(ptr, volumeData[i].size);
	}

	fileHandle.close();

	volumeDataLoaded = 1;

//#define dumpResources
#ifdef dumpResources

	for (i = 0; i < numOfDisks; i++) {
		int j;
		char nameBuffer[256];
		fileEntry *buffer;

		sprintf(nameBuffer, "D%d.", i + 1);

		fileHandle.open(nameBuffer);

		short int numEntry;
		short int sizeEntry;

		numEntry = fileHandle.readSint16BE();
		sizeEntry = fileHandle.readSint16BE();

		buffer = (fileEntry *) mallocAndZero(numEntry * sizeEntry);

		for (j = 0; j < numEntry; j++) {
			fileHandle.seek(4 + j*0x1E);
			fileHandle.read(buffer[j].name, 14);
			buffer[j].offset = fileHandle.readSint32BE();
			buffer[j].size = fileHandle.readSint32BE();
			buffer[j].extSize = fileHandle.readSint32BE();
			buffer[j].unk3 = fileHandle.readSint32BE();

			fileHandle.seek(buffer[j].offset);

			char *bufferLocal;
			bufferLocal = (char *)mallocAndZero(buffer[j].size);

			fileHandle.read(bufferLocal, buffer[j].size);

			char nameBuffer[256];

			sprintf(nameBuffer, "%s", buffer[j].name);

			if (buffer[j].size == buffer[j].extSize) {
				Common::DumpFile fout;
				fout.open(nameBuffer);
				if (fout.isOpen())
					fout.write(bufferLocal, buffer[j].size);
			} else {
				char *uncompBuffer = (char *)mallocAndZero(buffer[j].extSize + 500);

				delphineUnpack((uint8 *) uncompBuffer, (const uint8 *) bufferLocal, buffer[j].size);

				Common::File fout;
				fout.open(nameBuffer, Common::File::kFileWriteMode);
				if (fout.isOpen())
					fout.write(uncompBuffer, buffer[j].extSize);

				//MemFree(uncompBuffer);

			}

			MemFree(bufferLocal);
		}
		fileHandle.close();
	}

#endif

	return (1);
}

///////////////////////////::

// This code used to rely on "strupr", which is non existant on my system,
// thus I just implemented this function instead. - LordHoto
//
// TODO: This might be code duplication, please check this out.
void strToUpper(char *string) {
	while (*string) {
		*string = toupper(*string);
		++string;
	}
}

void drawMsgString(const char *string) {
	//printf("%s\n",string);
}

} // End of namespace Cruise
