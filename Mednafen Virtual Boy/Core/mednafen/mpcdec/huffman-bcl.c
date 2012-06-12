/*************************************************************************
* Huffman codes generation, part of the code from the Basic Compression
* Library ( http://bcl.sourceforge.net )
*
* Modified by Nicolas BOTTI rududu at laposte.net
*
*-------------------------------------------------------------------------
* Copyright (c) 2003-2006 Marcus Geelnard
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would
*    be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
* Marcus Geelnard
* marcus.geelnard at home.se
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int Symbol;
    unsigned int Count;
    unsigned int Code;
    unsigned int Bits;
} huff_sym_t;

typedef struct huff_node huff_node_t;

struct huff_node {
	huff_node_t * ChildA;
	union {
		huff_node_t * ChildB;
		huff_sym_t * Symbol;
	};
    int Count;
};

static void _Huffman_StoreTree( huff_node_t *node, unsigned int bits )
{
	/* Is this a leaf node? */
	if( node->ChildA == 0 ) {
		/* Store code info in symbol array */
		node->Symbol->Bits = bits;
		return;
	}

	/* Branch A */
	_Huffman_StoreTree( node->ChildA, bits+1 );

	/* Branch B */
	_Huffman_StoreTree( node->ChildB, bits+1 );
}

/**
 * Compare 2 symbols to sort as canonical huffman (more bits first)
 * @param sym1
 * @param sym2
 * @return
 */
static int _Huffman_CompBits(const huff_sym_t * sym1, const huff_sym_t * sym2)
{
	if (sym1->Bits == sym2->Bits){
		if (sym1->Symbol == sym2->Symbol)
			return 0;
		else
			return ((sym1->Symbol > sym2->Symbol) << 1) - 1;
	} else
		return ((sym1->Bits < sym2->Bits) << 1) - 1;
}

/**
 * Compare 2 symbols to sort in symbol order
 * @param sym1
 * @param sym2
 * @return
 */
static int _Huffman_CompSym(const huff_sym_t * sym1, const huff_sym_t * sym2)
{
	return ((sym1->Symbol > sym2->Symbol) << 1) - 1;
}

/**
 * Generate canonical huffman codes from symbols and bit lengths
 * @param sym
 * @param num_symbols
 */
static void _Huffman_MakeCodes(huff_sym_t * sym, unsigned int num_symbols)
{
	unsigned int code = 0, i;
	int bits;

	qsort(sym, num_symbols, sizeof(huff_sym_t),
		  (int (*)(const void *, const void *)) _Huffman_CompBits);

	bits = sym[0].Bits;
	sym[0].Code = 0;

	for( i = 1; i < num_symbols; i++){
		code >>= bits - sym[i].Bits;
		bits = sym[i].Bits;
		code++;
		sym[i].Code = code;
	}
}


/**
 * Make a canonical huffman tree from symbols and counts
 * @param sym
 * @param num_symbols
 */
void _Huffman_MakeTree( huff_sym_t * sym, unsigned int num_symbols)
{
	huff_node_t * nodes, * node_1, * node_2, * root;
	unsigned int k, nodes_left, next_idx;

	nodes = malloc(sizeof(huff_node_t) * (num_symbols * 2 - 1));

	/* Initialize all leaf nodes */
	for( k = 0; k < num_symbols; ++ k ) {
		nodes[k].Symbol = & sym[k];
		nodes[k].Count = sym[k].Count;
		nodes[k].ChildA = (huff_node_t *) 0;
	}

	/* Build tree by joining the lightest nodes until there is only
		one node left (the root node). */
	root = (huff_node_t *) 0;
	nodes_left = num_symbols;
	next_idx = num_symbols;
	while( nodes_left > 1 )	{
		/* Find the two lightest nodes */
		node_1 = (huff_node_t *) 0;
		node_2 = (huff_node_t *) 0;
		for( k = 0; k < next_idx; ++ k ) {
			if( nodes[k].Count >= 0 ) {
				if( !node_1 || (nodes[k].Count <= node_1->Count) ) {
					node_2 = node_1;
					node_1 = &nodes[k];
				} else if( !node_2 || (nodes[k].Count <= node_2->Count) )
					node_2 = &nodes[k];
			}
		}

		/* Join the two nodes into a new parent node */
		root = &nodes[next_idx];
		root->ChildA = node_1;
		root->ChildB = node_2;
		root->Count = node_1->Count + node_2->Count;
		node_1->Count = -1;
		node_2->Count = -1;
		++ next_idx;
		-- nodes_left;
	}

	/* Store the tree in the output stream, and in the sym[] array (the
		latter is used as a look-up-table for faster encoding) */
	if( root ) {
		_Huffman_StoreTree( root, 0 );
	} else {
		/* Special case: only one symbol => no binary tree */
		root = &nodes[0];
		_Huffman_StoreTree( root, 1 );
	}

	free(nodes);

	_Huffman_MakeCodes(sym, num_symbols);
}

#ifdef __GNUC__

/**
 * Print the huffman tables
 * print_type = 0 => print the coding table
 * print_type = 1 => print the decoding table
 * print_type = 2 => print the full codes in symbols order
 * print_type = 3 => print the full codes in codes order
 * @param sym
 * @param num_symbols
 * @param print_type
 * @param offset
 */
void _Huffman_PrintCodes(huff_sym_t * sym, unsigned int num_symbols, int print_type, int offset)
{
	unsigned int total_cnt = 0, total_size = 0, optim_size = 0, i;
	int packs[4];

	switch( print_type ) {
		case 0 :
			qsort(sym, num_symbols, sizeof(huff_sym_t),
				  (int (*)(const void *, const void *)) _Huffman_CompSym);
			printf("{\n	");
			for( i = 0; i < num_symbols; i++) {
				if (i != 0)
					printf(", ");
				printf("{%u, %u}", sym[i].Code, sym[i].Bits);
			}
			printf("\n}\n");
			break;
		case 1:
			qsort(sym, num_symbols, sizeof(huff_sym_t),
				  (int (*)(const void *, const void *)) _Huffman_CompBits);
			printf("{\n	");
			for( i = num_symbols - 1; i >= 0; i--) {
				printf("{0x%.8x, %u, %i}", sym[i].Code << (32 - sym[i].Bits), sym[i].Bits, sym[i].Symbol - offset);
				if (i != 0)
					printf(", ");
			}
			printf("\n}\n");
			break;
		case 4:
			qsort(sym, num_symbols, sizeof(huff_sym_t),
				  (int (*)(const void *, const void *)) _Huffman_CompBits);
			printf("{\n	");
			for( i = num_symbols - 1; i >= 0; i--) {
				int symbol =  sym[i].Symbol;
				packs[3] = symbol / (offset * offset * offset);
				packs[2] = (symbol - packs[3] * offset * offset * offset) / (offset * offset);
				packs[1] = (symbol - (packs[3] * offset + packs[2]) * offset * offset) / offset;
				packs[0] = symbol - ((packs[3] * offset + packs[2]) * offset + packs[1] * offset);
				packs[0] -= offset >> 1;
				packs[1] -= offset >> 1;
				packs[2] -= offset >> 1;
				packs[3] -= offset >> 1;
				symbol = ((packs[3] & 15) << 12) | ((packs[2] & 15) << 8) | ((packs[1] & 15) << 4) | (packs[0] & 15);
				printf("{0x%.8x, %u, %i}", sym[i].Code << (32 - sym[i].Bits), sym[i].Bits, symbol);
				if (i != 0)
					printf(", ");
			}
			printf("\n}\n");
			break;
		default:
			if (print_type == 2)
				qsort(sym, num_symbols, sizeof(huff_sym_t),
					  (int (*)(const void *, const void *)) _Huffman_CompSym);
			else
				qsort(sym, num_symbols, sizeof(huff_sym_t),
					(int (*)(const void *, const void *)) _Huffman_CompBits);

			printf("Symbol		Count		Lenth		Code\n");
			for( i = 0; i < num_symbols; i++) {
				int k = sym[i].Bits - 1;
				printf("%-10i	%-10u	%-10u	", sym[i].Symbol - offset, sym[i].Count, sym[i].Bits);
				for (; k >= 0 ; k--)
					printf("%u", (sym[i].Code >> k) & 1 );
				printf("\n");

				total_cnt += sym[i].Count;
				total_size += sym[i].Count * sym[i].Bits;
				if (sym[i].Count != 0)
					optim_size += sym[i].Count * __builtin_log2(sym[i].Count);
			}
			optim_size = total_cnt * __builtin_log2(total_cnt) - optim_size;
			printf("\ncount : %u huff : %f bps ", total_cnt, (float)total_size / total_cnt);
			printf("opt : %f bps ", (float)optim_size / total_cnt);
			printf("loss : %f bps (%f %%)\n", (float)(total_size - optim_size) / total_cnt, (float)(total_size - optim_size) * 100 / optim_size);
			break;
	}
}

#endif


