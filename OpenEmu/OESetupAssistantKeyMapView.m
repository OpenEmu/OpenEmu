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

#import "OESetupAssistantKeyMapView.h"
#import "NSImage+OEDrawingAdditions.h"
@interface OESetupAssistantKeyMapView (Private)
- (NSImage*)imageForKey:(OESetupAssistantKey)key;
- (void)setup;
- (void)_updateKeyView;
@end
@implementation OESetupAssistantKeyMapView
@synthesize key;
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OESetupAssistantKeyMapView class])
        return;

    NSSize itemSize = NSMakeSize(100, 101);
    NSImage *spritesheet = [NSImage imageNamed:@"installer_gamepad_graphics"];
    [spritesheet setName:@"installer_gamepad_up" forSubimageInRect:   (NSRect){{0*itemSize.width, 1*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_down" forSubimageInRect: (NSRect){{1*itemSize.width, 1*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_left" forSubimageInRect: (NSRect){{2*itemSize.width, 1*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_right" forSubimageInRect:(NSRect){{0*itemSize.width, 0*itemSize.height},itemSize}];
    
    [spritesheet setName:@"installer_gamepad_success"  forSubimageInRect:(NSRect){{2*itemSize.width, 0*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_questionMark" forSubimageInRect:(NSRect){{1*itemSize.width, 0*itemSize.height},itemSize}];
}
#pragma mark -
- (id)init
{
    self = [super init];
    if (self) 
    {
        [self setup];
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        [self setup];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self setup];
    }
    
    return self;
}

- (void)dealloc {
    [self removeObserver:self forKeyPath:@"key"];
}
#pragma mark -
- (void)setup
{
    [self addObserver:self forKeyPath:@"key" options:0 context:nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualTo:@"key"])
    {
        [self _updateKeyView];
    }
}

- (void)_updateKeyView
{
    NSImageView *imageView = [[NSImageView alloc] initWithFrame:(NSRect){{0,0},{100,101}}];
    NSImage *image = [self imageForKey:[self key]];
    [imageView setImage:image];
    
    for(NSView* subview in self.subviews)
        [subview removeFromSuperview];
    
    [self addSubview:imageView];
}
#pragma mark -
- (BOOL)acceptsFirstResponder
{
    return YES;
}
- (BOOL)canBecomeKeyView
{
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent
{
    [self setNeedsDisplay:YES];
}

- (NSImage*)imageForKey:(OESetupAssistantKey)aKey
{
    switch (aKey) {
        case OESetupAssistantKeyUp:
            return [NSImage imageNamed:@"installer_gamepad_up"];
            break;
        case OESetupAssistantKeyDown:
            return [NSImage imageNamed:@"installer_gamepad_down"];
            break;
        case OESetupAssistantKeyLeft:
            return [NSImage imageNamed:@"installer_gamepad_left"];
            break;
        case OESetupAssistantKeyRight:
            return [NSImage imageNamed:@"installer_gamepad_right"];
            break;
        case OESetupAssistantKeySucess:
            return [NSImage imageNamed:@"installer_gamepad_success"];
            break;
        default:
            return [NSImage imageNamed:@"installer_gamepad_questionMark"];
    }
}
@end
