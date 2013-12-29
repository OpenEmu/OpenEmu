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
