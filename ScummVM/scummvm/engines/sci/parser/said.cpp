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

#include "sci/engine/state.h"

namespace Sci {

#define SAID_BRANCH_NULL 0

#define MAX_SAID_TOKENS 128

// Maximum number of words to be expected in a parsed sentence
#define AUGMENT_MAX_WORDS 64

// uncomment to debug parse tree augmentation
//#define SCI_DEBUG_PARSE_TREE_AUGMENTATION


#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
#define scidprintf debugN
#else
void print_nothing(...) { }
#define scidprintf print_nothing
#endif


static int said_token;
static int said_tokens_nr;
static int said_tokens[MAX_SAID_TOKENS];

static int said_tree_pos;
#define SAID_TREE_START 4 // Reserve space for the 4 top nodes

enum SaidToken {
	TOKEN_COMMA = 0xF000,
	TOKEN_AMP = 0xF100,
	TOKEN_SLASH = 0xF200,
	TOKEN_PARENO = 0xF300,
	TOKEN_PARENC = 0xF400,
	TOKEN_BRACKETO = 0xF500,
	TOKEN_BRACKETC = 0xF600,
	TOKEN_HASH = 0xF700,
	TOKEN_LT = 0xF800,
	TOKEN_GT = 0xF900,
	TOKEN_TERM = 0xFF00
};

enum SaidWord {
	WORD_NONE = 0x0ffe,
	WORD_ANY = 0x0fff
};



// TODO: maybe turn this into a proper n-ary tree instead of an
//   n-ary tree implemented in terms of a binary tree.
//   (Together with _parserNodes in Vocabulary)

static ParseTreeNode said_tree[VOCAB_TREE_NODES];

typedef int wgroup_t;
typedef int said_spec_t;



static ParseTreeNode* said_next_node() {
	assert(said_tree_pos > 0 && said_tree_pos < VOCAB_TREE_NODES);

	return &said_tree[said_tree_pos++];
}

static ParseTreeNode* said_leaf_node(ParseTreeNode* pos, int value) {
	pos->type = kParseTreeLeafNode;
	pos->value = value;
	pos->right = 0;

	return pos;
}

static ParseTreeNode* said_word_node(ParseTreeNode* pos, int value) {
	pos->type = kParseTreeWordNode;
	pos->value = value;
	pos->right = 0;

	return pos;
}

static ParseTreeNode* said_branch_node(ParseTreeNode* pos,
                                       ParseTreeNode* left,
                                       ParseTreeNode* right) {
	pos->type = kParseTreeBranchNode;
	pos->left = left;
	pos->right = right;

	return pos;
}

static ParseTreeNode* said_branch_attach_left(ParseTreeNode* pos,
                                              ParseTreeNode* left) {
	pos->type = kParseTreeBranchNode;
	pos->left = left;

	return pos;

}

static ParseTreeNode* said_branch_attach_right(ParseTreeNode* pos,
                                               ParseTreeNode* right) {
	pos->type = kParseTreeBranchNode;
	pos->right = right;

	return pos;
}


/*
        pos
        / \
       .   \
            *
           / \
          /   0
         *
        / \
       /   \
      /   subtree
   major  /   \
         /     .
      minor

 . = unchanged child node
 * = new branch node
 0 = NULL child node. (Location for future siblings of the subtree)

*/

static bool said_attach_subtree(ParseTreeNode* pos, int major, int minor,
                                ParseTreeNode* subtree) {
	bool retval = true;

	said_branch_attach_right(pos,
		said_branch_node(said_next_node(),
			said_branch_node(said_next_node(),
				said_leaf_node(said_next_node(), major),
				said_branch_attach_left(subtree,
					said_leaf_node(said_next_node(), minor))),
			0));

	return retval;
}




/*****************/
/**** Parsing ****/
/*****************/

static bool parseSpec(ParseTreeNode* parentNode);
static bool parsePart2(ParseTreeNode* parentNode, bool& nonempty);
static bool parsePart3(ParseTreeNode* parentNode, bool& nonempty);
static bool parseSlash(ParseTreeNode* parentNode);
static bool parseExpr(ParseTreeNode* parentNode);
static bool parseRef(ParseTreeNode* parentNode);
static bool parseComma(ParseTreeNode* parentNode);
static bool parseList(ParseTreeNode* parentNode);
static bool parseListEntry(ParseTreeNode* parentNode);
static bool parseWord(ParseTreeNode* parentNode);

static bool parseWord(ParseTreeNode* parentNode)
{
	int token = said_tokens[said_token];
	if (token & 0x8000)
		return false;

	said_token++;

	ParseTreeNode* newNode = said_word_node(said_next_node(), token);

	parentNode->right = newNode;

	return true;
}

static bool parsePart2(ParseTreeNode* parentNode, bool& nonempty)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	ParseTreeNode* newNode = said_branch_node(said_next_node(), 0, 0);

	nonempty = true;

	bool found;

	found = parseSlash(newNode);

	if (found) {

		said_attach_subtree(parentNode, 0x142, 0x14a, newNode);

		return true;

	} else if (said_tokens[said_token] == TOKEN_BRACKETO) {
		said_token++;

		found = parsePart2(newNode, nonempty);

		if (found) {

			if (said_tokens[said_token] == TOKEN_BRACKETC) {
				said_token++;

				said_attach_subtree(parentNode, 0x152, 0x142, newNode);

				return true;
			}
		}

	}

	// CHECKME: this doesn't look right if the [] section matched partially
	// Should the below 'if' be an 'else if' ?

	if (said_tokens[said_token] == TOKEN_SLASH) {
		said_token++;

		nonempty = false;

		return true;

	}

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}

static bool parsePart3(ParseTreeNode* parentNode, bool& nonempty)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	ParseTreeNode* newNode = said_branch_node(said_next_node(), 0, 0);

	bool found;

	nonempty = true;

	found = parseSlash(newNode);

	if (found) {

		said_attach_subtree(parentNode, 0x143, 0x14a, newNode);

		return true;

	} else if (said_tokens[said_token] == TOKEN_BRACKETO) {
		said_token++;

		found = parsePart3(newNode, nonempty);

		if (found) {

			if (said_tokens[said_token] == TOKEN_BRACKETC) {
				said_token++;

				said_attach_subtree(parentNode, 0x152, 0x143, newNode);

				return true;
			}
		}

	}

	// CHECKME: this doesn't look right if the [] section matched partially
	// Should the below 'if' be an 'else if' ?

	if (said_tokens[said_token] == TOKEN_SLASH) {
		said_token++;

		nonempty = false;

		return true;

	}

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}


static bool parseSlash(ParseTreeNode* parentNode)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	if (said_tokens[said_token] == TOKEN_SLASH) {
		said_token++;

		bool found = parseExpr(parentNode);

		if (found)
			return true;

	}

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}


static bool parseRef(ParseTreeNode* parentNode)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	ParseTreeNode* newNode = said_branch_node(said_next_node(), 0, 0);

	ParseTreeNode* newParent = parentNode;

	bool found;

	if (said_tokens[said_token] == TOKEN_LT) {
		said_token++;

		found = parseList(newNode);

		if (found) {

			said_attach_subtree(newParent, 0x144, 0x14f, newNode);

			newParent = newParent->right;

			newNode = said_branch_node(said_next_node(), 0, 0);

			found = parseRef(newNode);

			if (found) {

				said_attach_subtree(newParent, 0x141, 0x144, newNode);

			}

			return true;

		}

	}

	// NB: This is not an "else if'.
	// If there is a "< [ ... ]", that is parsed as "< ..."

	if (said_tokens[said_token] == TOKEN_BRACKETO) {
		said_token++;

		found = parseRef(newNode);

		if (found) {

			if (said_tokens[said_token] == TOKEN_BRACKETC) {
				said_token++;

				said_attach_subtree(parentNode, 0x152, 0x144, newNode);

				return true;
			}
		}

	}

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}

static bool parseComma(ParseTreeNode* parentNode)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	if (said_tokens[said_token] == TOKEN_COMMA) {
		said_token++;

		bool found = parseList(parentNode);

		if (found)
			return true;

	}

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}

static bool parseListEntry(ParseTreeNode* parentNode)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	ParseTreeNode* newNode = said_branch_node(said_next_node(), 0, 0);

	bool found;

	if (said_tokens[said_token] == TOKEN_BRACKETO) {
		said_token++;

		found = parseExpr(newNode);

		if (found) {

			if (said_tokens[said_token] == TOKEN_BRACKETC) {
				said_token++;

				said_attach_subtree(parentNode, 0x152, 0x14c, newNode);

				return true;
			}
		}

	} else if (said_tokens[said_token] == TOKEN_PARENO) {
		said_token++;

		found = parseExpr(newNode);

		if (found) {

			if (said_tokens[said_token] == TOKEN_PARENC) {
				said_token++;

				said_attach_subtree(parentNode, 0x141, 0x14c, newNode);

				return true;
			}
		}

	} else if (parseWord(newNode)) {

		said_attach_subtree(parentNode, 0x141, 0x153, newNode);

		return true;

	}


	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}

static bool parseList(ParseTreeNode* parentNode)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	bool found;

	ParseTreeNode* newParent = parentNode;

	found = parseListEntry(newParent);

	if (found) {

		newParent = newParent->right;

		found = parseComma(newParent);

		return true;

	}

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}

static bool parseExpr(ParseTreeNode* parentNode)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	ParseTreeNode* newNode = said_branch_node(said_next_node(), 0, 0);

	bool ret = false;
	bool found;

	ParseTreeNode* newParent = parentNode;

	found = parseList(newNode);

	if (found) {
		ret = true;

		said_attach_subtree(newParent, 0x141, 0x14F, newNode);

		newParent = newParent->right;

	}

	found = parseRef(newParent);

	if (found || ret)
		return true;

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}

static bool parseSpec(ParseTreeNode* parentNode)
{
	// Store current state for rolling back if we fail
	int curToken = said_token;
	int curTreePos = said_tree_pos;
	ParseTreeNode* curRightChild = parentNode->right;

	ParseTreeNode* newNode = said_branch_node(said_next_node(), 0, 0);

	bool ret = false;

	bool found;

	ParseTreeNode* newParent = parentNode;

	found = parseExpr(newNode);

	if (found) {
		// Sentence part 1 found
		said_attach_subtree(newParent, 0x141, 0x149, newNode);

		newParent = newParent->right;

		ret = true;
	}

	bool nonempty;

	found = parsePart2(newParent, nonempty);

	if (found) {

		ret = true;

		if (nonempty) // non-empty part found
			newParent = newParent->right;


		found = parsePart3(newParent, nonempty);

		if (found) {

			if (nonempty)
				newParent = newParent->right;
		}
	}

	if (said_tokens[said_token] == TOKEN_GT) {
		said_token++;

		newNode = said_branch_node(said_next_node(), 0,
						said_leaf_node(said_next_node(), TOKEN_GT));

		said_attach_subtree(newParent, 0x14B, TOKEN_GT, newNode);

	}


	if (ret)
		return true;

	// Rollback
	said_token = curToken;
	said_tree_pos = curTreePos;
	parentNode->right = curRightChild;
	return false;
}


static bool buildSaidTree() {
	said_branch_node(said_tree, &said_tree[1], &said_tree[2]);
	said_leaf_node(&said_tree[1], 0x141); // Magic number #1
	said_branch_node(&said_tree[2], &said_tree[3], 0);
	said_leaf_node(&said_tree[3], 0x13f); // Magic number #2

	said_tree_pos = SAID_TREE_START;

	bool ret = parseSpec(&said_tree[2]);

	if (!ret)
		return false;

	if (said_tokens[said_token] != TOKEN_TERM) {
		// No terminator, so parse error.

		// Rollback
		said_tree[2].right = 0;
		said_token = 0;
		said_tree_pos = SAID_TREE_START;
		return false;
	}

	return true;
}

static int said_parse_spec(const byte *spec) {
	int nextitem;

	said_token = 0;
	said_tokens_nr = 0;

	said_tree_pos = SAID_TREE_START;

	do {
		nextitem = *spec++;
		if (nextitem < SAID_FIRST)
			said_tokens[said_tokens_nr++] = nextitem << 8 | *spec++;
		else
			said_tokens[said_tokens_nr++] = SAID_LONG(nextitem);

	} while ((nextitem != SAID_TERM) && (said_tokens_nr < MAX_SAID_TOKENS));

	if (nextitem != SAID_TERM) {
		warning("SAID spec is too long");
		return 1;
	}

	if (!buildSaidTree()) {
		warning("Error while parsing SAID spec");
		return 1;
	}

	return 0;
}

/**********************/
/**** Augmentation ****/
/**********************/

static bool dontclaim;
static int outputDepth;

enum ScanSaidType {
	SCAN_SAID_AND = 0,
	SCAN_SAID_OR = 1
};

static int matchTrees(ParseTreeNode* parseT, ParseTreeNode* saidT);
static int scanSaidChildren(ParseTreeNode* parseT, ParseTreeNode* saidT,
                            ScanSaidType type);
static int scanParseChildren(ParseTreeNode* parseT, ParseTreeNode* saidT);


static int node_major(ParseTreeNode* node) {
	assert(node->type == kParseTreeBranchNode);
	assert(node->left->type == kParseTreeLeafNode);
	return node->left->value;
}
static int node_minor(ParseTreeNode* node) {
	assert(node->type == kParseTreeBranchNode);
	assert(node->right->type == kParseTreeBranchNode);
	assert(node->right->left->type == kParseTreeLeafNode);
	return node->right->left->value;
}
static bool node_is_terminal(ParseTreeNode* node) {
	return (node->right->right &&
            node->right->right->type != kParseTreeBranchNode);
}
static int node_terminal_value(ParseTreeNode* node) {
	assert(node_is_terminal(node));
	return node->right->right->value;
}
#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
static void node_print_desc(ParseTreeNode* node) {
	assert(node);
	assert(node->left);
	if (node->left->type == kParseTreeBranchNode) {
		scidprintf("< ");
		node_print_desc(node->left);
		scidprintf(", ...>");
	} else {
		if (node_is_terminal(node)) {
			scidprintf("(%03x %03x %03x)", node_major(node),
			                               node_minor(node),
			                               node_terminal_value(node));
		} else {
			scidprintf("(%03x %03x <...>)", node_major(node),
			                                node_minor(node));
		}
	}
}
#else
static void node_print_desc(ParseTreeNode *) { }
#endif




static int matchTrees(ParseTreeNode* parseT, ParseTreeNode* saidT)
{
	outputDepth++;
	scidprintf("%*smatchTrees on ", outputDepth, "");
	node_print_desc(parseT);
	scidprintf(" and ");
	node_print_desc(saidT);
	scidprintf("\n");

	bool inParen = node_minor(saidT) == 0x14F || node_minor(saidT) == 0x150;
	bool inBracket = node_major(saidT) == 0x152;

	int ret;

	if (node_major(parseT) != 0x141 &&
	    node_major(saidT) != 0x141 && node_major(saidT) != 0x152 &&
	    node_major(saidT) != node_major(parseT))
	{
		ret = -1;
	}

	// parse major is 0x141 and/or
	// said major is 0x141/0x152 and/or
	// said major is parse major

	else if (node_is_terminal(saidT) && node_is_terminal(parseT) ) {

		// both saidT and parseT are terminals

		int said_val = node_terminal_value(saidT);

#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
		scidprintf("%*smatchTrees matching terminals: %03x", outputDepth, "", node_terminal_value(parseT));
		ParseTreeNode* t = parseT->right->right;
		while (t) {
			scidprintf(",%03x", t->value);
			t = t->right;
		}
		scidprintf(" vs %03x", said_val);
#endif

		if (said_val == WORD_NONE) {
			ret = -1;
		} else if (said_val == WORD_ANY) {
			ret = 1;
		} else {
			ret = -1;

			// scan through the word group ids in the parse tree leaf to see if
			// one matches the word group in the said tree
			parseT = parseT->right->right;
			do {
				assert(parseT->type != kParseTreeBranchNode);
				int parse_val = parseT->value;
				if (parse_val == WORD_ANY || parse_val == said_val) {
					ret = 1;
					break;
				}
				parseT = parseT->right;
			} while (parseT);
		}

		scidprintf(" (ret %d)\n", ret);

	} else if (node_is_terminal(saidT) && !node_is_terminal(parseT)) {

		// saidT is a terminal, but parseT isn't

		if (node_major(parseT) == 0x141 ||
		        node_major(parseT) == node_major(saidT))
			ret = scanParseChildren(parseT->right->right, saidT);
		else
			ret = 0;

	} else if (node_is_terminal(parseT)) {

		// parseT is a terminal, but saidT isn't

		if (node_major(saidT) == 0x141 || node_major(saidT) == 0x152 ||
		        node_major(saidT) == node_major(parseT))
			ret = scanSaidChildren(parseT, saidT->right->right,
			                       inParen ? SCAN_SAID_OR : SCAN_SAID_AND );
		else
			ret = 0;

	} else if (node_major(saidT) != 0x141 && node_major(saidT) != 0x152 &&
	           node_major(saidT) != node_major(parseT)) {

		// parseT and saidT both aren't terminals
		// said major is not 0x141 or 0x152 or parse major

		ret = scanParseChildren(parseT->right->right, saidT);

	} else {

		// parseT and saidT are both not terminals,
		// said major 0x141 or 0x152 or equal to parse major

		ret = scanSaidChildren(parseT->right->right, saidT->right->right,
		                       inParen ? SCAN_SAID_OR : SCAN_SAID_AND);

	}

	if (inBracket && ret == 0) {
		scidprintf("%*smatchTrees changing ret to 1 due to brackets\n",
		           outputDepth, "");
		ret = 1;
	}

	scidprintf("%*smatchTrees returning %d\n", outputDepth, "", ret);
	outputDepth--;

	return ret;
}


static int scanSaidChildren(ParseTreeNode* parseT, ParseTreeNode* saidT,
                            ScanSaidType type) {
	outputDepth++;
	scidprintf("%*sscanSaid(%s) on ", outputDepth, "",
	                                  type == SCAN_SAID_OR ? "OR" : "AND");
	node_print_desc(parseT);
	scidprintf(" and ");
	node_print_desc(saidT);
	scidprintf("\n");

	int ret = 1;

	assert(!(type == SCAN_SAID_OR && !saidT));

	while (saidT) {
		assert(saidT->type == kParseTreeBranchNode);

		ParseTreeNode* saidChild = saidT->left;
		assert(saidChild);

		if (node_major(saidChild) != 0x145) {

			ret = scanParseChildren(parseT, saidChild);

			if (type == SCAN_SAID_AND && ret != 1)
				break;

			if (type == SCAN_SAID_OR && ret == 1)
				break;

		}

		saidT = saidT->right;

	}
	scidprintf("%*sscanSaid returning %d\n", outputDepth, "", ret);

	outputDepth--;
	return ret;
}


static int scanParseChildren(ParseTreeNode* parseT, ParseTreeNode* saidT) {

	outputDepth++;
	scidprintf("%*sscanParse on ", outputDepth, "");
	node_print_desc(parseT);
	scidprintf(" and ");
	node_print_desc(saidT);
	scidprintf("\n");

	if (node_major(saidT) == 0x14B) {
		dontclaim = true;
		scidprintf("%*sscanParse returning 1 (0x14B)\n", outputDepth, "");
		outputDepth--;
		return 1;
	}

	bool inParen = node_minor(saidT) == 0x14F || node_minor(saidT) == 0x150;
	bool inBracket = node_major(saidT) == 0x152;

	int ret;

	// descend further down saidT before actually scanning parseT
	if ((node_major(saidT) == 0x141 || node_major(saidT) == 0x152) &&
	    !node_is_terminal(saidT)) {

		ret = scanSaidChildren(parseT, saidT->right->right,
		                       inParen ? SCAN_SAID_OR : SCAN_SAID_AND );

	} else if (parseT && parseT->left->type == kParseTreeBranchNode) {

		ret = 0;
		int subresult = 0;

		while (parseT) {
			assert(parseT->type == kParseTreeBranchNode);

			ParseTreeNode* parseChild = parseT->left;
			assert(parseChild);

			scidprintf("%*sscanning next: ", outputDepth, "");
			node_print_desc(parseChild);
			scidprintf("\n");

			if (node_major(parseChild) == node_major(saidT) ||
			        node_major(parseChild) == 0x141)
				subresult = matchTrees(parseChild, saidT);

			if (subresult != 0)
				ret = subresult;

			if (ret == 1)
				break;

			parseT = parseT->right;

		}

		// ret is now:
		// 1 if ANY matchTrees(parseSibling, saidTree) returned 1
		// ELSE: -1 if ANY returned -1
		// ELSE: 0

	} else {

		ret = matchTrees(parseT, saidT);

	}

	if (inBracket && ret == 0) {
		scidprintf("%*sscanParse changing ret to 1 due to brackets\n",
		           outputDepth, "");
		ret = 1;
	}

	scidprintf("%*sscanParse returning %d\n", outputDepth, "", ret);
	outputDepth--;

	return ret;
}



static int augment_parse_nodes(ParseTreeNode *parseT, ParseTreeNode *saidT) {
	outputDepth = 0;
	scidprintf("augment_parse_nodes on ");
	node_print_desc(parseT);
	scidprintf(" and ");
	node_print_desc(saidT);
	scidprintf("\n");

	dontclaim = false;

	int ret = matchTrees(parseT, saidT);

	scidprintf("matchTrees returned %d\n", ret);

	if (ret != 1)
		return 0;

	if (dontclaim)
		return SAID_PARTIAL_MATCH;

	return 1;
}


/*******************/
/**** Main code ****/
/*******************/

int said(const byte *spec, bool verbose) {
	int retval;
	Vocabulary *voc = g_sci->getVocabulary();

	ParseTreeNode *parse_tree_ptr = voc->_parserNodes;

	if (voc->parserIsValid) {
		if (said_parse_spec(spec))
			return SAID_NO_MATCH;

		if (verbose)
			vocab_dump_parse_tree("Said-tree", said_tree);
		retval = augment_parse_nodes(parse_tree_ptr, said_tree);

		if (!retval)
			return SAID_NO_MATCH;
		else if (retval != SAID_PARTIAL_MATCH)
			return SAID_FULL_MATCH;
		else
			return SAID_PARTIAL_MATCH;
	}

	return SAID_NO_MATCH;
}


/*

Some test expressions for in the ScummVM debugging console, using
Codename: ICEMAN's vocabulary:



said green board & [!*] / 8af < 1f6
True

said get green board & [!*] / 8af < 1f6
False

said green board & [!*] / 8af [< 1f6 ]
True

said climb up & 19b , 426 [< 142 ] [/ 81e ]
True

said climb up ladder & 19b , 426 [< 142 ] [/ 81e ]
True

said climb down & 19b , 426 [< 142 ] [/ 81e ]
False

said climb up tree & 19b , 426 [< 142 ] [/ 81e ]
False

said climb up & 19b , 446 , 426 [< 143 ] [/ 81e ]
False

said climb down & 19b , 446 , 426 [< 143 ] [/ 81e ]
True

said use green device & 1a5 / 8c1 [< 21d ]
False

said use electronic device & 1a5 / 8c1 [< 21d ]
True

said use device & 1a5 / 8c1 [< 21d ]
True

said eat & 429 [/ !* ]
True

said eat ladder & 429 [/ !* ]
False

said look at the ladder & 3f8 / 81e [< !* ]
True

said look at the green ladder & 3f8 / 81e [< !* ]
False

said look green book & / 7f6 [< 8d2 ]
False

said look green book & 3f8 [< ca ]
True

said get a blue board for the green ladder & 3f9 / 8af [ < 1f6 ] / 81e < 1f6
False

said get a board for the green ladder & 3f9 / 8af [ < 1f6 ] / 81e < 1f6
True

said get a blue board & 3f9 / 8af [ < 1f6 ]
False

said get up  & ( 3f8 , 3f9 ) [ < ( 142 , 143 ) ]
True

said get left  & ( 3f8 , 3f9 ) [ < ( 142 , 143 ) ]
False

said look down & ( 3f8 , 3f9 ) [ < ( 142 , 143 ) ]
True

said get & ( 3f8 , 3f9 ) [ < ( 142 , 143 ) ]
True

said put washer on shaft & 455 , ( 3fa < cb ) / 8c6
True

said depth correct & [!*] < 8b1 / 22b
True

said depth acknowledged & / 46d , 460 , 44d < 8b1
True

said depth confirmed & / 46d , 460 , 44d < 8b1
True

said depth attained & / 46d , 460 , 44d < 8b1
True


*/




} // End of namespace Sci
