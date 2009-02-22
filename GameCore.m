//
//  GameCore.m
//  OpenEmu
//
//  Created by Remy Demarest on 22/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "GameCore.h"
#import "GameDocument.h"
#import "GameDocumentController.h"
#import "OEAbstractAdditions.h"


@implementation GameCore

@property(assign) GameDocument *document;

- (id)init
{
	return [self initWithDocument:[[GameDocumentController sharedDocumentController] currentDocument]];
}

- (id)initWithDocument:(GameDocument *)aDocument
{
	self = [super init];
	if(self != nil)
	{
		document = aDocument; 
	}
	return self;
}

#pragma mark Execution
- (void)pauseEmulation:(BOOL)flag
{
	[self doesNotImplementSelector:_cmd];
}
- (void)setupEmulation
{
	[self doesNotImplementSelector:_cmd];
}
- (void)stopEmulation
{
	[self doesNotImplementSelector:_cmd];
}
- (void)startEmulation
{
	[self doesNotImplementSelector:_cmd];
}
- (void)resetEmulation
{
	[self doesNotImplementSelector:_cmd];
}
- (void)executeFrame
{
	[self doesNotImplementSelector:_cmd];
}

- (BOOL)loadFileAtPath:(NSString*)path
{
	[self doesNotImplementSelector:_cmd];
}

#pragma mark Video
- (NSInteger)width
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)height
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (const unsigned char *)width
{
	[self doesNotImplementSelector:_cmd];
	return NULL;
}

- (GLenum)pixelFormat
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (GLenum)pixelType
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (GLenum)internalPixelFormat
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

#pragma mark Audio
- (const UInt16 *)soundBuffer
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)channelCount
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)frameSampleCount
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}


- (NSInteger)soundBufferSize
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)sampleRate
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

#pragma mark Input
- (void)player:(NSInteger)thePlayer didPressButton:(NSInteger)gameButton
{
	[self doesNotImplementSelector:_cmd];
}

- (void)player:(NSInteger)thePlayer didReleaseButton:(NSInteger)gameButton
{
	[self doesNotImplementSelector:_cmd];
}

#pragma mark Optional
- (void)saveStateToFileAtPath:(NSString *)fileName
{
	[self doesNotImplementOptionalSelector:_cmd];
}

- (void)loadStateFromFileAtPath:(NSString *)fileName
{
	[self doesNotImplementOptionalSelector:_cmd];
}

@end
