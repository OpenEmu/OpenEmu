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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "common/scummsys.h"
#include "common/types.h"

#include "common/fs.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"
#include "common/memorypool.h"


namespace Common {

class SeekableReadStream;

#define MAX_XML_DEPTH 8

#define XML_KEY(keyName) {\
		lay = new CustomXMLKeyLayout;\
		lay->callback = (&kLocalParserName::parserCallback_##keyName);\
		layout.top()->children[#keyName] = lay;\
		layout.push(lay); \
		_layoutList.push_back(lay);

#define XML_KEY_RECURSIVE(keyName) {\
			layout.top()->children[#keyName] = layout.top();\
			layout.push(layout.top());\
		}

#define KEY_END() layout.pop(); }

#define XML_PROP(propName, req) {\
		prop.name = #propName; \
		prop.required = req; \
		layout.top()->properties.push_back(prop); }



#define CUSTOM_XML_PARSER(parserName) \
	protected: \
	typedef parserName kLocalParserName; \
	bool keyCallback(ParserNode *node) {return node->layout->doCallback(this, node); }\
	struct CustomXMLKeyLayout : public XMLKeyLayout {\
		typedef bool (parserName::*ParserCallback)(ParserNode *node);\
		ParserCallback callback;\
		bool doCallback(XMLParser *parent, ParserNode *node) {return ((kLocalParserName *)parent->*callback)(node);} };\
	virtual void buildLayout() { \
		Common::Stack<XMLKeyLayout *> layout; \
		CustomXMLKeyLayout *lay = 0; \
		XMLKeyLayout::XMLKeyProperty prop; \
		_XMLkeys = new CustomXMLKeyLayout; \
		layout.push(_XMLkeys);

#define PARSER_END() layout.clear(); }

/**
 * The base XMLParser class implements generic functionality for parsing
 * XML-like files.
 *
 * In order to use it, it must be inherited with a child class that implements
 * the XMLParser::keyCallback() function.
 *
 * @see XMLParser::keyCallback()
 */
class XMLParser {
public:
	/**
	 * Parser constructor.
	 */
	XMLParser() : _XMLkeys(0), _stream(0) {}

	virtual ~XMLParser();

	/** Active state for the parser */
	enum ParserState {
		kParserNeedHeader,
		kParserNeedKey,
		kParserNeedKeyName,

		kParserNeedPropertyName,
		kParserNeedPropertyOperator,
		kParserNeedPropertyValue,

		kParserError
	};

	struct XMLKeyLayout;
	struct ParserNode;

	typedef HashMap<String, XMLParser::XMLKeyLayout*, IgnoreCase_Hash, IgnoreCase_EqualTo> ChildMap;

	/** nested struct representing the layout of the XML file */
	struct XMLKeyLayout {
		struct XMLKeyProperty {
			String name;
			bool required;
		};

		List<XMLKeyProperty> properties;
		ChildMap children;

		virtual bool doCallback(XMLParser *parent, ParserNode *node) = 0;

		virtual ~XMLKeyLayout() {
			properties.clear();
		}
	};

	XMLKeyLayout *_XMLkeys;

	/** Struct representing a parsed node */
	struct ParserNode {
		String name;
		StringMap values;
		bool ignore;
		bool header;
		int depth;
		XMLKeyLayout *layout;
	};

	ObjectPool<ParserNode, MAX_XML_DEPTH> _nodePool;

	ParserNode *allocNode() {
		return new (_nodePool) ParserNode;
	}

	void freeNode(ParserNode *node) {
		_nodePool.deleteChunk(node);
	}

	/**
	 * Loads a file into the parser.
	 * Used for the loading of Theme Description files
	 * straight from the filesystem.
	 *
	 * @param filename Name of the file to load.
	 */
	bool loadFile(const String &filename);

	bool loadFile(const FSNode &node);

	/**
	 * Loads a memory buffer into the parser.
	 * Used for loading the default theme fallback directly
	 * from memory if no themes can be found.
	 *
	 * @param buffer Pointer to the buffer.
	 * @param size Size of the buffer
	 * @param disposable Sets if the XMLParser owns the buffer,
	 *                   i.e. if it can be freed safely after it's
	 *                   no longer needed by the parser.
	 */
	bool loadBuffer(const byte *buffer, uint32 size, DisposeAfterUse::Flag disposable = DisposeAfterUse::NO);

	bool loadStream(SeekableReadStream *stream);

	void close();

	/**
	 * The actual parsing function.
	 * Parses the loaded data stream, returns true if successful.
	 */
	bool parse();

	/**
	 * Returns the active node being parsed (the one on top of
	 * the node stack).
	 */
	ParserNode *getActiveNode() {
		if (!_activeKey.empty())
			return _activeKey.top();

		return 0;
	}

	/**
	 * Returns the parent of a given node in the stack.
	 */
	ParserNode *getParentNode(ParserNode *child) {
		return child->depth > 0 ? _activeKey[child->depth - 1] : 0;
	}

protected:

	/**
	 * The buildLayout function builds the layout for the parser to use
	 * based on a series of helper macros. This function is automatically
	 * generated by the CUSTOM_XML_PARSER() macro on custom parsers.
	 *
	 * See the documentation regarding XML layouts.
	 */
	virtual void buildLayout() = 0;

	/**
	 * The keycallback function is automatically overloaded on custom parsers
	 * when using the CUSTOM_XML_PARSER() macro.
	 *
	 * Its job is to call the corresponding Callback function for the given node.
	 * A function for each key type must be declared separately. See the custom
	 * parser creation instructions.
	 *
	 * When parsing a key in such function, one may chose to skip it, e.g. because it's not needed
	 * on the current configuration. In order to ignore a key, you must set
	 * the "ignore" field of its KeyNode struct to "true": The key and all its children
	 * will then be automatically ignored by the parser.
	 *
	 * The callback function must return true if the key was properly handled (this includes the case when the
	 * key is being ignored). False otherwise. The return of keyCallback() is the same as
	 * the callback function's.
	 * See the sample implementation in GUI::ThemeParser.
	 */
	virtual bool keyCallback(ParserNode *node) = 0;

	/**
	 * The closed key callback function MAY be overloaded by inheriting classes to
	 * implement parser-specific functions.
	 *
	 * The closedKeyCallback is issued once a key has been finished parsing, to let
	 * the parser verify that all the required subkeys, etc, were included.
	 *
	 * Unlike the keyCallbacks(), there's just a closedKeyCallback() for all keys.
	 * Use "node->name" to distinguish between each key type.
	 *
	 * Returns true if the key was properly closed, false otherwise.
	 * By default, all keys are properly closed.
	 */
	virtual bool closedKeyCallback(ParserNode *node) {
		return true;
	}

	/**
	 * Called when a node is closed. Manages its cleanup and calls the
	 * closing callback function if needed.
	 */
	bool closeKey();

	/**
	 * Parses the value of a given key. There's no reason to overload this.
	 */
	bool parseKeyValue(String keyName);

	/**
	 * Called once a key has been parsed. It handles the closing/cleanup of the
	 * node stack and calls the keyCallback.
	 */
	bool parseActiveKey(bool closed);

	/**
	 * Prints an error message when parsing fails and stops the parser.
	 * Parser error always returns "false" so we can pass the return value
	 * directly and break down the parsing.
	 */
	bool parserError(const String &errStr);

	/**
	 * Skips spaces/whitelines etc.
	 * @return true if any spaces were skipped.
	 */
	bool skipSpaces();

	/**
	 * Skips comment blocks and comment lines.
	 * @return true if any comments were skipped.
	 */
	bool skipComments();

	/**
	 * Check if a given character can be part of a KEY or VALUE name.
	 * Overload this if you want to support keys with strange characters
	 * in their name.
	 */
	virtual inline bool isValidNameChar(char c) {
		return isAlnum(c) || c == '_';
	}

	/**
	 * Parses a the first textual token found.
	 */
	bool parseToken();

	/**
	 * Parses the values inside an integer key.
	 * The count parameter specifies the number of values inside
	 * the key, which are expected to be separated with commas.
	 *
	 * Sample usage:
	 * parseIntegerKey("255, 255, 255", 3, &red, &green, &blue);
	 * [will parse each field into its own integer]
	 *
	 * parseIntegerKey("1234", 1, &number);
	 * [will parse the single number into the variable]
	 *
	 * @param key String containing the integers to be parsed.
	 * @param count Number of comma-separated ints in the string.
	 * @param ... Integer variables to store the parsed ints, passed
	 *            by reference.
	 * @returns True if the parsing succeeded.
	 */
	bool parseIntegerKey(const char *key, int count, ...);
	bool parseIntegerKey(const String &keyStr, int count, ...);
	bool vparseIntegerKey(const char *key, int count, va_list args);

	bool parseXMLHeader(ParserNode *node);

	/**
	 * Overload if your parser needs to support parsing the same file
	 * several times, so you can clean up the internal state of the
	 * parser before each parse.
	 */
	virtual void cleanup() {}

	List<XMLKeyLayout *> _layoutList;

private:
	char _char;
	SeekableReadStream *_stream;
	String _fileName;

	ParserState _state; /** Internal state of the parser */

	String _error; /** Current error message */
	String _token; /** Current text token */

	Stack<ParserNode *> _activeKey; /** Node stack of the parsed keys */
};

} // End of namespace Common

#endif
