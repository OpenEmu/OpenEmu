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

#import "OEControlsScrollView.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OEUIDrawingUtils.h"

@implementation OEControlsScrollView
- (void)awakeFromNib
{
    [self setVerticalScroller:[OEControlsScroller new]];
    [self setHasHorizontalScroller:NO];
}
@end
@implementation OEControlsScroller
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEControlsScroller class])
        return;

    NSImage *image = [NSImage imageNamed:@"wood_arrow_up"];
    [image setName:@"wood_arrow_up_normal" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    [image setName:@"wood_arrow_up_highlighted" forSubimageInRect:(NSRect){{image.size.width/2,0},{image.size.width/2,image.size.height}}];
    
    image = [NSImage imageNamed:@"wood_arrow_down"];
    [image setName:@"wood_arrow_down_normal" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    [image setName:@"wood_arrow_down_highlighted" forSubimageInRect:(NSRect){{image.size.width/2,0},{image.size.width/2,image.size.height}}];
    
    image = [NSImage imageNamed:@"wood_track_vertical"];
    [image setName:@"wood_track_vertical_normal" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    [image setName:@"wood_track_vertical_inactive" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    
}

- (id)init
{
    self = [super init];
    if (self) {
    }
    return self;
}

- (NSImage*)OE_trackImage
{
    return [NSImage imageNamed:@"wood_track_vertical_normal"];
}
- (NSImage*)OE_knobImage
{
    return [NSImage imageNamed:@"wood_knob_vertical"];
}

- (NSRect)OE_knobSubimageRectForState:(OEUIState)state{
    return NSZeroRect;
}

@end