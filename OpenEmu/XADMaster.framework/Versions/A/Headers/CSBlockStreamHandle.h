
/*
 * CSBlockStreamHandle.h
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
#import "CSStreamHandle.h"

#define CSBlockStreamHandle XADBlockStreamHandle

@interface CSBlockStreamHandle:CSStreamHandle
{
	uint8_t *_currblock;
	off_t _blockstartpos;
	int _blocklength;
	BOOL _endofblocks;
}

// Intializers
//-(id)initWithName:(NSString *)descname length:(off_t)length;
-(id)initWithInputBufferForHandle:(CSHandle *)handle length:(off_t)length bufferSize:(int)buffersize;
-(id)initAsCopyOf:(CSBlockStreamHandle *)other;

// Implemented by this class
-(void)seekToFileOffset:(off_t)offs;
-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

// Internal methods
-(void)_readNextBlock;

// Implemented by subclasses
-(void)resetBlockStream;
-(int)produceBlockAtOffset:(off_t)pos;

// Called by subclasses
-(void)setBlockPointer:(uint8_t *)blockpointer;
-(void)endBlockStream;

@end
