/*
 * XADPrefixCode.h
 *
 * Copyright (c) 2017-present, MacPaw Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */
#import "CSInputBuffer.h"

extern NSString *XADInvalidPrefixCodeException;

typedef struct XADCodeTreeNode XADCodeTreeNode;
typedef struct XADCodeTableEntry XADCodeTableEntry;

@interface XADPrefixCode:NSObject
{
	XADCodeTreeNode *tree;
	int numentries,minlength,maxlength;
	BOOL isstatic;

	int currnode;
	NSMutableArray *stack;

	int tablesize;
	XADCodeTableEntry *table1,*table2;
}

+(XADPrefixCode *)prefixCode;
+(XADPrefixCode *)prefixCodeWithLengths:(const int *)lengths numberOfSymbols:(int)numsymbols
maximumLength:(int)maxlength shortestCodeIsZeros:(BOOL)zeros;

-(id)init;
-(id)initWithStaticTable:(int (*)[2])statictable;
-(id)initWithLengths:(const int *)lengths numberOfSymbols:(int)numsymbols
maximumLength:(int)maxlength shortestCodeIsZeros:(BOOL)zeros;
-(void)dealloc;

-(void)addValue:(int)value forCodeWithHighBitFirst:(uint32_t)code length:(int)length;
-(void)addValue:(int)value forCodeWithHighBitFirst:(uint32_t)code length:(int)length repeatAt:(int)repeatpos;
-(void)addValue:(int)value forCodeWithLowBitFirst:(uint32_t)code length:(int)length;
-(void)addValue:(int)value forCodeWithLowBitFirst:(uint32_t)code length:(int)length repeatAt:(int)repeatpos;

-(void)startBuildingTree;
-(void)startZeroBranch;
-(void)startOneBranch;
-(void)finishBranches;
-(void)makeLeafWithValue:(int)value;
-(void)_pushNode;
-(void)_popNode;

-(void)_makeTable;
-(void)_makeTableLE;

@end

int CSInputNextSymbolUsingCode(CSInputBuffer *buf,XADPrefixCode *code);
int CSInputNextSymbolUsingCodeLE(CSInputBuffer *buf,XADPrefixCode *code);
