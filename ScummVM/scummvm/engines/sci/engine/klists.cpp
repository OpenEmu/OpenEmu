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

#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

namespace Sci {
//#define CHECK_LISTS	// adds sanity checking for lists and errors out when problems are found

#ifdef CHECK_LISTS

static bool isSaneNodePointer(SegManager *segMan, reg_t addr) {
	bool havePrev = false;
	reg_t prev = addr;

	do {
		Node *node = segMan->lookupNode(addr, false);

		if (!node) {
			if ((g_sci->getGameId() == GID_ICEMAN) && (g_sci->getEngineState()->currentRoomNumber() == 40)) {
				// ICEMAN: when plotting course, unDrawLast is called by startPlot::changeState
				//  there is no previous entry so we get 0 in here
			} else if ((g_sci->getGameId() == GID_HOYLE1) && (g_sci->getEngineState()->currentRoomNumber() == 3)) {
				// HOYLE1: after sorting cards in hearts, in the next round
				// we get an invalid node - bug #3038433
			} else {
				error("isSaneNodePointer: Node at %04x:%04x wasn't found", PRINT_REG(addr));
			}
			return false;
		}

		if (havePrev && node->pred != prev) {
			error("isSaneNodePointer: Node at %04x:%04x points to invalid predecessor %04x:%04x (should be %04x:%04x)",
					PRINT_REG(addr), PRINT_REG(node->pred), PRINT_REG(prev));

			//node->pred = prev;	// fix the problem in the node
			return false;
		}

		prev = addr;
		addr = node->succ;
		havePrev = true;
	} while (!addr.isNull());

	return true;
}

static void checkListPointer(SegManager *segMan, reg_t addr) {
	List *list = segMan->lookupList(addr);

	if (!list) {
		error("checkListPointer (list %04x:%04x): The requested list wasn't found",
				PRINT_REG(addr));
		return;
	}

	if (list->first.isNull() && list->last.isNull()) {
		// Empty list is fine
	} else if (!list->first.isNull() && !list->last.isNull()) {
		// Normal list
		Node *node_a = segMan->lookupNode(list->first, false);
		Node *node_z = segMan->lookupNode(list->last, false);

		if (!node_a) {
			error("checkListPointer (list %04x:%04x): missing first node", PRINT_REG(addr));
			return;
		}

		if (!node_z) {
			error("checkListPointer (list %04x:%04x): missing last node", PRINT_REG(addr));
			return;
		}

		if (!node_a->pred.isNull()) {
			error("checkListPointer (list %04x:%04x): First node of the list points to a predecessor node",
					PRINT_REG(addr));

			//node_a->pred = NULL_REG;	// fix the problem in the node

			return;
		}

		if (!node_z->succ.isNull()) {
			error("checkListPointer (list %04x:%04x): Last node of the list points to a successor node",
					PRINT_REG(addr));

			//node_z->succ = NULL_REG;	// fix the problem in the node

			return;
		}

		isSaneNodePointer(segMan, list->first);
	} else {
		// Not sane list... it's missing pointers to the first or last element
		if (list->first.isNull())
			error("checkListPointer (list %04x:%04x): missing pointer to first element",
					PRINT_REG(addr));
		if (list->last.isNull())
			error("checkListPointer (list %04x:%04x): missing pointer to last element",
					PRINT_REG(addr));
	}
}

#endif

reg_t kNewList(EngineState *s, int argc, reg_t *argv) {
	reg_t listRef;
	List *list = s->_segMan->allocateList(&listRef);
	list->first = list->last = NULL_REG;
	debugC(kDebugLevelNodes, "New listRef at %04x:%04x", PRINT_REG(listRef));

	return listRef; // Return list base address
}

reg_t kDisposeList(EngineState *s, int argc, reg_t *argv) {
	// This function is not needed in ScummVM. The garbage collector
	// cleans up unused objects automatically

	return s->r_acc;
}

reg_t kNewNode(EngineState *s, int argc, reg_t *argv) {
	reg_t nodeValue = argv[0];
	// Some SCI32 games call this with 1 parameter (e.g. the demo of Phantasmagoria).
	// Set the key to be the same as the value in this case
	reg_t nodeKey = (argc == 2) ? argv[1] : argv[0];
	s->r_acc = s->_segMan->newNode(nodeValue, nodeKey);

	debugC(kDebugLevelNodes, "New nodeRef at %04x:%04x", PRINT_REG(s->r_acc));

	return s->r_acc;
}

reg_t kFirstNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);

	if (list) {
#ifdef CHECK_LISTS
		checkListPointer(s->_segMan, argv[0]);
#endif
		return list->first;
	} else {
		return NULL_REG;
	}
}

reg_t kLastNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);

	if (list) {
#ifdef CHECK_LISTS
		checkListPointer(s->_segMan, argv[0]);
#endif
		return list->last;
	} else {
		return NULL_REG;
	}
}

reg_t kEmptyList(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);
#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, argv[0]);
#endif
	return make_reg(0, ((list) ? list->first.isNull() : 0));
}

static void addToFront(EngineState *s, reg_t listRef, reg_t nodeRef) {
	List *list = s->_segMan->lookupList(listRef);
	Node *newNode = s->_segMan->lookupNode(nodeRef);

	debugC(kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

	if (!newNode)
		error("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, listRef);
#endif

	newNode->pred = NULL_REG;
	newNode->succ = list->first;

	// Set node to be the first and last node if it's the only node of the list
	if (list->first.isNull())
		list->last = nodeRef;
	else {
		Node *oldNode = s->_segMan->lookupNode(list->first);
		oldNode->pred = nodeRef;
	}
	list->first = nodeRef;
}

static void addToEnd(EngineState *s, reg_t listRef, reg_t nodeRef) {
	List *list = s->_segMan->lookupList(listRef);
	Node *newNode = s->_segMan->lookupNode(nodeRef);

	debugC(kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

	if (!newNode)
		error("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, listRef);
#endif

	newNode->pred = list->last;
	newNode->succ = NULL_REG;

	// Set node to be the first and last node if it's the only node of the list
	if (list->last.isNull())
		list->first = nodeRef;
	else {
		Node *old_n = s->_segMan->lookupNode(list->last);
		old_n->succ = nodeRef;
	}
	list->last = nodeRef;
}

reg_t kNextNode(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);

#ifdef CHECK_LISTS
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;
#endif

	return n->succ;
}

reg_t kPrevNode(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);

#ifdef CHECK_LISTS
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;
#endif

	return n->pred;
}

reg_t kNodeValue(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);

#ifdef CHECK_LISTS
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;
#endif

	// ICEMAN: when plotting a course in room 40, unDrawLast is called by
	// startPlot::changeState, but there is no previous entry, so we get 0 here
	return n ? n->value : NULL_REG;
}

reg_t kAddToFront(EngineState *s, int argc, reg_t *argv) {
	addToFront(s, argv[0], argv[1]);

	if (argc == 3)
		s->_segMan->lookupNode(argv[1])->key = argv[2];

	return s->r_acc;
}

reg_t kAddToEnd(EngineState *s, int argc, reg_t *argv) {
	addToEnd(s, argv[0], argv[1]);

	if (argc == 3)
		s->_segMan->lookupNode(argv[1])->key = argv[2];

	return s->r_acc;
}

reg_t kAddAfter(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);
	Node *firstnode = argv[1].isNull() ? NULL : s->_segMan->lookupNode(argv[1]);
	Node *newnode = s->_segMan->lookupNode(argv[2]);

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, argv[0]);
#endif

	if (!newnode) {
		error("New 'node' %04x:%04x is not a node", PRINT_REG(argv[2]));
		return NULL_REG;
	}

	if (argc != 3 && argc != 4) {
		error("kAddAfter: Haven't got 3 or 4 arguments, aborting");
		return NULL_REG;
	}

	if (argc == 4)
		newnode->key = argv[3];

	if (firstnode) { // We're really appending after
		reg_t oldnext = firstnode->succ;

		newnode->pred = argv[1];
		firstnode->succ = argv[2];
		newnode->succ = oldnext;

		if (oldnext.isNull())  // Appended after last node?
			// Set new node as last list node
			list->last = argv[2];
		else
			s->_segMan->lookupNode(oldnext)->pred = argv[2];

	} else { // !firstnode
		addToFront(s, argv[0], argv[2]); // Set as initial list node
	}

	return s->r_acc;
}

reg_t kFindKey(EngineState *s, int argc, reg_t *argv) {
	reg_t node_pos;
	reg_t key = argv[1];
	reg_t list_pos = argv[0];

	debugC(kDebugLevelNodes, "Looking for key %04x:%04x in list %04x:%04x", PRINT_REG(key), PRINT_REG(list_pos));

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, argv[0]);
#endif

	node_pos = s->_segMan->lookupList(list_pos)->first;

	debugC(kDebugLevelNodes, "First node at %04x:%04x", PRINT_REG(node_pos));

	while (!node_pos.isNull()) {
		Node *n = s->_segMan->lookupNode(node_pos);
		if (n->key == key) {
			debugC(kDebugLevelNodes, " Found key at %04x:%04x", PRINT_REG(node_pos));
			return node_pos;
		}

		node_pos = n->succ;
		debugC(kDebugLevelNodes, "NextNode at %04x:%04x", PRINT_REG(node_pos));
	}

	debugC(kDebugLevelNodes, "Looking for key without success");
	return NULL_REG;
}

reg_t kDeleteKey(EngineState *s, int argc, reg_t *argv) {
	reg_t node_pos = kFindKey(s, 2, argv);
	Node *n;
	List *list = s->_segMan->lookupList(argv[0]);

	if (node_pos.isNull())
		return NULL_REG; // Signal failure

	n = s->_segMan->lookupNode(node_pos);
	if (list->first == node_pos)
		list->first = n->succ;
	if (list->last == node_pos)
		list->last = n->pred;

	if (!n->pred.isNull())
		s->_segMan->lookupNode(n->pred)->succ = n->succ;
	if (!n->succ.isNull())
		s->_segMan->lookupNode(n->succ)->pred = n->pred;

	// Erase references to the predecessor and successor nodes, as the game
	// scripts could reference the node itself again.
	// Happens in the intro of QFG1 and in Longbow, when exiting the cave.
	n->pred = NULL_REG;
	n->succ = NULL_REG;

	return make_reg(0, 1); // Signal success
}

struct sort_temp_t {
	reg_t key, value;
	reg_t order;
};

int sort_temp_cmp(const void *p1, const void *p2) {
	const sort_temp_t *st1 = (const sort_temp_t *)p1;
	const sort_temp_t *st2 = (const sort_temp_t *)p2;

	if (st1->order.getSegment() < st2->order.getSegment() ||
		(st1->order.getSegment() == st2->order.getSegment() &&
		st1->order.getOffset() < st2->order.getOffset()))
		return -1;

	if (st1->order.getSegment() > st2->order.getSegment() ||
		(st1->order.getSegment() == st2->order.getSegment() &&
		st1->order.getOffset() > st2->order.getOffset()))
		return 1;

	return 0;
}

reg_t kSort(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t source = argv[0];
	reg_t dest = argv[1];
	reg_t order_func = argv[2];

	int input_size = (int16)readSelectorValue(segMan, source, SELECTOR(size));
	reg_t input_data = readSelector(segMan, source, SELECTOR(elements));
	reg_t output_data = readSelector(segMan, dest, SELECTOR(elements));

	List *list;
	Node *node;

	if (!input_size)
		return s->r_acc;

	if (output_data.isNull()) {
		list = s->_segMan->allocateList(&output_data);
		list->first = list->last = NULL_REG;
		writeSelector(segMan, dest, SELECTOR(elements), output_data);
	}

	writeSelectorValue(segMan, dest, SELECTOR(size), input_size);

	list = s->_segMan->lookupList(input_data);
	node = s->_segMan->lookupNode(list->first);

	sort_temp_t *temp_array = (sort_temp_t *)malloc(sizeof(sort_temp_t) * input_size);

	int i = 0;
	while (node) {
		reg_t params[1] = { node->value };
		invokeSelector(s, order_func, SELECTOR(doit), argc, argv, 1, params);
		temp_array[i].key = node->key;
		temp_array[i].value = node->value;
		temp_array[i].order = s->r_acc;
		i++;
		node = s->_segMan->lookupNode(node->succ);
	}

	qsort(temp_array, input_size, sizeof(sort_temp_t), sort_temp_cmp);

	for (i = 0;i < input_size;i++) {
		reg_t lNode = s->_segMan->newNode(temp_array[i].value, temp_array[i].key);
		addToEnd(s, output_data, lNode);
	}

	free(temp_array);

	return s->r_acc;
}

// SCI32 list functions
#ifdef ENABLE_SCI32

reg_t kListAt(EngineState *s, int argc, reg_t *argv) {
	if (argc != 2) {
		error("kListAt called with %d parameters", argc);
		return NULL_REG;
	}

	List *list = s->_segMan->lookupList(argv[0]);
	reg_t curAddress = list->first;
	if (list->first.isNull()) {
		error("kListAt tried to reference empty list (%04x:%04x)", PRINT_REG(argv[0]));
		return NULL_REG;
	}
	Node *curNode = s->_segMan->lookupNode(curAddress);
	reg_t curObject = curNode->value;
	int16 listIndex = argv[1].toUint16();
	int curIndex = 0;

	while (curIndex != listIndex) {
		if (curNode->succ.isNull()) {	// end of the list?
			return NULL_REG;
		}

		curAddress = curNode->succ;
		curNode = s->_segMan->lookupNode(curAddress);
		curObject = curNode->value;

		curIndex++;
	}

	// Update the virtual file selected in the character import screen of QFG4.
	// For the SCI0-SCI1.1 version of this, check kDrawControl().
	if (g_sci->inQfGImportRoom() && !strcmp(s->_segMan->getObjectName(curObject), "SelectorDText"))
		s->_chosenQfGImportItem = listIndex;

	return curObject;
}

reg_t kListIndexOf(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	reg_t curAddress = list->first;
	Node *curNode = s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 curIndex = 0;

	while (curNode) {
		curObject = curNode->value;
		if (curObject == argv[1])
			return make_reg(0, curIndex);

		curAddress = curNode->succ;
		curNode = s->_segMan->lookupNode(curAddress);
		curIndex++;
	}

	return SIGNAL_REG;
}

reg_t kListEachElementDo(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	Node *curNode = s->_segMan->lookupNode(list->first);
	reg_t curObject;
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	while (curNode) {
		// We get the next node here as the current node might be gone after the invoke
		reg_t nextNode = curNode->succ;
		curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// This can only happen with 3 params (list, target selector, variable)
			if (argc != 3) {
				error("kListEachElementDo: Attempted to modify a variable selector with %d params", argc);
			} else {
				writeSelector(s->_segMan, curObject, slc, argv[2]);
			}
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);
		}

		curNode = s->_segMan->lookupNode(nextNode);
	}

	return s->r_acc;
}

reg_t kListFirstTrue(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	Node *curNode = s->_segMan->lookupNode(list->first);
	reg_t curObject;
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	s->r_acc = NULL_REG;	// reset the accumulator

	while (curNode) {
		reg_t nextNode = curNode->succ;
		curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// If it's a variable selector, check its value.
			// Example: script 64893 in Torin, MenuHandler::isHilited checks
			// all children for variable selector 0x03ba (bHilited).
			if (!readSelector(s->_segMan, curObject, slc).isNull())
				return curObject;
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);

			// Check if the result is true
			if (!s->r_acc.isNull())
				return curObject;
		}

		curNode = s->_segMan->lookupNode(nextNode);
	}

	// No selector returned true
	return NULL_REG;
}

reg_t kListAllTrue(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	Node *curNode = s->_segMan->lookupNode(list->first);
	reg_t curObject;
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	s->r_acc = make_reg(0, 1);	// reset the accumulator

	while (curNode) {
		reg_t nextNode = curNode->succ;
		curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// If it's a variable selector, check its value
			s->r_acc = readSelector(s->_segMan, curObject, slc);
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);
		}

		// Check if the result isn't true
		if (s->r_acc.isNull())
			break;

		curNode = s->_segMan->lookupNode(nextNode);
	}

	return s->r_acc;
}

reg_t kList(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kAddBefore(EngineState *s, int argc, reg_t *argv) {
	error("Unimplemented function kAddBefore called");
	return s->r_acc;
}

reg_t kMoveToFront(EngineState *s, int argc, reg_t *argv) {
	error("Unimplemented function kMoveToFront called");
	return s->r_acc;
}

reg_t kMoveToEnd(EngineState *s, int argc, reg_t *argv) {
	error("Unimplemented function kMoveToEnd called");
	return s->r_acc;
}

reg_t kArray(EngineState *s, int argc, reg_t *argv) {
	uint16 op = argv[0].toUint16();

	// Use kString when accessing strings
	// This is possible, as strings inherit from arrays
	// and in this case (type 3) arrays are of type char *.
	// kString is almost exactly the same as kArray, so
	// this call is possible
	// TODO: we need to either merge SCI2 strings and
	// arrays together, and in the future merge them with
	// the SCI1 strings and arrays in the segment manager
	if (op == 0) {
		// New, check if the target type is 3 (string)
		if (argv[2].toUint16() == 3)
			return kString(s, argc, argv);
	} else {
		if (s->_segMan->getSegmentType(argv[1].getSegment()) == SEG_TYPE_STRING ||
			s->_segMan->getSegmentType(argv[1].getSegment()) == SEG_TYPE_SCRIPT) {
			return kString(s, argc, argv);
		}

#if 0
		if (op == 6) {
			if (s->_segMan->getSegmentType(argv[3].getSegment()) == SEG_TYPE_STRING ||
				s->_segMan->getSegmentType(argv[3].getSegment()) == SEG_TYPE_SCRIPT) {
				return kString(s, argc, argv);
			}
		}
#endif
	}

	switch (op) {
	case 0: { // New
		reg_t arrayHandle;
		SciArray<reg_t> *array = s->_segMan->allocateArray(&arrayHandle);
		array->setType(argv[2].toUint16());
		array->setSize(argv[1].toUint16());
		return arrayHandle;
	}
	case 1: { // Size
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		return make_reg(0, array->getSize());
	}
	case 2: { // At (return value at an index)
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
			// HACK: Phantasmagoria 2 keeps trying to access past the end of an
			// array when it starts. I'm assuming it's trying to see where the
			// array ends, or tries to resize it. Adjust the array size
			// accordingly, and return NULL for now.
			if (array->getSize() == argv[2].toUint16()) {
				array->setSize(argv[2].toUint16());
				return NULL_REG;
			}
		}
		return array->getValue(argv[2].toUint16());
	}
	case 3: { // Atput (put value at an index)
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);

		uint32 index = argv[2].toUint16();
		uint32 count = argc - 3;

		if (index + count > 65535)
			break;

		if (array->getSize() < index + count)
			array->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			array->setValue(i + index, argv[i + 3]);

		return argv[1]; // We also have to return the handle
	}
	case 4: // Free
		// Freeing of arrays is handled by the garbage collector
		return s->r_acc;
	case 5: { // Fill
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		uint16 index = argv[2].toUint16();

		// A count of -1 means fill the rest of the array
		uint16 count = argv[3].toSint16() == -1 ? array->getSize() - index : argv[3].toUint16();
		uint16 arraySize = array->getSize();

		if (arraySize < index + count)
			array->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			array->setValue(i + index, argv[4]);

		return argv[1];
	}
	case 6: { // Cpy
		if (argv[1].isNull() || argv[3].isNull()) {
			if (getSciVersion() == SCI_VERSION_3) {
				// FIXME: Happens in SCI3, probably because of a missing kernel function.
				warning("kArray(Cpy): Request to copy from or to a null pointer");
				return NULL_REG;
			} else {
				// SCI2-2.1: error out
				error("kArray(Cpy): Request to copy from or to a null pointer");
			}
		}

		reg_t arrayHandle = argv[1];
		SciArray<reg_t> *array1 = s->_segMan->lookupArray(argv[1]);
		SciArray<reg_t> *array2 = s->_segMan->lookupArray(argv[3]);
		uint32 index1 = argv[2].toUint16();
		uint32 index2 = argv[4].toUint16();

		// The original engine ignores bad copies too
		if (index2 > array2->getSize())
			break;

		// A count of -1 means fill the rest of the array
		uint32 count = argv[5].toSint16() == -1 ? array2->getSize() - index2 : argv[5].toUint16();

		if (array1->getSize() < index1 + count)
			array1->setSize(index1 + count);

		for (uint16 i = 0; i < count; i++)
			array1->setValue(i + index1, array2->getValue(i + index2));

		return arrayHandle;
	}
	case 7: // Cmp
		// Not implemented in SSCI
		warning("kArray(Cmp) called");
		return s->r_acc;
	case 8: { // Dup
		if (argv[1].isNull()) {
			warning("kArray(Dup): Request to duplicate a null pointer");
#if 0
			// Allocate an array anyway
			reg_t arrayHandle;
			SciArray<reg_t> *dupArray = s->_segMan->allocateArray(&arrayHandle);
			dupArray->setType(3);
			dupArray->setSize(0);
			return arrayHandle;
#endif
			return NULL_REG;
		}
		if (s->_segMan->getSegmentObj(argv[1].getSegment())->getType() != SEG_TYPE_ARRAY)
			error("kArray(Dup): Request to duplicate a segment which isn't an array");

		reg_t arrayHandle;
		SciArray<reg_t> *dupArray = s->_segMan->allocateArray(&arrayHandle);
		// This must occur after allocateArray, as inserting a new object
		// in the heap object list might invalidate this pointer. Also refer
		// to the same issue in kClone()
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);

		dupArray->setType(array->getType());
		dupArray->setSize(array->getSize());

		for (uint32 i = 0; i < array->getSize(); i++)
			dupArray->setValue(i, array->getValue(i));

		return arrayHandle;
	}
	case 9: // Getdata
		if (!s->_segMan->isHeapObject(argv[1]))
			return argv[1];

		return readSelector(s->_segMan, argv[1], SELECTOR(data));
	default:
		error("Unknown kArray subop %d", op);
	}

	return NULL_REG;
}

#endif

} // End of namespace Sci
