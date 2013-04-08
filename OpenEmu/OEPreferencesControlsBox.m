/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OEPreferencesControlsBox.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEPreferencesControlsBox

- (void)drawRect:(NSRect)dirtyRect{
	NSImage *image = [NSImage imageNamed:@"wood_inset_box"];
	[image drawInRect:NSInsetRect([self bounds], -1, -1) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:NO hints:nil leftBorder:16 rightBorder:16 topBorder:16 bottomBorder:16];

    const NSColor *topColor = [NSColor colorWithDeviceRed:85/255.0 green:45/255 blue:0.0 alpha:1.0];
    const NSColor *bottomColor = [NSColor colorWithDeviceRed:1.0 green:1.0 blue:0.0 alpha:0.2];

    NSRect lineRect = [self bounds];
    lineRect.size.height = 1;

    // Draw top separator
    lineRect.origin.y = 315;

    [topColor setFill];
    NSRectFill(lineRect);

    lineRect.origin.y -= 1;
    [bottomColor setFill];
    NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);

    // Draw bottom separator
    lineRect.origin.y = 48;

    [topColor setFill];
    NSRectFill(lineRect);

    lineRect.origin.y -= 1;
    [bottomColor setFill];
    NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);
}

- (BOOL)isFlipped{
	return NO;
}

@end
