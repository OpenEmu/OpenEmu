/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
