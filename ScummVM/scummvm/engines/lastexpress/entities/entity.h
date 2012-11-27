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

#ifndef LASTEXPRESS_ENTITY_H
#define LASTEXPRESS_ENTITY_H

#include "lastexpress/shared.h"

#include "lastexpress/sound/sound.h"

#include "lastexpress/helpers.h"

#include "common/array.h"
#include "common/func.h"
#include "common/serializer.h"
#include "common/textconsole.h"

namespace LastExpress {

class LastExpressEngine;
class Sequence;
class SequenceFrame;
struct SavePoint;

//////////////////////////////////////////////////////////////////////////
// Declaration
//////////////////////////////////////////////////////////////////////////
#define DECLARE_FUNCTION(name) \
	void setup_##name(); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_1(name, param1) \
	void setup_##name(param1); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_2(name, param1, param2) \
	void setup_##name(param1, param2); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_3(name, param1, param2, param3) \
	void setup_##name(param1, param2, param3); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_4(name, param1, param2, param3, param4) \
	void setup_##name(param1, param2, param3, param4); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_NOSETUP(name) \
	void name(const SavePoint &savepoint);

#define DECLARE_NULL_FUNCTION() \
	void setup_nullfunction();

//////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////
#define ENTITY_CALLBACK(class, name, pointer) \
	Common::Functor1Mem<const SavePoint&, void, class>(pointer, &class::name)

#define ADD_CALLBACK_FUNCTION(class, name) \
	_callbacks.push_back(new ENTITY_CALLBACK(class, name, this));

#define ADD_NULL_FUNCTION() \
	_callbacks.push_back(new ENTITY_CALLBACK(Entity, nullfunction, this));

#define WRAP_SETUP_FUNCTION(className, method) \
	new Common::Functor0Mem<void, className>(this, &className::method)

#define WRAP_SETUP_FUNCTION_S(className, method) \
	new Common::Functor1Mem<const char *, void, className>(this, &className::method)

#define WRAP_SETUP_FUNCTION_B(className, method) \
	new Common::Functor1Mem<bool, void, className>(this, &className::method)

//////////////////////////////////////////////////////////////////////////
// Parameters macros
//////////////////////////////////////////////////////////////////////////
#define CURRENT_PARAM(index, id) \
	((EntityData::EntityParametersIIII*)_data->getCurrentParameters(index))->param##id

#define ENTITY_PARAM(index, id) \
	((EntityData::EntityParametersIIII*)_data->getParameters(8, index))->param##id

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
#define RESET_ENTITY_STATE(entity, class, function) \
	getEntities()->resetState(entity); \
	((class *)getEntities()->get(entity))->function();

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

// Expose parameters and check validity
#define EXPOSE_PARAMS(type) \
	type *params = (type *)_data->getCurrentParameters(); \
	if (!params) \
		error("[EXPOSE_PARAMS] Trying to call an entity function with invalid parameters"); \

// function signature without setup (we keep the index for consistency but never use it)
#define IMPLEMENT_FUNCTION_NOSETUP(index, class, name) \
	void class::name(const SavePoint &savepoint) { \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(index=" #index ")");

// simple setup with no parameters
#define IMPLEMENT_FUNCTION(index, class, name) \
	void class::setup_##name() { \
		Entity::setup(#class "::setup_" #name, index); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "() - action: %s", ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_END }

// nullfunction call
#define IMPLEMENT_NULL_FUNCTION(index, class) \
	void class::setup_nullfunction() { \
		Entity::setup(#class "::setup_nullfunction", index); \
	}

// setup with one uint parameter
#define IMPLEMENT_FUNCTION_I(index, class, name, paramType) \
	void class::setup_##name(paramType param1) { \
		Entity::setupI(#class "::setup_" #name, index, param1); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d) - action: %s", params->param1, ACTION_NAME(savepoint.action));

// setup with two uint parameters
#define IMPLEMENT_FUNCTION_II(index, class, name, paramType1, paramType2) \
	void class::setup_##name(paramType1 param1, paramType2 param2) { \
		Entity::setupII(#class "::setup_" #name, index, param1, param2); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d) - action: %s", params->param1, params->param2, ACTION_NAME(savepoint.action));

// setup with three uint parameters
#define IMPLEMENT_FUNCTION_III(index, class, name, paramType1, paramType2, paramType3) \
	void class::setup_##name(paramType1 param1, paramType2 param2, paramType3 param3) { \
		Entity::setupIII(#class "::setup_" #name, index, param1, param2, param3); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d, %d) - action: %s", params->param1, params->param2, params->param3, ACTION_NAME(savepoint.action));

// setup with one char *parameter
#define IMPLEMENT_FUNCTION_S(index, class, name) \
	void class::setup_##name(const char *seq1) { \
		Entity::setupS(#class "::setup_" #name, index, seq1); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s) - action: %s", (char *)&params->seq1, ACTION_NAME(savepoint.action));

// setup with one char *parameter and one uint
#define IMPLEMENT_FUNCTION_SI(index, class, name, paramType2) \
	void class::setup_##name(const char *seq1, paramType2 param4) { \
		Entity::setupSI(#class "::setup_" #name, index, seq1, param4); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d) - action: %s", (char *)&params->seq1, params->param4, ACTION_NAME(savepoint.action));

// setup with one char *parameter and two uints
#define IMPLEMENT_FUNCTION_SII(index, class, name, paramType2, paramType3) \
	void class::setup_##name(const char *seq1, paramType2 param4, paramType3 param5) { \
		Entity::setupSII(#class "::setup_" #name, index, seq1, param4, param5); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d, %d) - action: %s", (char *)&params->seq1, params->param4, params->param5, ACTION_NAME(savepoint.action));

// setup with one char *parameter and three uints
#define IMPLEMENT_FUNCTION_SIII(index, class, name, paramType2, paramType3, paramType4) \
	void class::setup_##name(const char *seq, paramType2 param4, paramType3 param5, paramType4 param6) { \
		Entity::setupSIII(#class "::setup_" #name, index, seq, param4, param5, param6); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersSIII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d, %d, %d) - action: %s", (char *)&params->seq, params->param4, params->param5, params->param6, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_SIIS(index, class, name, paramType2, paramType3) \
	void class::setup_##name(const char *seq1, paramType2 param4, paramType3 param5, const char *seq2) { \
		Entity::setupSIIS(#class "::setup_" #name, index, seq1, param4, param5, seq2); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d, %d, %s) - action: %s", (char *)&params->seq1, params->param4, params->param5, (char *)&params->seq2, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_SS(index, class, name) \
	void class::setup_##name(const char *seq1, const char *seq2) { \
		Entity::setupSS(#class "::setup_" #name, index, seq1, seq2); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersSSII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %s) - action: %s", (char *)&params->seq1, (char *)&params->seq2, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_SSI(index, class, name, paramType3) \
	void class::setup_##name(const char *seq1, const char *seq2, paramType3 param7) { \
		Entity::setupSSI(#class "::setup_" #name, index, seq1, seq2, param7); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersSSII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %s, %d) - action: %s", (char *)&params->seq1, (char *)&params->seq2, params->param7, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_IS(index, class, name, paramType) \
	void class::setup_##name(paramType param1, const char *seq) { \
		Entity::setupIS(#class "::setup_" #name, index, param1, seq); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersISII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %s) - action: %s", params->param1, (char *)&params->seq, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_ISS(index, class, name, paramType) \
	void class::setup_##name(paramType param1, const char *seq1, const char *seq2) { \
		Entity::setupISS(#class "::setup_" #name, index, param1, seq1, seq2); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersISSI) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %s, %s) - action: %s", params->param1, (char *)&params->seq1, (char *)&params->seq2, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_IIS(index, class, name, paramType1, paramType2) \
	void class::setup_##name(paramType1 param1, paramType2 param2, const char *seq) { \
		Entity::setupIIS(#class "::setup_" #name, index, param1, param2, seq); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersIISI) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d, %s) - action: %s", params->param1, params->param2, (char *)&params->seq, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_IISS(index, class, name, paramType1, paramType2) \
	void class::setup_##name(paramType1 param1, paramType2 param2, const char *seq1, const char *seq2) { \
		Entity::setupIISS(#class "::setup_" #name, index, param1, param2, seq1, seq2); \
	} \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersIISS) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d, %s, %s) - action: %s", params->param1, params->param2, (char *)&params->seq1, (char *)&params->seq2, ACTION_NAME(savepoint.action));


//////////////////////////////////////////////////////////////////////////
class EntityData : Common::Serializable {
public:

	struct EntityParameters : Common::Serializable {
		virtual ~EntityParameters() {}
		virtual Common::String toString() = 0;

		virtual void update(uint32 index) = 0;

		virtual void saveLoadWithSerializer(Common::Serializer &s) = 0;
	};

	struct EntityParametersIIII : EntityParameters {
		uint param1;
		uint param2;
		uint param3;
		uint param4;
		uint param5;
		uint param6;
		uint param7;
		uint param8;

		EntityParametersIIII() {
			param1 = 0;
			param2 = 0;
			param3 = 0;
			param4 = 0;
			param5 = 0;
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		bool hasNonNullParameter() {
			return param1 || param2 || param3 || param4 || param5 || param6 || param7 || param8;
		}

		Common::String toString() {
			return Common::String::format("IIII: %d %d %d %d %d %d %d %d\n", param1, param2, param3, param4, param5, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIIII::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			case 2: param3 = 1; break;
			case 3: param4 = 1; break;
			case 4: param5 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncAsUint32LE(param3);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSIII : EntityParameters {
		char seq[12];
		uint param4;
		uint param5;
		uint param6;
		uint param7;
		uint param8;

		EntityParametersSIII() {
			memset(&seq, 0, 12);
			param4 = 0;
			param5 = 0;
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("SIII: %s %d %d %d %d %d\n", seq, param4, param5, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersSIII::update] Invalid index (was: %d)", index);

			case 3: param4 = 1; break;
			case 4: param5 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSIIS : EntityParameters {
		char seq1[12];
		uint param4;
		uint param5;
		char seq2[12];

		EntityParametersSIIS() {
			memset(&seq1, 0, 12);
			param4 = 0;
			param5 = 0;
			memset(&seq2, 0, 12);
		}

		Common::String toString() {
			return Common::String::format("SIIS: %s %d %d %s\n", seq1, param4, param5, seq2);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersSIIS::update] Invalid index (was: %d)", index);

			case 3: param4 = 1; break;
			case 4: param5 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq1, 12);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncBytes((byte *)&seq2, 12);
		}
	};

	struct EntityParametersISSI : EntityParameters {
		uint param1;
		char seq1[12];
		char seq2[12];
		uint param8;

		EntityParametersISSI() {
			param1 = 0;
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("ISSI: %d %s %s %d\n", param1, seq1, seq2, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersISSI::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersISII : EntityParameters {
		uint param1;
		char seq[12];
		uint param5;
		uint param6;
		uint param7;
		uint param8;

		EntityParametersISII() {
			param1 = 0;
			memset(&seq, 0, 12);
			param5 = 0;
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("ISII: %d %s %d %d %d %d\n", param1, seq, param5, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersISII::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 4: param5 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param5);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSSII : EntityParameters {
		char seq1[12];
		char seq2[12];
		uint param7;
		uint param8;

		EntityParametersSSII() {
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("SSII: %s %s %d %d\n", seq1, seq2, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersSSII::update] Invalid index (was: %d)", index);

			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSSS : EntityParameters {
		char seq1[12];
		char seq2[12];
		char seq3[8];

		EntityParametersSSS() {
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
			memset(&seq3, 0, 8);
		}

		Common::String toString() {
			return Common::String::format("SSS: %s %s %s\n", seq1, seq2, seq3);
		}

		void update(uint32) {
			error("[EntityParametersSSS::update] Cannot update this type of parameters");
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
			s.syncBytes((byte *)&seq3, 8);
		}
	};

	struct EntityParametersIISS : EntityParameters {
		uint param1;
		uint param2;
		char seq1[12];
		char seq2[12];

		EntityParametersIISS() {
			param1 = 0;
			param2 = 0;
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
		}

		Common::String toString() {
			return Common::String::format("IISS: %d %d %s %s\n", param1, param2, seq1, seq2);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIISS::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
		}
	};

	struct EntityParametersIISI : EntityParameters {
		uint param1;
		uint param2;
		char seq[12];
		uint param6;
		uint param7;
		uint param8;

		EntityParametersIISI() {
			param1 = 0;
			param2 = 0;
			memset(&seq, 0, 12);
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("IISI: %d %d %s %d %d %d\n", param1, param2, seq, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIISI::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersIIIS : EntityParameters {
		uint param1;
		uint param2;
		uint param3;
		char seq[12];
		uint param7;
		uint param8;

		EntityParametersIIIS() {
			param1 = 0;
			param2 = 0;
			param3 = 0;
			memset(&seq, 0, 12);
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("IIIS: %d %d %d %s %d %d\n", param1, param2, param3, seq, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIIIS::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			case 2: param3 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncAsUint32LE(param3);
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersI5S : EntityParameters {
		uint param1;
		uint param2;
		uint param3;
		uint param4;
		uint param5;
		char seq[12];

		EntityParametersI5S() {
			param1 = 0;
			param2 = 0;
			param3 = 0;
			param4 = 0;
			param5 = 0;
			memset(&seq, 0, 12);
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncAsUint32LE(param3);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncBytes((byte *)&seq, 12);
		}
	};

	struct EntityCallParameters : Common::Serializable {
		EntityParameters *parameters[4];

		EntityCallParameters() {
			// We default to int parameters
			for (int i = 0; i < 4; i++)
				parameters[i] = new EntityParametersIIII();
		}

		~EntityCallParameters() {
			clear();
		}

		void clear() {
			for (int i = 0; i < 4; i++)
				SAFE_DELETE(parameters[i]);
		}

		// Serializable
		void saveLoadWithSerializer(Common::Serializer &s) {
			for (uint i = 0; i < ARRAYSIZE(parameters); i++)
				parameters[i]->saveLoadWithSerializer(s);
		}
	};

	struct EntityCallData : Common::Serializable {
		byte callbacks[16];
		byte currentCall;
		EntityPosition entityPosition;      // word
		Location location;                  // word
		CarIndex car;                       // word
		byte field_497;
		EntityIndex entity;                 // byte
		InventoryItem inventoryItem;        // byte
		EntityDirection direction;          // byte
		int16 field_49B;
		int16 currentFrame;
		int16 currentFrame2;
		int16 field_4A1;
		int16 field_4A3;
		ClothesIndex clothes;               // byte
		Position position;
		CarIndex car2;                      // byte
		bool doProcessEntity;               // byte
		bool field_4A9;	                    // byte
		bool field_4AA;                     // byte
		EntityDirection directionSwitch;
		Common::String sequenceName;        // char[13]
		Common::String sequenceName2;       // char[13]
		Common::String sequenceNamePrefix;  // char[7]
		Common::String sequenceNameCopy;    // char[13]
		SequenceFrame *frame;
		SequenceFrame *frame1;
		Sequence *sequence;
		Sequence *sequence2;
		Sequence *sequence3;

		/**
		 * Default constructor.
		 */
		EntityCallData() {
			memset(&callbacks, 0, 16 * sizeof(byte));
			currentCall = 0;
			entityPosition = kPositionNone;
			location = kLocationOutsideCompartment;
			car = kCarNone;
			field_497 = 0;
			entity = kEntityPlayer;
			inventoryItem = kItemNone;
			direction = kDirectionNone;
			field_49B = 0;
			currentFrame = 0;
			currentFrame2 = 0;
			field_4A1 = 0;
			field_4A3 = 30;
			clothes = kClothesDefault;
			position = 0;
			car2 = kCarNone;
			doProcessEntity = false;
			field_4A9 = false;
			field_4AA = false;
			directionSwitch = kDirectionNone;
			frame = NULL;
			frame1 = NULL;
			sequence = NULL;
			sequence2 = NULL;
			sequence3 = NULL;
		}

		~EntityCallData();

		/**
		 * Convert this object into a string representation.
		 *
		 * @return A string representation of this object.
		 */
		Common::String toString() {
			Common::String str = "";

			str += Common::String::format("Entity position: %d    - Location: %d       - Car: %d\n", entityPosition, location, car);
			str += Common::String::format("Entity: %d             - Item: %d          - Direction: %d\n", entity, inventoryItem, direction);
			str += Common::String::format("Clothes: %d            - Position: %d      - Direction switch: %d\n", clothes, position, directionSwitch);
			str += "\n";
			str += Common::String::format("field_497: %02d        - field_49B: %i     - field_4A1: %i\n", field_497, field_49B, field_4A1);
			str += Common::String::format("field_4A9: %02d        - field_4AA: %i     - Car 2: %d\n", field_4A9, field_4AA, car2);
			str += "\n";
			str += "Sequence: " + sequenceName + "                 - Sequence 2: " + sequenceName2 + "\n";
			str += "Sequence prefix: " + sequenceNamePrefix + "    - Sequence copy: " + sequenceNameCopy + "\n";
			str += Common::String::format("Current frame: %i    - Current frame 2: %i       - Process entity: %d\n", currentFrame, currentFrame2, doProcessEntity);
			str += "\n";
			str += Common::String::format("Current call: %d\n", currentCall);
			str += Common::String::format("Functions: %d %d %d %d %d %d %d %d\n", callbacks[0], callbacks[1], callbacks[2], callbacks[3], callbacks[4], callbacks[5], callbacks[6], callbacks[7]);
			str += Common::String::format("Callbacks: %d %d %d %d %d %d %d %d\n", callbacks[8], callbacks[9], callbacks[10], callbacks[11], callbacks[12], callbacks[13], callbacks[14], callbacks[15]);

			return str;
		}

		/**
		 * Synchronizes a string.
		 *
		 * @param	s	  	The Common::Serializer to use.
		 * @param	string	The string.
		 * @param	length	Length of the string.
		 */
		void syncString(Common::Serializer &s, Common::String &string, uint length) const;

		// Serializable
		void saveLoadWithSerializer(Common::Serializer &s);
	};

	EntityData() {}

	template<class T>
	void resetCurrentParameters() {
		EntityCallParameters *params = &_parameters[_data.currentCall];
		params->clear();

		for (int i = 0; i < 4; i++)
			params->parameters[i] = new T();
	}

	EntityCallData       *getCallData() { return &_data; }

	EntityParameters     *getParameters(uint callback, byte index) const;
	EntityParameters     *getCurrentParameters(byte index = 0) { return getParameters(_data.currentCall, index); }
	EntityCallParameters *getCurrentCallParameters() { return &_parameters[_data.currentCall]; }

	byte                  getCallback(uint callback) const;
	byte                  getCurrentCallback() { return getCallback(_data.currentCall); }
	void                  setCallback(uint callback, byte index);
	void                  setCurrentCallback(uint index) { setCallback(_data.currentCall, index); }

	void                  updateParameters(uint32 index) const;

	// Serializable
	void                  saveLoadWithSerializer(Common::Serializer &ser);

private:

	EntityCallData       _data;
	EntityCallParameters _parameters[9];
};

class Entity : Common::Serializable {
public:
	Entity(LastExpressEngine *engine, EntityIndex index);
	virtual ~Entity();

	// Accessors
	EntityData *getParamData() { return _data; }
	EntityData::EntityCallData *getData() { return _data->getCallData(); }

	// Callbacks
	byte getCallback() { return _data->getCallback(_data->getCallData()->currentCall + 8); }
	void setCallback(byte index) { _data->setCallback(_data->getCallData()->currentCall + 8, index); getData()->currentCall++; }

	// Setup
	void setup(ChapterIndex index);

	virtual void setup_chapter1() = 0;
	virtual void setup_chapter2() = 0;
	virtual void setup_chapter3() = 0;
	virtual void setup_chapter4() = 0;
	virtual void setup_chapter5() = 0;

	// Shared functions
	virtual void setup_savegame(SavegameType, uint32) { error("[Entity::setup_savegame] Trying to call the parent setup function. Use the specific entity function directly"); }
	virtual void setup_enterExitCompartment(const char *, ObjectIndex) { error("[Entity::setup_enterExitCompartment] Trying to call the parent setup function. Use the specific entity function directly"); }
	virtual void setup_updateEntity(CarIndex, EntityPosition) { error("[Entity::setup_updateEntity] Trying to call the parent setup function. Use the specific entity function directly"); }
	virtual void setup_playSound(const char*) { error("[Entity::setup_playSound] Trying to call the parent setup function. Use the specific entity function directly"); }

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser) { _data->saveLoadWithSerializer(ser); }

	void nullfunction(const SavePoint &savepoint) {}

protected:
	LastExpressEngine *_engine;
	typedef Common::Functor1<const SavePoint&, void> Callback;

	EntityIndex                _entityIndex;
	EntityData                *_data;
	Common::Array<Callback *>  _callbacks;

	/**
	 * Saves the game
	 *
	 * @param savepoint The savepoint
	 *                   - SavegameType
	 *                   - EventIndex
	 */
	void savegame(const SavePoint &savepoint);

	/**
	 * Saves the game before being found out with a blood covered jacket.
	 *
	 * @param	saveFunction	The setup function to call to save the game
	 */
	void savegameBloodJacket();

	/**
	 * Play sound
	 *
	 * @param savepoint The savepoint
	 *                    - Sound filename
	 * @param resetItem true to reset item.
	 * @param flag      sound flag
	 */
	void playSound(const SavePoint &savepoint, bool resetItem = false, SoundFlag flag = kFlagInvalid);

	/**
	 * Draws the entity
	 *
	 * @param savepoint 	 The savepoint
	 *                         - Sequence
	 *                         - ExcuseMe flag
	 * @param handleExcuseMe true to handle excuseMeCath action
	 */
	void draw(const SavePoint &savepoint, bool handleExcuseMe = false);

	/**
	 * Draws the entity along with another one
	 *
	 * @param savepoint The savepoint.
	 *                    - Sequence 1
	 *                    - Sequence 2
	 *                    - EntityIndex
	 */
	void draw2(const SavePoint &savepoint);

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param savepoint The savepoint
	 *                    - Number of ticks to add
	 */
	void updateFromTicks(const SavePoint &savepoint);

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param savepoint The savepoint.
	 *                    - Time to add
	 */
	void updateFromTime(const SavePoint &savepoint);

	/**
	 * Resets an entity
	 *
	 * @param savepoint    The savepoint.
	 * @param resetClothes true to reset clothes.
	 * @param resetItem    true to reset inventoryItem to kItemInvalid
	 */
	void reset(const SavePoint &savepoint, bool resetClothes = false, bool resetItem = false);

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 *
	 * @param savepoint The savepoint.
	 */
	void callbackActionOnDirection(const SavePoint &savepoint);

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 *
	 * @param savepoint The savepoint.
	 */
	void callbackActionRestaurantOrSalon(const SavePoint &savepoint);

	/**
	 * Updates the entity
	 *
	 * @param savepoint 	 The savepoint.
	 *                        - CarIndex
	 *                        - EntityPosition
	 * @param handleExcuseMe true to handle the kActionExcuseMe/kActionExcuseMeCath actions.
	 */
	void updateEntity(const SavePoint &savepoint, bool handleExcuseMe = false);

	/**
	 * Call a specific savepoint (or draw sequence in default case)
	 *
	 * @param savepoint 	 The savepoint.
	 *                         - Sequence to draw in default case
	 *                         - EntityIndex
	 *                         - ActionIndex
	 *                         - Sequence for the savepoint
	 * @param handleExcuseMe true to handle excuse me.
	 */
	void callSavepoint(const SavePoint &savepoint, bool handleExcuseMe = false);

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param savepoint   The savepoint.
	 * @param position1   The first position.
	 * @param position2   The second position.
	 * @param car 		  The car.
	 * @param compartment The compartment.
	 * @param alternate   true to use the alternate version of SceneManager::loadSceneFromObject()
	 */
	void enterExitCompartment(const SavePoint &savepoint, EntityPosition position1 = kPositionNone, EntityPosition position2 = kPositionNone, CarIndex car = kCarNone, ObjectIndex compartment = kObjectNone, bool alternate = false, bool updateLocation = false);

	/**
	 * Go to compartment.
	 *
	 * @param	savepoint			 	The savepoint.
	 * @param	compartmentFrom		 	The compartment from.
	 * @param	positionFrom		 	The position from.
	 * @param	sequenceFrom		 	The sequence from.
	 * @param	sequenceTo			 	The sequence to.
	 */
	void goToCompartment(const SavePoint &savepoint, ObjectIndex compartmentFrom, EntityPosition positionFrom, Common::String sequenceFrom, Common::String sequenceTo);

	/**
	 * Go to compartment from compartment.
	 *
	 * @param	savepoint			  	The savepoint.
	 * @param	compartmentFrom		  	The compartment from.
	 * @param	positionFrom		  	The position from.
	 * @param	sequenceFrom		  	The sequence from.
	 * @param	compartmentTo		  	The compartment to.
	 * @param	positionTo			  	The position to.
	 * @param	sequenceTo			  	The sequence to.
	 */
	void goToCompartmentFromCompartment(const SavePoint &savepoint, ObjectIndex compartmentFrom, EntityPosition positionFrom, Common::String sequenceFrom, ObjectIndex compartmentTo, EntityPosition positionTo, Common::String sequenceTo);

	/**
	 * Updates the position
	 *
	 * @param savepoint       The savepoint
	 *                        - Sequence name
	 *                        - CarIndex
	 *                        - Position
	 * @param handleExcuseMe true to handle excuseMe actions
	 */
	void updatePosition(const SavePoint &savepoint, bool handleExcuseMe = false);

	/**
	 * Store the current callback information and perform the callback action
	 */
	void callbackAction();

	//////////////////////////////////////////////////////////////////////////
	// Setup functions
	//////////////////////////////////////////////////////////////////////////
	void setup(const char *name, uint index);
	void setupI(const char *name, uint index, uint param1);
	void setupII(const char *name, uint index, uint param1, uint param2);
	void setupIII(const char *name, uint index, uint param1, uint param2, uint param3);
	void setupS(const char *name, uint index, const char *seq1);
	void setupSS(const char *name, uint index, const char *seq1, const char *seq2);
	void setupSI(const char *name, uint index, const char *seq1, uint param4);
	void setupSII(const char *name, uint index, const char *seq1, uint param4, uint param5);
	void setupSIII(const char *name, uint index, const char *seq, uint param4, uint param5, uint param6);
	void setupSIIS(const char *name, uint index, const char *seq1, uint param4, uint param5, const char *seq2);
	void setupSSI(const char *name, uint index, const char *seq1, const char *seq2, uint param7);
	void setupIS(const char *name, uint index, uint param1, const char *seq);
	void setupISS(const char *name, uint index, uint param1, const char *seq1, const char *seq2);
	void setupIIS(const char *name, uint index, uint param1, uint param2, const char *seq);
	void setupIISS(const char *name, uint index, uint param1, uint param2, const char *seq1, const char *seq2);

	//////////////////////////////////////////////////////////////////////////
	// Helper functions
	//////////////////////////////////////////////////////////////////////////

	bool updateParameter(uint &parameter, uint timeType, uint delta) const;
	bool updateParameterCheck(uint &parameter, uint timeType, uint delta) const;
	bool updateParameterTime(TimeValue timeValue, bool check, uint &parameter, uint delta) const;

	bool timeCheck(TimeValue timeValue, uint &parameter, Common::Functor0<void> *function) const;
	bool timeCheckCallback(TimeValue timeValue, uint &parameter, byte callback, Common::Functor0<void> *function);
	bool timeCheckCallback(TimeValue timeValue, uint &parameter, byte callback, const char *str, Common::Functor1<const char *, void> *function);
	bool timeCheckCallback(TimeValue timeValue, uint &parameter, byte callback, bool check, Common::Functor1<bool, void> *function);
	bool timeCheckCallbackInventory(TimeValue timeValue, uint &parameter, byte callback, Common::Functor0<void> *function);
	bool timeCheckCar(TimeValue timeValue, uint &parameter, byte callback, Common::Functor0<void> *function);
	void timeCheckSavepoint(TimeValue timeValue, uint &parameter, EntityIndex entity1, EntityIndex entity2, ActionIndex action) const;
	void timeCheckObject(TimeValue timeValue, uint &parameter, ObjectIndex index, ObjectLocation location) const;
	bool timeCheckCallbackAction(TimeValue timeValue, uint &parameter);
	bool timeCheckPlaySoundUpdatePosition(TimeValue timeValue, uint &parameter, byte callback, const char* sound, EntityPosition position);

};


} // End of namespace LastExpress

#endif // LASTEXPRESS_ENTITY_H
