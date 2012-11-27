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

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/event.h"
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/kernel_tables.h"
#include "sci/engine/state.h"
#include "sci/engine/workarounds.h"

#include "common/system.h"

namespace Sci {

Kernel::Kernel(ResourceManager *resMan, SegManager *segMan)
	: _resMan(resMan), _segMan(segMan), _invalid("<invalid>") {
	loadSelectorNames();
	mapSelectors();      // Map a few special selectors for later use
}

Kernel::~Kernel() {
	for (KernelFunctionArray::iterator it = _kernelFuncs.begin(); it != _kernelFuncs.end(); ++it) {
		if (it->subFunctionCount) {
			uint16 subFunctionNr = 0;
			while (subFunctionNr < it->subFunctionCount) {
				delete[] it->subFunctions[subFunctionNr].signature;
				subFunctionNr++;
			}
			delete[] it->subFunctions;
		}
		delete[] it->signature;
	}
}

uint Kernel::getSelectorNamesSize() const {
	return _selectorNames.size();
}

const Common::String &Kernel::getSelectorName(uint selector) {
	if (selector >= _selectorNames.size()) {
		// This should only occur in games w/o a selector-table
		//  We need this for proper workaround tables
		// TODO: maybe check, if there is a fixed selector-table and error() out in that case
		for (uint loopSelector = _selectorNames.size(); loopSelector <= selector; ++loopSelector)
			_selectorNames.push_back(Common::String::format("<noname%d>", loopSelector));
	}

	// Ensure that the selector has a name
	if (_selectorNames[selector].empty())
		_selectorNames[selector] = Common::String::format("<noname%d>", selector);

	return _selectorNames[selector];
}

uint Kernel::getKernelNamesSize() const {
	return _kernelNames.size();
}

const Common::String &Kernel::getKernelName(uint number) const {
	// FIXME: The following check is a temporary workaround for an issue
	// leading to crashes when using the debugger's backtrace command.
	if (number >= _kernelNames.size())
		return _invalid;
	return _kernelNames[number];
}

int Kernel::findKernelFuncPos(Common::String kernelFuncName) {
	for (uint32 i = 0; i < _kernelNames.size(); i++)
		if (_kernelNames[i] == kernelFuncName)
			return i;

	return -1;
}

int Kernel::findSelector(const char *selectorName) const {
	for (uint pos = 0; pos < _selectorNames.size(); ++pos) {
		if (_selectorNames[pos] == selectorName)
			return pos;
	}

	debugC(kDebugLevelVM, "Could not map '%s' to any selector", selectorName);

	return -1;
}

void Kernel::loadSelectorNames() {
	Resource *r = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SELECTORS), 0);
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	// Starting with KQ7, Mac versions have a BE name table. GK1 Mac and earlier (and all
	// other platforms) always use LE.
	bool isBE = (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_2_1
			&& g_sci->getGameId() != GID_GK1);

	if (!r) { // No such resource?
		// Check if we have a table for this game
		// Some demos do not have a selector table
		Common::StringArray staticSelectorTable = checkStaticSelectorNames();

		if (staticSelectorTable.empty())
			error("Kernel: Could not retrieve selector names");
		else
			warning("No selector vocabulary found, using a static one");

		for (uint32 i = 0; i < staticSelectorTable.size(); i++) {
			_selectorNames.push_back(staticSelectorTable[i]);
			if (oldScriptHeader)
				_selectorNames.push_back(staticSelectorTable[i]);
		}

		return;
	}

	int count = isBE ? READ_BE_UINT16(r->data) : READ_LE_UINT16(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = isBE ? READ_BE_UINT16(r->data + 2 + i * 2) : READ_LE_UINT16(r->data + 2 + i * 2);
		int len = isBE ? READ_BE_UINT16(r->data + offset) : READ_LE_UINT16(r->data + offset);

		Common::String tmp((const char *)r->data + offset + 2, len);
		_selectorNames.push_back(tmp);
		//debug("%s", tmp.c_str());

		// Early SCI versions used the LSB in the selector ID as a read/write
		// toggle. To compensate for that, we add every selector name twice.
		if (oldScriptHeader)
			_selectorNames.push_back(tmp);
	}
}

// this parses a written kernel signature into an internal memory format
// [io] -> either integer or object
// (io) -> optionally integer AND an object
// (i) -> optional integer
// . -> any type
// i* -> optional multiple integers
// .* -> any parameters afterwards (or none)
static uint16 *parseKernelSignature(const char *kernelName, const char *writtenSig) {
	const char *curPos;
	char curChar;
	uint16 *result = NULL;
	uint16 *writePos = NULL;
	int size = 0;
	bool validType = false;
	bool optionalType = false;
	bool eitherOr = false;
	bool optional = false;
	bool hadOptional = false;

	// No signature given? no signature out
	if (!writtenSig)
		return NULL;

	// First, we check how many bytes the result will be
	//  we also check, if the written signature makes any sense
	curPos = writtenSig;
	while (*curPos) {
		curChar = *curPos;
		switch (curChar) {
		case '[': // either or
			if (eitherOr)
				error("signature for k%s: '[' used within '[]'", kernelName);
			eitherOr = true;
			validType = false;
			break;
		case ']': // either or end
			if (!eitherOr)
				error("signature for k%s: ']' used without leading '['", kernelName);
			if (!validType)
				error("signature for k%s: '[]' does not surround valid type(s)", kernelName);
			eitherOr = false;
			validType = false;
			size++;
			break;
		case '(': // optional
			if (optional)
				error("signature for k%s: '(' used within '()' brackets", kernelName);
			if (eitherOr)
				error("signature for k%s: '(' used within '[]' brackets", kernelName);
			optional = true;
			validType = false;
			optionalType = false;
			break;
		case ')': // optional end
			if (!optional)
				error("signature for k%s: ')' used without leading '('", kernelName);
			if (!optionalType)
				error("signature for k%s: '()' does not to surround valid type(s)", kernelName);
			optional = false;
			validType = false;
			hadOptional = true;
			break;
		case '0': // allowed types
		case 'i':
		case 'o':
		case 'r':
		case 'l':
		case 'n':
		case '.':
		case '!':
			if ((hadOptional) & (!optional))
				error("signature for k%s: non-optional type may not follow optional type", kernelName);
			validType = true;
			if (optional)
				optionalType = true;
			if (!eitherOr)
				size++;
			break;
		case '*': // accepts more of the same parameter (must be last char)
			if (!validType) {
				if ((writtenSig == curPos) || (*(curPos - 1) != ']'))
					error("signature for k%s: a valid type must be in front of '*'", kernelName);
			}
			if (eitherOr)
				error("signature for k%s: '*' may not be inside '[]'", kernelName);
			if (optional) {
				if ((*(curPos + 1) != ')') || (*(curPos + 2) != 0))
					error("signature for k%s: '*' may only be used for last type", kernelName);
			} else {
				if (*(curPos + 1) != 0)
					error("signature for k%s: '*' may only be used for last type", kernelName);
			}
			break;
		default:
			error("signature for k%s: '%c' unknown", kernelName, *curPos);
		}
		curPos++;
	}

	uint16 signature = 0;

	// Now we allocate buffer with required size and fill it
	result = new uint16[size + 1];
	writePos = result;
	curPos = writtenSig;
	do {
		curChar = *curPos;
		if (!eitherOr) {
			// not within either-or, check if next character forces output
			switch (curChar) {
			case 0:
			case '[':
			case '(':
			case ')':
			case 'i':
			case 'o':
			case 'r':
			case 'l':
			case 'n':
			case '.':
			case '!':
				// and we also got some signature pending?
				if (signature) {
					if (!(signature & SIG_MAYBE_ANY))
						error("signature for k%s: invalid ('!') may only get used in combination with a real type", kernelName);
					if ((signature & SIG_IS_INVALID) && ((signature & SIG_MAYBE_ANY) == (SIG_TYPE_NULL | SIG_TYPE_INTEGER)))
						error("signature for k%s: invalid ('!') should not be used on exclusive null/integer type", kernelName);
					if (optional) {
						signature |= SIG_IS_OPTIONAL;
						if (curChar != ')')
							signature |= SIG_NEEDS_MORE;
					}
					*writePos = signature;
					writePos++;
					signature = 0;
				}
			}
		}
		switch (curChar) {
		case '[': // either or
			eitherOr = true;
			break;
		case ']': // either or end
			eitherOr = false;
			break;
		case '(': // optional
			optional = true;
			break;
		case ')': // optional end
			optional = false;
			break;
		case '0':
			if (signature & SIG_TYPE_NULL)
				error("signature for k%s: NULL ('0') specified more than once", kernelName);
			signature |= SIG_TYPE_NULL;
			break;
		case 'i':
			if (signature & SIG_TYPE_INTEGER)
				error("signature for k%s: integer ('i') specified more than once", kernelName);
			signature |= SIG_TYPE_INTEGER | SIG_TYPE_NULL;
			break;
		case 'o':
			if (signature & SIG_TYPE_OBJECT)
				error("signature for k%s: object ('o') specified more than once", kernelName);
			signature |= SIG_TYPE_OBJECT;
			break;
		case 'r':
			if (signature & SIG_TYPE_REFERENCE)
				error("signature for k%s: reference ('r') specified more than once", kernelName);
			signature |= SIG_TYPE_REFERENCE;
			break;
		case 'l':
			if (signature & SIG_TYPE_LIST)
				error("signature for k%s: list ('l') specified more than once", kernelName);
			signature |= SIG_TYPE_LIST;
			break;
		case 'n':
			if (signature & SIG_TYPE_NODE)
				error("signature for k%s: node ('n') specified more than once", kernelName);
			signature |= SIG_TYPE_NODE;
			break;
		case '.':
			if (signature & SIG_MAYBE_ANY)
				error("signature for k%s: maybe-any ('.') shouldn't get specified with other types in front of it", kernelName);
			signature |= SIG_MAYBE_ANY;
			break;
		case '!':
			if (signature & SIG_IS_INVALID)
				error("signature for k%s: invalid ('!') specified more than once", kernelName);
			signature |= SIG_IS_INVALID;
			break;
		case '*': // accepts more of the same parameter
			signature |= SIG_MORE_MAY_FOLLOW;
			break;
		default:
			break;
		}
		curPos++;
	} while (curChar);

	// Write terminator
	*writePos = 0;

	return result;
}

uint16 Kernel::findRegType(reg_t reg) {
	// No segment? Must be integer
	if (!reg.getSegment())
		return SIG_TYPE_INTEGER | (reg.getOffset() ? 0 : SIG_TYPE_NULL);

	if (reg.getSegment() == 0xFFFF)
		return SIG_TYPE_UNINITIALIZED;

	// Otherwise it's an object
	SegmentObj *mobj = _segMan->getSegmentObj(reg.getSegment());
	if (!mobj)
		return SIG_TYPE_ERROR;

	uint16 result = 0;
	if (!mobj->isValidOffset(reg.getOffset()))
		result |= SIG_IS_INVALID;

	switch (mobj->getType()) {
	case SEG_TYPE_SCRIPT:
		if (reg.getOffset() <= (*(Script *)mobj).getBufSize() &&
			reg.getOffset() >= (uint)-SCRIPT_OBJECT_MAGIC_OFFSET &&
			(*(Script *)mobj).offsetIsObject(reg.getOffset())) {
			result |= ((Script *)mobj)->getObject(reg.getOffset()) ? SIG_TYPE_OBJECT : SIG_TYPE_REFERENCE;
		} else
			result |= SIG_TYPE_REFERENCE;
		break;
	case SEG_TYPE_CLONES:
		result |= SIG_TYPE_OBJECT;
		break;
	case SEG_TYPE_LOCALS:
	case SEG_TYPE_STACK:
	case SEG_TYPE_DYNMEM:
	case SEG_TYPE_HUNK:
#ifdef ENABLE_SCI32
	case SEG_TYPE_ARRAY:
	case SEG_TYPE_STRING:
#endif
		result |= SIG_TYPE_REFERENCE;
		break;
	case SEG_TYPE_LISTS:
		result |= SIG_TYPE_LIST;
		break;
	case SEG_TYPE_NODES:
		result |= SIG_TYPE_NODE;
		break;
	default:
		return SIG_TYPE_ERROR;
	}
	return result;
}

struct SignatureDebugType {
	uint16 typeCheck;
	const char *text;
};

static const SignatureDebugType signatureDebugTypeList[] = {
	{ SIG_TYPE_NULL,          "null" },
	{ SIG_TYPE_INTEGER,       "integer" },
	{ SIG_TYPE_UNINITIALIZED, "uninitialized" },
	{ SIG_TYPE_OBJECT,        "object" },
	{ SIG_TYPE_REFERENCE,     "reference" },
	{ SIG_TYPE_LIST,          "list" },
	{ SIG_TYPE_NODE,          "node" },
	{ SIG_TYPE_ERROR,         "error" },
	{ SIG_IS_INVALID,         "invalid" },
	{ 0,                      NULL }
};

static void kernelSignatureDebugType(const uint16 type) {
	bool firstPrint = true;

	const SignatureDebugType *list = signatureDebugTypeList;
	while (list->typeCheck) {
		if (type & list->typeCheck) {
			if (!firstPrint)
				debugN(", ");
			debugN("%s", list->text);
			firstPrint = false;
		}
		list++;
	}
}

// Shows kernel call signature and current arguments for debugging purposes
void Kernel::signatureDebug(const uint16 *sig, int argc, const reg_t *argv) {
	int argnr = 0;
	while (*sig || argc) {
		debugN("parameter %d: ", argnr++);
		if (argc) {
			reg_t parameter = *argv;
			debugN("%04x:%04x (", PRINT_REG(parameter));
			int regType = findRegType(parameter);
			if (regType)
				kernelSignatureDebugType(regType);
			else
				debugN("unknown type of %04x:%04x", PRINT_REG(parameter));
			debugN(")");
			argv++;
			argc--;
		} else {
			debugN("not passed");
		}
		if (*sig) {
			const uint16 signature = *sig;
			if ((signature & SIG_MAYBE_ANY) == SIG_MAYBE_ANY) {
				debugN(", may be any");
			} else {
				debugN(", should be ");
				kernelSignatureDebugType(signature);
			}
			if (signature & SIG_IS_OPTIONAL)
				debugN(" (optional)");
			if (signature & SIG_NEEDS_MORE)
				debugN(" (needs more)");
			if (signature & SIG_MORE_MAY_FOLLOW)
				debugN(" (more may follow)");
			sig++;
		}
		debugN("\n");
	}
}

bool Kernel::signatureMatch(const uint16 *sig, int argc, const reg_t *argv) {
	uint16 nextSig = *sig;
	uint16 curSig = nextSig;
	while (nextSig && argc) {
		curSig = nextSig;
		int type = findRegType(*argv);

		if ((type & SIG_IS_INVALID) && (!(curSig & SIG_IS_INVALID)))
			return false; // pointer is invalid and signature doesn't allow that?

		if (!((type & ~SIG_IS_INVALID) & curSig)) {
			if ((type & ~SIG_IS_INVALID) == SIG_TYPE_ERROR && (curSig & SIG_IS_INVALID)) {
				// Type is unknown (error - usually because of a deallocated object or
				// stale pointer) and the signature allows invalid pointers. In this case,
				// ignore the invalid pointer.
			} else {
				return false; // type mismatch
			}
		}

		if (!(curSig & SIG_MORE_MAY_FOLLOW)) {
			sig++;
			nextSig = *sig;
		} else {
			nextSig |= SIG_IS_OPTIONAL; // more may follow -> assumes followers are optional
		}
		argv++;
		argc--;
	}

	// Too many arguments?
	if (argc)
		return false;
	// Signature end reached?
	if (nextSig == 0)
		return true;
	// current parameter is optional?
	if (curSig & SIG_IS_OPTIONAL) {
		// yes, check if nothing more is required
		if (!(curSig & SIG_NEEDS_MORE))
			return true;
	} else {
		// no, check if next parameter is optional
		if (nextSig & SIG_IS_OPTIONAL)
			return true;
	}
	// Too few arguments or more optional arguments required
	return false;
}

void Kernel::mapFunctions() {
	int mapped = 0;
	int ignored = 0;
	uint functionCount = _kernelNames.size();
	byte platformMask = 0;
	SciVersion myVersion = getSciVersion();

	switch (g_sci->getPlatform()) {
	case Common::kPlatformPC:
	case Common::kPlatformFMTowns:
		platformMask = SIGFOR_DOS;
		break;
	case Common::kPlatformPC98:
		platformMask = SIGFOR_PC98;
		break;
	case Common::kPlatformWindows:
		platformMask = SIGFOR_WIN;
		break;
	case Common::kPlatformMacintosh:
		platformMask = SIGFOR_MAC;
		break;
	case Common::kPlatformAmiga:
		platformMask = SIGFOR_AMIGA;
		break;
	case Common::kPlatformAtariST:
		platformMask = SIGFOR_ATARI;
		break;
	default:
		break;
	}

	_kernelFuncs.resize(functionCount);

	for (uint id = 0; id < functionCount; id++) {
		// First, get the name, if known, of the kernel function with number functnr
		Common::String kernelName = _kernelNames[id];

		// Reset the table entry
		_kernelFuncs[id].function = NULL;
		_kernelFuncs[id].signature = NULL;
		_kernelFuncs[id].name = NULL;
		_kernelFuncs[id].workarounds = NULL;
		_kernelFuncs[id].subFunctions = NULL;
		_kernelFuncs[id].subFunctionCount = 0;
		_kernelFuncs[id].debugLogging = false;
		if (kernelName.empty()) {
			// No name was given -> must be an unknown opcode
			warning("Kernel function %x unknown", id);
			continue;
		}

		// Don't map dummy functions - they will never be called
		if (kernelName == "Dummy") {
			_kernelFuncs[id].function = kDummy;
			continue;
		}

		// If the name is known, look it up in s_kernelMap. This table
		// maps kernel func names to actual function (pointers).
		SciKernelMapEntry *kernelMap = s_kernelMap;
		bool nameMatch = false;
		while (kernelMap->name) {
			if (kernelName == kernelMap->name) {
				if ((kernelMap->fromVersion == SCI_VERSION_NONE) || (kernelMap->fromVersion <= myVersion))
					if ((kernelMap->toVersion == SCI_VERSION_NONE) || (kernelMap->toVersion >= myVersion))
						if (platformMask & kernelMap->forPlatform)
							break;
				nameMatch = true;
			}
			kernelMap++;
		}

		if (kernelMap->name) {
			// A match was found
			_kernelFuncs[id].function = kernelMap->function;
			_kernelFuncs[id].name = kernelMap->name;
			_kernelFuncs[id].signature = parseKernelSignature(kernelMap->name, kernelMap->signature);
			_kernelFuncs[id].workarounds = kernelMap->workarounds;
			if (kernelMap->subFunctions) {
				// Get version for subfunction identification
				SciVersion mySubVersion = (SciVersion)kernelMap->function(NULL, 0, NULL).getOffset();
				// Now check whats the highest subfunction-id for this version
				const SciKernelMapSubEntry *kernelSubMap = kernelMap->subFunctions;
				uint16 subFunctionCount = 0;
				while (kernelSubMap->function) {
					if ((kernelSubMap->fromVersion == SCI_VERSION_NONE) || (kernelSubMap->fromVersion <= mySubVersion))
						if ((kernelSubMap->toVersion == SCI_VERSION_NONE) || (kernelSubMap->toVersion >= mySubVersion))
							if (subFunctionCount <= kernelSubMap->id)
								subFunctionCount = kernelSubMap->id + 1;
					kernelSubMap++;
				}
				if (!subFunctionCount)
					error("k%s[%x]: no subfunctions found for requested version", kernelName.c_str(), id);
				// Now allocate required memory and go through it again
				_kernelFuncs[id].subFunctionCount = subFunctionCount;
				KernelSubFunction *subFunctions = new KernelSubFunction[subFunctionCount];
				_kernelFuncs[id].subFunctions = subFunctions;
				memset(subFunctions, 0, sizeof(KernelSubFunction) * subFunctionCount);
				// And fill this info out
				kernelSubMap = kernelMap->subFunctions;
				uint kernelSubNr = 0;
				while (kernelSubMap->function) {
					if ((kernelSubMap->fromVersion == SCI_VERSION_NONE) || (kernelSubMap->fromVersion <= mySubVersion))
						if ((kernelSubMap->toVersion == SCI_VERSION_NONE) || (kernelSubMap->toVersion >= mySubVersion)) {
							uint subId = kernelSubMap->id;
							if (!subFunctions[subId].function) {
								subFunctions[subId].function = kernelSubMap->function;
								subFunctions[subId].name = kernelSubMap->name;
								subFunctions[subId].workarounds = kernelSubMap->workarounds;
								if (kernelSubMap->signature) {
									subFunctions[subId].signature = parseKernelSignature(kernelSubMap->name, kernelSubMap->signature);
								} else {
									// we go back the submap to find the previous signature for that kernel call
									const SciKernelMapSubEntry *kernelSubMapBack = kernelSubMap;
									uint kernelSubLeft = kernelSubNr;
									while (kernelSubLeft) {
										kernelSubLeft--;
										kernelSubMapBack--;
										if (kernelSubMapBack->name == kernelSubMap->name) {
											if (kernelSubMapBack->signature) {
												subFunctions[subId].signature = parseKernelSignature(kernelSubMap->name, kernelSubMapBack->signature);
												break;
											}
										}
									}
									if (!subFunctions[subId].signature)
										error("k%s: no previous signatures", kernelSubMap->name);
								}
							}
						}
					kernelSubMap++;
					kernelSubNr++;
				}
			}
			++mapped;
		} else {
			if (nameMatch)
				error("k%s[%x]: not found for this version/platform", kernelName.c_str(), id);
			// No match but a name was given -> stub
			warning("k%s[%x]: unmapped", kernelName.c_str(), id);
			_kernelFuncs[id].function = kStub;
		}
	} // for all functions requesting to be mapped

	debugC(kDebugLevelVM, "Handled %d/%d kernel functions, mapping %d and ignoring %d.",
				mapped + ignored, _kernelNames.size(), mapped, ignored);

	return;
}

bool Kernel::debugSetFunction(const char *kernelName, int logging, int breakpoint) {
	if (strcmp(kernelName, "*")) {
		for (uint id = 0; id < _kernelFuncs.size(); id++) {
			if (_kernelFuncs[id].name) {
				if (strcmp(kernelName, _kernelFuncs[id].name) == 0) {
					if (_kernelFuncs[id].subFunctions) {
						// sub-functions available and main name matched, in that case set logging of all sub-functions
						KernelSubFunction *kernelSubCall = _kernelFuncs[id].subFunctions;
						uint kernelSubCallCount = _kernelFuncs[id].subFunctionCount;
						for (uint subId = 0; subId < kernelSubCallCount; subId++) {
							if (kernelSubCall->function) {
								if (logging != -1)
									kernelSubCall->debugLogging = logging == 1 ? true : false;
								if (breakpoint != -1)
									kernelSubCall->debugBreakpoint = breakpoint == 1 ? true : false;
							}
							kernelSubCall++;
						}
						return true;
					}
					// function name matched, set for this one and exit
					if (logging != -1)
						_kernelFuncs[id].debugLogging = logging == 1 ? true : false;
					if (breakpoint != -1)
						_kernelFuncs[id].debugBreakpoint = breakpoint == 1 ? true : false;
					return true;
				} else {
					// main name was not matched
					if (_kernelFuncs[id].subFunctions) {
						// Sub-Functions available
						KernelSubFunction *kernelSubCall = _kernelFuncs[id].subFunctions;
						uint kernelSubCallCount = _kernelFuncs[id].subFunctionCount;
						for (uint subId = 0; subId < kernelSubCallCount; subId++) {
							if (kernelSubCall->function) {
								if (strcmp(kernelName, kernelSubCall->name) == 0) {
									// sub-function name matched, set for this one and exit
									if (logging != -1)
										kernelSubCall->debugLogging = logging == 1 ? true : false;
									if (breakpoint != -1)
										kernelSubCall->debugBreakpoint = breakpoint == 1 ? true : false;
									return true;
								}
							}
							kernelSubCall++;
						}
					}
				}
			}
		}
		return false;
	}
	// Set debugLogging for all calls
	for (uint id = 0; id < _kernelFuncs.size(); id++) {
		if (_kernelFuncs[id].name) {
			if (!_kernelFuncs[id].subFunctions) {
				// No sub-functions, enable actual kernel function
				if (logging != -1)
					_kernelFuncs[id].debugLogging = logging == 1 ? true : false;
				if (breakpoint != -1)
					_kernelFuncs[id].debugBreakpoint = breakpoint == 1 ? true : false;
			} else {
				// Sub-Functions available, enable those too
				KernelSubFunction *kernelSubCall = _kernelFuncs[id].subFunctions;
				uint kernelSubCallCount = _kernelFuncs[id].subFunctionCount;
				for (uint subId = 0; subId < kernelSubCallCount; subId++) {
					if (kernelSubCall->function) {
						if (logging != -1)
							kernelSubCall->debugLogging = logging == 1 ? true : false;
						if (breakpoint != -1)
							kernelSubCall->debugBreakpoint = breakpoint == 1 ? true : false;
					}
					kernelSubCall++;
				}
			}
		}
	}
	return true;
}

#ifdef ENABLE_SCI32
enum {
	kKernelEntriesSci2 = 0x8b,
	kKernelEntriesGk2Demo = 0xa0,
	kKernelEntriesSci21 = 0x9d,
	kKernelEntriesSci3 = 0xa1
};
#endif

void Kernel::loadKernelNames(GameFeatures *features) {
	_kernelNames.clear();

	if (getSciVersion() <= SCI_VERSION_1_1) {
		_kernelNames = Common::StringArray(s_defaultKernelNames, ARRAYSIZE(s_defaultKernelNames));

		// Some (later) SCI versions replaced CanBeHere by CantBeHere
		// If vocab.999 exists, the kernel function is still named CanBeHere
		if (_selectorCache.cantBeHere != -1)
			_kernelNames[0x4d] = "CantBeHere";
	}

	switch (getSciVersion()) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		// Insert SCI0 file functions after SetCursor (0x28)
		_kernelNames.insert_at(0x29, "FOpen");
		_kernelNames.insert_at(0x2A, "FPuts");
		_kernelNames.insert_at(0x2B, "FGets");
		_kernelNames.insert_at(0x2C, "FClose");

		// Function 0x55 is DoAvoider
		_kernelNames[0x55] = "DoAvoider";

		// Cut off unused functions
		_kernelNames.resize(0x72);
		break;

	case SCI_VERSION_01:
		// Multilingual SCI01 games have StrSplit as function 0x78
		_kernelNames[0x78] = "StrSplit";

		// Cut off unused functions
		_kernelNames.resize(0x79);
		break;

	case SCI_VERSION_1_LATE:
		_kernelNames[0x71] = "MoveCursor";
		break;

	case SCI_VERSION_1_1:
		// In SCI1.1, kSetSynonyms is an empty function
		_kernelNames[0x26] = "Empty";

		if (g_sci->getGameId() == GID_KQ6) {
			// In the Windows version of KQ6 CD, the empty kSetSynonyms
			// function has been replaced with kPortrait. In KQ6 Mac,
			// kPlayBack has been replaced by kShowMovie.
			if (g_sci->getPlatform() == Common::kPlatformWindows)
				_kernelNames[0x26] = "Portrait";
			else if (g_sci->getPlatform() == Common::kPlatformMacintosh)
				_kernelNames[0x84] = "ShowMovie";
		} else if (g_sci->getGameId() == GID_QFG4 && g_sci->isDemo()) {
			_kernelNames[0x7b] = "RemapColors"; // QFG4 Demo has this SCI2 function instead of StrSplit
		}

		_kernelNames[0x71] = "PalVary";

		// At least EcoQuest 1 demo uses kGetMessage instead of kMessage.
		// Detect which function to use.
		if (features->detectMessageFunctionType() == SCI_VERSION_1_1)
			_kernelNames[0x7c] = "Message";
		break;

#ifdef ENABLE_SCI32
	case SCI_VERSION_2:
		_kernelNames = Common::StringArray(sci2_default_knames, kKernelEntriesSci2);
		break;

	case SCI_VERSION_2_1:
		if (features->detectSci21KernelType() == SCI_VERSION_2) {
			// Some early SCI2.1 games use a modified SCI2 kernel table instead of
			// the SCI2.1 kernel table. We detect which version to use based on
			// how kDoSound is called from Sound::play().
			// Known games that use this:
			// GK2 demo
			// KQ7 1.4
			// PQ4 SWAT demo
			// LSL6
			// PQ4CD
			// QFG4CD

			// This is interesting because they all have the same interpreter
			// version (2.100.002), yet they would not be compatible with other
			// games of the same interpreter.

			_kernelNames = Common::StringArray(sci2_default_knames, kKernelEntriesGk2Demo);
			// OnMe is IsOnMe here, but they should be compatible
			_kernelNames[0x23] = "Robot"; // Graph in SCI2
			_kernelNames[0x2e] = "Priority"; // DisposeTextBitmap in SCI2
		} else {
			// Normal SCI2.1 kernel table
			_kernelNames = Common::StringArray(sci21_default_knames, kKernelEntriesSci21);
		}
		break;

	case SCI_VERSION_3:
		_kernelNames = Common::StringArray(sci21_default_knames, kKernelEntriesSci3);

		// In SCI3, some kernel functions have been removed, and others have been added
		_kernelNames[0x18] = "Dummy";	// AddMagnify in SCI2.1
		_kernelNames[0x19] = "Dummy";	// DeleteMagnify in SCI2.1
		_kernelNames[0x30] = "Dummy";	// SetScroll in SCI2.1
		_kernelNames[0x39] = "Dummy";	// ShowMovie in SCI2.1
		_kernelNames[0x4c] = "Dummy";	// ScrollWindow in SCI2.1
		_kernelNames[0x56] = "Dummy";	// VibrateMouse in SCI2.1 (only used in QFG4 floppy)
		_kernelNames[0x64] = "Dummy";	// AvoidPath in SCI2.1
		_kernelNames[0x66] = "Dummy";	// MergePoly in SCI2.1
		_kernelNames[0x8d] = "MessageBox";	// Dummy in SCI2.1
		_kernelNames[0x9b] = "Minimize";	// Dummy in SCI2.1

		break;
#endif

	default:
		// Use default table for the other versions
		break;
	}

	mapFunctions();
}

Common::String Kernel::lookupText(reg_t address, int index) {
	char *seeker;
	Resource *textres;

	if (address.getSegment())
		return _segMan->getString(address);

	int textlen;
	int _index = index;
	textres = _resMan->findResource(ResourceId(kResourceTypeText, address.getOffset()), 0);

	if (!textres) {
		error("text.%03d not found", address.getOffset());
		return NULL; /* Will probably segfault */
	}

	textlen = textres->size;
	seeker = (char *) textres->data;

	while (index--)
		while ((textlen--) && (*seeker++))
			;

	if (textlen)
		return seeker;

	error("Index %d out of bounds in text.%03d", _index, address.getOffset());
	return NULL;
}

// TODO: script_adjust_opcode_formats should probably be part of the
// constructor (?) of a VirtualMachine or a ScriptManager class.
void script_adjust_opcode_formats() {

	g_sci->_opcode_formats = new opcode_format[128][4];
	memcpy(g_sci->_opcode_formats, g_base_opcode_formats, 128*4*sizeof(opcode_format));

	if (g_sci->_features->detectLofsType() != SCI_VERSION_0_EARLY) {
		g_sci->_opcode_formats[op_lofsa][0] = Script_Offset;
		g_sci->_opcode_formats[op_lofss][0] = Script_Offset;
	}

#ifdef ENABLE_SCI32
	// In SCI32, some arguments are now words instead of bytes
	if (getSciVersion() >= SCI_VERSION_2) {
		g_sci->_opcode_formats[op_calle][2] = Script_Word;
		g_sci->_opcode_formats[op_callk][1] = Script_Word;
		g_sci->_opcode_formats[op_super][1] = Script_Word;
		g_sci->_opcode_formats[op_send][0] = Script_Word;
		g_sci->_opcode_formats[op_self][0] = Script_Word;
		g_sci->_opcode_formats[op_call][1] = Script_Word;
		g_sci->_opcode_formats[op_callb][1] = Script_Word;
	}

	if (getSciVersion() >= SCI_VERSION_3) {
		// TODO: There are also opcodes in
		// here to get the superclass, and possibly the species too.
		g_sci->_opcode_formats[0x4d/2][0] = Script_None;
		g_sci->_opcode_formats[0x4e/2][0] = Script_None;
	}
#endif
}

} // End of namespace Sci
