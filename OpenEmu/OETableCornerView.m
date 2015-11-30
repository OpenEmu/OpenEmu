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

#import "OETableCornerView.h"
#import	"OEUIDrawingUtils.h"
#import "OETheme.h"
@implementation OETableCornerView

- (BOOL)isFlipped
{
	return YES;
}

- (void)drawRect:(NSRect)rect
{
	NSRect cellFrame = rect;
	cellFrame.size.width = 1;
	cellFrame.size.height -= 1;
	
	[[NSColor colorWithDeviceWhite:0.082 alpha:1.0] setFill];
	NSRectFill(cellFrame);
	
	cellFrame = rect;
	cellFrame.size.width -= 1;
	cellFrame.origin.x += 1;
	
    NSImage *image = [[OETheme sharedTheme] imageForKey:@"table_header_background" forState:OEThemeStateDefault];
	[image drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];

	NSRect leftHighlightRect = cellFrame;
	leftHighlightRect.size.width = 1;

	[[NSColor colorWithDeviceWhite:1.0 alpha:0.04] setFill];
	NSRectFillUsingOperation(leftHighlightRect, NSCompositeSourceOver);
}

@end
