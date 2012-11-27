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

#include "sci/engine/gc.h"
#include "common/array.h"
#include "sci/graphics/ports.h"

namespace Sci {

//#define GC_DEBUG_CODE

#ifdef GC_DEBUG_CODE
const char *segmentTypeNames[] = {
	"invalid",   // 0
	"script",    // 1
	"clones",    // 2
	"locals",    // 3
	"stack",     // 4
	"obsolete",  // 5: obsolete system strings
	"lists",     // 6
	"nodes",     // 7
	"hunk",      // 8
	"dynmem",    // 9
	"obsolete",  // 10: obsolete string fragments
	"array",     // 11: SCI32 arrays
	"string"     // 12: SCI32 strings
};
#endif

void WorklistManager::push(reg_t reg) {
	if (!reg.getSegment()) // No numbers
		return;

	debugC(kDebugLevelGC, "[GC] Adding %04x:%04x", PRINT_REG(reg));

	if (_map.contains(reg))
		return; // already dealt with it

	_map.setVal(reg, true);
	_worklist.push_back(reg);
}

void WorklistManager::pushArray(const Common::Array<reg_t> &tmp) {
	for (Common::Array<reg_t>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
		push(*it);
}

static AddrSet *normalizeAddresses(SegManager *segMan, const AddrSet &nonnormal_map) {
	AddrSet *normal_map = new AddrSet();

	for (AddrSet::const_iterator i = nonnormal_map.begin(); i != nonnormal_map.end(); ++i) {
		reg_t reg = i->_key;
		SegmentObj *mobj = segMan->getSegmentObj(reg.getSegment());

		if (mobj) {
			reg = mobj->findCanonicAddress(segMan, reg);
			normal_map->setVal(reg, true);
		}
	}

	return normal_map;
}

static void processWorkList(SegManager *segMan, WorklistManager &wm, const Common::Array<SegmentObj *> &heap) {
	SegmentId stackSegment = segMan->findSegmentByType(SEG_TYPE_STACK);
	while (!wm._worklist.empty()) {
		reg_t reg = wm._worklist.back();
		wm._worklist.pop_back();
		if (reg.getSegment() != stackSegment) { // No need to repeat this one
			debugC(kDebugLevelGC, "[GC] Checking %04x:%04x", PRINT_REG(reg));
			if (reg.getSegment() < heap.size() && heap[reg.getSegment()]) {
				// Valid heap object? Find its outgoing references!
				wm.pushArray(heap[reg.getSegment()]->listAllOutgoingReferences(reg));
			}
		}
	}
}

AddrSet *findAllActiveReferences(EngineState *s) {
	assert(!s->_executionStack.empty());

	WorklistManager wm;

	// Initialize registers
	wm.push(s->r_acc);
	wm.push(s->r_prev);

	// Initialize value stack
	// We do this one by hand since the stack doesn't know the current execution stack
	Common::List<ExecStack>::const_iterator iter = s->_executionStack.reverse_begin();

	// Skip fake kernel stack frame if it's on top
	if ((*iter).type == EXEC_STACK_TYPE_KERNEL)
		--iter;

	assert((iter != s->_executionStack.end()) && ((*iter).type != EXEC_STACK_TYPE_KERNEL));

	const StackPtr sp = iter->sp;

	for (reg_t *pos = s->stack_base; pos < sp; pos++)
		wm.push(*pos);

	debugC(kDebugLevelGC, "[GC] -- Finished adding value stack");

	// Init: Execution Stack
	for (iter = s->_executionStack.begin();
	     iter != s->_executionStack.end(); ++iter) {
		const ExecStack &es = *iter;

		if (es.type != EXEC_STACK_TYPE_KERNEL) {
			wm.push(es.objp);
			wm.push(es.sendp);
			if (es.type == EXEC_STACK_TYPE_VARSELECTOR)
				wm.push(*(es.getVarPointer(s->_segMan)));
		}
	}

	debugC(kDebugLevelGC, "[GC] -- Finished adding execution stack");

	const Common::Array<SegmentObj *> &heap = s->_segMan->getSegments();
	uint heapSize = heap.size();

	// Init: Explicitly loaded scripts
	for (uint i = 1; i < heapSize; i++) {
		if (heap[i] && heap[i]->getType() == SEG_TYPE_SCRIPT) {
			Script *script = (Script *)heap[i];

			if (script->getLockers()) { // Explicitly loaded?
				wm.pushArray(script->listObjectReferences());
			}
		}
	}

	debugC(kDebugLevelGC, "[GC] -- Finished explicitly loaded scripts, done with root set");

	processWorkList(s->_segMan, wm, heap);

	if (g_sci->_gfxPorts)
		g_sci->_gfxPorts->processEngineHunkList(wm);

	return normalizeAddresses(s->_segMan, wm._map);
}

void run_gc(EngineState *s) {
	SegManager *segMan = s->_segMan;

	// Some debug stuff
	debugC(kDebugLevelGC, "[GC] Running...");
#ifdef GC_DEBUG_CODE
	const char *segnames[SEG_TYPE_MAX + 1];
	int segcount[SEG_TYPE_MAX + 1];
	memset(segnames, 0, sizeof(segnames));
	memset(segcount, 0, sizeof(segcount));
#endif

	// Compute the set of all segments references currently in use.
	AddrSet *activeRefs = findAllActiveReferences(s);

	// Iterate over all segments, and check for each whether it
	// contains stuff that can be collected.
	const Common::Array<SegmentObj *> &heap = segMan->getSegments();
	for (uint seg = 1; seg < heap.size(); seg++) {
		SegmentObj *mobj = heap[seg];

		if (mobj != NULL) {
#ifdef GC_DEBUG_CODE
			const SegmentType type = mobj->getType();
			segnames[type] = segmentTypeNames[type];
#endif

			// Get a list of all deallocatable objects in this segment,
			// then free any which are not referenced from somewhere.
			const Common::Array<reg_t> tmp = mobj->listAllDeallocatable(seg);
			for (Common::Array<reg_t>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
				const reg_t addr = *it;
				if (!activeRefs->contains(addr)) {
					// Not found -> we can free it
					mobj->freeAtAddress(segMan, addr);
					debugC(kDebugLevelGC, "[GC] Deallocating %04x:%04x", PRINT_REG(addr));
#ifdef GC_DEBUG_CODE
					segcount[type]++;
#endif
				}
			}

		}
	}

	delete activeRefs;

#ifdef GC_DEBUG_CODE
	// Output debug summary of garbage collection
	debugC(kDebugLevelGC, "[GC] Summary:");
	for (int i = 0; i <= SEG_TYPE_MAX; i++)
		if (segcount[i])
			debugC(kDebugLevelGC, "\t%d\t* %s", segcount[i], segnames[i]);
#endif
}

} // End of namespace Sci
