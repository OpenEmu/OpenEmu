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

#ifndef MOHAWK_LIVINGBOOKS_LBX_H
#define MOHAWK_LIVINGBOOKS_LBX_H

#include "engines/mohawk/livingbooks_code.h"

#include "common/ptr.h"

namespace Mohawk {

class LBXObject {
public:
	LBXObject(MohawkEngine_LivingBooks *vm) : _vm(vm) { }
	virtual ~LBXObject() { }

	virtual bool call(uint callId, const Common::Array<LBValue> &params, LBValue &result) = 0;

protected:
	MohawkEngine_LivingBooks *_vm;
};

Common::SharedPtr<LBXObject> createLBXObject(MohawkEngine_LivingBooks *vm, uint16 type);

} // End of namespace Mohawk

#endif
