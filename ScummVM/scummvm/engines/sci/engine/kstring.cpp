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

/* String and parser handling */

#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/message.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

namespace Sci {

reg_t kStrEnd(EngineState *s, int argc, reg_t *argv) {
	reg_t address = argv[0];
	address.incOffset(s->_segMan->strlen(address));

	return address;
}

reg_t kStrCat(EngineState *s, int argc, reg_t *argv) {
	Common::String s1 = s->_segMan->getString(argv[0]);
	Common::String s2 = s->_segMan->getString(argv[1]);

	// The Japanese version of PQ2 splits the two strings here
	// (check bug #3396887).
	if (g_sci->getGameId() == GID_PQ2 &&
		g_sci->getLanguage() == Common::JA_JPN) {
		s1 = g_sci->strSplit(s1.c_str(), NULL);
		s2 = g_sci->strSplit(s2.c_str(), NULL);
	}

	s1 += s2;
	s->_segMan->strcpy(argv[0], s1.c_str());
	return argv[0];
}

reg_t kStrCmp(EngineState *s, int argc, reg_t *argv) {
	Common::String s1 = s->_segMan->getString(argv[0]);
	Common::String s2 = s->_segMan->getString(argv[1]);

	if (argc > 2)
		return make_reg(0, strncmp(s1.c_str(), s2.c_str(), argv[2].toUint16()));
	else
		return make_reg(0, strcmp(s1.c_str(), s2.c_str()));
}


reg_t kStrCpy(EngineState *s, int argc, reg_t *argv) {
	if (argc > 2) {
		int length = argv[2].toSint16();

		if (length >= 0)
			s->_segMan->strncpy(argv[0], argv[1], length);
		else
			s->_segMan->memcpy(argv[0], argv[1], -length);
	} else {
		s->_segMan->strcpy(argv[0], argv[1]);
	}

	return argv[0];
}


reg_t kStrAt(EngineState *s, int argc, reg_t *argv) {
	if (argv[0] == SIGNAL_REG) {
		warning("Attempt to perform kStrAt() on a signal reg");
		return NULL_REG;
	}

	SegmentRef dest_r = s->_segMan->dereference(argv[0]);
	if (!dest_r.isValid()) {
		warning("Attempt to StrAt at invalid pointer %04x:%04x", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	byte value;
	byte newvalue = 0;
	uint16 offset = argv[1].toUint16();
	if (argc > 2)
		newvalue = argv[2].toSint16();

	// in kq5 this here gets called with offset 0xFFFF
	//  (in the desert wheng getting the staff)
	if ((int)offset >= dest_r.maxSize) {
		warning("kStrAt offset %X exceeds maxSize", offset);
		return s->r_acc;
	}

	// FIXME: Move this to segman
	if (dest_r.isRaw) {
		value = dest_r.raw[offset];
		if (argc > 2) /* Request to modify this char */
			dest_r.raw[offset] = newvalue;
	} else {
		if (dest_r.skipByte)
			offset++;

		reg_t &tmp = dest_r.reg[offset / 2];

		bool oddOffset = offset & 1;
		if (g_sci->isBE())
			oddOffset = !oddOffset;

		if (!oddOffset) {
			value = tmp.getOffset() & 0x00ff;
			if (argc > 2) { /* Request to modify this char */
				uint16 tmpOffset = tmp.toUint16();
				tmpOffset &= 0xff00;
				tmpOffset |= newvalue;
				tmp.setOffset(tmpOffset);
				tmp.setSegment(0);
			}
		} else {
			value = tmp.getOffset() >> 8;
			if (argc > 2)  { /* Request to modify this char */
				uint16 tmpOffset = tmp.toUint16();
				tmpOffset &= 0x00ff;
				tmpOffset |= newvalue << 8;
				tmp.setOffset(tmpOffset);
				tmp.setSegment(0);
			}
		}
	}

	return make_reg(0, value);
}


reg_t kReadNumber(EngineState *s, int argc, reg_t *argv) {
	Common::String source_str = s->_segMan->getString(argv[0]);
	const char *source = source_str.c_str();

	while (Common::isSpace(*source))
		source++; /* Skip whitespace */

	int16 result = 0;

	if (*source == '$') {
		// Hexadecimal input
		result = (int16)strtol(source + 1, NULL, 16);
	} else {
		// Decimal input. We can not use strtol/atoi in here, because while
		// Sierra used atoi, it was a non standard compliant atoi, that didn't
		// do clipping. In SQ4 we get the door code in here and that's even
		// larger than uint32!
		if (*source == '-') {
			// FIXME: Setting result to -1 does _not_ negate the output.
			result = -1;
			source++;
		}
		while (*source) {
			if ((*source < '0') || (*source > '9'))
				// Stop if we encounter anything other than a digit (like atoi)
				break;
			result *= 10;
			result += *source - 0x30;
			source++;
		}
	}

	return make_reg(0, result);
}


#define ALIGN_NONE 0
#define ALIGN_RIGHT 1
#define ALIGN_LEFT -1
#define ALIGN_CENTER 2

/*  Format(targ_address, textresnr, index_inside_res, ...)
** or
**  Format(targ_address, heap_text_addr, ...)
** Formats the text from text.textresnr (offset index_inside_res) or heap_text_addr according to
** the supplied parameters and writes it to the targ_address.
*/
reg_t kFormat(EngineState *s, int argc, reg_t *argv) {
	uint16 *arguments;
	reg_t dest = argv[0];
	int maxsize = 4096; /* Arbitrary... */
	char targetbuf[4096];
	char *target = targetbuf;
	reg_t position = argv[1]; /* source */
	int mode = 0;
	int paramindex = 0; /* Next parameter to evaluate */
	char xfer;
	int i;
	int startarg;
	int strLength = 0; /* Used for stuff like "%13s" */
	bool unsignedVar = false;

	if (position.getSegment())
		startarg = 2;
	else {
		// WORKAROUND: QFG1 VGA Mac calls this without the first parameter (dest). It then
		// treats the source as the dest and overwrites the source string with an empty string.
		if (argc < 3)
			return NULL_REG;

		startarg = 3; /* First parameter to use for formatting */
	}

	int index = (startarg == 3) ? argv[2].toUint16() : 0;
	Common::String source_str = g_sci->getKernel()->lookupText(position, index);
	const char* source = source_str.c_str();

	debugC(kDebugLevelStrings, "Formatting \"%s\"", source);


	arguments = (uint16 *)malloc(sizeof(uint16) * argc);
	memset(arguments, 0, sizeof(uint16) * argc);

	for (i = startarg; i < argc; i++)
		arguments[i-startarg] = argv[i].toUint16(); /* Parameters are copied to prevent overwriting */

	while ((xfer = *source++)) {
		if (xfer == '%') {
			if (mode == 1) {
				assert((target - targetbuf) + 2 <= maxsize);
				*target++ = '%'; /* Literal % by using "%%" */
				mode = 0;
			} else {
				mode = 1;
				strLength = 0;
			}
		} else if (mode == 1) { /* xfer != '%' */
			char fillchar = ' ';
			int align = ALIGN_NONE;

			char *writestart = target; /* Start of the written string, used after the switch */

			/* int writelength; -- unused atm */

			if (xfer && (Common::isDigit(xfer) || xfer == '-' || xfer == '=')) {
				char *destp;

				if (xfer == '0')
					fillchar = '0';
				else if (xfer == '=')
					align = ALIGN_CENTER;
				else if (Common::isDigit(xfer) || (xfer == '-'))
					source--; // Go to start of length argument

				strLength = strtol(source, &destp, 10);

				if (destp > source)
					source = destp;

				if (strLength < 0) {
					align = ALIGN_LEFT;
					strLength = -strLength;
				} else if (align != ALIGN_CENTER)
					align = ALIGN_RIGHT;

				xfer = *source++;
			} else
				strLength = 0;

			assert((target - targetbuf) + strLength + 1 <= maxsize);

			switch (xfer) {
			case 's': { /* Copy string */
				reg_t reg = argv[startarg + paramindex];

#ifdef ENABLE_SCI32
				// If the string is a string object, get to the actual string in the data selector
				if (s->_segMan->isObject(reg))
					reg = readSelector(s->_segMan, reg, SELECTOR(data));
#endif

				Common::String tempsource = g_sci->getKernel()->lookupText(reg,
				                                  arguments[paramindex + 1]);
				int slen = strlen(tempsource.c_str());
				int extralen = strLength - slen;
				assert((target - targetbuf) + extralen <= maxsize);
				if (extralen < 0)
					extralen = 0;

				if (reg.getSegment()) /* Heap address? */
					paramindex++;
				else
					paramindex += 2; /* No, text resource address */

				switch (align) {

				case ALIGN_NONE:
				case ALIGN_RIGHT:
					while (extralen-- > 0)
						*target++ = ' '; /* Format into the text */
					break;

				case ALIGN_CENTER: {
					int half_extralen = extralen >> 1;
					while (half_extralen-- > 0)
						*target++ = ' '; /* Format into the text */
					break;
				}

				default:
					break;

				}

				strcpy(target, tempsource.c_str());
				target += slen;

				switch (align) {

				case ALIGN_CENTER: {
					int half_extralen;
					align = 0;
					half_extralen = extralen - (extralen >> 1);
					while (half_extralen-- > 0)
						*target++ = ' '; /* Format into the text */
					break;
				}

				default:
					break;

				}

				mode = 0;
			}
			break;

			case 'c': { /* insert character */
				assert((target - targetbuf) + 2 <= maxsize);
				if (align >= 0)
					while (strLength-- > 1)
						*target++ = ' '; /* Format into the text */
				char argchar = arguments[paramindex++];
				if (argchar)
					*target++ = argchar;
				mode = 0;
			}
			break;

			case 'x':
			case 'u':
				unsignedVar = true;
			case 'd': { /* Copy decimal */
				// In the new SCI2 kString function, %d is used for unsigned
				// integers. An example is script 962 in Shivers - it uses %d
				// to create file names.
				if (getSciVersion() >= SCI_VERSION_2)
					unsignedVar = true;

				/* int templen; -- unused atm */
				const char *format_string = "%d";

				if (xfer == 'x')
					format_string = "%x";

				int val = arguments[paramindex];
				if (!unsignedVar)
					val = (int16)arguments[paramindex];

				target += sprintf(target, format_string, val);
				paramindex++;
				assert((target - targetbuf) <= maxsize);

				unsignedVar = false;

				mode = 0;
			}
			break;
			default:
				*target = '%';
				target++;
				*target = xfer;
				target++;
				mode = 0;
			}

			if (align) {
				int written = target - writestart;
				int padding = strLength - written;

				if (padding > 0) {
					if (align > 0) {
						memmove(writestart + padding,
						        writestart, written);
						memset(writestart, fillchar, padding);
					} else {
						memset(target, ' ', padding);
					}
					target += padding;
				}
			}
		} else { /* mode != 1 */
			*target = xfer;
			target++;
		}
	}

	free(arguments);

	*target = 0; /* Terminate string */

#ifdef ENABLE_SCI32
	// Resize SCI32 strings if necessary
	if (getSciVersion() >= SCI_VERSION_2) {
		SciString *string = s->_segMan->lookupString(dest);
		string->setSize(strlen(targetbuf) + 1);
	}
#endif

	s->_segMan->strcpy(dest, targetbuf);

	return dest; /* Return target addr */
}

reg_t kStrLen(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, s->_segMan->strlen(argv[0]));
}


reg_t kGetFarText(EngineState *s, int argc, reg_t *argv) {
	Resource *textres = g_sci->getResMan()->findResource(ResourceId(kResourceTypeText, argv[0].toUint16()), 0);
	char *seeker;
	int counter = argv[1].toUint16();

	if (!textres) {
		error("text.%d does not exist", argv[0].toUint16());
		return NULL_REG;
	}

	seeker = (char *)textres->data;

	// The second parameter (counter) determines the number of the string
	// inside the text resource.
	while (counter--) {
		while (*seeker++)
			;
	}

	// If the third argument is NULL, allocate memory for the destination. This
	// occurs in SCI1 Mac games. The memory will later be freed by the game's
	// scripts.
	if (argv[2] == NULL_REG)
		s->_segMan->allocDynmem(strlen(seeker) + 1, "Mac FarText", &argv[2]);

	s->_segMan->strcpy(argv[2], seeker); // Copy the string and get return value
	return argv[2];
}

#define DUMMY_MESSAGE "Message not found!"

enum kMessageFunc {
	K_MESSAGE_GET,
	K_MESSAGE_NEXT,
	K_MESSAGE_SIZE,
	K_MESSAGE_REFCOND,
	K_MESSAGE_REFVERB,
	K_MESSAGE_REFNOUN,
	K_MESSAGE_PUSH,
	K_MESSAGE_POP,
	K_MESSAGE_LASTMESSAGE
};

reg_t kGetMessage(EngineState *s, int argc, reg_t *argv) {
	MessageTuple tuple = MessageTuple(argv[0].toUint16(), argv[2].toUint16());

	s->_msgState->getMessage(argv[1].toUint16(), tuple, argv[3]);

	return argv[3];
}

reg_t kMessage(EngineState *s, int argc, reg_t *argv) {
	uint func = argv[0].toUint16();

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		// In complete weirdness, SCI32 bumps up subops 3-8 to 4-9 and stubs off subop 3.
		// In addition, SCI32 reorders the REF* subops.
		if (func == 3)
			error("SCI32 kMessage(3)");
		else if (func > 3) {
			func--;
			if (func == K_MESSAGE_REFCOND)
				func = K_MESSAGE_REFNOUN;
			else if (func == K_MESSAGE_REFNOUN || func == K_MESSAGE_REFVERB)
				func--;
		}
	}
#endif

//	TODO: Perhaps fix this check, currently doesn't work with PUSH and POP subfunctions
//	Pepper uses them to to handle the glossary
//	if ((func != K_MESSAGE_NEXT) && (argc < 2)) {
//		warning("Message: not enough arguments passed to subfunction %d", func);
//		return NULL_REG;
//	}

	MessageTuple tuple;

	if (argc >= 6)
		tuple = MessageTuple(argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(), argv[5].toUint16());

	switch (func) {
	case K_MESSAGE_GET:
		return make_reg(0, s->_msgState->getMessage(argv[1].toUint16(), tuple, (argc == 7 ? argv[6] : NULL_REG)));
	case K_MESSAGE_NEXT:
		return make_reg(0, s->_msgState->nextMessage((argc == 2 ? argv[1] : NULL_REG)));
	case K_MESSAGE_SIZE:
		return make_reg(0, s->_msgState->messageSize(argv[1].toUint16(), tuple));
	case K_MESSAGE_REFCOND:
	case K_MESSAGE_REFVERB:
	case K_MESSAGE_REFNOUN: {
		MessageTuple t;

		if (s->_msgState->messageRef(argv[1].toUint16(), tuple, t)) {
			switch (func) {
			case K_MESSAGE_REFCOND:
				return make_reg(0, t.cond);
			case K_MESSAGE_REFVERB:
				return make_reg(0, t.verb);
			case K_MESSAGE_REFNOUN:
				return make_reg(0, t.noun);
			}
		}

		return SIGNAL_REG;
	}
	case K_MESSAGE_LASTMESSAGE: {
		MessageTuple msg;
		int module;

		s->_msgState->lastQuery(module, msg);

		bool ok = false;

		if (s->_segMan->dereference(argv[1]).isRaw) {
			byte *buffer = s->_segMan->derefBulkPtr(argv[1], 10);

			if (buffer) {
				ok = true;
				WRITE_LE_UINT16(buffer, module);
				WRITE_LE_UINT16(buffer + 2, msg.noun);
				WRITE_LE_UINT16(buffer + 4, msg.verb);
				WRITE_LE_UINT16(buffer + 6, msg.cond);
				WRITE_LE_UINT16(buffer + 8, msg.seq);
			}
		} else {
			reg_t *buffer = s->_segMan->derefRegPtr(argv[1], 5);

			if (buffer) {
				ok = true;
				buffer[0] = make_reg(0, module);
				buffer[1] = make_reg(0, msg.noun);
				buffer[2] = make_reg(0, msg.verb);
				buffer[3] = make_reg(0, msg.cond);
				buffer[4] = make_reg(0, msg.seq);
			}
		}

		if (!ok)
			warning("Message: buffer %04x:%04x invalid or too small to hold the tuple", PRINT_REG(argv[1]));

		return NULL_REG;
	}
	case K_MESSAGE_PUSH:
		s->_msgState->pushCursorStack();
		break;
	case K_MESSAGE_POP:
		s->_msgState->popCursorStack();
		break;
	default:
		warning("Message: subfunction %i invoked (not implemented)", func);
	}

	return NULL_REG;
}

reg_t kSetQuitStr(EngineState *s, int argc, reg_t *argv) {
	//Common::String quitStr = s->_segMan->getString(argv[0]);
	//debug("Setting quit string to '%s'", quitStr.c_str());
	return s->r_acc;
}

reg_t kStrSplit(EngineState *s, int argc, reg_t *argv) {
	Common::String format = s->_segMan->getString(argv[1]);
	Common::String sep_str;
	const char *sep = NULL;
	if (!argv[2].isNull()) {
		sep_str = s->_segMan->getString(argv[2]);
		sep = sep_str.c_str();
	}
	Common::String str = g_sci->strSplit(format.c_str(), sep);

	// Make sure target buffer is large enough
	SegmentRef buf_r = s->_segMan->dereference(argv[0]);
	if (!buf_r.isValid() || buf_r.maxSize < (int)str.size() + 1) {
		warning("StrSplit: buffer %04x:%04x invalid or too small to hold the following text of %i bytes: '%s'",
						PRINT_REG(argv[0]), str.size() + 1, str.c_str());
		return NULL_REG;
	}
	s->_segMan->strcpy(argv[0], str.c_str());
	return argv[0];
}

#ifdef ENABLE_SCI32

reg_t kText(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0:
		return kTextSize(s, argc - 1, argv + 1);
	default:
		// TODO: Other subops here too, perhaps kTextColors and kTextFonts
		warning("kText(%d)", argv[0].toUint16());
		break;
	}

	return s->r_acc;
}

reg_t kString(EngineState *s, int argc, reg_t *argv) {
	uint16 op = argv[0].toUint16();

	if (g_sci->_features->detectSci2StringFunctionType() == kSci2StringFunctionNew) {
		if (op >= 8)	// Dup, GetData have been removed
			op += 2;
	}

	switch (op) {
	case 0: { // New
		reg_t stringHandle;
		SciString *string = s->_segMan->allocateString(&stringHandle);
		string->setSize(argv[1].toUint16());

		// Make sure the first character is a null character
		if (string->getSize() > 0)
			string->setValue(0, 0);

		return stringHandle;
		}
	case 1: // Size
		return make_reg(0, s->_segMan->getString(argv[1]).size());
	case 2: { // At (return value at an index)
		// Note that values are put in bytes to avoid sign extension
		if (argv[1].getSegment() == s->_segMan->getStringSegmentId()) {
			SciString *string = s->_segMan->lookupString(argv[1]);
			byte val = string->getRawData()[argv[2].toUint16()];
			return make_reg(0, val);
		} else {
			Common::String string = s->_segMan->getString(argv[1]);
			byte val = string[argv[2].toUint16()];
			return make_reg(0, val);
		}
	}
	case 3: { // Atput (put value at an index)
		SciString *string = s->_segMan->lookupString(argv[1]);

		uint32 index = argv[2].toUint16();
		uint32 count = argc - 3;

		if (index + count > 65535)
			break;

		if (string->getSize() < index + count)
			string->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			string->setValue(i + index, argv[i + 3].toUint16());

		return argv[1]; // We also have to return the handle
	}
	case 4: // Free
		// Freeing of strings is handled by the garbage collector
		return s->r_acc;
	case 5: { // Fill
		SciString *string = s->_segMan->lookupString(argv[1]);
		uint16 index = argv[2].toUint16();

		// A count of -1 means fill the rest of the array
		uint16 count = argv[3].toSint16() == -1 ? string->getSize() - index : argv[3].toUint16();
		uint16 stringSize = string->getSize();

		if (stringSize < index + count)
			string->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			string->setValue(i + index, argv[4].toUint16());

		return argv[1];
	}
	case 6: { // Cpy
		const char *string2 = 0;
		uint32 string2Size = 0;
		Common::String string;

		if (argv[3].getSegment() == s->_segMan->getStringSegmentId()) {
			SciString *sstr;
			sstr = s->_segMan->lookupString(argv[3]);
			string2 = sstr->getRawData();
			string2Size = sstr->getSize();
		} else {
			string = s->_segMan->getString(argv[3]);
			string2 = string.c_str();
			string2Size = string.size() + 1;
		}

		uint32 index1 = argv[2].toUint16();
		uint32 index2 = argv[4].toUint16();

		// The original engine ignores bad copies too
		if (index2 > string2Size)
			break;

		// A count of -1 means fill the rest of the array
		uint32 count = argv[5].toSint16() == -1 ? string2Size - index2 + 1 : argv[5].toUint16();
		reg_t strAddress = argv[1];

		SciString *string1 = s->_segMan->lookupString(argv[1]);
		//SciString *string1 = !argv[1].isNull() ? s->_segMan->lookupString(argv[1]) : s->_segMan->allocateString(&strAddress);

		if (string1->getSize() < index1 + count)
			string1->setSize(index1 + count);

		// Note: We're accessing from c_str() here because the
		// string's size ignores the trailing 0 and therefore
		// triggers an assert when doing string2[i + index2].
		for (uint16 i = 0; i < count; i++)
			string1->setValue(i + index1, string2[i + index2]);

		return strAddress;
	}
	case 7: { // Cmp
		Common::String string1 = argv[1].isNull() ? "" : s->_segMan->getString(argv[1]);
		Common::String string2 = argv[2].isNull() ? "" : s->_segMan->getString(argv[2]);

		if (argc == 4) // Strncmp
			return make_reg(0, strncmp(string1.c_str(), string2.c_str(), argv[3].toUint16()));
		else           // Strcmp
			return make_reg(0, strcmp(string1.c_str(), string2.c_str()));
	}
	case 8: { // Dup
		reg_t stringHandle;

		SciString *dupString = s->_segMan->allocateString(&stringHandle);

		if (argv[1].getSegment() == s->_segMan->getStringSegmentId()) {
			*dupString = *s->_segMan->lookupString(argv[1]);
		} else {
			dupString->fromString(s->_segMan->getString(argv[1]));
		}

		return stringHandle;
	}
	case 9: // Getdata
		if (!s->_segMan->isHeapObject(argv[1]))
			return argv[1];

		return readSelector(s->_segMan, argv[1], SELECTOR(data));
	case 10: // Stringlen
		return make_reg(0, s->_segMan->strlen(argv[1]));
	case 11: { // Printf
		reg_t stringHandle;
		s->_segMan->allocateString(&stringHandle);

		reg_t *adjustedArgs = new reg_t[argc];
		adjustedArgs[0] = stringHandle;
		memcpy(&adjustedArgs[1], argv + 1, (argc - 1) * sizeof(reg_t));

		kFormat(s, argc, adjustedArgs);
		delete[] adjustedArgs;
		return stringHandle;
		}
	case 12: // Printf Buf
		return kFormat(s, argc - 1, argv + 1);
	case 13: { // atoi
		Common::String string = s->_segMan->getString(argv[1]);
		return make_reg(0, (uint16)atoi(string.c_str()));
	}
	// New subops in SCI2.1 late / SCI3
	case 14:	// unknown
		warning("kString, subop %d", op);
		return NULL_REG;
	case 15: { // upper
		Common::String string = s->_segMan->getString(argv[1]);

		string.toUppercase();
		s->_segMan->strcpy(argv[1], string.c_str());
		return NULL_REG;
	}
	case 16: { // lower
		Common::String string = s->_segMan->getString(argv[1]);

		string.toLowercase();
		s->_segMan->strcpy(argv[1], string.c_str());
		return NULL_REG;
	}
	default:
		error("Unknown kString subop %d", argv[0].toUint16());
	}

	return NULL_REG;
}

#endif

} // End of namespace Sci
