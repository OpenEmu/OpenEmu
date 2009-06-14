//
//  GameBuffer.mm
//  OpenEmu
//
//  Created by Josh Weinberg on 9/15/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameBuffer.h"
#import "GameCore.h"

@implementation GameBuffer

@dynamic width;
@dynamic height;
@dynamic pixelForm;
@dynamic pixelType;
@dynamic internalForm;

@synthesize gameCore, filter;

// No default version for this class
- (id)init
{
    [self release];
    return nil;
}

// Designated Initializer
- (id)initWithGameCore:(GameCore *)core
{
	self = [super init];
	if(self)
	{
		gameCore = core;
		
		NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
		[udc addObserver:self forKeyPath:@"values.Filter" options:0xF context:NULL];
	}
	return self;	
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ( [keyPath isEqualToString:@"values.Filter"] )
	{
		int f = [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:keyPath] intValue];
		NSLog(@"New filter is %d", f);
		[self setFilter:(eFilter)f];
	}	
}

- (GLenum)pixelForm
{
	return [gameCore pixelFormat];
}

- (GLenum)pixelType
{
	return [gameCore pixelType];
}

- (GLenum)internalForm
{
	return [gameCore internalPixelFormat];
}

- (const unsigned char *)buffer
{
	return [gameCore videoBuffer];
}

- (int) width
{
	return [gameCore width];
}

- (int) height
{
	return [gameCore height];
}

- (void) dealloc
{
	[super dealloc];
}
@end
