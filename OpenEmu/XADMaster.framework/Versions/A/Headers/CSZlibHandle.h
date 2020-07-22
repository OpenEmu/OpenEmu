/*
 * CSZlibHandle.h
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

#ifndef __MACTYPES__
#define Byte zlibByte
#include <zlib.h>
#undef Byte
#else
#include <zlib.h>
#endif

#define CSZlibHandle XADZlibHandle

extern NSString *CSZlibException;

@interface CSZlibHandle:CSStreamHandle
{
	off_t startoffs;
	z_stream zs;
	BOOL inited,seekback,endstreamateof;

	uint8_t inbuffer[0x4000];
}

+(CSZlibHandle *)zlibHandleWithHandle:(CSHandle *)handle;
+(CSZlibHandle *)zlibHandleWithHandle:(CSHandle *)handle length:(off_t)length;
+(CSZlibHandle *)deflateHandleWithHandle:(CSHandle *)handle;
+(CSZlibHandle *)deflateHandleWithHandle:(CSHandle *)handle length:(off_t)length;

// Intializers
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length header:(BOOL)header ;
-(id)initAsCopyOf:(CSZlibHandle *)other;
-(void)dealloc;

// Public methods
-(void)setSeekBackAtEOF:(BOOL)seekateof;
-(void)setEndStreamAtInputEOF:(BOOL)endateof;

// Implemented by this class
-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

// Internal methods
-(void)_raiseZlib;

@end
