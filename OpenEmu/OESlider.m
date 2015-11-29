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

#import "OESlider.h"
#import "OESliderCell.h"
#import "OETheme.h"
#import "OEThemeImage.h"

#import "OpenEmu-Swift.h"

@interface OESlider ()
{
    NSTrackingArea *_trackingArea;
}
- (void)OE_windowKeyChanged:(NSNotification *)notification;
- (void)OE_updateNotifications;
@end

@implementation OESlider
@synthesize trackWindowActivity = _trackWindowActivity;
@synthesize trackMouseActivity = _trackMouseActivity;
@synthesize trackModifierActivity = _trackModifierActivity;
@synthesize modifierEventMonitor = _modifierEventMonitor;
@synthesize toolTipStyle = _toolTipStyle;

+ (Class)cellClass
{
    return [OESliderCell class];
}

- (void)setCell:(NSCell *)aCell
{
    [super setCell:aCell];
    [self updateTrackingAreas];
}

#pragma mark - Window activity
- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
    [super viewWillMoveToWindow:newWindow];

    if([self window])
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidBecomeMainNotification object:[self window]];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResignMainNotification object:[self window]];
    }

    if(newWindow && _trackWindowActivity)
    {
        // Register with the default notification center for changes in the window's keyedness only if one of the themed elements (the state mask) is influenced by the window's activity
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowKeyChanged:) name:NSWindowDidBecomeMainNotification object:newWindow];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowKeyChanged:) name:NSWindowDidResignMainNotification object:newWindow];
    }
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    [self updateTrackingAreas];
}

- (void)OE_windowKeyChanged:(NSNotification *)notification
{
    // The keyedness of the window has changed, we want to redisplay the button with the new state, this is only fired when NSWindowDidBecomeMainNotification and NSWindowDidResignMainNotification is registered.
    [self setNeedsDisplay];
}

- (void)OE_setShouldTrackWindowActivity:(BOOL)shouldTrackWindowActivity
{
    if(_trackWindowActivity != shouldTrackWindowActivity)
    {
        _trackWindowActivity = shouldTrackWindowActivity;
        [self viewWillMoveToWindow:[self window]];
        [self setNeedsDisplay:YES];
    }
}

#pragma mark - Mouse Activity
- (void)updateTrackingAreas
{
    if(_trackingArea) [self removeTrackingArea:_trackingArea];
    if(_trackMouseActivity)
    {
        // Track mouse enter and exit (hover and off) events only if the one of the themed elements (the state mask) is influenced by the mouse
        _trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingActiveInActiveApp | NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved owner:self userInfo:nil];
        [self addTrackingArea:_trackingArea];
    }
}

- (void)updateHoverFlagWithMousePoint:(NSPoint)point
{
    id<OECell> cell = [self cell];
    if(![cell conformsToProtocol:@protocol(OECell)]) return;

    const NSRect  bounds   = [self bounds];
    const BOOL    hovering = NSPointInRect(point, bounds);

    if([cell isHovering] != hovering)
    {
        [cell setHovering:hovering];
        [self setNeedsDisplayInRect:bounds];
    }
}

- (void)OE_updateHoverFlag:(NSEvent *)theEvent
{
    const NSPoint locationInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    [self updateHoverFlagWithMousePoint:locationInView];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
    [super mouseEntered:theEvent];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
    [super mouseExited:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
    [super mouseMoved:theEvent];
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    if ([self isTrackingModifierActivity])
    {
        const NSRect bounds = [self bounds];
        [self setNeedsDisplayInRect:bounds];
    }
    [super flagsChanged:theEvent];
}

- (void)OE_setShouldTrackMouseActivity:(BOOL)shouldTrackMouseActivity
{
    if(_trackMouseActivity != shouldTrackMouseActivity)
    {
        _trackMouseActivity = shouldTrackMouseActivity;
        [self updateTrackingAreas];
        [self setNeedsDisplay];

    }
}

- (void)OE_setShouldTrackModifierActivity:(BOOL)shouldTrackModifierActivity
{
    if(_trackModifierActivity != shouldTrackModifierActivity)
    {
        _trackModifierActivity = shouldTrackModifierActivity;
        if(shouldTrackModifierActivity == FALSE)
        {
            [NSEvent removeMonitor:_modifierEventMonitor];
        }
        else
        {
            __block id blockself = self;
            _modifierEventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskFromType(NSFlagsChanged) handler:^NSEvent*(NSEvent* e) {
                [blockself setNeedsDisplayInRect:[self bounds]];
                return e;
            }];
        }
        [self setNeedsDisplayInRect:[self bounds]];
    }
}

#pragma mark - Theming
- (void)OE_updateNotifications
{
    // This method determines if we need to register ourselves with the notification center and/or we need to add mouse tracking
    OESliderCell *cell = [self cell];
    if([cell isKindOfClass:[OESliderCell class]])
    {
        [self OE_setShouldTrackWindowActivity:([cell stateMask] & OEThemeStateAnyWindowActivity) != 0];
        [self OE_setShouldTrackMouseActivity:([cell stateMask] & OEThemeStateAnyMouse) != 0];
        [self OE_setShouldTrackModifierActivity:([cell stateMask] & OEThemeStateAnyModifier) != 0];
    }
}

- (void)setThemeKey:(NSString *)key
{
    NSString *backgroundKey = [key stringByAppendingString:@"_background"];
    NSString *levelKey      = [key stringByAppendingString:@"_level"];
    [self setThemeImageKey:key];
    [self setBackgroundThemeImageKey:backgroundKey];
    [self setLevelThemeImageKey:levelKey];
}

- (void)setThemeImageKey:(NSString *)key
{
    [self setThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setBackgroundThemeImageKey:(NSString *)key
{
    [self setBackgroundThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setBackgroundThemeImage:(OEThemeImage *)backgroundThemeImage
{
    OESliderCell *cell = [self cell];
    if([cell isKindOfClass:[OESliderCell class]])
    {
        [cell setBackgroundThemeImage:backgroundThemeImage];
        [self OE_updateNotifications];
        [self setNeedsDisplay];
    }
}

- (OEThemeImage *)backgroundThemeImage
{
    OESliderCell *cell = [self cell];
    return ([cell isKindOfClass:[OESliderCell class]] ? [cell backgroundThemeImage] : nil);
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
    OESliderCell *cell = [self cell];
    if([cell isKindOfClass:[OESliderCell class]])
    {
        [cell setThemeImage:themeImage];
        [self OE_updateNotifications];
        [self setNeedsDisplay];
    }
}

- (OEThemeImage *)themeImage
{
    OESliderCell *cell = [self cell];
    return ([cell isKindOfClass:[OESliderCell class]] ? [cell themeImage] : nil);
}

- (void)setLevelThemeImageKey:(NSString*)levelImageKey
{
    [self setLevelThemeImage:[[OETheme sharedTheme] themeImageForKey:levelImageKey]];
}

- (void)setLevelThemeImage:(OEThemeImage*)levelImage
{
    OESliderCell *cell = [self cell];
    if([cell isKindOfClass:[OESliderCell class]])
    {
        [cell setLevelThemeImage:levelImage];
        [self OE_updateNotifications];
        [self setNeedsDisplay];
    }
}

- (OEThemeImage*)levelThemeImage
{
    OESliderCell *cell = [self cell];
    return ([cell isKindOfClass:[OESliderCell class]] ? [cell levelThemeImage] : nil);
}
#pragma mark - Unused Theming
- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{}

- (OEThemeTextAttributes *)themeTextAttributes
{
    return nil;
}
- (void)setThemeTextAttributesKey:(NSString *)key
{}


#pragma mark - Old Stuff
@synthesize maxHint, minHint;
- (instancetype)init
{
    self = [super init];
    if (self) {
        hintImagesState = -1;
    }
    return self;
}

- (void)updateHintImages
{
    BOOL enabled = [self isEnabled];
    BOOL active = [[self window] isMainWindow];

    OEThemeState state = active ? OEThemeInputStateWindowActive : OEThemeInputStateWindowInactive;
    state |= enabled ? OEThemeInputStateEnabled : OEThemeInputStateDisabled;

    if(hintImagesState == state) return;

    OEThemeImage *smallHint = [[OETheme sharedTheme] themeImageForKey:@"grid_slider_small"];
    OEThemeImage *largeHint = [[OETheme sharedTheme] themeImageForKey:@"grid_slider_large"];

    [[self maxHint] setImage:[largeHint imageForState:state]];
    [[self minHint] setImage:[smallHint imageForState:state]];

    hintImagesState = state;
}

- (void)awakeFromNib
{
    [[self maxHint] setImageAlignment:NSImageAlignTopLeft];
    
    [self updateHintImages];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    [self updateHintImages];
}

@end
