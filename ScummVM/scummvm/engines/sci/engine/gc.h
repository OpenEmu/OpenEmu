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


#ifndef SCI_ENGINE_GC_H
#define SCI_ENGINE_GC_H

#include "common/hashmap.h"
#include "sci/engine/vm_types.h"
#include "sci/engine/state.h"

namespace Sci {

struct reg_t_Hash {
	uint operator()(const reg_t& x) const {
		return (x.getSegment() << 3) ^ x.getOffset() ^ (x.getOffset() << 16);
	}
};

/*
 * The AddrSet is a "set" of reg_t values.
 * We don't have a HashSet type, so we abuse a HashMap for this.
 */
typedef Common::HashMap<reg_t, bool, reg_t_Hash> AddrSet;

/**
 * Finds all used references and normalises them to their memory addresses
 * @param s The state to gather all information from
 * @return A hash map containing entries for all used references
 */
AddrSet *findAllActiveReferences(EngineState *s);

/**
 * Runs garbage collection on the current system state
 * @param s The state in which we should gc
 */
void run_gc(EngineState *s);

struct WorklistManager {
	Common::Array<reg_t> _worklist;
	AddrSet _map;	// used for 2 contains() calls, inside push() and run_gc()

	void push(reg_t reg);
	void pushArray(const Common::Array<reg_t> &tmp);
};


} // End of namespace Sci

#endif // SCI_ENGINE_GC_H
