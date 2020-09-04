/*
 * XADSWFTagParser.h
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
#import <Foundation/Foundation.h>

#import "XADSWFGeometry.h"

#import "CSHandle.h"

#define SWFEndTag 0
#define SWFShowFrameTag 1
#define SWFDefineShapeTag 2
#define SWFPlaceObjectTag 4
#define SWFRemoveObjectTag 5
#define SWFDefineBitsJPEGTag 6
#define SWFJPEGTables 8
#define SWFDefineTextTag 11
#define SWFDefineFontInfoTag 13
#define SWFDefineSoundTag 14
#define SWFSoundStreamHeadTag 18
#define SWFSoundStreamBlockTag 19
#define SWFDefineBitsLosslessTag 20
#define SWFDefineBitsJPEG2Tag 21
#define SWFPlaceObject2Tag 26
#define SWFRemoveObject2Tag 28
#define SWFDefineText2Tag 33
#define SWFDefineBitsJPEG3Tag 35
#define SWFDefineBitsLossless2Tag 36
#define SWFDefineSpriteTag 39
#define SWFSoundStreamHead2Tag 45
#define SWFDefineFont2Tag 48
#define SWFPlaceObject3Tag 70
#define SWFDefineFont3Tag 75
#define SWFDefineBitsJPEG4Tag 90

extern NSString *SWFWrongMagicException;
extern NSString *SWFNoMoreTagsException;

@interface XADSWFTagParser:NSObject
{
	CSHandle *fh;
	off_t nexttag,nextsubtag;

	int totallen,version;
	BOOL compressed;
	SWFRect rect;
	int frames,fps;

	int currtag,currlen;
	int currframe;

	int spriteid,subframes;
	int subtag,sublen;
	int subframe;
}

+(XADSWFTagParser *)parserWithHandle:(CSHandle *)handle;
+(XADSWFTagParser *)parserForPath:(NSString *)path;

-(id)initWithHandle:(CSHandle *)handle;
-(void)dealloc;

-(void)parseHeader;

-(int)version;
-(BOOL)isCompressed;
-(SWFRect)rect;
-(int)frames;
-(int)framesPerSecond;

-(int)nextTag;

-(int)tag;
-(int)tagLength;
-(int)tagBytesLeft;
-(int)frame;
-(double)time;

-(CSHandle *)handle;
-(CSHandle *)tagHandle;
-(NSData *)tagContents;

-(void)parseDefineSpriteTag;

-(int)spriteID;
-(int)subFrames;

-(int)nextSubTag;
-(int)subTag;
-(int)subTagLength;
-(int)subTagBytesLeft;
-(int)subFrame;
-(double)subTime;

@end
