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

#import "OEControlsKeyButton.h"
#import "NSImage+OEDrawingAdditions.h"

@interface OEControlsKeyButton ()
- (void)OE_commonControlsKeyButtonInit;
@end

@implementation OEControlsKeyButton
@synthesize highlightPoint;
@synthesize target;
@synthesize action;
@synthesize title;
@synthesize state;

+ (void)initialize
{
    [self exposeBinding:@"title"];
    
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEControlsKeyButton class]) return;

    NSImage *image = [NSImage imageNamed:@"wood_textfield"];
    
    [image setName:@"wood_textfield_active" forSubimageInRect:NSMakeRect(0, 0, 5, 24)];
    [image setName:@"wood_textfield_inactive" forSubimageInRect:NSMakeRect(0, 24, 5, 24)];
}

- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        [self OE_commonControlsKeyButtonInit];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self OE_commonControlsKeyButtonInit];
    }
    return self;
}

- (void)OE_commonControlsKeyButtonInit
{
    state = NSOffState;
}

- (void)dealloc
{
    self.target = nil;
    self.action = NULL;
}

- (void)setTitle:(NSString *)value
{
    title = [value copy];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self setNeedsDisplay:YES];
    });
}

#pragma mark -
#pragma mark NSView Overrides
- (void)drawRect:(NSRect)dirtyRect
{
    // Draw Backgrounds
    NSImage *backgroundImage = self.state==NSOnState? [NSImage imageNamed:@"wood_textfield_active"] : [NSImage imageNamed:@"wood_textfield_inactive"];
    [backgroundImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:2 rightBorder:2 topBorder:5 bottomBorder:5];
        
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:15 size:11.0];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowBlurRadius:1.0];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.25]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    [attributes setObject:[NSColor colorWithDeviceWhite:0.0 alpha:1.0] forKey:NSForegroundColorAttributeName];
    [attributes setObject:font forKey:NSFontAttributeName];
    [attributes setObject:shadow forKey:NSShadowAttributeName];
    
    NSPoint p = NSMakePoint([self bounds].origin.x + 4, [self bounds].origin.y + 4);
    [self.title drawAtPoint:p withAttributes:attributes];
}

- (BOOL)isOpaque
{
    return NO;
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [self setState:[self state] == NSOnState ? NSOffState : NSOnState];
    
    // FIXME: It's not great to have the action triggered on mouseDown:
    if([self action] != NULL) [NSApp sendAction:[self action] to:[self target] from:self];
}

- (void)setState:(NSCellStateValue)aState
{
    if(state == aState) return;
    
    state = aState;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self setNeedsDisplay:YES];
    });
}

@end