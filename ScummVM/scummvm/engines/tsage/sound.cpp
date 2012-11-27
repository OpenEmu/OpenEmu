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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/decoders/raw.h"
#include "common/config-manager.h"
#include "tsage/core.h"
#include "tsage/globals.h"
#include "tsage/debugger.h"
#include "tsage/graphics.h"
#include "tsage/tsage.h"

namespace TsAGE {

static SoundManager *_soundManager = NULL;

/*--------------------------------------------------------------------------*/

SoundManager::SoundManager() {
	_soundManager = this;
	_sndmgrReady = false;
	_ourSndResVersion = 0x102;
	_ourDrvResVersion = 0x10A;

	for (int i = 0; i < SOUND_ARR_SIZE; ++i)
		_voiceTypeStructPtrs[i] = NULL;

	_groupsAvail = 0;
	_newVolume = _masterVol = 127;
	_driversDetected = false;
	_needToRethink = false;

	_soTimeIndexFlag = false;
}

SoundManager::~SoundManager() {
	if (_sndmgrReady) {
		Common::StackLock slock(_serverDisabledMutex);
		g_vm->_mixer->stopAll();

		for (Common::List<Sound *>::iterator i = _soundList.begin(); i != _soundList.end(); ) {
			Sound *s = *i;
			++i;
			s->stop();
		}
		for (Common::List<SoundDriver *>::iterator i = _installedDrivers.begin(); i != _installedDrivers.end(); ) {
			SoundDriver *driver = *i;
			++i;
			delete driver;
		}
		sfTerminate();

//		g_system->getTimerManager()->removeTimerProc(_sfUpdateCallback);
	}

	// Free any allocated voice type structures
	for (int idx = 0; idx < SOUND_ARR_SIZE; ++idx) {
		if (sfManager()._voiceTypeStructPtrs[idx]) {
			delete sfManager()._voiceTypeStructPtrs[idx];
			sfManager()._voiceTypeStructPtrs[idx] = NULL;
		}
	}

	_soundManager = NULL;
}

void SoundManager::postInit() {
	if (!_sndmgrReady) {
		g_saver->addSaveNotifier(&SoundManager::saveNotifier);
		g_saver->addLoadNotifier(&SoundManager::loadNotifier);
		g_saver->addListener(this);


//	I originally separated the sound manager update method into a separate thread, since
//  it handles updates for both music and Fx. However, since Adlib updates also get done in a
//	thread, and doesn't get too far ahead, I've left it to the AdlibSoundDriver class to
//	call the update method, rather than having it be called separately
//		g_system->getTimerManager()->installTimerProc(_sfUpdateCallback, 1000000 / SOUND_FREQUENCY, NULL, "tsageSoundUpdate");
		_sndmgrReady = true;
	}
}

/**
 * Loops through all the loaded sounds, and stops any that have been flagged for stopping
 */
void SoundManager::dispatch() {
	Common::List<Sound *>::iterator i = _soundList.begin();
	while (i != _soundList.end()) {
		Sound *sound = *i;
		++i;

		// If the sound is flagged for stopping, then stop it
		if (sound->_stoppedAsynchronously) {
			sound->stop();
		}
	}
}

void SoundManager::syncSounds() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	bool music_mute = mute;

	if (!mute) {
		music_mute = ConfMan.getBool("music_mute");
	}

	// Get the new music volume
	int musicVolume = music_mute ? 0 : MIN(255, ConfMan.getInt("music_volume"));

	this->setMasterVol(musicVolume / 2);
}

void SoundManager::update() {
	sfSoundServer();
}

Common::List<SoundDriverEntry> &SoundManager::buildDriverList(bool detectFlag) {
	assert(_sndmgrReady);
	_availableDrivers.clear();

	// Build up a list of available drivers. Currently we only implement an Adlib music
	// and SoundBlaster FX driver

	// Adlib driver
	SoundDriverEntry sd;
	sd._driverNum = ADLIB_DRIVER_NUM;
	sd._status = detectFlag ? SNDSTATUS_DETECTED : SNDSTATUS_SKIPPED;
	sd._field2 = 0;
	sd._field6 = 15000;
	sd._shortDescription = "Adlib or SoundBlaster";
	sd._longDescription = "3812fm";
	_availableDrivers.push_back(sd);

	// SoundBlaster entry
	SoundDriverEntry sdFx;
	sdFx._driverNum = SBLASTER_DRIVER_NUM;
	sdFx._status = detectFlag ? SNDSTATUS_DETECTED : SNDSTATUS_SKIPPED;
	sdFx._field2 = 0;
	sdFx._field6 = 15000;
	sdFx._shortDescription = "SndBlast";
	sdFx._longDescription = "SoundBlaster";
	_availableDrivers.push_back(sdFx);

	_driversDetected = true;
	return _availableDrivers;
}

void SoundManager::installConfigDrivers() {
	installDriver(ADLIB_DRIVER_NUM);
	installDriver(SBLASTER_DRIVER_NUM);
}

Common::List<SoundDriverEntry> &SoundManager::getDriverList(bool detectFlag) {
	if (detectFlag)
		return _availableDrivers;
	else
		return buildDriverList(false);
}

void SoundManager::dumpDriverList() {
	_availableDrivers.clear();
}

/**
 * Install the specified driver number
 */
void SoundManager::installDriver(int driverNum) {
	// If driver is already installed, no need to install it
	if (isInstalled(driverNum))
		return;

	// Instantiate the sound driver
	SoundDriver *driver = instantiateDriver(driverNum);
	if (!driver)
		return;

	assert((_ourDrvResVersion >= driver->_minVersion) && (_ourDrvResVersion <= driver->_maxVersion));

	// Mute any loaded sounds
	Common::StackLock slock(_serverDisabledMutex);

	for (Common::List<Sound *>::iterator i = _playList.begin(); i != _playList.end(); ++i)
		(*i)->mute(true);

	// Install the driver
	if (!sfInstallDriver(driver))
		error("Sound driver initialization failed");

	switch (driverNum) {
	case ROLAND_DRIVER_NUM:
	case ADLIB_DRIVER_NUM: {
		// Handle loading bank infomation
		byte *bankData = g_resourceManager->getResource(RES_BANK, driverNum, 0, true);
		if (bankData) {
			// Install the patch bank data
			sfInstallPatchBank(driver, bankData);
			DEALLOCATE(bankData);
		} else {
			// Could not locate patch bank data, so unload the driver
			sfUnInstallDriver(driver);

			// Unmute currently active sounds
			for (Common::List<Sound *>::iterator i = _playList.begin(); i != _playList.end(); ++i)
				(*i)->mute(false);
		}
		break;
	}
	}
}

/**
 * Instantiate a driver class for the specified driver number
 */
SoundDriver *SoundManager::instantiateDriver(int driverNum) {
	switch (driverNum) {
	case ADLIB_DRIVER_NUM:
		return new AdlibSoundDriver();
	case SBLASTER_DRIVER_NUM:
		return new SoundBlasterDriver();
	default:
		error("Unknown sound driver - %d", driverNum);
	}
}

/**
 * Uninstall the specified driver
 */
void SoundManager::unInstallDriver(int driverNum) {
	Common::List<SoundDriver *>::const_iterator i;
	for (i = _installedDrivers.begin(); i != _installedDrivers.end(); ++i) {
		if ((*i)->_driverResID == driverNum) {
			// Found driver to remove

			// Mute any loaded sounds
			Common::StackLock slock(_serverDisabledMutex);

			Common::List<Sound *>::iterator j;
			for (j = _playList.begin(); j != _playList.end(); ++j)
				(*j)->mute(true);

			// Uninstall the driver
			sfUnInstallDriver(*i);

			// Re-orient all the loaded sounds
			for (j = _soundList.begin(); j != _soundList.end(); ++j)
				(*j)->orientAfterDriverChange();

			// Unmute currently active sounds
			for (j = _playList.begin(); j != _playList.end(); ++j)
				(*j)->mute(false);
		}
	}
}

/**
 * Returns true if a specified driver number is currently installed
 */
bool SoundManager::isInstalled(int driverNum) const {
	Common::List<SoundDriver *>::const_iterator i;
	for (i = _installedDrivers.begin(); i != _installedDrivers.end(); ++i) {
		if ((*i)->_driverResID == driverNum)
			return true;
	}

	return false;
}

void SoundManager::setMasterVol(int volume) {
	_newVolume = volume;
}

int SoundManager::getMasterVol() const {
	return _masterVol;
}

void SoundManager::loadSound(int soundNum, bool showErrors) {
	// This method preloaded the data associated with a given sound, so is now redundant
}

void SoundManager::unloadSound(int soundNum) {
	// This method signalled the resource manager to unload the data for a sound, and is now redundant
}

int SoundManager::determineGroup(const byte *soundData) {
	return sfDetermineGroup(soundData);
}

void SoundManager::checkResVersion(const byte *soundData) {
	int maxVersion = READ_LE_UINT16(soundData + 4);
	int minVersion = READ_LE_UINT16(soundData + 6);

	if (_soundManager->_ourSndResVersion < minVersion)
		error("Attempt to play/prime sound resource that is too new");
	if (_soundManager->_ourSndResVersion > maxVersion)
		error("Attempt to play/prime sound resource that is too old");
}

int SoundManager::extractPriority(const byte *soundData) {
	return READ_LE_UINT16(soundData + 12);
}

int SoundManager::extractLoop(const byte *soundData) {
	return READ_LE_UINT16(soundData + 14);
}

void SoundManager::extractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum) {
	sfExtractTrackInfo(trackInfo, soundData, groupNum);
}

void SoundManager::addToSoundList(Sound *sound) {
	if (!contains(_soundList, sound))
		_soundList.push_back(sound);
}

void SoundManager::removeFromSoundList(Sound *sound) {
	_soundList.remove(sound);
}

void SoundManager::addToPlayList(Sound *sound) {
	sfAddToPlayList(sound);
}

void SoundManager::removeFromPlayList(Sound *sound) {
	if (_soundManager)
		sfRemoveFromPlayList(sound);
}

bool SoundManager::isOnPlayList(Sound *sound) {
	return sfIsOnPlayList(sound);
}

void SoundManager::updateSoundVol(Sound *sound) {
	sfUpdateVolume(sound);
}

void SoundManager::updateSoundPri(Sound *sound) {
	sfUpdatePriority(sound);
}

void SoundManager::updateSoundLoop(Sound *sound) {
	sfUpdateLoop(sound);
}

void SoundManager::rethinkVoiceTypes() {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);
	sfRethinkVoiceTypes();
}

void SoundManager::sfSoundServer() {
	if (sfManager()._needToRethink) {
		sfRethinkVoiceTypes();
		sfManager()._needToRethink = false;
	} else {
		sfDereferenceAll();
	}

	// If the master volume has changed, update it
	if (sfManager()._newVolume != sfManager()._masterVol)
		sfSetMasterVol(sfManager()._newVolume);

	// If a time index has been set for any sound, fast forward to it
	SynchronizedList<Sound *>::iterator i;
	for (i = sfManager()._playList.begin(); i != sfManager()._playList.end(); ++i) {
		Sound *s = *i;
		if (s->_newTimeIndex != 0) {
			s->mute(true);
			s->soSetTimeIndex(s->_newTimeIndex);
			s->mute(false);
			s->_newTimeIndex = 0;
		}
	}

	// Handle any fading if necessary
	sfProcessFading();

	// Poll all sound drivers in case they need it
	for (Common::List<SoundDriver *>::iterator j = sfManager()._installedDrivers.begin();
				j != sfManager()._installedDrivers.end(); ++j) {
		(*j)->poll();
	}
}

void SoundManager::sfProcessFading() {
	// Loop through processing active sounds
	bool removeFlag = false;
	Common::List<Sound *>::iterator i = sfManager()._playList.begin();
	while (i != sfManager()._playList.end()) {
		Sound *s = *i;
		++i;

		if (!s->_pausedCount)
			removeFlag = s->soServiceTracks();
		if (removeFlag) {
			sfDoRemoveFromPlayList(s);
			s->_stoppedAsynchronously = true;
			sfManager()._needToRethink = true;
		}

		if (s->_fadeDest != -1) {
			if (s->_fadeCounter != 0)
				--s->_fadeCounter;
			else {
				if (s->_volume >= s->_fadeDest) {
					s->_volume = ((s->_volume - s->_fadeDest) > s->_fadeSteps) ?
						s->_volume - s->_fadeSteps : s->_fadeDest;
				} else {
					s->_volume = ((s->_fadeDest - s->_volume) > s->_fadeSteps) ?
						s->_volume + s->_fadeSteps : s->_fadeDest;
				}

				sfDoUpdateVolume(s);
				if (s->_volume != s->_fadeDest)
					s->_fadeCounter = s->_fadeTicks;
				else {
					s->_fadeDest = -1;
					if (s->_stopAfterFadeFlag) {
						sfDoRemoveFromPlayList(s);
						s->_stoppedAsynchronously = true;
						sfManager()._needToRethink = true;
					}
				}
			}
		}
	}

	// Loop through the voiceType list
	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceTypeStruct *vtStruct = sfManager()._voiceTypeStructPtrs[voiceIndex];
		if (!vtStruct)
			continue;

		if (vtStruct->_voiceType == VOICETYPE_1) {
			for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
				if (vtStruct->_entries[idx]._type1._field6 >= -1)
					++vtStruct->_entries[idx]._type1._field6;
			}
		}
	}
}

bool SoundManager::isFading() {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);

	// Loop through any active sounds to see if any are being actively faded
	Common::List<Sound *>::iterator i = sfManager()._playList.begin();
	while (i != sfManager()._playList.end()) {
		Sound *s = *i;
		++i;

		if (s->_fadeDest != -1)
			return true;
	}

	return false;
}

void SoundManager::sfUpdateVoiceStructs() {
	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceTypeStruct *vs = sfManager()._voiceTypeStructPtrs[voiceIndex];
		if (!vs)
			continue;

		if (vs->_voiceType == VOICETYPE_0) {
			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntry &vse = vs->_entries[idx];

				vse._type0._sound = vse._type0._sound2;
				vse._type0._channelNum = vse._type0._channelNum2;
				vse._type0._priority = vse._type0._priority2;
				vse._type0._fieldA = vse._type0._field12;
			}
		} else {
			vs->_field3 = vs->_numVoices;

			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntry &vse = vs->_entries[idx];

				vse._type1._sound = vse._type1._sound2;
				vse._type1._channelNum = vse._type1._channelNum2;
				vse._type1._priority = vse._type1._priority2;
			}
		}
	}
}

void SoundManager::sfUpdateVoiceStructs2() {
	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceTypeStruct *vtStruct = sfManager()._voiceTypeStructPtrs[voiceIndex];
		if (!vtStruct)
			continue;

		for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {

			if (vtStruct->_voiceType == VOICETYPE_0) {
				VoiceStructEntryType0 &vse = vtStruct->_entries[idx]._type0;
				vse._sound2 = vse._sound;
				vse._channelNum2 = vse._channelNum;
				vse._priority2 = vse._priority;
				vse._field12 = vse._fieldA;
			} else {
				VoiceStructEntryType1 &vse = vtStruct->_entries[idx]._type1;
				vse._sound2 = vse._sound;
				vse._channelNum2 = vse._channelNum;
				vse._priority2 = vse._priority;
			}
		}
	}
}

void SoundManager::sfUpdateCallback(void *ref) {
	((SoundManager *)ref)->update();
}

/*--------------------------------------------------------------------------*/

void SoundManager::saveNotifier(bool postFlag) {
	_soundManager->saveNotifierProc(postFlag);
}

void SoundManager::saveNotifierProc(bool postFlag) {
	// Nothing needs to be done when saving the game
}

void SoundManager::loadNotifier(bool postFlag) {
	_soundManager->loadNotifierProc(postFlag);
}

void SoundManager::loadNotifierProc(bool postFlag) {
	if (!postFlag) {
		// Stop any currently playing sounds
		if (_sndmgrReady) {
			Common::StackLock slock(_serverDisabledMutex);

			for (Common::List<Sound *>::iterator i = _soundList.begin(); i != _soundList.end(); ) {
				Sound *s = *i;
				++i;
				s->stop();
			}
		}
	} else {
		// Savegame is now loaded, so iterate over the sound list to prime any sounds as necessary
		for (Common::List<Sound *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
			Sound *s = *i;
			s->orientAfterRestore();
		}
	}
}

void SoundManager::listenerSynchronize(Serializer &s) {
	s.validate("SoundManager");
	assert(_sndmgrReady && _driversDetected);

	if (s.getVersion() < 6)
		return;

	Common::StackLock slock(_serverDisabledMutex);
	_playList.synchronize(s);

	_soundList.synchronize(s);
}

/*--------------------------------------------------------------------------*/

SoundManager &SoundManager::sfManager() {
	assert(_soundManager);
	return *_soundManager;
}

int SoundManager::sfDetermineGroup(const byte *soundData) {
	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		if ((v & _soundManager->_groupsAvail) == v)
			return v;

		p += 6 + (READ_LE_UINT16(p + 4) * 4);
	}

	return 0;
}

void SoundManager::sfAddToPlayList(Sound *sound) {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);

	sfDoAddToPlayList(sound);
	sound->_stoppedAsynchronously = false;
	sfRethinkVoiceTypes();
}

void SoundManager::sfRemoveFromPlayList(Sound *sound) {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);

	if (sfDoRemoveFromPlayList(sound))
		sfRethinkVoiceTypes();
}

bool SoundManager::sfIsOnPlayList(Sound *sound) {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);

	bool result = contains(_soundManager->_playList, sound);

	return result;
}

void SoundManager::sfRethinkSoundDrivers() {
	// Free any existing entries
	int idx;

	for (idx = 0; idx < SOUND_ARR_SIZE; ++idx) {
		if (sfManager()._voiceTypeStructPtrs[idx]) {
			delete sfManager()._voiceTypeStructPtrs[idx];
			sfManager()._voiceTypeStructPtrs[idx] = NULL;
		}
	}

	for (idx = 0; idx < SOUND_ARR_SIZE; ++idx) {
		byte flag = 0xff;
		int total = 0;

		// Loop through the sound drivers
		for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin();
				i != sfManager()._installedDrivers.end(); ++i) {
			// Process the group data for each sound driver
			SoundDriver *driver = *i;
			const byte *groupData = driver->_groupOffset->_pData;

			while (*groupData != 0xff) {
				byte byteVal = *groupData++;

				if (byteVal == idx) {
					byte byteVal2 = *groupData++;
					if (flag == 0xff)
						flag = byteVal2;
					else {
						assert(flag == byteVal2);
					}

					if (!flag) {
						while (*groupData++ != 0xff)
							++total;
					} else {
						total += *groupData;
						groupData += 2;
					}
				} else if (*groupData++ == 0) {
					while (*groupData != 0xff)
						++groupData;
					++groupData;
				} else {
					groupData += 2;
				}
			}
		}

		if (total) {
			VoiceTypeStruct *vs = new VoiceTypeStruct();
			sfManager()._voiceTypeStructPtrs[idx] = vs;

			if (!flag) {
				vs->_voiceType = VOICETYPE_0;
			} else {
				vs->_voiceType = VOICETYPE_1;
			}

			vs->_total = vs->_numVoices = total;
			vs->_field3 = 0;

			for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin();
							i != sfManager()._installedDrivers.end(); ++i) {
				// Process the group data for each sound driver
				SoundDriver *driver = *i;
				const byte *groupData = driver->_groupOffset->_pData;

				while (*groupData != 0xff) {
					byte byteVal = *groupData++;

					if (byteVal == idx) {
						++groupData;

						if (!flag) {
							while ((byteVal = *groupData++) != 0xff) {
								VoiceStructEntry ve;
								memset(&ve, 0, sizeof(VoiceStructEntry));

								ve._field1 = (byteVal & 0x80) ? 0 : 1;
								ve._driver = driver;
								ve._type0._sound = NULL;
								ve._type0._channelNum = 0;
								ve._type0._priority = 0;
								ve._type0._fieldA = 0;

								vs->_entries.push_back(ve);
							}
						} else {
							byteVal = *groupData;
							groupData += 2;

							for (int entryIndez = 0; entryIndez < byteVal; ++entryIndez) {
								VoiceStructEntry ve;
								memset(&ve, 0, sizeof(VoiceStructEntry));

								ve._voiceNum = entryIndez;
								ve._driver = driver;
								ve._type1._field4 = -1;
								ve._type1._field5 = 0;
								ve._type1._field6 = 0;
								ve._type1._sound = NULL;
								ve._type1._channelNum = 0;
								ve._type1._priority = 0;

								vs->_entries.push_back(ve);
							}
						}
					} else {
						if (*groupData++ != 0) {
							while (*groupData != 0xff)
								++groupData;
						} else {
							groupData += 2;
						}
					}
				}
			}
		}
	}
}

void SoundManager::sfRethinkVoiceTypes() {
	sfDereferenceAll();

	// Pre-processing
	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceTypeStruct *vs = sfManager()._voiceTypeStructPtrs[voiceIndex];
		if (!vs)
			continue;

		if (vs->_voiceType == VOICETYPE_0) {
			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntry &vse = vs->_entries[idx];
				vse._type0._sound3 = vse._type0._sound;
				vse._type0._channelNum3 = vse._type0._channelNum;
				vse._type0._priority3 = vse._type0._priority;
				vse._type0._field1A = vse._type0._fieldA;
				vse._type0._sound = NULL;
				vse._type0._channelNum = 0;
				vse._type0._priority = 0;
				vse._type0._fieldA = 0;
				vse._type0._sound2 = NULL;
				vse._type0._channelNum2 = 0;
				vse._type0._priority2 = 0;
				vse._type0._field12 = 0;
			}
		} else {
			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntry &vse = vs->_entries[idx];
				vse._type1._sound3 = vse._type1._sound;
				vse._type1._channelNum3 = vse._type1._channelNum;
				vse._type1._priority3 = vse._type1._priority;
				vse._type1._sound = NULL;
				vse._type1._channelNum = 0;
				vse._type1._priority = 0;
				vse._type1._sound2 = NULL;
				vse._type1._channelNum2 = 0;
				vse._type1._priority2 = 0;
			}

			// Reset the number of voices available
			vs->_numVoices = vs->_total;
		}
	}

	// Main processing loop
	int priorityOffset = 0;
	for (Common::List<Sound *>::iterator i = sfManager()._playList.begin(); i != sfManager()._playList.end(); ++i, priorityOffset += 16) {
		Sound *sound = *i;
		if ((sound->_mutedCount != 0) || (sound->_pausedCount != 0))
			continue;

		sfUpdateVoiceStructs();
		Common::fill(sound->_chWork, sound->_chWork + SOUND_ARR_SIZE, false);

		for (;;) {
			// Scan for sub priority
			int foundIndex = -1, foundPriority = 0;
			for (int idx = 0; idx < SOUND_ARR_SIZE; ++idx) {
				if (!(sound->_chFlags[idx] & 0x8000) & !sound->_chWork[idx]) {
					int subPriority = sound->_chSubPriority[idx];
					if (subPriority)
						subPriority = 16 - subPriority + priorityOffset;

					if (foundIndex != -1) {
						if (subPriority < foundPriority) {
							foundIndex = idx;
							foundPriority = subPriority;
						}
					} else {
						foundIndex = idx;
						foundPriority = subPriority;
					}
				}
			}
			if (foundIndex == -1)
				break;

			int chNumVoices = sound->_chNumVoices[foundIndex];
			sound->_chWork[foundIndex] = true;

			VoiceTypeStruct *vtStruct = sfManager()._voiceTypeStructPtrs[sound->_chVoiceType[foundIndex]];
			if (!vtStruct) {
				if (foundPriority)
					continue;

				sfUpdateVoiceStructs2();
				break;
			}

			if (vtStruct->_voiceType != VOICETYPE_0) {
				// Type 1
				int numVoices = vtStruct->_numVoices;

				if (numVoices >= chNumVoices) {
					int channelCount = chNumVoices, idx = 0;
					while (channelCount > 0) {
						if (!vtStruct->_entries[idx]._type1._sound2) {
							vtStruct->_entries[idx]._type1._sound2 = sound;
							vtStruct->_entries[idx]._type1._channelNum2 = foundIndex;
							vtStruct->_entries[idx]._type1._priority2 = foundPriority;
							--channelCount;
						}
						++idx;
					}

					vtStruct->_numVoices -= chNumVoices;
					continue;
				} else if (!foundPriority) {
					do {
						int maxPriority = 0;
						for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx)
							maxPriority = MAX(maxPriority, vtStruct->_entries[idx]._type1._priority2);

						if (!maxPriority) {
							sfUpdateVoiceStructs2();
							break;
						}

						for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
							if (vtStruct->_entries[idx]._type1._priority2 == maxPriority) {
								vtStruct->_entries[idx]._type1._sound2 = NULL;
								vtStruct->_entries[idx]._type1._channelNum2 = 0;
								vtStruct->_entries[idx]._type1._priority2 = 0;
								++numVoices;
							}
						}
					} while (chNumVoices > numVoices);

					int voicesCtr = chNumVoices;
					for (uint idx = 0; (idx < vtStruct->_entries.size()) && (voicesCtr > 0); ++idx) {
						if (!vtStruct->_entries[idx]._type1._sound2) {
							vtStruct->_entries[idx]._type1._sound2 = sound;
							vtStruct->_entries[idx]._type1._channelNum2 = foundIndex;
							vtStruct->_entries[idx]._type1._priority2 = foundPriority;
							--voicesCtr;
						}
					}

					numVoices -= chNumVoices;
					vtStruct->_numVoices = numVoices;
					continue;
				} else if (!numVoices) {
					break;
				}
				continue;
			} else {
				// Type 0
				if (sound->_isEmpty) {
					uint idx = 0;
					while ((idx < vtStruct->_entries.size()) &&
							(vtStruct->_entries[idx]._voiceNum == foundIndex))
						++idx;
					if (idx == vtStruct->_entries.size())
						continue;
				}

				int flagsVal = sound->_chFlags[foundIndex] & 3;
				if (flagsVal != 1) {
					// All modes except mode 1 (loc_23EDF)
					int entryIndex = -1, maxVoiceNum = 0;

					for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
						if (!vtStruct->_entries[idx]._type0._sound2 && (vtStruct->_entries[idx]._field1 != 0) &&
								(vtStruct->_entries[idx]._voiceNum > maxVoiceNum)) {
							maxVoiceNum = vtStruct->_entries[idx]._voiceNum;
							entryIndex = idx;
						}
					}

					if (entryIndex != -1) {
						vtStruct->_entries[entryIndex]._type0._sound2 = sound;
						vtStruct->_entries[entryIndex]._type0._channelNum2 = foundIndex;
						vtStruct->_entries[entryIndex]._type0._priority2 = foundPriority;
						vtStruct->_entries[entryIndex]._type0._field12 = 0;
						continue;
					}

					if (foundPriority != 0)
						continue;

					int maxPriority = 0;
					entryIndex = -1;
					for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
						if ((vtStruct->_entries[idx]._field1 != 0) &&
								(vtStruct->_entries[idx]._type0._priority2 > maxPriority)) {
							maxPriority = vtStruct->_entries[idx]._type0._priority2;
							entryIndex = idx;
						}
					}

					if (entryIndex != -1) {
						vtStruct->_entries[entryIndex]._type0._sound2 = sound;
						vtStruct->_entries[entryIndex]._type0._channelNum2 = foundIndex;
						vtStruct->_entries[entryIndex]._type0._priority2 = foundPriority;
						vtStruct->_entries[entryIndex]._type0._field12 = 0;
						continue;
					}

					sfUpdateVoiceStructs2();
					break;
				} else {
					// Channel mode 1 handling (loc_23FAC)

					bool foundMatch = false;
					int entryIndex = -1;
					for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
						if (vtStruct->_entries[idx]._voiceNum == foundIndex) {
							foundIndex = true;
							if (!vtStruct->_entries[idx]._type0._sound2) {
								entryIndex = idx;
								break;
							}
						}
					}

					if (entryIndex != -1) {
						vtStruct->_entries[entryIndex]._type0._sound2 = sound;
						vtStruct->_entries[entryIndex]._type0._channelNum2 = foundIndex;
						vtStruct->_entries[entryIndex]._type0._priority2 = foundPriority;
						vtStruct->_entries[entryIndex]._type0._field12 = 0;
						continue;
					}

					if (!foundMatch) {
						if (foundPriority)
							continue;
						if (entryIndex == -1) {
							sfUpdateVoiceStructs2();
							break;
						}
					}

					// Find the entry with the highest priority
					int maxPriority = 0;
					foundMatch = false;
					entryIndex = -1;
					for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
						if (vtStruct->_entries[idx]._voiceNum != foundIndex)
							continue;
						if (!vtStruct->_entries[idx]._type0._field12) {
							foundMatch = true;
							break;
						}

						if (vtStruct->_entries[idx]._type0._priority2 > maxPriority) {
							maxPriority = vtStruct->_entries[idx]._type0._priority2;
							entryIndex = -1;
						}
					}

					if (!foundMatch) {
						if (foundPriority)
							continue;

						if (entryIndex != -1) {
							vtStruct->_entries[entryIndex]._type0._sound2 = sound;
							vtStruct->_entries[entryIndex]._type0._channelNum2 = foundIndex;
							vtStruct->_entries[entryIndex]._type0._priority2 = foundPriority;
							vtStruct->_entries[entryIndex]._type0._field12 = 1;
							continue;
						}

						sfUpdateVoiceStructs2();
						break;
					}

					// Found a match (loc_24061)
					maxPriority = 0;
					int maxVoiceNum = 0;
					int priorityIndex = -1, voiceIndex = -1;

					for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
						if (vtStruct->_entries[idx]._field1) {
							if (!vtStruct->_entries[idx]._type0._sound2) {
								if (vtStruct->_entries[idx]._voiceNum > maxVoiceNum) {
									maxVoiceNum = vtStruct->_entries[idx]._voiceNum;
									voiceIndex = idx;
								}
							} else {
								if (vtStruct->_entries[idx]._type0._priority2 > maxPriority) {
									maxPriority = vtStruct->_entries[idx]._type0._priority2;
									priorityIndex = idx;
								}
							}
						}
					}

					if (voiceIndex != -1) {
						VoiceStructEntryType0 &vteSrc = vtStruct->_entries[foundIndex]._type0;
						VoiceStructEntryType0 &vteDest = vtStruct->_entries[voiceIndex]._type0;

						vteDest._sound2 = vteSrc._sound2;
						vteDest._channelNum2 = vteSrc._channelNum2;
						vteDest._priority2 = vteSrc._priority2;

						vteSrc._sound2 = sound;
						vteSrc._channelNum2 = foundIndex;
						vteSrc._priority2 = foundPriority;
						vteSrc._field12 = 1;
						continue;
					}

					if (!foundPriority)
						continue;
					if (priorityIndex == -1) {
						sfUpdateVoiceStructs2();
						break;
					}

					VoiceStructEntryType0 &vteSrc = vtStruct->_entries[foundIndex]._type0;
					VoiceStructEntryType0 &vteDest = vtStruct->_entries[priorityIndex]._type0;

					if (priorityIndex != foundIndex) {
						vteDest._sound2 = vteSrc._sound2;
						vteDest._channelNum2 = vteSrc._channelNum2;
						vteDest._priority2 = vteSrc._priority2;
						vteDest._field12 = vteSrc._field12;
					}

					vteSrc._sound2 = sound;
					vteSrc._channelNum2 = foundIndex;
					vteSrc._priority2 = foundPriority;
					vteSrc._field12 = 1;
					continue;
				}
			}
		}
	}

	// Post-processing
	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceTypeStruct *vs = sfManager()._voiceTypeStructPtrs[voiceIndex];
		if (!vs)
			continue;

		if (vs->_voiceType == VOICETYPE_0) {
			// Type 0
			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntryType0 &vse = vs->_entries[idx]._type0;
				SoundDriver *driver = vs->_entries[idx]._driver;
				assert(driver);

				if (vse._field12) {
					int total = 0;
					vse._sound = vse._sound2;
					if (vse._sound3 != vse._sound)
						++total;

					vse._channelNum = vse._channelNum2;
					if (vse._channelNum3 != vse._channelNum)
						++total;

					vse._priority = vse._priority2;
					vse._fieldA = 1;
					vse._sound2 = NULL;

					if (total) {
						driver->proc24(vse._channelNum, idx, vse._sound, 123, 0);
						driver->proc24(vse._channelNum, idx, vse._sound, 1, vse._sound->_chModulation[vse._channelNum]);
						driver->proc24(vse._channelNum, idx, vse._sound, 7,
							vse._sound->_chVolume[vse._channelNum] * vse._sound->_volume / 127);
						driver->proc24(vse._channelNum, idx, vse._sound, 10, vse._sound->_chPan[vse._channelNum]);
						driver->proc24(vse._channelNum, idx, vse._sound, 64, vse._sound->_chDamper[vse._channelNum]);

						driver->setProgram(vse._channelNum, vse._sound->_chProgram[vse._channelNum]);
						driver->setPitchBlend(vse._channelNum, vse._sound->_chPitchBlend[vse._channelNum]);

						vse._sound3 = NULL;
					}
				} else {
					vse._sound = NULL;
					vse._channelNum = 0;
					vse._priority = 0;
					vse._fieldA = 0;
				}
			}

			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntryType0 &vse = vs->_entries[idx]._type0;
				Sound *sound = vse._sound2;
				int channelNum = vse._channelNum2;

				if (!sound)
					continue;

				for (uint entryIndex = 0; entryIndex < vs->_entries.size(); ++entryIndex) {
					if ((vs->_entries[entryIndex]._type0._sound3 != sound) ||
						(vs->_entries[entryIndex]._type0._channelNum3 != channelNum)) {
						// Found match
						vs->_entries[entryIndex]._type0._sound = sound;
						vs->_entries[entryIndex]._type0._channelNum = channelNum;
						vs->_entries[entryIndex]._type0._priority = vse._priority2;
						vs->_entries[entryIndex]._type0._fieldA = 0;
						vse._sound2 = NULL;
						break;
					}
				}
			}

			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntryType0 &vse = vs->_entries[idx]._type0;
				Sound *sound = vse._sound2;
				if (!sound)
					continue;

				int voiceNum = 0, foundIndex = -1;
				for (uint entryIndex = 0; entryIndex < vs->_entries.size(); ++entryIndex) {
					if ((vs->_entries[entryIndex]._field1) && !vs->_entries[entryIndex]._type0._sound) {
						int tempVoice = vs->_entries[entryIndex]._voiceNum;

						if (voiceNum <= tempVoice) {
							voiceNum = tempVoice;
							foundIndex = entryIndex;
						}
					}
				}
				assert(foundIndex != -1);

				VoiceStructEntryType0 &vseFound = vs->_entries[foundIndex]._type0;

				vseFound._sound = vse._sound2;
				vseFound._channelNum = vse._channelNum2;
				vseFound._priority = vse._priority2;
				vseFound._fieldA = 0;

				SoundDriver *driver = vs->_entries[foundIndex]._driver;
				assert(driver);

				driver->proc24(vseFound._channelNum, voiceIndex, vseFound._sound, 123, 0);
				driver->proc24(vseFound._channelNum, voiceIndex, vseFound._sound,
					1, vseFound._sound->_chModulation[vseFound._channelNum]);
				driver->proc24(vseFound._channelNum, voiceIndex, vseFound._sound,
					7, vseFound._sound->_chVolume[vseFound._channelNum] * vseFound._sound->_volume / 127);
				driver->proc24(vseFound._channelNum, voiceIndex, vseFound._sound,
					10, vseFound._sound->_chPan[vseFound._channelNum]);
				driver->setProgram(vseFound._channelNum, vseFound._sound->_chProgram[vseFound._channelNum]);
				driver->setPitchBlend(vseFound._channelNum, vseFound._sound->_chPitchBlend[vseFound._channelNum]);
			}

			// Final loop
			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntryType0 &vse = vs->_entries[idx]._type0;

				if (!vse._sound && (vse._sound3)) {
					SoundDriver *driver = vs->_entries[idx]._driver;
					assert(driver);
					driver->proc24(vs->_entries[idx]._voiceNum, voiceIndex, vse._sound3, 123, 0);
				}
			}

		} else {
			// Type 1
			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntry &vse = vs->_entries[idx];
				vse._type1._sound = NULL;
				vse._type1._channelNum = 0;
				vse._type1._priority = 0;
			}

			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntryType1 &vse = vs->_entries[idx]._type1;
				Sound *sound = vse._sound2;
				int channelNum = vse._channelNum2;

				if (!sound)
					continue;

				for (uint entryIndex = 0; entryIndex < vs->_entries.size(); ++entryIndex) {
					VoiceStructEntryType1 &vse2 = vs->_entries[entryIndex]._type1;
					if (!vse2._sound && (vse2._sound3 == sound) && (vse2._channelNum3 == channelNum)) {
						vse2._sound = sound;
						vse2._channelNum = channelNum;
						vse2._priority = vse._priority2;
						vse._sound2 = NULL;
						break;
					}
				}
			}

			uint idx2 = 0;
			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntryType1 &vse = vs->_entries[idx]._type1;
				Sound *sound = vse._sound2;
				if (!sound)
					continue;

				while (vs->_entries[idx2]._type1._sound)
					++idx2;

				VoiceStructEntryType1 &vse2 = vs->_entries[idx2]._type1;
				vse2._sound = vse._sound2;
				vse2._channelNum = vse._channelNum2;
				vse2._priority = vse._priority2;
				vse2._field4 = -1;
				vse2._field5 = 0;
				vse2._field6 = 0;

				SoundDriver *driver = vs->_entries[idx2]._driver;
				assert(driver);

				driver->updateVoice(vs->_entries[idx2]._voiceNum);
				driver->proc38(vs->_entries[idx2]._voiceNum, 1, vse2._sound->_chModulation[vse2._channelNum]);
				driver->proc38(vs->_entries[idx2]._voiceNum, 7,
					vse2._sound->_chVolume[vse2._channelNum] * vse2._sound->_volume / 127);
				driver->proc38(vs->_entries[idx2]._voiceNum, 10, vse2._sound->_chPan[vse2._channelNum]);
				driver->setPitch(vs->_entries[idx2]._voiceNum, vse2._sound->_chPitchBlend[vse2._channelNum]);
			}

			for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
				VoiceStructEntryType1 &vse = vs->_entries[idx]._type1;

				if (!vse._sound && (vse._sound3)) {
					vse._field4 = -1;
					vse._field5 = 0;
					vse._field6 = 0;

					SoundDriver *driver = vs->_entries[idx]._driver;
					assert(driver);
					driver->updateVoice(vs->_entries[idx]._voiceNum);
				}
			}
		}
	}
}

void SoundManager::sfUpdateVolume(Sound *sound) {
	sfDereferenceAll();
	sfDoUpdateVolume(sound);
}

void SoundManager::sfDereferenceAll() {
	// Orignal used handles for both the driver list and voiceTypeStructPtrs list. This method then refreshed
	// pointer lists based on the handles. Since in ScummVM we're just using pointers directly, this
	// method doesn't need any implementation
}

void SoundManager::sfUpdatePriority(Sound *sound) {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);

	int tempPriority = (sound->_fixedPriority == 255) ? sound->_sndResPriority : sound->_priority;
	if (sound->_priority != tempPriority) {
		sound->_priority = tempPriority;
		if (sfDoRemoveFromPlayList(sound)) {
			sfDoAddToPlayList(sound);
			sfRethinkVoiceTypes();
		}
	}
}

void SoundManager::sfUpdateLoop(Sound *sound) {
	if (sound->_fixedLoop)
		sound->_loop = sound->_sndResLoop;
	else
		sound->_loop = sound->_fixedLoop;
}

void SoundManager::sfSetMasterVol(int volume) {
	if (volume > 127)
		volume = 127;

	if (volume != _soundManager->_masterVol) {
		_soundManager->_masterVol = volume;

		for (Common::List<SoundDriver *>::iterator i = _soundManager->_installedDrivers.begin();
				i != _soundManager->_installedDrivers.end(); ++i) {
			(*i)->setMasterVolume(volume);
		}
	}
}

void SoundManager::sfExtractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum) {
	trackInfo->_numTracks = 0;

	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		if ((v == 0x80000000) || (v == (uint)groupNum)) {
			// Found group to process
			int count = READ_LE_UINT16(p + 4);
			p += 6;

			for (int idx = 0; idx < count; ++idx) {
				if (trackInfo->_numTracks == 16) {
					trackInfo->_numTracks = -1;
					return;
				}

				trackInfo->_chunks[trackInfo->_numTracks] = READ_LE_UINT16(p);
				trackInfo->_voiceTypes[trackInfo->_numTracks] = READ_LE_UINT16(p + 2);
				++trackInfo->_numTracks;
				p += 4;
			}
		} else {
			// Not correct group, so move to next one
			p += 6 + (READ_LE_UINT16(p + 4) * 4);
		}
	}
}

void SoundManager::sfTerminate() {

}

void SoundManager::sfExtractGroupMask() {
	uint32 mask = 0;

	for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin();
				i != sfManager()._installedDrivers.end(); ++i)
		mask |= (*i)->_groupMask;

	_soundManager->_groupsAvail = mask;
}

bool SoundManager::sfInstallDriver(SoundDriver *driver) {
	if (!driver->open())
		return false;

	sfManager()._installedDrivers.push_back(driver);
	driver->_groupOffset = driver->getGroupData();
	driver->_groupMask = driver->_groupOffset->_groupMask;

	sfExtractGroupMask();
	sfRethinkSoundDrivers();
	driver->setMasterVolume(sfManager()._masterVol);

	return true;
}

void SoundManager::sfUnInstallDriver(SoundDriver *driver) {
	sfManager()._installedDrivers.remove(driver);
	delete driver;

	sfExtractGroupMask();
	sfRethinkSoundDrivers();
}

void SoundManager::sfInstallPatchBank(SoundDriver *driver, const byte *bankData) {
	driver->installPatch(bankData, g_vm->_memoryManager.getSize(bankData));
}

/**
 * Adds the specified sound in the playing sound list, inserting in order of priority
 */
void SoundManager::sfDoAddToPlayList(Sound *sound) {
	Common::StackLock slock2(sfManager()._serverSuspendedMutex);

	Common::List<Sound *>::iterator i = sfManager()._playList.begin();
	while ((i != sfManager()._playList.end()) && (sound->_priority > (*i)->_priority))
		++i;

	sfManager()._playList.insert(i, sound);
}

/**
 * Removes the specified sound from the play list
 */
bool SoundManager::sfDoRemoveFromPlayList(Sound *sound) {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);

	bool result = false;
	for (Common::List<Sound *>::iterator i = sfManager()._playList.begin(); i != sfManager()._playList.end(); ++i) {
		if (*i == sound) {
			result = true;
			sfManager()._playList.erase(i);
			break;
		}
	}

	return result;
}

void SoundManager::sfDoUpdateVolume(Sound *sound) {
	Common::StackLock slock(sfManager()._serverSuspendedMutex);

	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceTypeStruct *vs = sfManager()._voiceTypeStructPtrs[voiceIndex];
		if (!vs)
			continue;

		for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
			VoiceStructEntry &vse = vs->_entries[idx];
			SoundDriver *driver = vse._driver;

			if (vs->_voiceType == VOICETYPE_0) {
				if (vse._type0._sound) {
					int vol = sound->_volume * sound->_chVolume[vse._type0._channelNum] / 127;
					driver->proc24(voiceIndex, vse._voiceNum, sound, 7, vol);
				}
			} else {
				if (vse._type1._sound) {
					int vol = sound->_volume * sound->_chVolume[vse._type1._channelNum] / 127;
					driver->proc38(vse._voiceNum, 7, vol);
				}
			}
		}
	}
}

/*--------------------------------------------------------------------------*/

Sound::Sound() {
	_stoppedAsynchronously = false;
	_soundResID = 0;
	_group = 0;
	_sndResPriority = 0;
	_fixedPriority = -1;
	_sndResLoop = 1;
	_fixedLoop = -1;
	_priority = 0;
	_volume = 127;
	_loop = 0;
	_pausedCount = 0;
	_mutedCount = 0;
	_hold = 0xff;
	_cueValue = -1;
	_fadeDest = -1;
	_fadeSteps = 0;
	_fadeTicks = 0;
	_fadeCounter = 0;
	_stopAfterFadeFlag = false;
	_timer = 0;
	_newTimeIndex = 0;
	_loopTimer = 0;
	_trackInfo._numTracks = 0;
	_primed = false;
	_isEmpty = false;
	_remoteReceiver = NULL;


	memset(_chProgram, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chModulation, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chVolume, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chPan, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chDamper, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chPitchBlend, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chVoiceType, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chNumVoices, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chSubPriority, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_chFlags, 0, SOUND_ARR_SIZE * sizeof(int));
	Common::fill(_chWork, _chWork + SOUND_ARR_SIZE, false);
	memset(_channelData, 0, SOUND_ARR_SIZE * sizeof(byte *));
	memset(_trkChannel, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_trkState, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_trkLoopState, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_trkIndex, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_trkLoopIndex, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_trkRest, 0, SOUND_ARR_SIZE * sizeof(int));
	memset(_trkLoopRest, 0, SOUND_ARR_SIZE * sizeof(int));
}

Sound::~Sound() {
	stop();
}

void Sound::synchronize(Serializer &s) {
	if (s.getVersion() < 6)
		return;

	assert(!_remoteReceiver);

	s.syncAsSint16LE(_soundResID);
	s.syncAsByte(_primed);
	s.syncAsByte(_stoppedAsynchronously);
	s.syncAsSint16LE(_group);
	s.syncAsSint16LE(_sndResPriority);
	s.syncAsSint16LE(_fixedPriority);
	s.syncAsSint16LE(_sndResLoop);
	s.syncAsSint16LE(_fixedLoop);
	s.syncAsSint16LE(_priority);
	s.syncAsSint16LE(_volume);
	s.syncAsSint16LE(_loop);
	s.syncAsSint16LE(_pausedCount);
	s.syncAsSint16LE(_mutedCount);
	s.syncAsSint16LE(_hold);
	s.syncAsSint16LE(_cueValue);
	s.syncAsSint16LE(_fadeDest);
	s.syncAsSint16LE(_fadeSteps);
	s.syncAsUint32LE(_fadeTicks);
	s.syncAsUint32LE(_fadeCounter);
	s.syncAsByte(_stopAfterFadeFlag);
	s.syncAsUint32LE(_timer);
	s.syncAsSint16LE(_loopTimer);
}

void Sound::play(int soundNum) {
	prime(soundNum);
	_soundManager->addToPlayList(this);
}

void Sound::stop() {
	g_globals->_soundManager.removeFromPlayList(this);
	_unPrime();
}

void Sound::prime(int soundResID) {
	if (_soundResID != -1) {
		stop();
		_prime(soundResID, false);
	}
}

void Sound::unPrime() {
	stop();
}

void Sound::_prime(int soundResID, bool dontQueue) {
	if (_primed)
		unPrime();

	_soundResID = soundResID;
	if (_soundResID != -1) {
		// Sound number specified
		_isEmpty = false;
		_remoteReceiver = NULL;
		byte *soundData = g_resourceManager->getResource(RES_SOUND, soundResID, 0);
		_soundManager->checkResVersion(soundData);
		_group = _soundManager->determineGroup(soundData);
		_sndResPriority = _soundManager->extractPriority(soundData);
		_sndResLoop = _soundManager->extractLoop(soundData);
		_soundManager->extractTrackInfo(&_trackInfo, soundData, _group);

		for (int idx = 0; idx < _trackInfo._numTracks; ++idx) {
			_channelData[idx] = g_resourceManager->getResource(RES_SOUND, soundResID, _trackInfo._chunks[idx]);
		}

		DEALLOCATE(soundData);
	} else {
		// No sound specified
		_isEmpty = true;
		_group = 0;
		_sndResPriority = 0;
		_sndResLoop = 0;
		_trackInfo._numTracks = 0;
		_channelData[0] = ALLOCATE(200);
		_remoteReceiver = ALLOCATE(200);
	}

	soPrimeSound(dontQueue);
	if (!dontQueue)
		_soundManager->addToSoundList(this);

	_primed = true;
}

void Sound::_unPrime() {
	if (_primed) {
		if (_isEmpty) {
			DEALLOCATE(_channelData[0]);
			DEALLOCATE(_remoteReceiver);
			_remoteReceiver = NULL;
		} else {
			for (int idx = 0; idx < _trackInfo._numTracks; ++idx) {
				DEALLOCATE(_channelData[idx]);
			}
		}

		_trackInfo._numTracks = 0;
		if (_soundManager)
			_soundManager->removeFromSoundList(this);

		_primed = false;
		_stoppedAsynchronously = false;
	}
}

void Sound::orientAfterDriverChange() {
	if (!_isEmpty) {
		int timeIndex = getTimeIndex();

		for (int idx = 0; idx < _trackInfo._numTracks; ++idx)
			DEALLOCATE(_channelData[idx]);

		_trackInfo._numTracks = 0;
		_primed = false;
		_prime(_soundResID, true);

		setTimeIndex(timeIndex);
	}
}

void Sound::orientAfterRestore() {
	if (!_isEmpty) {
		int timeIndex = getTimeIndex();
		_primed = false;
		_prime(_soundResID, true);
		setTimeIndex(timeIndex);
	}
}

void Sound::go() {
	if (!_primed)
		error("Attempt to execute Sound::go() on an unprimed Sound");

	_soundManager->addToPlayList(this);
}

void Sound::halt(void) {
	_soundManager->removeFromPlayList(this);
}

int Sound::getSoundNum() const {
	return _soundResID;
}

bool Sound::isPlaying() {
	return _soundManager->isOnPlayList(this);
}

bool Sound::isPrimed() const {
	return _primed;
}

bool Sound::isPaused() const {
	return _pausedCount != 0;
}

bool Sound::isMuted() const {
	return _mutedCount != 0;
}

void Sound::pause(bool flag) {
	Common::StackLock slock(g_globals->_soundManager._serverSuspendedMutex);

	if (flag)
		++_pausedCount;
	else if (_pausedCount > 0)
		--_pausedCount;

	_soundManager->rethinkVoiceTypes();
}

void Sound::mute(bool flag) {
	Common::StackLock slock(g_globals->_soundManager._serverSuspendedMutex);

	if (flag)
		++_mutedCount;
	else if (_mutedCount > 0)
		--_mutedCount;

	_soundManager->rethinkVoiceTypes();
}

void Sound::fade(int fadeDest, int fadeSteps, int fadeTicks, bool stopAfterFadeFlag) {
	Common::StackLock slock(g_globals->_soundManager._serverSuspendedMutex);

	if (fadeDest > 127)
		fadeDest = 127;
	if (fadeTicks > 127)
		fadeTicks = 127;
	if (fadeSteps > 255)
		fadeSteps = 255;

	_fadeDest = fadeDest;
	_fadeTicks = fadeTicks;
	_fadeSteps = fadeSteps;
	_fadeCounter = 0;
	_stopAfterFadeFlag = stopAfterFadeFlag;
}

void Sound::setTimeIndex(uint32 timeIndex) {
	if (_primed)
		_newTimeIndex = timeIndex;
}

uint32 Sound::getTimeIndex() const {
	return _timer;
}

int Sound::getCueValue() const {
	return _cueValue;
}

void Sound::setCueValue(int cueValue) {
	_cueValue = cueValue;
}

void Sound::setVol(int volume) {
	if (volume > 127)
		volume = 127;

	if (_volume != volume) {
		_volume = volume;
		if (isPlaying())
			_soundManager->updateSoundVol(this);
	}
}

int Sound::getVol() const {
	return _volume;
}

void Sound::setPri(int priority) {
	if (priority > 127)
		priority = 127;
	_fixedPriority = priority;
	_soundManager->updateSoundPri(this);
}

void Sound::setLoop(int flag) {
	_fixedLoop = flag;
	_soundManager->updateSoundLoop(this);
}

int Sound::getPri() const {
	return _priority;
}

int Sound::getLoop() {
	return _loop;
}

void Sound::holdAt(int amount) {
	if (amount > 127)
		amount = 127;
	_hold = amount;
}

void Sound::release() {
	_hold = -1;
}

void Sound::soPrimeSound(bool dontQueue) {
	if (!dontQueue) {
		_priority = (_fixedPriority != -1) ? _fixedPriority : _sndResPriority;
		_loop = !_fixedLoop ? _fixedLoop : _sndResLoop;
		_pausedCount = 0;
		_mutedCount = 0;
		_hold = -1;
		_cueValue = -1;
		_fadeDest = -1;
		_fadeSteps = 0;
		_fadeTicks = 0;
		_fadeCounter = 0;
		_stopAfterFadeFlag = false;
	}

	_timer = 0;
	_newTimeIndex = 0;
	_loopTimer = 0;
	soPrimeChannelData();
}

void Sound::soSetTimeIndex(uint timeIndex) {
	Common::StackLock slock(g_globals->_soundManager._serverSuspendedMutex);

	if (timeIndex != _timer) {
		_soundManager->_soTimeIndexFlag = true;
		_timer = 0;
		_loopTimer = 0;
		soPrimeChannelData();

		while (timeIndex > 0) {
			if (soServiceTracks()) {
				SoundManager::sfDoRemoveFromPlayList(this);
				_stoppedAsynchronously = true;
				_soundManager->_needToRethink = true;
				break;
			}

			--timeIndex;
		}

		_soundManager->_soTimeIndexFlag = false;
	}
}

bool Sound::soServiceTracks() {
	if (_isEmpty) {
		soRemoteReceive();
		return false;
	}

	bool flag = true;
	for (int trackCtr = 0; trackCtr < _trackInfo._numTracks; ++trackCtr) {
		int mode = *_channelData[trackCtr];

		if (mode == 0) {
			soServiceTrackType0(trackCtr, _channelData[trackCtr]);
		} else if (mode == 1) {
			soServiceTrackType1(trackCtr, _channelData[trackCtr]);
		} else {
			error("Unknown sound mode encountered");
		}

		if (_trkState[trackCtr])
			flag = false;
	}

	++_timer;
	if (!flag)
		return false;
	else if ((_loop > 0) && (--_loop == 0))
		return true;
	else {
		for (int trackCtr = 0; trackCtr < _trackInfo._numTracks; ++trackCtr) {
			_trkState[trackCtr] = _trkLoopState[trackCtr];
			_trkRest[trackCtr] = _trkLoopRest[trackCtr];
			_trkIndex[trackCtr] = _trkLoopIndex[trackCtr];
		}

		_timer = _loopTimer;
		return false;
	}
}

void Sound::soPrimeChannelData() {
	if (_isEmpty) {
		for (int idx = 0; idx < 16; ++idx) {
			_chProgram[idx] = 0;
			_chModulation[idx] = 0;
			_chVolume[idx] = 127;
			_chPan[idx] = 64;
			_chDamper[idx] = 0;
			_chVoiceType[idx] = VOICETYPE_0;
			_chNumVoices[idx] = 0;
			_chSubPriority[idx] = 0;
			_chPitchBlend[idx] = 0x2000;
			_chFlags[idx] = 1;
		}

		_trkChannel[0] = 0;
		_trkState[0] = 1;
		_trkLoopState[0] = 1;
		_trkIndex[0] = 0;
		_trkLoopIndex[0] = 0;
	} else {
		for (int idx = 0; idx < SOUND_ARR_SIZE; ++idx)
			_chFlags[idx] = 0x8000;

		for (int idx = 0; idx < _trackInfo._numTracks; ++idx) {
			byte *d = _channelData[idx];
			int mode = *d;
			int channelNum = (int8)*(d + 1);

			_trkChannel[idx] = channelNum;
			assert((channelNum >= -1) && (channelNum < 16));

			if (channelNum >= 0) {
				_chProgram[channelNum] = *(d + 10);
				_chModulation[channelNum] = 0;
				_chVolume[channelNum] = *(d + 11);
				_chPan[channelNum] = *(d + 12);
				_chDamper[channelNum] = 0;
				_chVoiceType[channelNum] = _trackInfo._voiceTypes[idx];
				_chNumVoices[channelNum] = *(d + 6);
				_chSubPriority[channelNum] = *(d + 7);
				_chPitchBlend[channelNum] = 0x2000;
				_chFlags[channelNum] = READ_LE_UINT16(d + 8);
			}

			if (mode == 0) {
				_trkState[idx] = 1;
				_trkLoopState[idx] = 1;
				_trkIndex[idx] = 14;
				_trkLoopIndex[idx] = 14;
				_trkRest[idx] = 0;
				_trkLoopRest[idx] = 0;
			} else if (mode == 1) {
				_trkState[idx] = 1;
				_trkLoopState[idx] = 1;
				_trkIndex[idx] = 0;
				_trkLoopIndex[idx] = 0;
				_trkRest[idx] = 0;
				_trkLoopRest[idx] = 0;
			} else {
				error("Unknown sound mode encountered");
			}
		}
	}
}

void Sound::soRemoteReceive() {
	error("soRemoteReceive not implemented");
}

void Sound::soServiceTrackType0(int trackIndex, const byte *channelData) {
	if (_trkRest[trackIndex]) {
		--_trkRest[trackIndex];
		return;
	}
	if (!_trkState[trackIndex])
		return;

	int channelNum = _trkChannel[trackIndex];
	assert((channelNum >= -1) && (channelNum < SOUND_ARR_SIZE));
	int chFlags = (channelNum == -1) ? 0 : _chFlags[channelNum];
	int voiceNum = -1;
	SoundDriver *driver = NULL;

	VoiceTypeStruct *vtStruct;
	VoiceType voiceType = VOICETYPE_0, chVoiceType = VOICETYPE_0;

	if ((channelNum == -1) || _soundManager->_soTimeIndexFlag) {
		vtStruct = NULL;
		voiceType = VOICETYPE_0;
	} else {
		chVoiceType = (VoiceType)_chVoiceType[channelNum];
		vtStruct = _soundManager->_voiceTypeStructPtrs[(int)chVoiceType];

		if (vtStruct) {
			voiceType = vtStruct->_voiceType;
			if (voiceType == VOICETYPE_0) {
				for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
					if (!vtStruct->_entries[idx]._type0._sound &&
							(vtStruct->_entries[idx]._type0._channelNum != channelNum)) {
						voiceNum = vtStruct->_entries[idx]._voiceNum;
						driver = vtStruct->_entries[idx]._driver;
						break;
					}
				}
			}
		}
	}

	const byte *pData = channelData + _trkIndex[trackIndex];

	for (;;) {
		byte v = *pData++;
		if (!(v & 0x80)) {
			// Area #1
			if (!_soundManager->_soTimeIndexFlag) {
				// Only do processing if fast forwarding to a given time index
				if (channelNum != -1) {
					if (voiceType == VOICETYPE_1) {
						soUpdateDamper(vtStruct, channelNum, chVoiceType, v);
					} else if (voiceNum != -1) {
						assert(driver);
						driver->proc18(voiceNum, chVoiceType);
					}
				}
			}
		} else if (!(v & 0x40)) {
			// Area #2
			if (!_soundManager->_soTimeIndexFlag) {
				// Only do processing if fast forwarding to a given time index
				byte b = *pData++;
				v <<= 1;
				if (b & 0x80)
					v |= 1;

				b &= 0x7f;

				if (channelNum != -1) {
					if (voiceType != VOICETYPE_0) {
						if (chFlags & 0x10)
							soPlaySound2(vtStruct, channelData, channelNum, chVoiceType, v);
						else
							soPlaySound(vtStruct, channelData, channelNum, chVoiceType, v, b);
					} else if (voiceNum != -1) {
						assert(driver);
						driver->proc20(voiceNum, chVoiceType);
					}
				}
			} else {
				++pData;
			}
		} else if (!(v & 0x20)) {
			// Area #3
			v &= 0x1f;

			// Gather up an extended number
			int trkRest = v;
			while ((*pData & 0xE0) == 0xC0) {
				byte b = *pData++;
				trkRest = (trkRest << 5) | (b & 0x1f);
			}

			_trkRest[trackIndex] = trkRest - 1;
			_trkIndex[trackIndex] = pData - channelData;
			return;
		} else if (!(v & 0x10)) {
			// Area #4
			v = (v & 0xf) << 1;

			byte b = *pData++;
			if (b & 0x80)
				v |= 1;
			b &= 0x7f;

			assert(v < 4);
			int cmdList[32] = { 1, 7, 10, 64 };
			int cmdVal = cmdList[v];

			if (channelNum == -1) {
				if (soDoUpdateTracks(cmdVal, b))
					return;
			} else {
				soDoTrackCommand(_trkChannel[trackIndex], cmdVal, b);

				if (!_soundManager->_soTimeIndexFlag) {
					if (cmdVal == 7)
						b = static_cast<byte>(_volume * (int)b / 127);

					if (voiceType != VOICETYPE_0) {
						soProc38(vtStruct, channelNum, chVoiceType, cmdVal, b);
					} else if (voiceNum != -1) {
						assert(driver);
						driver->proc24(voiceNum, chVoiceType, this, cmdVal, b);
					}
				}
			}
		} else if (!(v & 0x8)) {
			// Area #5
			if (!_soundManager->_soTimeIndexFlag) {
				// Only do processing if fast forwarding to a given time index
				int cx = READ_LE_UINT16(pData);
				pData += 2;

				if (channelNum != -1) {
					assert(driver);
					driver->proc22(voiceNum, chVoiceType, cx);
				}
			} else {
				pData += 2;
			}
		} else if (!(v & 0x4)) {
			// Area #6
			int cmd = *pData++;
			int value = *pData++;

			if (channelNum != -1) {
				soDoTrackCommand(_trkChannel[trackIndex], cmd, value);

				if (!_soundManager->_soTimeIndexFlag) {
					if (voiceType != VOICETYPE_0) {
						soProc38(vtStruct, channelNum, chVoiceType, cmd, value);
					} else if (voiceNum != -1) {
						assert(driver);
						driver->proc24(voiceNum, chVoiceType, this, cmd, value);
					}
				}
			} else if (soDoUpdateTracks(cmd, value)) {
				return;
			}
		} else if (!(v & 0x2)) {
			// Area #7
			if (!_soundManager->_soTimeIndexFlag) {
				int pitchBlend = READ_BE_UINT16(pData);
				pData += 2;

				if (channelNum != -1) {
					int channel = _trkChannel[trackIndex];
					_chPitchBlend[channel] = pitchBlend;

					if (voiceType != VOICETYPE_0) {
						soProc40(vtStruct, channelNum, pitchBlend);
					} else if (voiceNum != -1) {
						assert(driver);
						driver->setPitchBlend(channel, pitchBlend);
					}
				}
			} else {
				pData += 2;
			}
		} else if (!(v & 0x1)) {
			// Area #8
			int program = *pData++;

			if (channelNum != -1) {
				int channel = _trkChannel[trackIndex];
				_chProgram[channel] = program;

				if (!_soundManager->_soTimeIndexFlag) {
					if ((voiceType == VOICETYPE_0) && (voiceNum != -1)) {
						assert(driver);
						driver->setProgram(voiceNum, program);
					}
				}
			} else {
				soSetTrackPos(trackIndex, pData - channelData, program);
			}

		} else {
			// Area #9
			byte b = *pData++;

			if (b & 0x80) {
				_trkState[trackIndex] = 0;
				_trkIndex[trackIndex] = pData - channelData;
				return;
			}

			if (!_soundManager->_soTimeIndexFlag) {
				if ((channelNum != -1) && (voiceType == VOICETYPE_0) && (voiceNum != -1)) {
					assert(driver);
					driver->setVolume1(voiceNum, chVoiceType, 0, b);
				}

			}
		}
	}
}

void Sound::soUpdateDamper(VoiceTypeStruct *voiceType, int channelNum, VoiceType mode, int v0) {
	bool hasDamper = _chDamper[channelNum] != 0;

	for (uint idx = 0; idx < voiceType->_entries.size(); ++idx) {
		VoiceStructEntryType1 &vte = voiceType->_entries[idx]._type1;

		if ((vte._field4 == v0) && (vte._channelNum == channelNum) && (vte._sound == this)) {
			if (hasDamper)
				vte._field5 = 1;
			else {
				SoundDriver *driver = voiceType->_entries[idx]._driver;
				assert(driver);

				vte._field4 = -1;
				vte._field5 = 0;
				driver->updateVoice(voiceType->_entries[idx]._voiceNum);
			}
			return;
		}
	}
}

void Sound::soPlaySound(VoiceTypeStruct *vtStruct, const byte *channelData, int channelNum, VoiceType voiceType, int v0, int v1) {
	int entryIndex = soFindSound(vtStruct, channelNum);
	if (entryIndex != -1) {
		SoundDriver *driver = vtStruct->_entries[entryIndex]._driver;
		assert(driver);

		vtStruct->_entries[entryIndex]._type1._field6 = 0;
		vtStruct->_entries[entryIndex]._type1._field4 = v0;
		vtStruct->_entries[entryIndex]._type1._field5 = 0;

		driver->playSound(channelData, 0, _chProgram[channelNum], vtStruct->_entries[entryIndex]._voiceNum, v0, v1);
	}
}

void Sound::soPlaySound2(VoiceTypeStruct *vtStruct, const byte *channelData, int channelNum, VoiceType voiceType, int v0) {
	for (int trackCtr = 0; trackCtr < _trackInfo._numTracks; ++trackCtr) {
		const byte *instrument = _channelData[trackCtr];
		if ((*(instrument + 13) == v0) && (*instrument == 1)) {
			int entryIndex = soFindSound(vtStruct, channelNum);

			if (entryIndex != -1) {
				SoundDriver *driver = vtStruct->_entries[entryIndex]._driver;
				assert(driver);
				byte *trackData = _channelData[trackCtr];

				vtStruct->_entries[entryIndex]._type1._field6 = 0;
				vtStruct->_entries[entryIndex]._type1._field4 = v0;
				vtStruct->_entries[entryIndex]._type1._field5 = 0;

				int v1, v2;
				driver->playSound(trackData, 14, -1, vtStruct->_entries[entryIndex]._voiceNum, v0, 0x7F);
				driver->proc42(vtStruct->_entries[entryIndex]._voiceNum, voiceType, 0, &v1, &v2);
			}
			break;
		}
	}
}

void Sound::soProc38(VoiceTypeStruct *vtStruct, int channelNum, VoiceType voiceType, int cmd, int value) {
	if (cmd == 64) {
		if (value == 0) {
			for (uint entryIndex = 0; entryIndex < vtStruct->_entries.size(); ++entryIndex) {
				VoiceStructEntryType1 &vte = vtStruct->_entries[entryIndex]._type1;

				if ((vte._sound == this) && (vte._channelNum == channelNum) && (vte._field5 != 0)) {
					SoundDriver *driver = vtStruct->_entries[entryIndex]._driver;
					assert(driver);

					vte._field4 = -1;
					vte._field5 = 0;
					driver->updateVoice(vtStruct->_entries[entryIndex]._voiceNum);
				}
			}
		}
	} else if (cmd == 75) {
		_soundManager->_needToRethink = true;
	} else {
		for (uint entryIndex = 0; entryIndex < vtStruct->_entries.size(); ++entryIndex) {
			VoiceStructEntry &vte = vtStruct->_entries[entryIndex];

			if ((vte._type1._sound == this) && (vte._type1._channelNum == channelNum)) {
				SoundDriver *driver = vte._driver;
				assert(driver);

				driver->proc38(vte._voiceNum, cmd, value);
			}
		}
	}
}

void Sound::soProc40(VoiceTypeStruct *vtStruct, int channelNum, int pitchBlend) {
	for (uint entryIndex = 0; entryIndex < vtStruct->_entries.size(); ++entryIndex) {
		VoiceStructEntryType1 &vte = vtStruct->_entries[entryIndex]._type1;

		if ((vte._sound == this) && (vte._channelNum == channelNum)) {
			SoundDriver *driver = vtStruct->_entries[entryIndex]._driver;
			assert(driver);

			driver->setPitch(vtStruct->_entries[entryIndex]._voiceNum, pitchBlend);
		}
	}
}

void Sound::soDoTrackCommand(int channelNum, int command, int value) {
	switch (command) {
	case 1:
		_chModulation[channelNum] = value;
		break;
	case 7:
		_chVolume[channelNum] = value;
		break;
	case 10:
		_chPan[channelNum] = value;
		break;
	case 64:
		_chDamper[channelNum] = value;
		break;
	case 75:
		_chNumVoices[channelNum] = value;
		break;
	}
}

bool Sound::soDoUpdateTracks(int command, int value) {
	if ((command == 76) || (_hold != value))
		return false;

	for (int trackIndex = 0; trackIndex < _trackInfo._numTracks; ++trackIndex) {
		_trkState[trackIndex] = _trkLoopState[trackIndex];
		_trkRest[trackIndex] = _trkLoopRest[trackIndex];
		_trkIndex[trackIndex] = _trkLoopIndex[trackIndex];
	}

	_timer = _loopTimer;
	return true;
}

void Sound::soSetTrackPos(int trackIndex, int trackPos, int cueValue) {
	_trkIndex[trackIndex] = trackPos;
	if (cueValue == 127) {
		if (!_soundManager->_soTimeIndexFlag)
			_cueValue = cueValue;
	} else {
		for (int idx = 0; idx < _trackInfo._numTracks; ++idx) {
			_trkLoopState[idx] = _trkState[idx];
			_trkLoopRest[idx] = _trkRest[idx];
			_trkLoopIndex[idx] = _trkIndex[idx];
		}

		_loopTimer = _timer;
	}
}

void Sound::soServiceTrackType1(int trackIndex, const byte *channelData) {
	if (_soundManager->_soTimeIndexFlag || !_trkState[trackIndex])
		return;

	int channel = _trkChannel[trackIndex];
	if (channel == -1)
		_trkState[trackIndex] = 0;
	else {
		int voiceType = _chVoiceType[channel];
		VoiceTypeStruct *vtStruct = _soundManager->_voiceTypeStructPtrs[voiceType];

		if (!vtStruct)
			_trkState[trackIndex] = 0;
		else {
			if (vtStruct->_voiceType != VOICETYPE_0) {
				if (_trkState[trackIndex] == 1) {
					int entryIndex = soFindSound(vtStruct, *(channelData + 1));
					if (entryIndex != -1) {
						SoundDriver *driver = vtStruct->_entries[entryIndex]._driver;
						assert(driver);

						vtStruct->_entries[entryIndex]._type1._field6 = 0;
						vtStruct->_entries[entryIndex]._type1._field4 = *(channelData + 1);
						vtStruct->_entries[entryIndex]._type1._field5 = 0;

						int v1, v2;
						driver->playSound(channelData, 14, -1, vtStruct->_entries[entryIndex]._voiceNum, *(channelData + 1), 0x7f);
						driver->proc42(vtStruct->_entries[entryIndex]._voiceNum, *(channelData + 1), _loop ? 1 : 0,
							&v1, &v2);
						_trkState[trackIndex] = 2;
					}
				} else {
					for (uint entryIndex = 0; entryIndex < vtStruct->_entries.size(); ++entryIndex) {
						VoiceStructEntry &vte = vtStruct->_entries[entryIndex];
						VoiceStructEntryType1 &vse = vte._type1;
						if ((vse._sound == this) && (vse._channelNum == channel) && (vse._field4 == *(channelData + 1))) {
							SoundDriver *driver = vte._driver;

							int isEnded, resetTimer;
							driver->proc42(vte._voiceNum, vtStruct->_total, _loop ? 1 : 0, &isEnded, &resetTimer);
							if (isEnded) {
								_trkState[trackIndex] = 0;
							} else if (resetTimer) {
								_timer = 0;
							}
							return;
						}
					}

					_trkState[trackIndex] = 0;
				}
			} else {
				_trkState[trackIndex] = 0;
			}
		}
	}
}

int Sound::soFindSound(VoiceTypeStruct *vtStruct, int channelNum) {
	int entryIndex = -1, entry2Index = -1;
	int v6 = 0, v8 = 0;

	for (uint idx = 0; idx < vtStruct->_entries.size(); ++idx) {
		VoiceStructEntryType1 &vte = vtStruct->_entries[idx]._type1;
		if ((vte._channelNum == channelNum) && (vte._sound == this)) {
			int v = vte._field6;
			if (vte._field4 != -1) {
				if (v8 <= v) {
					v8 = v;
					entry2Index = idx;
				}
			} else {
				if (v6 <= v) {
					v6 = v;
					entryIndex = idx;
				}
			}
		}
	}

	if (entryIndex != -1)
		return entryIndex;
	else if ((entryIndex == -1) && (entry2Index == -1))
		return -1;
	else {
		SoundDriver *driver = vtStruct->_entries[entry2Index]._driver;
		assert(driver);
		driver->updateVoice(vtStruct->_entries[entry2Index]._voiceNum);

		return entry2Index;
	}
}

/*--------------------------------------------------------------------------*/

ASound::ASound(): EventHandler() {
	_action = NULL;
	_cueValue = -1;
	if (g_globals)
		g_globals->_sounds.push_back(this);
}

ASound::~ASound() {
	if (g_globals)
		g_globals->_sounds.remove(this);
}

void ASound::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	SYNC_POINTER(_action);
	s.syncAsByte(_cueValue);

}

void ASound::dispatch() {
	EventHandler::dispatch();

	int cueValue = _sound.getCueValue();
	if (cueValue != -1) {
		_cueValue = cueValue;
		_sound.setCueValue(-1);

		if (_action)
			_action->signal();
	}

	if (_cueValue != -1) {
		if (!_sound.isPrimed()) {
			_cueValue = -1;
			if (_action) {
				_action->signal();
				_action = NULL;
			}
		}
	}
}

void ASound::play(int soundNum, EventHandler *action, int volume) {
	_action = action;
	_cueValue = 0;

	setVol(volume);
	_sound.play(soundNum);
}

void ASound::stop() {
	_sound.stop();
	_action = NULL;
}

void ASound::prime(int soundResID, Action *action) {
	_action = action;
	_cueValue = 0;
	_sound.prime(soundResID);
}

void ASound::unPrime() {
	_sound.unPrime();
	_action = NULL;
}

void ASound::fade(int fadeDest, int fadeSteps, int fadeTicks, bool stopAfterFadeFlag, EventHandler *action) {
	if (action)
		_action = action;

	_sound.fade(fadeDest, fadeSteps, fadeTicks, stopAfterFadeFlag);
}

void ASound::fadeSound(int soundNum) {
	play(soundNum, NULL, 0);
	fade(127, 5, 1, false, NULL);
}

/*--------------------------------------------------------------------------*/

ASoundExt::ASoundExt(): ASound() {
	_soundNum = 0;
}

void ASoundExt::synchronize(Serializer &s) {
	ASound::synchronize(s);
	s.syncAsSint16LE(_soundNum);
}

void ASoundExt::signal() {
	if (_soundNum != 0) {
		fadeSound(_soundNum);
	}
}

void ASoundExt::fadeOut2(EventHandler *action) {
	fade(0, 10, 10, true, action);
}

void ASoundExt::changeSound(int soundNum) {
	if (isPlaying()) {
		_soundNum = soundNum;
		fadeOut2(this);
	} else {
		fadeSound(soundNum);
	}
}

/*--------------------------------------------------------------------------*/

SoundDriver::SoundDriver() {
	_driverResID = 0;
	_minVersion = _maxVersion = 0;
	_groupMask = 0;
}

/*--------------------------------------------------------------------------*/

const byte adlib_group_data[] = { 1, 1, 9, 1, 0xff };

const byte v440B0[9] = { 0, 1, 2, 6, 7, 8, 12, 13, 14 };

const byte v440B9[9] = { 3, 4, 5, 9, 10, 11, 15, 16, 17 };

const byte v440C2[18] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21
};

const byte v44134[64] = {
	0, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
	33, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 54, 55,
	56, 56, 57, 57, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61,
	61, 62, 62, 62, 62, 63, 63, 63
};

const int v440D4[48] = {
	343, 348, 353, 358, 363, 369, 374, 379, 385, 391, 396,
	402, 408, 414, 420, 426, 432, 438, 445, 451, 458, 465,
	471, 478, 485, 492, 499, 507, 514, 521, 529, 537, 544,
	552, 560, 569, 577, 585, 594, 602, 611, 620, 629, 638,
	647, 657, 666, 676
};

AdlibSoundDriver::AdlibSoundDriver(): SoundDriver() {
	_minVersion = 0x102;
	_maxVersion = 0x10A;
	_masterVolume = 0;

	_groupData._groupMask = 9;
	_groupData._v1 = 0x46;
	_groupData._v2 = 0;
	_groupData._pData = &adlib_group_data[0];

	_mixer = g_vm->_mixer;
	_sampleRate = _mixer->getOutputRate();
	_opl = OPL::Config::create();
	assert(_opl);
	_opl->init(_sampleRate);

	_samplesTillCallback = 0;
	_samplesTillCallbackRemainder = 0;
	_samplesPerCallback = getRate() / CALLBACKS_PER_SECOND;
	_samplesPerCallbackRemainder = getRate() % CALLBACKS_PER_SECOND;

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	Common::fill(_channelVoiced, _channelVoiced + ADLIB_CHANNEL_COUNT, false);
	memset(_channelVolume, 0, ADLIB_CHANNEL_COUNT * sizeof(int));
	memset(_v4405E, 0, ADLIB_CHANNEL_COUNT * sizeof(int));
	memset(_v44067, 0, ADLIB_CHANNEL_COUNT * sizeof(int));
	memset(_v44070, 0, ADLIB_CHANNEL_COUNT * sizeof(int));
	memset(_v44079, 0, ADLIB_CHANNEL_COUNT * sizeof(int));
	memset(_v44082, 0, ADLIB_CHANNEL_COUNT * sizeof(int));
	_v44082[ADLIB_CHANNEL_COUNT] = 0x90;
	Common::fill(_pitchBlend, _pitchBlend + ADLIB_CHANNEL_COUNT, 0x2000);
	memset(_v4409E, 0, ADLIB_CHANNEL_COUNT * sizeof(int));
	_patchData = NULL;
}

AdlibSoundDriver::~AdlibSoundDriver() {
	DEALLOCATE(_patchData);
	_mixer->stopHandle(_soundHandle);
	delete _opl;
}

bool AdlibSoundDriver::open() {
	write(1, 0x20);
	if (!reset())
		return false;

	write(8, 0);
	for (int idx = 0x20; idx < 0xF6; ++idx)
		write(idx, 0);

	write(0xBD, 0);
	return true;
}

void AdlibSoundDriver::close() {
	for (int idx = 0xB0; idx < 0xB8; ++idx)
		write(idx, _portContents[idx] & 0xDF);
	for (int idx = 0x40; idx < 0x55; ++idx)
		write(idx, 0x3F);
	reset();
}

bool AdlibSoundDriver::reset() {
	write(1, 0x20);
	write(1, 0x20);

	return true;
}

const GroupData *AdlibSoundDriver::getGroupData() {
	return &_groupData;
}

void AdlibSoundDriver::installPatch(const byte *data, int size) {
	byte *patchData = ALLOCATE(size);
	Common::copy(data, data + size, patchData);
	_patchData = patchData;
}

int AdlibSoundDriver::setMasterVolume(int volume) {
	int oldVolume = _masterVolume;
	_masterVolume = volume;

	for (int channelNum = 0; channelNum < ADLIB_CHANNEL_COUNT; ++channelNum)
		updateChannelVolume(channelNum);

	return oldVolume;
}

void AdlibSoundDriver::playSound(const byte *channelData, int dataOffset, int program, int channel, int v0, int v1) {
	if (program == -1)
		return;

	int offset = READ_LE_UINT16(_patchData + program * 2);
	if (offset) {
		const byte *dataP = _patchData + offset;
		int id;

		for (offset = 2, id = 0; id != READ_LE_UINT16(dataP); offset += 30, ++id) {
			if ((dataP[offset] <= v0) && (dataP[offset + 1] >= v0)) {
				if (dataP[offset + 2] != 0xff)
					v0 = dataP[offset + 2];

				_v4409E[channel] = dataP + offset - _patchData;

				// Set sustain/release
				int portNum = v440C2[v440B0[channel]] + 0x80;
				write(portNum, (_portContents[portNum] & 0xF0) | 0xF);

				portNum = v440C2[v440B9[channel]] + 0x80;
				write(portNum, (_portContents[portNum] & 0xF0) | 0xF);

				if (_channelVoiced[channel])
					clearVoice(channel);

				_v44067[channel] = v0;
				_v4405E[channel] = v1;

				updateChannel(channel);
				setFrequency(channel);
				updateChannelVolume(channel);
				setVoice(channel);
				break;
			}
		}
	}
}

void AdlibSoundDriver::updateVoice(int channel) {
	if (_channelVoiced[channel])
		clearVoice(channel);
}

void AdlibSoundDriver::proc38(int channel, int cmd, int value) {
	if (cmd == 7) {
		// Set channel volume
		_channelVolume[channel] = value;
		updateChannelVolume(channel);
	}
}

void AdlibSoundDriver::setPitch(int channel, int pitchBlend) {
	_pitchBlend[channel] = pitchBlend;
	setFrequency(channel);
}

void AdlibSoundDriver::write(byte reg, byte value) {
	_portContents[reg] = value;
	_queue.push(RegisterValue(reg, value));
}

void AdlibSoundDriver::flush() {
	Common::StackLock slock(SoundManager::sfManager()._serverDisabledMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
}

void AdlibSoundDriver::updateChannelVolume(int channelNum) {
	int volume = (_masterVolume * _channelVolume[channelNum] / 127 * _v4405E[channelNum] / 127) / 2;
	int level2 = 63 - v44134[volume * _v44079[channelNum] / 63];
	int level1 = !_v44082[channelNum] ? 63 - _v44070[channelNum] :
		63 - v44134[volume * _v44070[channelNum] / 63];

	int portNum = v440C2[v440B0[channelNum]] + 0x40;
	write(portNum, (_portContents[portNum] & 0x80) | level1);

	portNum = v440C2[v440B9[channelNum]] + 0x40;
	write(portNum, (_portContents[portNum] & 0x80) | level2);
}

void AdlibSoundDriver::setVoice(int channel) {
	int portNum = 0xB0 + channel;
	write(portNum, _portContents[portNum] | 0x20);
	_channelVoiced[channel] = true;
}

void AdlibSoundDriver::clearVoice(int channel) {
	write(0xB0 + channel, _portContents[0xB0 + channel] & ~0x20);
	_channelVoiced[channel] = false;
}

void AdlibSoundDriver::updateChannel(int channel) {
	const byte *dataP = _patchData + _v4409E[channel];
	int portOffset = v440C2[v440B0[channel]];

	int portNum = portOffset + 0x20;
	int portValue = 0;
	if (*(dataP + 4))
		portValue |= 0x80;
	if (*(dataP + 5))
		portValue |= 0x40;
	if (*(dataP + 8))
		portValue |= 0x20;
	if (*(dataP + 6))
		portValue |= 0x10;
	portValue |= *(dataP + 7);
	write(portNum, portValue);

	portValue = (_portContents[0x40 + portOffset] & 0x3F) | (*(dataP + 9) << 6);
	write(0x40 + portOffset, portValue);

	_v44070[channel] = 63 - *(dataP + 10);
	write(0x60 + portOffset, *(dataP + 12) | (*(dataP + 11) << 4));
	write(0x80 + portOffset, *(dataP + 14) | (*(dataP + 13) << 4));
	write(0xE0 + portOffset, (_portContents[0xE0 + portOffset] & 0xFC) | *(dataP + 15));

	portOffset = v440C2[v440B9[channel]];
	portNum = portOffset + 0x20;
	portValue = 0;
	if (*(dataP + 17))
		portValue |= 0x80;
	if (*(dataP + 18))
		portValue |= 0x40;
	if (*(dataP + 21))
		portValue |= 0x20;
	if (*(dataP + 19))
		portValue |= 0x10;
	portValue |= *(dataP + 20);
	write(portNum, portValue);

	write(0x40 + portOffset, (_portContents[0x40 + portOffset] & 0x3f) | (*(dataP + 22) << 6));
	_v44079[channel] = 0x3F - *(dataP + 23);
	write(0x60 + portOffset, *(dataP + 25) | (*(dataP + 24) << 4));
	write(0x80 + portOffset, *(dataP + 27) | (*(dataP + 26) << 4));
	write(0xE0 + portOffset, (_portContents[0xE0 + portOffset] & 0xFC) | *(dataP + 28));

	write(0xC0 + channel, (_portContents[0xC0 + channel] & 0xF0)
		| (*(dataP + 16) << 1) | *(dataP + 3));

	_v44082[channel] = *(dataP + 3);
}

void AdlibSoundDriver::setFrequency(int channel) {
	int offset, ch;

	int v = _pitchBlend[channel];
	if (v == 0x2000) {
		offset = 0;
		ch = _v44067[channel];
	} else if (v > 0x2000) {
		ch = _v44067[channel];
		v -= 0x2000;
		if (v == 0x1fff)
			v = 0x2000;

		offset = (v / 170) & 3;
		ch += (v / 170) >> 2;

		if (ch >= 128)
			ch = 127;
	} else {
		ch = _v44067[channel];
		int tempVal = (0x2000 - v) / 170;
		int tempVal2 = 4 - (tempVal & 3);

		if (tempVal2 == 4)
			offset = 0;
		else {
			offset = tempVal2;
			--ch;
		}

		ch -= tempVal >> 2;
		if (ch < 0)
			ch = 0;
	}

	int var2 = ch / 12;
	if (var2)
		--var2;

	int dataWord = v440D4[((ch % 12) << 2) + offset];
	write(0xA0 + channel, dataWord & 0xff);
	write(0xB0 + channel, (_portContents[0xB0 + channel] & 0xE0) |
		((dataWord >> 8) & 3) | (var2 << 2));
}

int AdlibSoundDriver::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock1(SoundManager::sfManager()._serverDisabledMutex);
	Common::StackLock slock2(SoundManager::sfManager()._serverSuspendedMutex);

	int32 samplesLeft = numSamples;
	memset(buffer, 0, sizeof(int16) * numSamples);
	while (samplesLeft) {
		if (!_samplesTillCallback) {
			SoundManager::sfUpdateCallback(NULL);
			flush();

			_samplesTillCallback = _samplesPerCallback;
			_samplesTillCallbackRemainder += _samplesPerCallbackRemainder;
			if (_samplesTillCallbackRemainder >= CALLBACKS_PER_SECOND) {
				_samplesTillCallback++;
				_samplesTillCallbackRemainder -= CALLBACKS_PER_SECOND;
			}
		}

		int32 render = MIN<int>(samplesLeft, _samplesTillCallback);
		samplesLeft -= render;
		_samplesTillCallback -= render;

		_opl->readBuffer(buffer, render);
		buffer += render;
	}
	return numSamples;
}

/*--------------------------------------------------------------------------*/


SoundBlasterDriver::SoundBlasterDriver(): SoundDriver() {
	_minVersion = 0x102;
	_maxVersion = 0x10A;
	_masterVolume = 0;

	_groupData._groupMask = 1;
	_groupData._v1 = 0x3E;
	_groupData._v2 = 0;
	static byte const group_data[] = { 3, 1, 1, 0, 0xff };
	_groupData._pData = group_data;

	_mixer = g_vm->_mixer;
	_sampleRate = _mixer->getOutputRate();
	_audioStream = NULL;
	_channelData = NULL;
}

SoundBlasterDriver::~SoundBlasterDriver() {
	_mixer->stopHandle(_soundHandle);
}

bool SoundBlasterDriver::open() {
	return true;
}

void SoundBlasterDriver::close() {
}

bool SoundBlasterDriver::reset() {
	return true;
}

const GroupData *SoundBlasterDriver::getGroupData() {
	return &_groupData;
}

int SoundBlasterDriver::setMasterVolume(int volume) {
	int oldVolume = _masterVolume;
	_masterVolume = volume;

	return oldVolume;
}

void SoundBlasterDriver::playSound(const byte *channelData, int dataOffset, int program, int channel, int v0, int v1) {
	if (program != -1)
		return;

	assert(channel == 0);

	// If sound data has been previously set, then release it
	if (_channelData)
		updateVoice(channel);

	// Set the new channel data
	_channelData = channelData + dataOffset;

	// Make a copy of the buffer
	int dataSize = g_vm->_memoryManager.getSize(channelData);
	byte *soundData = (byte *)malloc(dataSize - dataOffset);
	Common::copy(_channelData, _channelData + (dataSize - dataOffset), soundData);

	_audioStream = Audio::makeQueuingAudioStream(11025, false);
	_audioStream->queueBuffer(soundData, dataSize - dataOffset, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	// Start the new sound
	if (!_mixer->isSoundHandleActive(_soundHandle))
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);
}

void SoundBlasterDriver::updateVoice(int channel) {
	// Stop the playing voice
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);

	_audioStream = NULL;
	_channelData = NULL;
}

void SoundBlasterDriver::proc38(int channel, int cmd, int value) {
	if (cmd == 7) {
		// Set channel volume
		_channelVolume = value;
		_mixer->setChannelVolume(_soundHandle, (byte)MIN(255, value * 2));
	}
}

void SoundBlasterDriver::proc42(int channel, int cmd, int value, int *v1, int *v2) {
	// TODO: v2 is used for flagging a reset of the timer. I'm not sure if it's needed
	*v1 = 0;
	*v2 = 0;

	// Note: Checking whether a playing Fx sound had finished was originally done in another
	// method in the sample playing code. But since we're using the ScummVM audio soundsystem,
	// it's easier simply to do the check right here
	if (_audioStream && (_audioStream->numQueuedStreams() == 0)) {
		updateVoice(channel);
	}

	if (!_channelData)
		// Flag that sound isn't playing
		*v1 = 1;
}

} // End of namespace TsAGE
