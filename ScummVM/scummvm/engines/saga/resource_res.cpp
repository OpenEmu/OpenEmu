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

// RSC Resource file management module (SAGA 1, used in IHNM)

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "engines/advancedDetector.h"

namespace Saga {

#ifdef ENABLE_IHNM

static int metaResourceTable[] = { 0, 326, 517, 677, 805, 968, 1165, 0, 1271 };
static int metaResourceTableDemo[] = { 0, 0, 0, 0, 0, 0, 0, 285, 0 };

void Resource_RES::loadGlobalResources(int chapter, int actorsEntrance) {
	if (chapter < 0)
		chapter = !_vm->isIHNMDemo() ? 8 : 7;

	_vm->_script->_globalVoiceLUT.clear();

	ResourceContext *resourceContext;
	ResourceContext *soundContext;
	uint i;

	resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Resource::loadGlobalResources() resource context not found");
	}

	soundContext = _vm->_resource->getContext(GAME_SOUNDFILE);
	if (soundContext == NULL) {
		error("Resource::loadGlobalResources() sound context not found");
	}

	ByteArray resourceData;

	if (!_vm->isIHNMDemo()) {
		_vm->_resource->loadResource(resourceContext, metaResourceTable[chapter], resourceData);
	} else {
		_vm->_resource->loadResource(resourceContext, metaResourceTableDemo[chapter], resourceData);
	}

	if (resourceData.empty()) {
		error("Resource::loadGlobalResources wrong metaResource");
	}

	{
		ByteArrayReadStreamEndian metaS(resourceData);

		_metaResource.sceneIndex = metaS.readSint16LE();
		_metaResource.objectCount = metaS.readSint16LE();
		_metaResource.objectsStringsResourceID = metaS.readSint32LE();
		_metaResource.inventorySpritesID = metaS.readSint32LE();
		_metaResource.mainSpritesID = metaS.readSint32LE();
		_metaResource.objectsResourceID = metaS.readSint32LE();
		_metaResource.actorCount = metaS.readSint16LE();
		_metaResource.actorsStringsResourceID = metaS.readSint32LE();
		_metaResource.actorsResourceID = metaS.readSint32LE();
		_metaResource.protagFaceSpritesID = metaS.readSint32LE();
		_metaResource.field_22 = metaS.readSint32LE();
		_metaResource.field_26 = metaS.readSint16LE();
		_metaResource.protagStatesCount = metaS.readSint16LE();
		_metaResource.protagStatesResourceID = metaS.readSint32LE();
		_metaResource.cutawayListResourceID = metaS.readSint32LE();
		_metaResource.songTableID = metaS.readSint32LE();
	}

	_vm->_actor->loadActorList(actorsEntrance, _metaResource.actorCount,
						  _metaResource.actorsResourceID, _metaResource.protagStatesCount,
						  _metaResource.protagStatesResourceID);

	_vm->_actor->_protagonist->_sceneNumber = _metaResource.sceneIndex;

	_vm->_actor->_objectsStrings.clear();

	_vm->_resource->loadResource(resourceContext, _metaResource.objectsStringsResourceID, resourceData);
	_vm->loadStrings(_vm->_actor->_objectsStrings, resourceData);

	if (uint(chapter) >= _vm->_sndRes->_fxTableIDs.size()) {
		error("Chapter ID exceeds fxTableIDs length");
	}

	debug(0, "Going to read %d of %d", chapter, _vm->_sndRes->_fxTableIDs[chapter]);
	_vm->_resource->loadResource(soundContext, _vm->_sndRes->_fxTableIDs[chapter],
								 resourceData);

	if (resourceData.empty()) {
		error("Resource::loadGlobalResources Can't load sound effects for current track");
	}

	_vm->_sndRes->_fxTable.resize(resourceData.size() / 4);

	{
		ByteArrayReadStreamEndian fxS(resourceData);

		for (i = 0; i < _vm->_sndRes->_fxTable.size(); i++) {
			_vm->_sndRes->_fxTable[i].res = fxS.readSint16LE();
			_vm->_sndRes->_fxTable[i].vol = fxS.readSint16LE();
		}
	}

	_vm->_interface->_defPortraits.clear();
	_vm->_sprite->loadList(_metaResource.protagFaceSpritesID, _vm->_interface->_defPortraits);

	_vm->_actor->_actorsStrings.clear();

	_vm->_resource->loadResource(resourceContext, _metaResource.actorsStringsResourceID, resourceData);
	_vm->loadStrings(_vm->_actor->_actorsStrings, resourceData);

	_vm->_sprite->_inventorySprites.clear();
	_vm->_sprite->loadList(_metaResource.inventorySpritesID, _vm->_sprite->_inventorySprites);

	_vm->_sprite->_mainSprites.clear();
	_vm->_sprite->loadList(_metaResource.mainSpritesID, _vm->_sprite->_mainSprites);

	_vm->_actor->loadObjList(_metaResource.objectCount, _metaResource.objectsResourceID);

	_vm->_resource->loadResource(resourceContext, _metaResource.cutawayListResourceID, resourceData);

	if (resourceData.empty()) {
		error("Resource::loadGlobalResources Can't load cutaway list");
	}

	_vm->_anim->loadCutawayList(resourceData);

	if (_metaResource.songTableID > 0) {
		_vm->_resource->loadResource(resourceContext, _metaResource.songTableID, resourceData);

		if (chapter == 6) {
			if (resourceData.size() < (uint(actorsEntrance) * 4 + 4)) {
				error("Resource::loadGlobalResources chapter 6 has wrong resource");
			}
			int32 id = READ_LE_UINT32(&resourceData[actorsEntrance * 4]);
			_vm->_resource->loadResource(resourceContext, id, resourceData);
		}

		if (resourceData.empty()) {
			error("Resource::loadGlobalResources Can't load songs list for current track");
		}

		_vm->_music->_songTable.resize(resourceData.size() / 4);

		ByteArrayReadStreamEndian songS(resourceData);

		for (i = 0; i < _vm->_music->_songTable.size(); i++)
			_vm->_music->_songTable[i] = songS.readSint32LE();
	} else {
		// The IHNM demo has a fixed music track and doesn't load a song table
		_vm->_music->setVolume(_vm->_musicVolume, 1);
		_vm->_music->play(3, MUSIC_LOOP);
	}

	int voiceLUTResourceID = 0;

	if (chapter != 7) {
		int voiceBank = (chapter == 8) ? 0 : chapter;
		_vm->_sndRes->setVoiceBank(voiceBank);
		voiceLUTResourceID = 22 + voiceBank;
	} else {
		// IHNM demo
		_vm->_sndRes->setVoiceBank(0);
		voiceLUTResourceID = 17;
	}

	if (voiceLUTResourceID) {
		_vm->_resource->loadResource(resourceContext, voiceLUTResourceID, resourceData);
		_vm->_script->loadVoiceLUT(_vm->_script->_globalVoiceLUT, resourceData);
	}

	_vm->_spiritualBarometer = 0;
	_vm->_scene->setChapterNumber(chapter);
}

void ResourceContext_RES::processPatches(Resource *resource, const GamePatchDescription *patchFiles) {
	uint16 subjectResourceType;
	ResourceContext *subjectContext;
	uint32 subjectResourceId;
	uint32 patchResourceId;
	ResourceData *subjectResourceData;
	ResourceData *resourceData;

	// Process internal patch files
	if (_fileType & GAME_PATCHFILE) {
		subjectResourceType = ~GAME_PATCHFILE & _fileType;
		subjectContext = resource->getContext((GameFileTypes)subjectResourceType);
		if (subjectContext == NULL) {
			error("ResourceContext::load() Subject context not found");
		}
		ByteArray tableBuffer;

		resource->loadResource(this, _table.size() - 1, tableBuffer);

		ByteArrayReadStreamEndian readS2(tableBuffer, _isBigEndian);
		for (uint32 i = 0; i < tableBuffer.size() / 8; i++) {
			subjectResourceId = readS2.readUint32();
			patchResourceId = readS2.readUint32();
			subjectResourceData = subjectContext->getResourceData(subjectResourceId);
			resourceData = getResourceData(patchResourceId);
			subjectResourceData->patchData = new PatchData(&_file, _fileName);
			subjectResourceData->offset = resourceData->offset;
			subjectResourceData->size = resourceData->size;
		}
	}
}

#endif

} // End of namespace Saga
