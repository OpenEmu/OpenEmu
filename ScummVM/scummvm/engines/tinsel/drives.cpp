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
 * CD/drive handling functions
 */

#include "common/textconsole.h"
#include "tinsel/drives.h"
#include "tinsel/scene.h"
#include "tinsel/tinsel.h"
#include "tinsel/sched.h"
#include "tinsel/strres.h"

namespace Tinsel {

// FIXME: Avoid non-const global vars

char g_currentCD = '1';

static bool g_bChangingCD = false;
static char g_nextCD = '\0';

static uint32 g_lastTime = 0;
extern LANGUAGE g_sampleLanguage;


void CdCD(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (g_bChangingCD) {
		if (CoroScheduler.getCurrentProcess()) {
			// FIXME: CdCD gets passed a Common::nullContext in RegisterGlobals() and
			//        PrimeSceneHopper(), because I didn't know how to get a proper
			//        context without converting the whole calling stack to CORO'd
			//        functions. If these functions really get called while a CD
			//        change is requested, this needs to be resolved.
			if (coroParam == Common::nullContext)
				error("CdCD needs context");
			CORO_SLEEP(1);
		} else
			error("No current process in CdCD()");
	}

	CORO_END_CODE;
}

int GetCurrentCD() {
	// count from 1
	return (g_currentCD - '1' + 1);
}

static const uint32 cdFlags[] = { fCd1, fCd2, fCd3, fCd4, fCd5, fCd6, fCd7, fCd8 };

void SetCD(int flags) {
	if (flags & cdFlags[g_currentCD - '1'])
		return;

	error("SetCD() problem");
}

int GetCD(int flags) {
	int i;
	char cd = '\0';

	if (flags & cdFlags[g_currentCD - '1'])
		return GetCurrentCD();

	for (i = 0; i < 8; i++) {
		if (flags & cdFlags[i]) {
			cd = (char)(i + '1');
			break;
		}
	}
	assert(i != 8);

	g_nextCD = cd;
	return cd;
}

void DoCdChange() {
	if (g_bChangingCD && (g_system->getMillis() > (g_lastTime + 1000))) {
		g_lastTime = g_system->getMillis();
		_vm->_sound->closeSampleStream();

		// Use the filesize of the sample file to determine, for Discworld 2, which CD it is
		if (TinselV2) {
			TinselFile f;
			if (!f.open(_vm->getSampleFile(g_sampleLanguage)))
				// No CD present
				return;

			char sampleCdNumber = (f.size() >= (200 * 1024 * 1024)) ? '1' : '2';

			f.close();

			if (g_currentCD != sampleCdNumber)
				return;
		}

		_vm->_sound->openSampleFiles();
		ChangeLanguage(TextLanguage());
		g_bChangingCD = false;
	}
}

void SetNextCD(int cdNumber) {
	assert(cdNumber == 1 || cdNumber == 2);

	g_nextCD = (char)(cdNumber + '1' - 1);
}

bool GotoCD() {
	// WORKAROUND: Somehow, CdDoChange() is called twice... Hopefully, this guard helps
	if (g_currentCD == g_nextCD)
		return false;

	g_currentCD = g_nextCD;

/*	if (bNoCD)	{
		strcpy(cdDirectory, hdDirectory);
		cdLastBit[3] = currentCD;
		strcat(cdDirectory, cdLastBit);
	}
*/
	g_bChangingCD = true;

	return true;
}

bool TinselFile::_warningShown = false;

TinselFile::TinselFile() : ReadStreamEndian(TinselV1Mac) {
	_stream = NULL;
}

TinselFile::~TinselFile() {
	delete _stream;
}

bool TinselFile::openInternal(const Common::String &filename) {
	_stream = SearchMan.createReadStreamForMember(filename);
	if (!_stream)
		_stream = SearchMan.createReadStreamForMember(filename + ".");
	return _stream != 0;
}

bool TinselFile::open(const Common::String &filename) {
	if (openInternal(filename))
		return true;

	if (!TinselV2)
		return false;

	// Check if the file being requested is the *1.* or *2.* files
	const char *fname = filename.c_str();
	const char *p = strchr(fname, '1');
	if (!p)
		p = strchr(fname, '2');
	if (!p || (*(p + 1) != '.'))
		return false;

	// Form a filename without the CD number character
	char newFilename[50];
	strncpy(newFilename, fname, p - fname);
	strcpy(newFilename + (p - fname), p + 1);

	return openInternal(newFilename);
}

void TinselFile::close() {
	delete _stream;
	_stream = NULL;
}

int32 TinselFile::pos() const {
	assert(_stream);
	return _stream->pos();
}

int32 TinselFile::size() const {
	assert(_stream);
	return _stream->size();
}

bool TinselFile::seek(int32 offset, int whence) {
	assert(_stream);
	return _stream->seek(offset, whence);
}

bool TinselFile::eos() const {
	assert(_stream);
	return _stream->eos();
}

bool TinselFile::err() const {
	assert(_stream);
	return _stream->err();
}

void TinselFile::clearErr() {
	assert(_stream);
	_stream->clearErr();
}

uint32 TinselFile::read(void *dataPtr, uint32 dataSize) {
	assert(_stream);
	return _stream->read(dataPtr, dataSize);
}




} // End of namespace Tinsel
