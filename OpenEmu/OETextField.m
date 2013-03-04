//
//  OETextField.m
//  OEThemeFactory
//
//  Created by Christoph Leimbrock on 2/25/13.
//
//

#import "OETextField.h"


@interface OETextField ()
{
    NSTrackingArea *_trackingArea;   // Mouse tracking area used only if the control reacts to the mouse's location
}
@end
@implementation OETextField

+ (Class)cellClass
{
    return [OETextFieldCell class];
}

#pragma mark - OEControl + State Support
@synthesize trackWindowActivity = _trackWindowActivity;
@synthesize trackMouseActivity = _trackMouseActivity;
@synthesize trackModifierActivity = _trackModifierActivity;
@synthesize modifierEventMonitor = _modifierEventMonitor;

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
}

- (void)mouseExited:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    if ([self isTrackingModifierActivity])
    {
        const NSRect bounds = [self bounds];
        [self setNeedsDisplayInRect:bounds];
    }
}

- (void)OE_windowKeyChanged:(NSNotification *)notification
{
    // The keyedness of the window has changed, we want to redisplay the button with the new state, this is only fired when NSWindowDidBecomeMainNotification and NSWindowDidResignMainNotification is registered.
    [self setNeedsDisplay];
    [self setStringValue:[self stringValue]];
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

- (void)OE_updateNotifications
{
    // This method determines if we need to register ourselves with the notification center and/or we need to add mouse tracking
    OETextFieldCell *cell = [self cell];
    if([cell isKindOfClass:[OETextFieldCell class]])
    {
        [self OE_setShouldTrackWindowActivity:([cell stateMask] & OEThemeStateAnyWindowActivity) != 0];
        [self OE_setShouldTrackMouseActivity:([cell stateMask] & OEThemeStateAnyMouse) != 0];
        [self OE_setShouldTrackModifierActivity:([cell stateMask] & OEThemeStateAnyModifier) != 0];
    }
}

- (void)setThemeKey:(NSString *)key
{
    NSString *backgroundKey = key;
    if(![key hasSuffix:@"_background"])
    {
        [self setThemeImageKey:key];
        backgroundKey = [key stringByAppendingString:@"_background"];
    }
    [self setBackgroundThemeImageKey:backgroundKey];
    
    NSString *selectionKey = key;
    if(![key hasSuffix:@"_selection"])
    {
        [self setThemeTextAttributesKey:key];
        selectionKey = [key stringByAppendingString:@"_selection"];
    }
    [self setSelectedThemeTextAttributesKey:selectionKey];

}

- (void)setBackgroundThemeImageKey:(NSString *)key
{
    [self setBackgroundThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setThemeImageKey:(NSString *)key
{
    [self setThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setThemeTextAttributesKey:(NSString *)key
{
    [self setThemeTextAttributes:[[OETheme sharedTheme] themeTextAttributesForKey:key]];
}

- (void)setSelectedThemeTextAttributesKey:(NSString*)key
{
    [self setSelectedThemeTextAttributes:[[OETheme sharedTheme] themeTextAttributesForKey:key]];
}

- (void)setBackgroundThemeImage:(OEThemeImage *)backgroundThemeImage
{
    OETextFieldCell *cell = [self cell];
    if([cell isKindOfClass:[OETextFieldCell class]])
    {
        [cell setBackgroundThemeImage:backgroundThemeImage];
        [self OE_updateNotifications];
        [self setNeedsDisplay];
    }
}

- (OEThemeImage *)backgroundThemeImage
{
    OETextFieldCell *cell = [self cell];
    return ([cell isKindOfClass:[OETextFieldCell class]] ? [cell backgroundThemeImage] : nil);
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
    OETextFieldCell *cell = [self cell];
    if([cell isKindOfClass:[OETextFieldCell class]])
    {
        [cell setThemeImage:themeImage];
        [self OE_updateNotifications];
        [self setNeedsDisplay];
    }
}

- (OEThemeImage *)themeImage
{
    OETextFieldCell *cell = [self cell];
    return ([cell isKindOfClass:[OETextFieldCell class]] ? [cell themeImage] : nil);
}

- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
    OETextFieldCell *cell = [self cell];
    if([cell isKindOfClass:[OETextFieldCell class]])
    {
        [cell setThemeTextAttributes:themeTextAttributes];
        [self OE_updateNotifications];
        [self setNeedsDisplay];
    }
}

- (OEThemeTextAttributes *)themeTextAttributes
{
    OETextFieldCell *cell = [self cell];
    return ([cell isKindOfClass:[OETextFieldCell class]] ? [cell themeTextAttributes] : nil);
}

- (void)setSelectedThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
    OETextFieldCell *cell = [self cell];
    if([cell isKindOfClass:[OETextFieldCell class]])
    {
        [cell setSelectedThemeTextAttributes:themeTextAttributes];
        [self OE_updateNotifications];
        [self setNeedsDisplay];
    }
}

- (OEThemeTextAttributes*)selectedThemeTextAttributes
{
    OETextFieldCell *cell = [self cell];
    return ([cell isKindOfClass:[OETextFieldCell class]] ? [cell selectedThemeTextAttributes] : nil);
 
}

- (void)setCell:(NSCell *)aCell
{
    [super setCell:aCell];
    [self updateTrackingAreas];
}

@end
