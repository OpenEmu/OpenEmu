/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OEMenu.h"
#import "OEMenu+OEMenuViewAdditions.h"
#import "OEMenuView.h"
#import "OEMenuView+OEMenuAdditions.h"
#import "OEMenuDocumentView.h"
#import "NSMenuItem+OEMenuItemExtraDataAdditions.h"
#import "OEPopUpButton.h"
#import "OEMenuItemExtraData.h"

#import <objc/runtime.h>

#pragma mark -
#pragma mark Menu option keys

NSString * const OEMenuOptionsStyleKey           = @"OEMenuOptionsStyle";
NSString * const OEMenuOptionsArrowEdgeKey       = @"OEMenuOptionsArrowEdge";
NSString * const OEMenuOptionsMaximumSizeKey     = @"OEMenuOptionsMaximumSize";
NSString * const OEMenuOptionsHighlightedItemKey = @"OEMenuOptionsHighlightedItem";
NSString * const OEMenuOptionsScreenRectKey      = @"OEMenuOptionsScreenRect";

#pragma mark -
#pragma mark Timing

static const CGFloat OEMenuFadeOutDuration = 0.075; // Animation duration to fade the menu out
static const CGFloat OEMenuClickDelay      = 0.5;   // Amount of time before menu interprets a mouse down event between a click or drag operation

#pragma mark -
#pragma Implementation

static NSMutableArray *__sharedMenuStack; // Array of all the open instances of OEMenu

@implementation OEMenu
@synthesize highlightedItem = _highlightedItem;

+ (OEMenu *)OE_menuWithMenu:(NSMenu *)menu forScreen:(NSScreen *)screen options:(NSDictionary *)options
{
    // When instantiating a new window, we have to establish an initial content rect size, otherwise, it will never be associated with a specific NSScreen
    OEMenu *result = [[self alloc] initWithContentRect:NSMakeRect(0.0, 0.0, 10.0, 10.0) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES screen:screen];
    NSAssert(result != nil, @"out of memory");
    [result setMenu:menu];
    [result OE_parseOptions:options];
    [menu setOEMenu:result];

    return result;
}

+ (void)openMenuForPopUpButton:(OEPopUpButton *)button withEvent:(NSEvent *)event options:(NSDictionary *)options
{
    // Calculate the frame for the popup menu so that the popup menu's selected item hovers exactly over the popup button's title
    const NSRect titleRectInButton = [[button cell] titleRectForBounds:[button bounds]];
    const NSRect titleRectInScreen = [[button window] convertRectToScreen:[button convertRect:titleRectInButton toView:nil]];

    // Override the arrow edge and highlighted item options
    NSMutableDictionary *newOptions = (options ? [options mutableCopy] : [NSMutableDictionary dictionary]);
    [newOptions setValue:[NSNumber numberWithUnsignedInteger:OENoEdge] forKey:OEMenuOptionsArrowEdgeKey];
    [newOptions setValue:[button selectedItem]                         forKey:OEMenuOptionsHighlightedItemKey];

    OEMenu *result = [self OE_menuWithMenu:[button menu] forScreen:[[button window] screen] options:newOptions];
    [result OE_updateFrameAttachedToPopupButton:button alignSelectedItemWithRect:titleRectInScreen];
    [result OE_showMenuAttachedToWindow:[button window] withEvent:event];
}

+ (void)openMenu:(NSMenu *)menu withEvent:(NSEvent *)event forView:(NSView *)view options:(NSDictionary *)options
{
    // Calculate the frame for the popup menu so that the menu appears to be attached to the specified view
    OEMenu *result = [self OE_menuWithMenu:menu forScreen:[[view window] screen] options:options];

    // If a reference rect has not been specified, use the specified view as a reference point
    NSRect   rect       = NSZeroRect;
    NSValue *rectValue  = [options objectForKey:OEMenuOptionsScreenRectKey];
    if(rectValue)  rect = [rectValue rectValue];
    else if(event) rect = [[event window] convertRectToScreen:(NSRect){ .origin = [event locationInWindow] }];
    else           rect = [[view window]  convertRectToScreen:[view convertRect:[view bounds] toView:nil]];

    [result OE_updateFrameAttachedToScreenRect:rect];
    NSEvent *postEvent = [result OE_showMenuAttachedToWindow:[event window] withEvent:event];
    
    if(postEvent)
        [NSApp postEvent:postEvent atStart:NO];
}

+ (NSSize)sizeOfMenu:(NSMenu *)menu forView:(NSView *)view options:(NSDictionary *)options
{
    OEMenu *oemenu = [self OE_menuWithMenu:menu forScreen:[[view window] screen] options:options];
    return oemenu.intrinsicSize;
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSWindowStyleMask)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag screen:(NSScreen *)screen
{
    if((self = [super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag screen:screen]))
    {
        _view = [[OEMenuView alloc] initWithFrame:[[self contentView] bounds]];
        [_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [self setContentView:_view];

        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
        [self setLevel:NSTornOffMenuWindowLevel];
        [self setHasShadow:NO];
        [self setReleasedWhenClosed:YES];
        [self setExcludedFromWindowsMenu:YES];
        [self setCollectionBehavior:NSWindowCollectionBehaviorTransient | NSWindowCollectionBehaviorIgnoresCycle];
    }
    return self;
}

- (void)dealloc
{
    [[self menu] setOEMenu:nil];
}

- (void)orderWindow:(NSWindowOrderingMode)place relativeTo:(NSInteger)otherWin
{
    [super orderWindow:place relativeTo:otherWin];
    if(_submenu)
    {
        [self OE_updateFrameForSubmenu];
        [_submenu orderFrontRegardless];
    }
}

- (void)removeChildWindow:(NSWindow *)childWin
{
    // Make sure we nullify the _submenu reference if it's one of the windows that we are trying to remove from the hierarchy
    if(childWin == _submenu) _submenu = nil;
    [super removeChildWindow:childWin];
}

- (void)OE_hideWindowWithFadeDuration:(CGFloat)duration completionHandler:(void (^)(void))completionHandler
{
    if(![self isVisible] || [self alphaValue] == 0.0) return;

    // Create a collection of menus that include this menu and all of its descendents
    NSUInteger  index = [__sharedMenuStack indexOfObject:self];
    NSUInteger  len   = [__sharedMenuStack count] - index;
    NSArray    *menus = [__sharedMenuStack objectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(index, len)]];
    for(NSMenuItem *item in [[[__sharedMenuStack objectAtIndex:index] menu] itemArray])
    {
        [item setSubmenu:nil];
    }
    [__sharedMenuStack removeObjectsInArray:menus];

    void (^changes)(NSAnimationContext *context) =
    ^ (NSAnimationContext *context)
    {
        // Set hide all the menus in one
        [context setDuration:duration];
        [menus enumerateObjectsUsingBlock:
         ^ (OEMenu *obj, NSUInteger idx, BOOL *stop)
         {
             [[obj animator] setAlphaValue:0.0];
         }];
    };

    id<NSMenuDelegate> delegate = [[self menu] delegate];
    void (^fireCompletionHandler)(void) = ^{
        if(completionHandler) completionHandler();
        [[self parentWindow] removeChildWindow:self];

        // Invoked after a menu closed.
        if([delegate respondsToSelector:@selector(menuDidClose:)]) [delegate menuDidClose:[self->_view menu]];
    };

    [NSAnimationContext runAnimationGroup:changes completionHandler:fireCompletionHandler];
}

- (void)OE_cancelTrackingWithFadeDuration:(CGFloat)duration completionHandler:(void (^)(void))completionHandler
{
    if(_cancelTracking || [__sharedMenuStack count]==0) return;
    _cancelTracking = YES;

    if(self != [__sharedMenuStack objectAtIndex:0]) [[__sharedMenuStack objectAtIndex:0] OE_cancelTrackingWithFadeDuration:duration completionHandler:completionHandler];
    else
        [self OE_hideWindowWithFadeDuration:duration completionHandler:completionHandler];
}

- (void)cancelTracking
{
    [self OE_cancelTrackingWithFadeDuration:OEMenuFadeOutDuration completionHandler:nil];
}

- (void)cancelTrackingWithoutAnimation
{
    [self OE_cancelTrackingWithFadeDuration:0.0 completionHandler:nil];
}

- (void)OE_parseOptions:(NSDictionary *)options
{
    NSNumber   *style           = [options objectForKey:OEMenuOptionsStyleKey];
    NSNumber   *edge            = [options objectForKey:OEMenuOptionsArrowEdgeKey];
    NSValue    *maxSize         = [options objectForKey:OEMenuOptionsMaximumSizeKey];
    NSMenuItem *highlightedItem = [options objectForKey:OEMenuOptionsHighlightedItemKey];

    // Only set the option if it's been expliticly specified in the options dictionary
    if(style)           [_view setStyle:[style unsignedIntegerValue]];
    if(edge)            [_view setArrowEdge:[edge unsignedIntegerValue]];
    if(maxSize)         [self setMaxSize:[maxSize sizeValue]];
    if(highlightedItem) [self setHighlightedItem:highlightedItem];
}

- (NSRect)OE_confinementRectForScreen
{
    NSRect results = NSZeroRect;

    // Invoked to allow the delegate to specify a display location for the menu.
    id<NSMenuDelegate> delegate = [[self menu] delegate];
    if([delegate respondsToSelector:@selector(confinementRectForMenu:onScreen:)]) results = [delegate confinementRectForMenu:[self menu] onScreen:[self screen]];

    // If confinementRectForMenu:onScreen: is not implemented or it returns NSZeroRect then we use the screen's visible frame, otherwise we make sure that the result is confined with the screen's visible frame
    NSRect visibleFrame = [[self screen] visibleFrame];
    if(NSEqualRects(results, NSZeroRect)) results = visibleFrame;
    else                                  results = NSIntersectionRect(visibleFrame, results);

    return results;
}

- (void)OE_updateFrameAttachedToPopupButton:(OEPopUpButton *)button alignSelectedItemWithRect:(NSRect)rect
{
    // Make sure that the window's size has been adjusted early as the calculations below depend on it's positioning
    [self setContentSize:[self size]];
    [_view OE_layoutIfNeeded];

    // Calculate the positioning frames
    const NSRect        selectedItemRectOnView   = [[[self highlightedItem] extraData] frame];
    const NSRect        selectedItemRectOnWindow = [[_view documentView] convertRect:selectedItemRectOnView toView:nil];
    const NSRect        selectedItemRectOnScreen  = [[[_view documentView] window] convertRectToScreen:selectedItemRectOnWindow];

    const NSRect        actualScreenFrame  = [[self screen] frame];
    const NSRect        visibleScreenFrame = [self OE_confinementRectForScreen];
    const NSEdgeInsets  edgeInsets         = [_view backgroundEdgeInsets];
    const NSRect        buttonFrame        = [button bounds];
    const BOOL          doesContainImages  = [[_view documentView] doesContainImages];

    NSRect frame = {  .origin = rect.origin, .size = [self size] };
    
    // TODO: Adjust origin based on the button's and menu item's shadows
    frame.origin.x   -= edgeInsets.left + OEMenuItemTickMarkWidth + (doesContainImages ? OEMenuItemImageWidth : 0.0);
    frame.origin.y   -= NSMinY(selectedItemRectOnScreen)-NSMinY(actualScreenFrame) + 2.0 + (doesContainImages ? 1.0 : - 1.0);
    frame.size.width  = buttonFrame.size.width  + edgeInsets.left + edgeInsets.right + OEMenuContentEdgeInsets.left + OEMenuContentEdgeInsets.right + OEMenuItemInsets.left + OEMenuItemInsets.right;

    // Make sure menu is big enough to display all items
    frame.size.width = MAX(NSWidth(frame), self.size.width);

    // Adjust the frame's dimensions not to be bigger than the screen
    frame.size.height = MIN(NSHeight(frame), NSHeight(visibleScreenFrame));
    frame.size.width  = MIN(NSWidth(frame), NSWidth(visibleScreenFrame));

    // Adjust the frame's position to make the menu completely visible
    frame.origin.x = MIN(MAX(NSMinX(frame), NSMinX(visibleScreenFrame)), NSMaxX(visibleScreenFrame) - NSWidth(frame));
    frame.origin.y = MIN(MAX(NSMinY(frame), NSMinY(visibleScreenFrame)), NSMaxY(visibleScreenFrame) - NSHeight(frame));
        
    [self setFrame:frame display:[self isVisible]];
}

// Updates the frame's position and dimensions as it relates to the rect specified on the screen
- (void)OE_updateFrameAttachedToScreenRect:(NSRect)rect
{
    // Calculate the positioning frames
    const NSRect screenFrame = [self OE_confinementRectForScreen];

    // Figure out the size and position of the frame, as well as the anchor point.
    OERectEdge edge          = [_view arrowEdge];
    NSRect     frame         = { .size = [self size] };
    NSPoint    attachedPoint = NSZeroPoint;

    // Calculates the origin of the frame, this position is dependent on the edge that the arrow is visible on
    NSPoint (^originForEdge)(OERectEdge edge) =
    ^ (OERectEdge edge)
    {
        const NSEdgeInsets edgeInsets = [OEMenuView OE_backgroundEdgeInsetsForEdge:edge];
        const NSRect       bounds     = { .size = frame.size };

        switch(edge)
        {
            case OEMinXEdge: return NSMakePoint(NSMaxX(rect) - edgeInsets.right + 1.0, NSMidY(rect) - NSMidY(bounds));
            case OEMaxXEdge: return NSMakePoint(NSMinX(rect) - NSWidth(bounds) + edgeInsets.left - 1.0, NSMidY(rect) - NSMidY(bounds));
            case OEMinYEdge: return NSMakePoint(NSMidX(rect) - NSMidX(bounds), NSMaxY(rect) - edgeInsets.top - 1.0);
            case OEMaxYEdge: return NSMakePoint(NSMidX(rect) - NSMidX(bounds), NSMinY(rect) - NSHeight(bounds) + edgeInsets.bottom + 1.0);
            case OENoEdge:
            default:          return NSMakePoint(NSMinX(rect) - edgeInsets.left + 1.0,  NSMinY(rect) - NSHeight(bounds));
        }
    };

    // Set the frame's initial origin
    frame.origin      = originForEdge(edge);

    // Adjust the frame's dimensions not to be bigger than the screen
    frame.size.height = MIN(NSHeight(frame), NSHeight(screenFrame));
    frame.size.width  = MIN(NSWidth(frame), NSWidth(screenFrame));

    switch(edge)
    {
        case OEMinXEdge:
        case OEMaxXEdge:
            if(NSMinX(frame) < NSMinX(screenFrame) || NSMaxX(frame) > NSMaxX(screenFrame))
            {
                OERectEdge newEdge = ((edge == OEMinXEdge) ? OEMaxXEdge : OEMinXEdge);
                frame.origin       = originForEdge(newEdge);

                if((NSMinX(frame) < NSMinX(screenFrame)) || (NSMaxX(frame) > NSMaxX(screenFrame)))
                {
                    // TODO: Make view smaller
                    NSLog(@"TODO: Make view smaller");
                }
                else
                {
                    // Flip successful
                    [_view setArrowEdge:newEdge];
                }
            }

            // Adjust the frame's position to make the menu completely visible
            frame.origin.x = MIN(MAX(NSMinX(frame), NSMinX(screenFrame)), NSMaxX(screenFrame) - NSWidth(frame));
            frame.origin.y = MIN(MAX(NSMinY(frame), NSMinY(screenFrame)), NSMaxY(screenFrame) - NSHeight(frame));

            attachedPoint.x = (([_view arrowEdge] == OEMinXEdge) ? NSMinX(frame) : NSMaxX(frame));
            attachedPoint.y = NSMidY(rect);
            break;
        case OEMinYEdge:
        case OEMaxYEdge:
            if(NSMinY(frame) < NSMinY(screenFrame) || NSMaxY(frame) > NSMaxY(screenFrame))
            {
                OERectEdge newEdge = ((edge == OEMinYEdge) ? OEMaxYEdge : OEMinYEdge);
                frame.origin       = originForEdge(newEdge);

                if((NSMinY(frame) < NSMinY(screenFrame)) || (NSMaxY(frame) > NSMaxY(screenFrame)))
                {
                    // TODO: Make view smaller
                    NSLog(@"TODO: Make view smaller");
                }
                else
                {
                    // Flip successful
                    [_view setArrowEdge:newEdge];
                }
            }

            // Adjust the frame's position to make the menu completely visible
            frame.origin.x = MIN(MAX(NSMinX(frame), NSMinX(screenFrame)), NSMaxX(screenFrame) - NSWidth(frame));
            frame.origin.y = MIN(MAX(NSMinY(frame), NSMinY(screenFrame)), NSMaxY(screenFrame) - NSHeight(frame));

            attachedPoint.x = NSMidX(rect);
            attachedPoint.y = (([_view arrowEdge] == OEMinYEdge) ? NSMinY(frame) : NSMaxY(frame));
            break;
        case OENoEdge:
        default:
            // Adjust the frame's position to make the menu completely visible
            frame.origin.x = MIN(MAX(NSMinX(frame), NSMinX(screenFrame)), NSMaxX(screenFrame) - NSWidth(frame));
            frame.origin.y = MIN(MAX(NSMinY(frame), NSMinY(screenFrame)), NSMaxY(screenFrame) - NSHeight(frame));
            break;
    }
    [self setFrame:frame display:[self isVisible]];

    if(!NSEqualPoints(attachedPoint, NSZeroPoint))
    {
        NSPoint p = [[_view window] convertRectFromScreen:(NSRect){attachedPoint, {0,0}}].origin;
        attachedPoint = [_view convertPoint:p fromView:nil];
        [_view setAttachedPoint:attachedPoint];
    }
}

- (void)OE_updateFrameForSubmenu
{
    // Calculate the positioning frames
    const NSRect        rectInScreen  = [self convertRectToScreen:[[_view documentView] convertRect:[[[self highlightedItem] extraData] frame] toView:nil]];
    const NSRect        screenFrame   = [self OE_confinementRectForScreen];
    const NSEdgeInsets  edgeInsets    = [OEMenuView OE_backgroundEdgeInsetsForEdge:OENoEdge];
    const NSSize        size          = [_submenu size];

    // Calculates the origin for the specified edge
    CGFloat (^xForEdge)(OERectEdge edge) =
    ^ (OERectEdge edge)
    {
        switch(edge)
        {
            case OEMinXEdge: return NSMinX(rectInScreen) - size.width + edgeInsets.right - 1.0;
            case OEMaxXEdge: return NSMaxX(rectInScreen) - edgeInsets.left
                + 1.0;
            default:          break;
        }
        return 0.0;
    };

    OERectEdge edge  = (![self isSubmenu] || [_view arrowEdge] == OENoEdge ? OEMaxXEdge : [_view arrowEdge]);
    NSRect     frame = { .origin = { .x = xForEdge(edge), .y = NSMaxY(rectInScreen) - size.height + edgeInsets.top + OEMenuContentEdgeInsets.top + OEMenuItemInsets.top }, .size = size };

    // Adjust the frame's dimensions not to be bigger than the screen
    frame.size.height = MIN(NSHeight(frame), NSHeight(screenFrame));
    frame.size.width  = MIN(NSWidth(frame), NSWidth(screenFrame));
    frame.origin.y    = MIN(MAX(NSMinY(frame), NSMinY(screenFrame)), NSMaxY(screenFrame) - NSHeight(frame));

    // Adjust the frame's position to make the menu completely visible
    if(NSMinX(frame) < NSMinX(screenFrame))
    {
        // Flip to the other side
        frame.origin.x = xForEdge(OEMaxXEdge);
        edge           = OEMaxXEdge;
    }
    else if(NSMaxX(frame) > NSMaxX(screenFrame))
    {
        // Flip to the other side
        frame.origin.x = xForEdge(OEMinXEdge);
        edge           = OEMinXEdge;
    }
    [_submenu->_view setArrowEdge:edge];
    [_submenu setFrame:frame display:[self isVisible]];
}

- (void)OE_applicationActivityNotification:(NSNotification *)notification
{
    // The aplication has lost focus...lets cancel tracking
    [self cancelTrackingWithoutAnimation];
}

- (void)OE_menuWillBeginTrackingNotification:(NSNotification *)notification
{
    // The OS is about to open a native instance of NSMenu somewhere...lets cancel tracking
    if([notification object] != [self menu]) [self cancelTrackingWithoutAnimation];
}

- (void)OE_showMenuAttachedToWindow:(NSWindow *)parentWindow
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __sharedMenuStack = [NSMutableArray array];
    });

    [__sharedMenuStack addObject:self];
    if([__sharedMenuStack count] == 1)
    {
        // We only need to register for these notifications once, so just do it to the first menu that becomes visible
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_applicationActivityNotification:) name:NSApplicationDidResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_applicationActivityNotification:) name:NSApplicationDidHideNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_menuWillBeginTrackingNotification:) name:NSMenuDidBeginTrackingNotification object:nil];
    }

#if 0
    // TODO: Track additions and subtractions from the menu
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_menuDidAddItemNotification:) name:NSMenuDidAddItemNotification object:[self menu]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_menuDidRemoveItemNotification:) name:NSMenuDidRemoveItemNotification object:[self menu]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_menuDidChangeItemNotification:) name:NSMenuDidChangeItemNotification object:[self menu]];
#endif

    // Invoked when a menu is about to open.
    id<NSMenuDelegate> delegate = [[_view menu] delegate];
    if([delegate respondsToSelector:@selector(menuWillOpen:)]) [delegate menuWillOpen:[_view menu]];

#if 0
    // TODO: -numberOfItemsInMenu:
    // Invoked when a menu is about to be displayed at the start of a tracking session so the delegate can specify the number of items in the menu.
    if([delegate respondsToSelector:@selector(numberOfItemsInMenu:)] && [delegate respondsToSelector:@selector(menu:updateItem:atIndex:shouldCancel:)])
    {
        NSInteger numberOfMenuItems = [delegate numberOfItemsInMenu:[self menu]];
        if(numberOfMenuItems > 0)
        {
            // Resize the menu
            NSArray *itemArray = [[self menu] itemArray];
            for(NSInteger i = 0; i < numberOfMenuItems; i++)
            {
                if([delegate menu:[self menu] updateItem:[itemArray objectAtIndex:i] atIndex:i shouldCancel:([OEMenu OE_closing] || _cancelTracking)]) break;
            }
        }
    }
#endif

    [_view scrollToBeginningOfDocument:nil];
    [parentWindow addChildWindow:self ordered:NSWindowAbove];
    if(![parentWindow isKindOfClass:[OEMenu class]] && ![parentWindow isKeyWindow]) [parentWindow makeKeyAndOrderFront:self];
    if(![parentWindow isKindOfClass:[OEMenu class]] || [parentWindow isVisible]) [self orderFrontRegardless];

    if(![parentWindow isKindOfClass:[OEMenu class]])
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(parentWindowWillClose:) name:NSWindowWillCloseNotification object:parentWindow];
}

+ (NSPoint)OE_locationInScreenForEvent:(NSEvent *)event
{
    const NSPoint locationInWindow = [event locationInWindow];
    NSWindow *window               = [event window];
    return window == nil ? locationInWindow : [window convertRectToScreen:(NSRect){.origin=locationInWindow}].origin;
}

- (NSEvent *)OE_mockMouseEvent:(NSEvent *)event
{
    if([event window] == self) return event;

    NSPoint screenLocation = [OEMenu OE_locationInScreenForEvent:event];
    const NSPoint location = [self convertRectFromScreen:(NSRect){screenLocation, {0,0}}].origin;
    return [NSEvent mouseEventWithType:[event type] location:location modifierFlags:[event modifierFlags] timestamp:[event timestamp] windowNumber:[self windowNumber] context:[event context] eventNumber:[event eventNumber] clickCount:[event clickCount] pressure:[event pressure]];
}

- (NSEvent *)OE_showMenuAttachedToWindow:(NSWindow *)parentWindow withEvent:(NSEvent *)initialEvent
{
    NSEvent *postEvent = nil;
    [self OE_showMenuAttachedToWindow:parentWindow];

    // Invoked when a menu is about to be displayed at the start of a tracking session so the delegate can modify the menu.
    id<NSMenuDelegate> delegate = [[self menu] delegate];
    if([delegate respondsToSelector:@selector(menuNeedsUpdate:)]) [delegate menuNeedsUpdate:[self menu]];

    // Posted when menu tracking begins.
    [[NSNotificationCenter defaultCenter] postNotificationName:NSMenuDidBeginTrackingNotification object:[self menu]];

    OEMenu *menuWithMouseFocus = self; // Tracks menu that is currently under the cursor
    BOOL    dragged            = NO;   // Identifies if the mouse has seen a drag operation

    NSEvent *event;
    while(!_closing && !_cancelTracking && (event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES]))
    {
        @autoreleasepool
        {
            const NSEventType type = [event type];
            if(((type == NSLeftMouseUp) || (type == NSRightMouseUp) || (type == NSOtherMouseUp)) && (dragged || [event timestamp] - [initialEvent timestamp] > OEMenuClickDelay))
            {
                // Forward the mouse up message to the menu with the current focus
                [menuWithMouseFocus->_view mouseUp:[self OE_mockMouseEvent:event]];
                event = nil;  // There is no need to forward this message to NSApp
            }
            else if((type == NSLeftMouseDragged) || (type == NSRightMouseDragged) || (type == NSOtherMouseDragged))
            {
                // Notify mouse up event that we've seen a mouse drag event
                dragged = YES;

                // During mouse drag operations, the window that the user initiated the mouse down operation will be sent the subsequent mouse drag messages.  We want to forward this messages to the last menu that the user focused in on, the following allows us to mimick mouse entered and exited events while tracking the last focused menu.
                const NSPoint  locationInScreen = [OEMenu OE_locationInScreenForEvent:event];
                OEMenu        *newMenuFocus     = [OEMenu OE_menuAtPoint:locationInScreen];
                if((newMenuFocus != nil) && (menuWithMouseFocus != newMenuFocus))
                {
                    // If the menu with the focus has changed, let the old menu know that the mouse has exited it's view
                    if(menuWithMouseFocus) [menuWithMouseFocus->_view mouseExited:[menuWithMouseFocus OE_mockMouseEvent:event]];
                    if([newMenuFocus isKindOfClass:[OEMenu class]])
                    {
                        // Let the new menu know that the mouse has enterd it's view
                        menuWithMouseFocus = newMenuFocus;
                        [menuWithMouseFocus->_view mouseEntered:[menuWithMouseFocus OE_mockMouseEvent:event]];
                    }
                }
                else if(menuWithMouseFocus != nil)
                {
                    // Forward the message to the last menu with the focus
                    [menuWithMouseFocus->_view mouseDragged:[menuWithMouseFocus OE_mockMouseEvent:event]];
                }
                event = nil;  // There is no need to forward this message to NSApp
            }
            else if((type == NSMouseMoved) || (type == NSMouseEntered))
            {
                // Since the user can potentially start a mouse down followed by drag operation, we need to keep track of the focused window so that the future drag operation can function correctly
                if([[event window] isKindOfClass:[OEMenu class]]) menuWithMouseFocus = (OEMenu *)[event window];
            }
            else if(((type == NSLeftMouseDown) || (type == NSRightMouseDown) || (type == NSOtherMouseDown)) && ![[event window] isKindOfClass:[OEMenu class]])
            {
                // If a mouse down event occurred outside of a menu, then cancel tracking
                [self cancelTracking];

                postEvent = event; // Run this event again, after the OEMenu was cleaned up
                event = nil;  // There is no need to forward this message to NSApp
            }
            else if(type == NSScrollWheel && ![[event window] isKindOfClass:[OEMenu class]])
            {
                // Blackhole any scroll wheel messages not destined to a menu.
                event = nil;
            }
            else if((type == NSKeyDown) || (type == NSKeyUp))
            {
                // TODO: -performKeyEquivalent:

                // Key down messages should be sent to the deepest submenu that is open
                [[__sharedMenuStack lastObject] sendEvent:event];
                event = nil;  // There is no need to forward this message to NSApp
            }
            else if(type == NSFlagsChanged)
            {
                // Flags changes should be sent to all submenu's so that they can be updated appropriately
                [__sharedMenuStack makeObjectsPerformSelector:@selector(sendEvent:) withObject:event];
                event = nil;  // There is no need to forward this message to NSApp
            }

            // If we've gotten this far, then we need to forward the event to NSApp for additional processing
            if(event) [NSApp sendEvent:event];
        }
    }
    [NSApp discardEventsMatchingMask:NSAnyEventMask beforeEvent:event];

    // Posted when menu tracking ends, even if no action is sent.
    [[NSNotificationCenter defaultCenter] postNotificationName:NSMenuDidEndTrackingNotification object:[self menu]];

    return postEvent;
}

- (BOOL)isSubmenu
{
    // If this is not a menu at the top of the stack, then it must be a submenu
    return !((__sharedMenuStack == nil) || ([__sharedMenuStack count] == 0) || ([__sharedMenuStack objectAtIndex:0] == self));
}

- (OEMenuStyle)style
{
    return [_view style];
}

- (OERectEdge)arrowEdge
{
    return [_view arrowEdge];
}

- (NSSize)intrinsicSize
{
    return [_view intrinsicSize];
}

- (NSSize)size
{
    const NSSize maxSize = [self maxSize];
    NSSize       results = [self intrinsicSize];

    // TODO: What do we do if minimumWidth may be larger than maximum width
    // Make sure width is not smaller than the menu's minimumWidth
    results.width  = MAX(results.width, [[self menu] minimumWidth]);

    // Make sure that the size is not larger than the max size
    results.width  = MIN(results.width, maxSize.width);
    results.height = MIN(results.height, maxSize.height);
    
    return results;
}

- (void)setHighlightedItem:(NSMenuItem *)highlightedItem
{
    if(_highlightedItem != highlightedItem)
    {
        // Mark the old and new item's frames to be redrawn
        NSView *documentView = [_view documentView];
        if(_highlightedItem) [documentView setNeedsDisplayInRect:[[_highlightedItem extraData] frame]];
        if(highlightedItem)  [documentView setNeedsDisplayInRect:[[highlightedItem extraData] frame]];

        _highlightedItem = highlightedItem;
    }
}

- (void)setMenu:(NSMenu *)menu
{
    [super setMenu:menu];
    [_view setMenu:menu];
}

- (void)parentWindowWillClose:(NSNotification*)notification
{
    [self cancelTrackingWithoutAnimation];
}

@end

@implementation OEMenu (OEMenuViewAdditions)

+ (OEMenu *)OE_menuAtPoint:(NSPoint)point
{
    __block OEMenu *result = nil;
    [__sharedMenuStack enumerateObjectsWithOptions:NSEnumerationReverse usingBlock:
     ^ (OEMenu *obj, NSUInteger idx, BOOL *stop)
     {
         if(NSPointInRect(point, [obj frame]))
         {
             result = obj;
             *stop = YES;
         }
     }];
    return result;
}

- (void)OE_setClosing:(BOOL)closing
{
    if([__sharedMenuStack objectAtIndex:0] != self) [[__sharedMenuStack objectAtIndex:0] OE_setClosing:closing];
    else                                            _closing = closing;
}

- (BOOL)OE_closing
{
    if([__sharedMenuStack count] == 0) return YES;
    if([__sharedMenuStack objectAtIndex:0] != self) return [[__sharedMenuStack objectAtIndex:0] OE_closing];
    return _closing;
}

- (void)OE_setSubmenu:(NSMenu *)submenu
{
    if([_submenu menu] == submenu) return;
    [_submenu OE_hideWindowWithoutAnimation];

    if(submenu == nil)
    {
        _submenu = nil;
        return;
    }

    _submenu = [OEMenu OE_menuWithMenu:submenu forScreen:[self screen] options:[NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInteger:[_view style]] forKey:OEMenuOptionsStyleKey]];
    [self OE_updateFrameForSubmenu];
    [_submenu OE_showMenuAttachedToWindow:[_view window]];
}

- (OEMenu *)OE_submenu
{
    return _submenu;
}

- (OEMenu *)OE_supermenu
{
    NSWindow *supermenu = [self parentWindow];
    return ([supermenu isKindOfClass:[OEMenu class]] ? (OEMenu *)supermenu : nil);
}

- (OEMenuView *)OE_view
{
    return _view;
}

- (void)OE_hideWindowWithoutAnimation
{
    [self OE_hideWindowWithFadeDuration:0.0 completionHandler:nil];
}

- (void)OE_cancelTrackingWithCompletionHandler:(void (^)(void))completionHandler
{
    [self OE_cancelTrackingWithFadeDuration:OEMenuFadeOutDuration completionHandler:completionHandler];
}

@end


const static char OEMenuReference;
@implementation NSMenu (OEMenuAdditions)
- (void)setOEMenu:(OEMenu *)oeMenu
{
    objc_setAssociatedObject(self, &OEMenuReference, oeMenu, OBJC_ASSOCIATION_ASSIGN);
}

- (OEMenu*)oeMenu
{
    return objc_getAssociatedObject(self, &OEMenuReference);
}
@end
