/*
 * CSFileHandle.h
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

#import <stdio.h>

#define CSFileHandle XADFileHandle

extern NSString *CSCannotOpenFileException;
extern NSString *CSFileErrorException;

@interface CSFileHandle:CSHandle
{
	FILE *fh;
	NSString *path;
	BOOL close;

	NSLock *multilock;
	CSFileHandle *fhowner;
	off_t pos;
}

+(CSFileHandle *)fileHandleForReadingAtPath:(NSString *)path;
+(CSFileHandle *)fileHandleForWritingAtPath:(NSString *)path;
+(CSFileHandle *)fileHandleForPath:(NSString *)path modes:(NSString *)modes;
+(CSFileHandle *)fileHandleForStandardInput;
+(CSFileHandle *)fileHandleForStandardOutput;
+(CSFileHandle *)fileHandleForStandardError;

// Initializers
-(id)initWithFilePointer:(FILE *)file closeOnDealloc:(BOOL)closeondealloc path:(NSString *)filepath;
-(id)initAsCopyOf:(CSFileHandle *)other;
-(void)dealloc;
-(void)close;

// Public methods
-(FILE *)filePointer;

// Implemented by this class
-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;

-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(void)pushBackByte:(int)byte;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;
-(void)writeBytes:(int)num fromBuffer:(const void *)buffer;

-(NSString *)name;

// Internal methods
-(void)_raiseError;
-(void)_setMultiMode;

@end
