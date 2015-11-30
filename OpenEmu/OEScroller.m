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

#define minKnobHeight 20

#import "OEScroller.h"
#import "OEUIDrawingUtils.h"

#import "OETheme.h"
#import "OEThemeImage.h"

@interface OEScroller ()
- (void)OE_detectOrientation;
@property (strong) OEThemeImage *trackImage;
@property (strong) OEThemeImage *knobImage;
@end
@implementation OEScroller
@synthesize backgroundThemeImage, themeImage, themeTextAttributes;
@synthesize trackWindowActivity, trackMouseActivity, trackModifierActivity, modifierEventMonitor;

@synthesize isVertical;

- (BOOL)autohidesScrollers
{
    return YES;
}

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self OE_detectOrientation];
        
        if([self respondsToSelector:@selector(setKnobStyle:)]){
            [self setKnobStyle:NSScrollerKnobStyleDefault];
        }
    }
    return self;
}

- (id)init
{
    self = [super init];
    if (self) {
        [self OE_detectOrientation];
        
        if([self respondsToSelector:@selector(setKnobStyle:)]){
            [self setKnobStyle:NSScrollerKnobStyleDefault];
        }
    }
    return self;
}

#pragma mark - Config
+ (BOOL)isCompatibleWithOverlayScrollers {
    return YES;
}

+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize scrollerStyle:(NSScrollerStyle)scrollerStyle
{
    if(scrollerStyle == NSScrollerStyleLegacy) 
        return 15.0;   
    
    return [super scrollerWidthForControlSize:controlSize scrollerStyle:scrollerStyle];
}

#pragma mark - Themeing
- (void)setThemeKey:(NSString *)key
{
    [self OE_detectOrientation];

    NSString *directionSuffix = [self isVertical] ? @"_vertical" : @"_horizontal";

    NSString *trackKey = [key stringByAppendingFormat:@"_track%@", directionSuffix];
    NSString *knobKey  = [key stringByAppendingFormat:@"_knob%@",  directionSuffix];

    OETheme *theme = [OETheme sharedTheme];
    [self setTrackImage:[theme themeImageForKey:trackKey]];
    [self setKnobImage:[theme themeImageForKey:knobKey]];

    _stateMask = [[self trackImage] stateMask] | [[self knobImage] stateMask];
}

- (void)setBackgroundThemeImageKey:(NSString *)key
{}
- (void)setThemeImageKey:(NSString *)key
{}
- (void)setThemeTextAttributesKey:(NSString *)key
{}

- (OEThemeState)OE_currentState
{
    // This is a convenience method that retrieves the current state of the scroller
    BOOL focused      = NO;
    BOOL windowActive = NO;
    BOOL highlighted  = NO;
    BOOL buttonState  = NO;
    BOOL hovering     = NO;

    if(((_stateMask & OEThemeStateAnyFocus) != 0) || ((_stateMask & OEThemeStateAnyWindowActivity) != 0))
    {
        // Set the focused, windowActive, and hover properties only if the state mask is tracking the button's focus, mouse hover, and window activity properties
        NSWindow *window = [self window];

        focused      = [window firstResponder] == self || ([window firstResponder] && [self respondsToSelector:@selector(currentEditor)] && [window firstResponder]==[self currentEditor]);
        windowActive = ((_stateMask & OEThemeStateAnyWindowActivity) != 0) && ([window isMainWindow] || ([window parentWindow] && [[window parentWindow] isMainWindow]));
    }

    return [OEThemeObject themeStateWithWindowActive:windowActive buttonState:buttonState selected:highlighted enabled:[self isEnabled] focused:focused houseHover:hovering modifierMask:[NSEvent modifierFlags]] & _stateMask;
}

#pragma mark - Scroller Drawing
- (void)drawArrow:(NSScrollerArrow)arrow highlight:(BOOL)flag
{
    return [super drawArrow:arrow highlight:flag];
}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{
    if([self scrollerStyle] == NSScrollerStyleOverlay)
        return [super drawKnobSlotInRect:slotRect highlight:flag];

    NSRect bounds = [self bounds];
    OEThemeState state = [self OE_currentState];
    NSImage *image = [[self trackImage] imageForState:state];
    [image drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}

- (void)drawKnob
{
    if([self scrollerStyle] == NSScrollerStyleOverlay)
        return [super drawKnob];
    
    NSRect targetRect = [self rectForPart:NSScrollerKnob];

    OEThemeState state = [self OE_currentState];
    NSImage *knobImage = [[self knobImage] imageForState:state];

    [knobImage drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}

- (NSRect)rectForPart:(NSScrollerPart)aPart{    
    if([self scrollerStyle] == NSScrollerStyleOverlay)
        return [super rectForPart:aPart];
    
    switch (aPart) {
        case NSScrollerKnob:;
            NSRect knobRect = [self rectForPart:NSScrollerKnobSlot];
            if (isVertical){
                knobRect = NSInsetRect(knobRect, 0, 1);

                NSImage *knobImage = [[self knobImage] imageForState:OEThemeStateDefault];
                NSSize size = [knobImage size];
                float knobHeight = roundf(knobRect.size.height  *[self knobProportion]);
                knobHeight = knobHeight < size.height ? size.height : knobHeight;
                
                knobRect.size.width -= 2;
                
                float knobY = knobRect.origin.y + roundf((knobRect.size.height - knobHeight)  *[self floatValue]);
                knobRect = NSMakeRect(0, knobY, knobRect.size.width, knobHeight);
                knobRect.origin.x += 1;
                
                return knobRect;
            } else {
                knobRect = NSInsetRect(knobRect, 1, 0);
                
                float knobWidth = roundf(knobRect.size.width  *[self knobProportion]);
                knobRect.size.height -= 2;
                
                float knobX = knobRect.origin.x + roundf((knobRect.size.width-knobWidth)  *[self floatValue]);
                knobRect = NSMakeRect(knobX, 0, knobWidth, knobRect.size.height);
                
                knobRect.origin.y += 1;
                
                return knobRect;
            }
            break;
            
        default:
            break;
    }
    
    return [super rectForPart:aPart];
}

- (void)OE_detectOrientation{
    if([self bounds].size.height == 0) return;
    
    if ([self bounds].size.width / [self bounds].size.height < 1)
        isVertical = YES;
    else
        isVertical = NO;
}
@end
