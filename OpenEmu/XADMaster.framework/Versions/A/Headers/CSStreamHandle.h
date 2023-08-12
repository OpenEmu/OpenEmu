/*
 * CSStreamHandle.h
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
#import "CSHandle.h"
#import "CSInputBuffer.h"

#define CSStreamHandle XADStreamHandle

@interface CSStreamHandle:CSHandle
{
	off_t streampos,streamlength;
	BOOL needsreset,endofstream;
	int nextstreambyte;

	@public
	CSInputBuffer *input;
}

// Initializers
//-(id)initWithName:(NSString *)descname;
//-(id)initWithName:(NSString *)descname length:(off_t)length;
-(id)initWithParentHandle:(CSHandle *)handle;
-(id)initWithParentHandle:(CSHandle *)handle length:(off_t)length;
-(id)initWithInputBufferForHandle:(CSHandle *)handle;
-(id)initWithInputBufferForHandle:(CSHandle *)handle length:(off_t)length;
-(id)initWithInputBufferForHandle:(CSHandle *)handle bufferSize:(int)buffersize;
-(id)initWithInputBufferForHandle:(CSHandle *)handle length:(off_t)length bufferSize:(int)buffersize;
-(id)initAsCopyOf:(CSStreamHandle *)other;
-(void)dealloc;

// Implemented by this class
-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;
-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;

// Implemented by subclasses
-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

// Called by subclasses
-(void)endStream;
-(BOOL)_prepareStreamSeekTo:(off_t)offs;
-(void)setStreamLength:(off_t)length;
-(void)setInputBuffer:(CSInputBuffer *)inputbuffer;

@end
