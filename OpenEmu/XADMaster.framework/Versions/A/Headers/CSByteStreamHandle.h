/*
 * CSByteStreamHandle.h
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

#import <setjmp.h>

#define CSByteStreamHandle XADByteStreamHandle

@interface CSByteStreamHandle:CSStreamHandle
{
	uint8_t (*bytestreamproducebyte_ptr)(id,SEL,off_t);
	int bytesproduced;
	@public
	jmp_buf eofenv;
}

// Intializers
//-(id)initWithName:(NSString *)descname length:(off_t)length;
-(id)initWithInputBufferForHandle:(CSHandle *)handle length:(off_t)length bufferSize:(int)buffersize;
-(id)initAsCopyOf:(CSByteStreamHandle *)other;

// Implemented by this class
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;
-(void)resetStream;

// Implemented by subclasses
-(void)resetByteStream;
-(uint8_t)produceByteAtOffset:(off_t)pos;

// Called by subclasses
-(void)endByteStream;

@end



extern NSString *CSByteStreamEOFReachedException;

static inline void CSByteStreamEOF(CSByteStreamHandle *self) __attribute__((noreturn));
static inline void CSByteStreamEOF(CSByteStreamHandle *self) { longjmp(self->eofenv,1); }
