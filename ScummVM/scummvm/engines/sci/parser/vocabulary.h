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

#ifndef SCI_SCICORE_VOCABULARY_H
#define SCI_SCICORE_VOCABULARY_H

#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/list.h"

#include "sci/sci.h"
#include "sci/engine/vm_types.h"

namespace Common {

class Serializer;

}

namespace Sci {

class ResourceManager;

/*#define VOCABULARY_DEBUG */

enum {
	VOCAB_RESOURCE_SELECTORS = 997,

	VOCAB_RESOURCE_SCI0_MAIN_VOCAB = 0,
	VOCAB_RESOURCE_SCI0_PARSE_TREE_BRANCHES = 900,
	VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB = 901,

	VOCAB_RESOURCE_SCI1_MAIN_VOCAB = 900,
	VOCAB_RESOURCE_SCI1_PARSE_TREE_BRANCHES = 901,
	VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB = 902,

	VOCAB_RESOURCE_ALT_INPUTS = 913
};


enum {
	VOCAB_CLASS_PREPOSITION = 0x01,
	VOCAB_CLASS_ARTICLE = 0x02,
	VOCAB_CLASS_ADJECTIVE = 0x04,
	VOCAB_CLASS_PRONOUN = 0x08,
	VOCAB_CLASS_NOUN = 0x10,
	VOCAB_CLASS_INDICATIVE_VERB = 0x20,
	VOCAB_CLASS_ADVERB = 0x40,
	VOCAB_CLASS_IMPERATIVE_VERB = 0x80,
	VOCAB_CLASS_NUMBER = 0x001
};

enum {
	kParseEndOfInput = 0,
	kParseOpeningParenthesis = 1,
	kParseClosingParenthesis = 2,
	kParseNil = 3,
	kParseNumber = 4
};

#define VOCAB_MAX_WORDLENGTH 256

/* Anywords are ignored by the parser */
#define VOCAB_CLASS_ANYWORD 0xff

/* This word class is used for numbers */
#define VOCAB_MAGIC_NUMBER_GROUP 0xffd /* 0xffe ? */
#define VOCAB_MAGIC_NOTHING_GROUP 0xffe

/* Number of nodes for each ParseTreeNode structure */
#define VOCAB_TREE_NODES 500

#define VOCAB_TREE_NODE_LAST_WORD_STORAGE 0x140
#define VOCAB_TREE_NODE_COMPARE_TYPE 0x146
#define VOCAB_TREE_NODE_COMPARE_GROUP 0x14d
#define VOCAB_TREE_NODE_FORCE_STORAGE 0x154

#define SAID_COMMA   0xf0
#define SAID_AMP     0xf1
#define SAID_SLASH   0xf2
#define SAID_PARENO  0xf3
#define SAID_PARENC  0xf4
#define SAID_BRACKO  0xf5
#define SAID_BRACKC  0xf6
#define SAID_HASH    0xf7
#define SAID_LT      0xf8
#define SAID_GT      0xf9
#define SAID_TERM    0xff

#define SAID_FIRST SAID_COMMA

/* There was no 'last matching word': */
#define SAID_FULL_MATCH 0xffff
#define SAID_NO_MATCH 0xfffe
#define SAID_PARTIAL_MATCH 0xfffd

#define SAID_LONG(x) ((x) << 8)

struct ResultWord {
	int _class; /**< Word class */
	int _group; /**< Word group */
};

typedef Common::List<ResultWord> ResultWordList;
typedef Common::List<ResultWordList> ResultWordListList;

typedef Common::HashMap<Common::String, ResultWordList, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> WordMap;


struct ParseRuleList;

struct suffix_t {

	int class_mask; /**< the word class this suffix applies to */
	int result_class; /**< the word class a word is morphed to if it doesn't fail this check */

	int alt_suffix_length; /**< String length of the suffix */
	int word_suffix_length; /**< String length of the other suffix */

	const char *alt_suffix; /**< The alternative suffix */
	const char *word_suffix; /**< The suffix as used in the word vocabulary */

};

typedef Common::List<suffix_t> SuffixList;


struct synonym_t {
	uint16 replaceant; /**< The word group to replace */
	uint16 replacement; /**< The replacement word group for this one */
};

typedef Common::Array<synonym_t> SynonymList;


struct AltInput {
	const char *_input;
	const char *_replacement;
	unsigned int _inputLength;
	bool _prefix;
};


struct parse_tree_branch_t {
	int id;
	int data[10];
};

enum ParseTypes {
	kParseTreeWordNode = 4,
	kParseTreeLeafNode = 5,
	kParseTreeBranchNode = 6
};

struct ParseTreeNode {
	ParseTypes type;  /**< leaf or branch */
	int value; /**< For leaves */
	ParseTreeNode* left; /**< Left child, for branches */
	ParseTreeNode* right; /**< Right child, for branches (and word leaves) */
};

enum VocabularyVersions {
	kVocabularySCI0 = 0,
	kVocabularySCI1 = 1
};

class Vocabulary {
public:
	Vocabulary(ResourceManager *resMan, bool foreign);
	~Vocabulary();

	// reset parser status
	void reset();

	/**
	 * Gets any word from the specified group. For debugging only.
	 * @param group		Group number
	 */
	const char *getAnyWordFromGroup(int group);


	/**
	 * Looks up a single word in the words and suffixes list.
	 * @param retval	the list of matches
	 * @param word		pointer to the word to look up
	 * @param word_len	length of the word to look up
	 */
	void lookupWord(ResultWordList &retval, const char *word, int word_len);


	/**
	 * Tokenizes a string and compiles it into word_ts.
	 * @param[in] retval		A list of words which will be set to the result
	 * @param[out] sentence		The sentence to examine
	 * @param[out] error		Points to a malloc'd copy of the offending text or to NULL on error
	 * @return true on success, false on failure
	 *
	 * On error, false is returned. If *error is NULL, the sentence did not
	 * contain any useful words; if not, *error points to a malloc'd copy of
	 * the offending word. The returned list may contain anywords.
	 */
	bool tokenizeString(ResultWordListList &retval, const char *sentence, char **error);

	/**
	 * Builds a parse tree from a list of words, using a set of Greibach Normal
	 * Form rules.
	 * @param words		The words to build the tree from
	 * @param verbose	Set to true for debugging
	 * @return	0 on success, 1 if the tree couldn't be built in VOCAB_TREE_NODES
	 *			nodes or if the sentence structure in 'words' is not part of the
	 *			language described by the grammar passed in 'rules'.
	 */
	int parseGNF(const ResultWordListList &words, bool verbose = false);

	/**
	 * Constructs the Greibach Normal Form of the grammar supplied in 'branches'.
	 * @param verbose	Set to true for debugging. If true, the list is
	 *					freed before the function ends
	 * @return	Pointer to a list of singly linked GNF rules describing the same
	 *			language that was described by 'branches'
	 *
	 * The original SCI rules are in almost-CNF (Chomsky Normal Form). Note that
	 * branch[0] is used only for a few magical incantations, as it is treated
	 * specially by the SCI parser.
	 */
	ParseRuleList *buildGNF(bool verbose = false);

	/**
	 * Deciphers a said block and dumps its content via debugN.
	 * For debugging only.
	 * @param pos	pointer to the data to dump
	 */
	void debugDecipherSaidBlock(const byte *pos);

	/**
	 * Prints the parser suffixes to the debug console.
	 */
	void printSuffixes() const;

	/**
	 * Prints the parser words to the debug console.
	 */
	void printParserWords() const;

	uint getParserBranchesSize() const { return _parserBranches.size(); }
	const parse_tree_branch_t &getParseTreeBranch(int number) const { return _parserBranches[number]; }

	/**
	 * Adds a new synonym to the list
	 */
	void addSynonym(synonym_t syn) { _synonyms.push_back(syn); }

	/**
	 * Clears the list of synonyms
	 */
	void clearSynonyms() { _synonyms.clear(); }

	/**
	 * Synonymizes a token list
	 * Parameters: (ResultWordListList &) words: The word list to synonymize
	 */
	void synonymizeTokens(ResultWordListList &words);

	void printParserNodes(int num);

	void dumpParseTree();

	int parseNodes(int *i, int *pos, int type, int nr, int argc, const char **argv);

	/**
	 * Check text input against alternative inputs.
	 * @param text The text to process. It will be modified in-place
	 * @param cursorPos The cursor position
	 * @return true if anything changed
	 */
	bool checkAltInput(Common::String& text, uint16& cursorPos);

	/**
	 * Save/load vocabulary data
	 */
	void saveLoadWithSerializer(Common::Serializer &ser);

private:
	/**
	 * Loads all words from the main vocabulary.
	 * @return true on success, false on failure
	 */
	bool loadParserWords();

	/**
	 * Loads all suffixes from the suffix vocabulary.
	 * @return true on success, false on failure
	 */
	bool loadSuffixes();

	/**
	 * Frees all suffixes in the given list.
	 * @param suffixes: The suffixes to free
	 */
	void freeSuffixes();

	/**
	 * Retrieves all grammar rules from the resource data.
	 * @param branches		The rules are stored into this Array
	 * @return true on success, false on error
	 */
	bool loadBranches();

	/**
	 * Frees a parser rule list as returned by vocab_build_gnf().
	 * @param rule_list		the rule list to free
	 */
	void freeRuleList(ParseRuleList *rule_list);


	/**
	 * Retrieves all alternative input combinations from vocab 913.
	 * @return true on success, false on error
	 */
	bool loadAltInputs();

	/**
	 * Frees all alternative input combinations.
	 */
	void freeAltInputs();

	ResourceManager *_resMan;
	VocabularyVersions _vocabVersion;

	bool _foreign;
	uint16 _resourceIdWords;
	uint16 _resourceIdSuffixes;
	uint16 _resourceIdBranches;

	// Parser-related lists
	SuffixList _parserSuffixes;
	ParseRuleList *_parserRules; /**< GNF rules used in the parser algorithm */
	Common::Array<parse_tree_branch_t> _parserBranches;
	WordMap _parserWords;
	SynonymList _synonyms; /**< The list of synonyms */
	Common::Array<Common::List<AltInput> > _altInputs;

public:
	// Accessed by said()
	ParseTreeNode _parserNodes[VOCAB_TREE_NODES]; /**< The parse tree */

	// Parser data:
	reg_t parser_event; /**< The event passed to Parse() and later used by Said() */
	bool parserIsValid; /**< If something has been correctly parsed */
};

/**
 * Prints a parse tree.
 * @param tree_name		Name of the tree to dump (free-form)
 * @param nodes			The nodes containing the parse tree
 */
void vocab_dump_parse_tree(const char *tree_name, ParseTreeNode *nodes);



/**
 * Builds a parse tree from a spec and compares it to a parse tree.
 * @param spec		Pointer to the spec to build
 * @param verbose	Whether to display the parse tree after building it
 * @return 1 on a match, 0 otherwise
 */
int said(const byte *spec, bool verbose);

} // End of namespace Sci

#endif // SCI_SCICORE_VOCABULARY_H
