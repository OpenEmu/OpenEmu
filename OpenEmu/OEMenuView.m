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

#import "OEMenuView.h"
#import "OEMenuView+OEMenuAdditions.h"
#import "OEMenu+OEMenuViewAdditions.h"
#import "OEMenuDocumentView.h"
#import "OEMenuDocumentView+OEMenuView.h"
#import "NSMenuItem+OEMenuItemExtraDataAdditions.h"
#import "OETheme.h"
#import "OEMenuItemExtraData.h"

#pragma mark -
#pragma mark Background Image Insets

static const NSEdgeInsets OEMenuBackgroundNoEdgeInsets   = { 5.0,  5.0,  5.0,  5.0};
static const NSEdgeInsets OEMenuBackgroundMinXEdgeInsets = { 5.0, 14.0,  5.0,  5.0};
static const NSEdgeInsets OEMenuBackgroundMaxXEdgeInsets = { 5.0,  5.0,  5.0, 14.0};
static const NSEdgeInsets OEMenuBackgroundMinYEdgeInsets = { 5.0,  5.0, 14.0,  5.0};
static const NSEdgeInsets OEMenuBackgroundMaxYEdgeInsets = {14.0,  5.0,  5.0,  5.0};

#pragma mark -
#pragma mark Content Insets

const NSEdgeInsets OEMenuContentEdgeInsets = { 7.0,  0.0,  7.0,  0.0}; // Value is extern, used by OEMenu.m to calculate menu placement

#pragma mark -
#pragma mark Edge Arrow Sizes

static const NSSize OEMinXEdgeArrowSize = (NSSize){10.0, 15.0};
static const NSSize OEMaxXEdgeArrowSize = (NSSize){10.0, 15.0};
static const NSSize OEMinYEdgeArrowSize = (NSSize){15.0, 10.0};
static const NSSize OEMaxYEdgeArrowSize = (NSSize){15.0, 10.0};

#pragma mark -
#pragma mark Animation Timing

static const CGFloat OEMenuItemFlashDelay       = 0.075; // Duration to flash an item on and off, after user wants to perform a menu item action
static const CGFloat OEMenuItemHighlightDelay   = 1.0;   // Delay before changing the highlight of an item with a submenu
static const CGFloat OEMenuItemShowSubmenuDelay = 0.07;  // Delay before showing an item's submenu
static const CGFloat OEMenuScrollAutoDelay      = 0.1;

const CGFloat OEMenuItemIndentation = 10.0;
#pragma mark -

static const CGFloat OEMenuScrollArrowHeight = 19.0;
static const CGFloat OEMenuScrollAutoStep    = 8.0;

#pragma mark -

// Fake menu scroller (doesn't render anything)
@interface _OEMenuScroller : NSScroller

@end

@interface _OEMenuScrollIndicatorView : NSView

@property(nonatomic, retain) NSImage *arrow;

@end

@interface OEMenuView ()

- (void)OE_updateTheme;
- (void)OE_setNeedsLayout;
- (OEMenu *)OE_menu;

@end

@implementation OEMenuView
@synthesize scrollView = _scrollView;
@synthesize documentView = _documentView;
@synthesize style = _style;
@synthesize arrowEdge = _arrowEdge;
@synthesize attachedPoint = _attachedPoint;
@synthesize backgroundEdgeInsets = _backgroundEdgeInsets;
@synthesize clippingRect = _clippingRect;

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        _scrollView              = [[NSScrollView alloc] initWithFrame:[self bounds]];
        _documentView            = [[OEMenuDocumentView alloc] initWithFrame:[self bounds]];
        _scrollUpIndicatorView   = [[_OEMenuScrollIndicatorView alloc] initWithFrame:NSZeroRect];
        _scrollDownIndicatorView = [[_OEMenuScrollIndicatorView alloc] initWithFrame:NSZeroRect];

        [_scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [_scrollView setDrawsBackground:NO];
        [_scrollView setBorderType:NSNoBorder];
        [_scrollView setHasVerticalScroller:YES];
        [_scrollView setVerticalScroller:[[_OEMenuScroller alloc] init]];
        [_scrollView setVerticalScrollElasticity:NSScrollElasticityNone];
        [_scrollView setDocumentView:_documentView];

        [_documentView setStyle:_style];

        [self addSubview:_scrollView];
        [self addSubview:_scrollUpIndicatorView];
        [self addSubview:_scrollDownIndicatorView];

        [self OE_updateTheme];
        [self OE_setNeedsLayout];

        // Add an observer to the  bounds of the scroller's content view as it will determine if / when the view has been scrolled
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(documentViewBoundsDidChange:) name:NSViewBoundsDidChangeNotification object:[_scrollView contentView]];
    }

    return self;
}

- (void)viewDidMoveToSuperview
{
    // If the superview has been changed, then make sure we invalidate any of the timers that affect the visual feedback
    [_autoScrollTimer invalidate];
    _autoScrollTimer = nil;
    [_delayedHighlightTimer invalidate];
    _delayedHighlightTimer = nil;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self OE_layoutIfNeeded];

    if(_backgroundColor != nil && _backgroundColor != [NSColor clearColor])
    {
        [_backgroundColor setFill];
        [_borderPath fill];
    }
    [_backgroundGradient drawInBezierPath:_borderPath angle:90];

    if([[self OE_menu] isSubmenu] || _effectiveArrowEdge == OENoEdge)
    {
        [_backgroundImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
    }
    else
    {
        NSRect borderRect = [self bounds];
        switch(_effectiveArrowEdge)
        {
            case OEMaxXEdge:
                borderRect.size.width -= OEMinXEdgeArrowSize.width - 1.0;
                break;
            case OEMinXEdge:
                borderRect.origin.x   += OEMaxXEdgeArrowSize.width - 1.0;
                borderRect.size.width -= OEMaxXEdgeArrowSize.width - 1.0;
                break;
            case OEMaxYEdge:
                borderRect.size.height -= OEMinYEdgeArrowSize.height - 1.0;
                break;
            case OEMinYEdge:
                borderRect.origin.y    += OEMaxYEdgeArrowSize.height - 1.0;
                borderRect.size.height -= OEMaxYEdgeArrowSize.height - 1.0;
                break;
            default:
                break;
        }

        [_arrowImage drawInRect:_rectForArrow fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];

        NSBezierPath *clipPath = [NSBezierPath bezierPathWithRect:borderRect];
        [clipPath appendBezierPathWithRect:_rectForArrow];
        [clipPath setWindingRule:NSEvenOddWindingRule];

        [NSGraphicsContext saveGraphicsState];
        [clipPath addClip];
        [_backgroundImage drawInRect:borderRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        [NSGraphicsContext restoreGraphicsState];
    }
}

- (void)updateTrackingAreas
{
    if(_trackingArea) [self removeTrackingArea:_trackingArea];

    const NSRect bounds = OENSInsetRectWithEdgeInsets([self bounds], _backgroundEdgeInsets);
    _trackingArea = [[NSTrackingArea alloc] initWithRect:bounds options:NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways owner:self userInfo:nil];
    [self addTrackingArea:_trackingArea];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (NSMenu *)menuForEvent:(NSEvent *)event
{
    return nil;
}

- (void)OE_autoScroll:(NSTimer *)timer
{
    id scrollIndicator = [timer userInfo];
    if([scrollIndicator isKindOfClass:[_OEMenuScrollIndicatorView class]])
    {
        NSPoint newPoint  = [[_scrollView contentView] bounds].origin;
        newPoint.y       -= OEMenuScrollAutoStep * (scrollIndicator == _scrollUpIndicatorView ? -1 : 1);
        [self OE_scrollToPoint:newPoint];

        if([scrollIndicator isHidden])
        {
            [_autoScrollTimer invalidate];
            _autoScrollTimer = nil;
            [self OE_highlightItemUnderMouse];
        }
    }
    else
    {
        [_documentView autoscroll:_lastDragEvent];
    }
}

- (void)OE_cancelDelayedHighlightTimer
{
    // The following prevents an ancestor of this focused menu from being closed
    OEMenu *supermenu = [self OE_menu];
    while(supermenu)
    {
        OEMenuView *view = [supermenu OE_view];
        if(view->_delayedHighlightTimer)
        {
            [view->_delayedHighlightTimer invalidate];
            view->_delayedHighlightTimer = nil;
        }
        supermenu = [supermenu OE_supermenu];
    }
}

- (void)OE_highlightItemOrAutoScrollWithPoint:(NSPoint)point
{
    [[[[self OE_menu] OE_supermenu] OE_view] OE_cancelDelayedHighlightTimer];

    // Check to see if we are hovering over the scroll up or down indicators
    id userInfo = nil;
    if(![_scrollUpIndicatorView isHidden] && NSPointInRect(point, [_scrollUpIndicatorView frame]))
    {
        userInfo = _scrollUpIndicatorView;
    }
    else if(![_scrollDownIndicatorView isHidden] && NSPointInRect(point, [_scrollDownIndicatorView frame]))
    {
        userInfo = _scrollDownIndicatorView;
    }
    else if((_lastDragEvent != nil) && ((point.y < NSMinY([_scrollView frame])) || (point.y > NSMaxY([_scrollView frame]))))
    {
        userInfo = _documentView;
    }
    else if(_autoScrollTimer != nil)
    {
        // We are not hovering over the scroll indicators, there is no need to keep the timer around (if it exists)
        [_autoScrollTimer invalidate];
        _autoScrollTimer = nil;
    }

    // -highlightItemAtPoint: will clear out the highlight if we are hovering over the scroll indicators
    [self highlightItemAtPoint:point];

    if(userInfo != nil && [_autoScrollTimer userInfo] != userInfo)
    {
        // Go ahead and set up the automatic scroll timer
        [_autoScrollTimer invalidate];
        _autoScrollTimer = [NSTimer timerWithTimeInterval:OEMenuScrollAutoDelay target:self selector:@selector(OE_autoScroll:) userInfo:userInfo repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:_autoScrollTimer forMode:NSDefaultRunLoopMode];
    }
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    if([[self OE_menu] OE_closing]) return;
    [self OE_highlightItemOrAutoScrollWithPoint:[self convertPoint:[theEvent locationInWindow] fromView:nil]];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [_autoScrollTimer invalidate];
    _autoScrollTimer = nil;

    if([[self OE_menu] OE_closing]) return;

    // If we are recovering from a mouse drag operation and the selected menu item has a submenu, then cancel our tracking
    OEMenu *menu = [self OE_menu];
    if ((_lastDragEvent != nil) && [[menu highlightedItem] hasSubmenu]) [menu cancelTracking];
    else                                                                [self OE_performAction];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    if([[self OE_menu] OE_closing]) return;
    [self OE_highlightItemOrAutoScrollWithPoint:[self convertPoint:[theEvent locationInWindow] fromView:nil]];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if([[self OE_menu] OE_closing]) return;
    [self OE_highlightItemOrAutoScrollWithPoint:[self convertPoint:[theEvent locationInWindow] fromView:nil]];
}

- (void)OE_highlightItemUnderMouse
{
    NSPoint screenLocation = [NSEvent mouseLocation];
    NSPoint windowLocation = [[self window] convertRectFromScreen:(NSRect){screenLocation, {0,0}}].origin;
    NSPoint viewLocation   = [self convertPoint:windowLocation fromView:nil];
    [self highlightItemAtPoint:viewLocation];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if([[self OE_menu] OE_closing]) return;

    _lastDragEvent = theEvent;
    [self OE_highlightItemOrAutoScrollWithPoint:[self convertPoint:[theEvent locationInWindow] fromView:nil]];
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    if([[self OE_menu] OE_closing]) return;

    // Figure out if any of the modifier flags that we are interested have changed
    NSUInteger modifierFlags = [theEvent modifierFlags] & [_documentView keyModifierMask];
    if(_lastKeyModifierMask != modifierFlags)
    {
        _lastKeyModifierMask = modifierFlags;

        OEMenu *menu = [self OE_menu];
        NSMenuItem *previousHighlightedItem = [menu highlightedItem];
        NSMenuItem *newHighlightedItem = ([previousHighlightedItem isAlternate] ?
                                          [[previousHighlightedItem extraData] primaryItem] :
                                          [[previousHighlightedItem extraData] itemWithModifierMask:modifierFlags]);
        [menu setHighlightedItem:newHighlightedItem];

        [_documentView flagsChanged:theEvent];
    }
}

- (BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
    if([[self OE_menu] OE_closing]) return YES;

    // I couldn't find an NSResponder method that was tied to the Return and Space key
    if([theEvent keyCode] == kVK_Return || [theEvent keyCode] == kVK_Space)
    {
        // Open the associated submenu or perform the associated action
        if([[[self OE_menu] highlightedItem] hasSubmenu]) [self moveRight:nil];
        else                                              [self OE_performAction];

        return YES;
    }

    return NO;
}

- (void)OE_scrollToPoint:(NSPoint)point
{
    const CGFloat maxY = NSHeight([_documentView frame]) - NSHeight([_scrollView frame]);
    [_documentView scrollPoint:NSMakePoint(0.0, MIN(MAX(point.y, 0.0), maxY))];
}

- (void)OE_setHighlightedItemByKeyboard:(NSMenuItem *)item
{
    [[self OE_menu] setHighlightedItem:item];

    if(item != nil)
    {
        const NSRect visibleRect   = [self convertRect:_clippingRect toView:_documentView];
        const NSRect menuItemFrame = [[item extraData] frame];

        if(NSMaxY(menuItemFrame) > NSMaxY(visibleRect))      [self OE_scrollToPoint:NSMakePoint(0.0, NSMaxY(menuItemFrame) - NSHeight(visibleRect) - (NSMinY(_clippingRect) - NSMinY([_scrollView frame])))];
        else if(NSMinY(menuItemFrame) < NSMinY(visibleRect)) [self OE_scrollToPoint:NSMakePoint(0.0, NSMinY(menuItemFrame) - (NSMinY(_clippingRect) - NSMinY([_scrollView frame])))];
    }
}

- (void)moveUp:(id)sender
{
    OEMenu *menu = [self OE_menu];
    if([menu OE_closing]) return;

    NSArray         *itemArray = [[self menu] itemArray];
    const NSInteger  count     = [itemArray count];
    if(count == 0) return;

    // On Mac OS X, the up key does not roll over the highlighted item to the bottom -- if we are at the top, then we are done
    NSMenuItem *item  = [menu highlightedItem];
    NSInteger   index = [itemArray indexOfObject:item];
    if(index == 0) return;

    // If no item is highlighted then we begin from the bottom of the list, if an item is highlighted we go to the next preceeding valid item (not separator, not disabled, and not hidden)
    for(NSInteger i = (item == nil ? count : index) - 1; i >= 0; i--)
    {
        NSMenuItem *obj = [[[itemArray objectAtIndex:i] extraData] itemWithModifierMask:_lastKeyModifierMask];
        if(![obj isHidden] && ![obj isSeparatorItem] && [obj isEnabled] && ![obj isAlternate])
        {
            item = obj;
            break;
        }
    }

    [self OE_setHighlightedItemByKeyboard:item];
}

- (void)moveDown:(id)sender
{
    OEMenu *menu = [self OE_menu];
    if([menu OE_closing]) return;

    NSArray         *itemArray = [[self menu] itemArray];
    const NSInteger  count     = [itemArray count];
    if(count == 0) return;

    // On Mac OS X, the up key does not roll over the highlighted item to the top -- if we are at the bottom, then we are done
    NSMenuItem *item  = [menu highlightedItem];
    NSInteger   index = [itemArray indexOfObject:item];
    if(index == count - 1) return;

    // If no item is highlighted then we begin from the top of the list, if an item is highlighted we go to the next proceeding valid item (not separator, not disabled, and not hidden)
    for(NSInteger i = (item == nil ? -1 : index) + 1; i < count; i++)
    {
        NSMenuItem *obj = [[[itemArray objectAtIndex:i] extraData] itemWithModifierMask:_lastKeyModifierMask];
        if(![obj isHidden] && ![obj isSeparatorItem] && [obj isEnabled] && ![obj isAlternate])
        {
            item = obj;
            break;
        }
    }

    [self OE_setHighlightedItemByKeyboard:item];
}

- (void)moveLeft:(id)sender
{
    OEMenu *menu = [self OE_menu];
    if([menu OE_closing]) return;

    // Hide the menu, if this is a submenu
    if([menu isSubmenu]) [menu OE_hideWindowWithoutAnimation];
}

- (void)moveRight:(id)sender
{
    OEMenu *menu = [self OE_menu];
    if([menu OE_closing]) return;

    // If there was a submenu to expand, select the first entry of the submenu
    [self OE_immediatelyExpandHighlightedItemSubmenu];
    if([[menu highlightedItem] hasSubmenu]) [[[menu OE_submenu] OE_view] moveDown:self];
}

- (void)cancelOperation:(id)sender
{
    OEMenu *menu = [self OE_menu];
    if([menu OE_closing]) return;
    [menu cancelTracking];
}

- (void)scrollToBeginningOfDocument:(id)sender
{
    [self OE_layoutIfNeeded];
    [self OE_scrollToPoint:NSMakePoint(0.0, NSMaxY([_documentView frame]))];
}

- (void)scrollToEndOfDocument:(id)sender
{
    [self OE_layoutIfNeeded];
    [self OE_scrollToPoint:NSZeroPoint];
}

- (void)OE_flashItem:(NSMenuItem *)highlightedItem
{
    [[self OE_menu] setHighlightedItem:highlightedItem];
    [self performSelector:@selector(OE_sendAction:) withObject:highlightedItem afterDelay:OEMenuItemFlashDelay];
}

- (void)OE_sendAction:(NSMenuItem *)highlightedItem
{
    [[self OE_menu] OE_cancelTrackingWithCompletionHandler:^{
        NSDictionary *userInfo = [NSDictionary dictionaryWithObject:highlightedItem forKey:@"MenuItem"];
        [[NSNotificationCenter defaultCenter] postNotificationName:NSMenuWillSendActionNotification object:[self menu] userInfo:userInfo];
        [NSApp sendAction:[highlightedItem action] to:[highlightedItem target] from:highlightedItem];
        [[NSNotificationCenter defaultCenter] postNotificationName:NSMenuDidSendActionNotification object:[self menu] userInfo:userInfo];
    }];
}

- (void)OE_performAction
{
    OEMenu     *menu            = [self OE_menu];
    NSMenuItem *highlightedItem = [menu highlightedItem];

    if([menu OE_closing] || ([highlightedItem hasSubmenu] && [highlightedItem isEnabled])) return;
    [menu OE_setClosing:YES];

    if(highlightedItem != nil && ![highlightedItem isSeparatorItem] && [highlightedItem isEnabled])
    {
        // Flash the highlighted item right before closing the submenu
        [self performSelector:@selector(OE_flashItem:) withObject:highlightedItem afterDelay:OEMenuItemFlashDelay];
        [menu setHighlightedItem:nil];
    }
    else
    {
        [menu cancelTracking];
    }
}

- (void)OE_immediatelyExpandHighlightedItemSubmenu
{
    OEMenu *menu = [self OE_menu];
    if([[menu highlightedItem] isEnabled])
        [menu OE_setSubmenu:[[menu highlightedItem] submenu]];
}

- (void)OE_expandHighlightedItemSubmenu
{
    [self performSelector:@selector(OE_immediatelyExpandHighlightedItemSubmenu) withObject:nil afterDelay:OEMenuItemShowSubmenuDelay];
}

- (void)OE_setHighlightedItemByMouse:(NSMenuItem *)highlightedItem
{
    // Go ahead and switch the highlighted item (and expand the submenu as appropriate)
    OEMenu *menu = [self OE_menu];
    [menu OE_setSubmenu:nil];
    [menu setHighlightedItem:highlightedItem];
    if([highlightedItem hasSubmenu] && [highlightedItem isEnabled])
        [self OE_expandHighlightedItemSubmenu];
}

- (void)OE_delayedSetHighlightedItem:(NSTimer *)timer
{
    NSMenuItem *highlightedItem = [timer userInfo];
    _delayedHighlightTimer      = nil;
    [self OE_setHighlightedItemByMouse:highlightedItem];
}

- (void)highlightItemAtPoint:(NSPoint)point
{
    NSMenuItem *highlightedItem = [self itemAtPoint:point];

    if(_delayedHighlightTimer)
    {
        // Check to see if we are already waiting to highlight the requested item
        if ([_delayedHighlightTimer userInfo] == highlightedItem)
        {
            _lastHighlightPoint = point;
            return;
        }

        // We intend on highlighting a different item now
        [_delayedHighlightTimer invalidate];
        _delayedHighlightTimer = nil;
    }

    OEMenu *menu    = [self OE_menu];
    OEMenu *submenu = [menu OE_submenu];

    // Check to see if the item is already highlighted
    if([menu highlightedItem] == highlightedItem)
    {
        _lastHighlightPoint = point;
        return;
    }

    // If there is a menu open, then check to see if we are trying to move our mouse to that menu -- if we are, then delay any changes to see if the user makes it to the menu
    BOOL isMouseMovingCloser = NO;
    if(submenu)
    {
        const CGFloat distance = point.x - _lastHighlightPoint.x;
        isMouseMovingCloser    = ([submenu arrowEdge] == OEMinXEdge) ? (distance < -1.0) : (distance > 1.0);
    }

    if(isMouseMovingCloser)
    {
        _delayedHighlightTimer = [NSTimer timerWithTimeInterval:OEMenuItemHighlightDelay target:self selector:@selector(OE_delayedSetHighlightedItem:) userInfo:highlightedItem repeats:NO];
        [[NSRunLoop currentRunLoop] addTimer:_delayedHighlightTimer forMode:NSDefaultRunLoopMode];
    }
    else
    {
        [self OE_setHighlightedItemByMouse:highlightedItem];
    }
    _lastHighlightPoint = point;
}

- (NSMenuItem *)itemAtPoint:(NSPoint)point
{
    if(NSPointInRect(point, _clippingRect))
    {
        NSView *view = [self hitTest:point];
        if((view != nil) && (view != self) && [view isKindOfClass:[OEMenuDocumentView class]]) return [(OEMenuDocumentView *)view OE_itemAtPoint:[self convertPoint:point toView:view]];
    }

    return nil;
}

- (void)OE_updateScrollerVisibility
{
    if(NSIsEmptyRect([self bounds])) return;

    const NSRect bounds  = [self bounds];
    NSRect documentFrame = [_documentView frame];
    _clippingRect        = [_scrollView frame];

    if(NSHeight(bounds) < NSHeight(documentFrame))
    {
        const NSRect contentBounds    = [[_scrollView contentView] bounds];
        const BOOL   hideDown         = NSMinY(contentBounds) <= 0.0;
        const BOOL   hideUp           = NSMaxY(contentBounds) >= NSHeight(documentFrame);
        const BOOL   updateVisibility = ([_scrollUpIndicatorView isHidden] != hideUp) || ([_scrollDownIndicatorView isHidden] != hideDown);

        [_scrollUpIndicatorView setHidden:hideUp];
        [_scrollDownIndicatorView setHidden:hideDown];

        if(!hideDown) _clippingRect.origin.y = NSMaxY([_scrollDownIndicatorView frame]);
        _clippingRect.size.height            = (hideUp ? NSHeight(bounds) : NSMinY([_scrollUpIndicatorView frame])) - NSMinY(_clippingRect);

        if(updateVisibility) [self updateTrackingAreas];
    }
}

- (void)documentViewBoundsDidChange:(NSNotification *)notification
{
    if(NSIsEmptyRect([self bounds])) return;

    [self OE_updateScrollerVisibility];

    if([[[self window] currentEvent] type] == NSScrollWheel) [self OE_highlightItemUnderMouse];
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldSize
{
    [self OE_setNeedsLayout];
}

- (void)setMenu:(NSMenu *)menu
{
    [super setMenu:menu];
    [_documentView setItemArray:[menu itemArray]];
    // update the lastKeyModifierMask as the documentView may have updated
    _lastKeyModifierMask = [[NSApp currentEvent] modifierFlags] & [_documentView keyModifierMask];
}

- (void)setStyle:(OEMenuStyle)style
{
    if(_style != style)
    {
        _style = style;

        [_documentView setStyle:_style];
        [self OE_updateTheme];
    }
}

- (void)setArrowEdge:(OERectEdge)arrowEdge
{
    if(_arrowEdge != arrowEdge)
    {
        _arrowEdge = arrowEdge;
        [self OE_updateTheme];
    }
}

- (void)setAttachedPoint:(NSPoint)attachedPoint
{
    if(!NSEqualPoints(attachedPoint, _attachedPoint))
    {
        _attachedPoint = attachedPoint;
        [self OE_setNeedsLayout];
    }
}

- (NSSize)intrinsicSize
{
    [self OE_layoutIfNeeded];

    // Go through each item and calculate the maximum width and the sum of the height
    NSSize intrinsicSize = [_documentView intrinsicSize];

    __block NSInteger maxIndentationLevel = 0;
    [[[self menu] itemArray] enumerateObjectsUsingBlock:^(NSMenuItem *item, NSUInteger idx, BOOL *stop) {
        maxIndentationLevel = MAX([item indentationLevel], maxIndentationLevel);
    }];
    
    // Return a size with the appropriate padding
    return NSMakeSize(OEMenuItemIndentation*maxIndentationLevel + intrinsicSize.width + _backgroundEdgeInsets.left + _backgroundEdgeInsets.right + OEMenuContentEdgeInsets.left + OEMenuContentEdgeInsets.right, intrinsicSize.height + _backgroundEdgeInsets.top + _backgroundEdgeInsets.bottom + OEMenuContentEdgeInsets.top + OEMenuContentEdgeInsets.bottom);
}

- (void)OE_setNeedsLayout
{
    _needsLayout = YES;
    [self setNeedsDisplay:YES];
}

- (OEMenu *)OE_menu
{
    return (OEMenu *)[self window];
}

- (void)OE_updateTheme
{
    const BOOL isSubmenu = [[self OE_menu] isSubmenu];
    NSString *styleKeyPrefix = (_style == OEMenuStyleDark ? @"dark_menu_" : @"light_menu_");
    _backgroundImage         = [[OETheme sharedTheme] imageForKey:[styleKeyPrefix stringByAppendingString:[NSString stringWithFormat:@"%@background", (isSubmenu ? @"submenu_" : @"")]] forState:OEThemeStateDefault];
    _backgroundColor         = [[OETheme sharedTheme] colorForKey:[styleKeyPrefix stringByAppendingString:@"background"] forState:OEThemeStateDefault];
    _backgroundGradient      = [[OETheme sharedTheme] gradientForKey:[styleKeyPrefix stringByAppendingString:@"background"] forState:OEThemeStateDefault];
    NSImage *upArrow         = [[OETheme sharedTheme] imageForKey:[styleKeyPrefix stringByAppendingString:@"scroll_up_arrow"] forState:OEThemeStateDefault];
    NSImage *downArrorw      = [[OETheme sharedTheme] imageForKey:[styleKeyPrefix stringByAppendingString:@"scroll_down_arrow"] forState:OEThemeStateDefault];

    if(isSubmenu || (_arrowEdge == OENoEdge))
    {
        _arrowImage = nil;
    }
    else
    {
        NSString *edgeComponent = nil;
        switch (_arrowEdge) {
            case OEMinXEdge:
                edgeComponent = @"minx_arrow_body";
                break;
            case OEMaxXEdge:
                edgeComponent = @"maxx_arrow_body";
                break;
            case OEMinYEdge:
                edgeComponent = @"miny_arrow_body";
                break;
            case OEMaxYEdge:
                edgeComponent = @"maxy_arrow_body";
                break;
            default:
                break;
        }
        _arrowImage = [[OETheme sharedTheme] imageForKey:[styleKeyPrefix stringByAppendingString:edgeComponent] forState:OEThemeStateDefault];
    }

    [(_OEMenuScrollIndicatorView *)_scrollUpIndicatorView setArrow:upArrow];
    [(_OEMenuScrollIndicatorView *)_scrollDownIndicatorView setArrow:downArrorw];

    [self OE_setNeedsLayout];
}

@end

@implementation OEMenuView (OEMenuAdditions)

+ (NSEdgeInsets)OE_backgroundEdgeInsetsForEdge:(OERectEdge)edge
{
    switch(edge)
    {
        case OEMinXEdge: return OEMenuBackgroundMinXEdgeInsets;
        case OEMaxXEdge: return OEMenuBackgroundMaxXEdgeInsets;
        case OEMinYEdge: return OEMenuBackgroundMinYEdgeInsets;
        case OEMaxYEdge: return OEMenuBackgroundMaxYEdgeInsets;
        case OENoEdge:
        default:         return OEMenuBackgroundNoEdgeInsets;
    }
}

- (void)OE_layoutIfNeeded
{
    if(!_needsLayout) return;
    _needsLayout = NO;

    const BOOL isSubmenu  = [[self OE_menu] isSubmenu];
    const NSRect bounds   = [self bounds];
    NSPoint attachedPoint = _attachedPoint;

    if(NSEqualPoints(attachedPoint, NSZeroPoint))
    {
        // Calculate the attached point if it is not set but the arrow edge is set
        switch(_arrowEdge)
        {
            case OEMinXEdge:
                attachedPoint.x = NSMinX([self bounds]);
                attachedPoint.y = NSMidY([self bounds]);
                break;
            case OEMaxXEdge:
                attachedPoint.x = NSMaxX([self bounds]);
                attachedPoint.y = NSMidY([self bounds]);
                break;
            case OEMinYEdge:
                attachedPoint.x = NSMidX([self bounds]);
                attachedPoint.y = NSMinY([self bounds]);
                break;
            case OEMaxYEdge:
                attachedPoint.x = NSMidX([self bounds]);
                attachedPoint.y = NSMaxY([self bounds]);
                break;
            default:
                break;
        }
        _effectiveArrowEdge = _arrowEdge;
    }
    else if(attachedPoint.x < NSMinX(bounds) || attachedPoint.x > NSMaxX(bounds) || attachedPoint.y < NSMinY(bounds) || attachedPoint.y > NSMaxY(bounds))
    {
        // If the attached point is not visible, then effectively hide the arrow
        _effectiveArrowEdge = OENoEdge;
    }
    else
    {
        // There are no problems...so the effective arrow edge is the edge that was requested
        _effectiveArrowEdge = _arrowEdge;
    }

    // Recalculate border path
    _backgroundEdgeInsets         = [OEMenuView OE_backgroundEdgeInsetsForEdge:(isSubmenu ? OENoEdge : _effectiveArrowEdge)];
    const NSRect backgroundBounds = OENSInsetRectWithEdgeInsets([self bounds], _backgroundEdgeInsets);

    if(isSubmenu || _effectiveArrowEdge == OENoEdge)
    {
        _borderPath = [NSBezierPath bezierPathWithRoundedRect:backgroundBounds xRadius:(isSubmenu ? 0 : 2) yRadius:2];
    }
    else
    {
        NSRect  arrowRect = NSZeroRect;
        NSPoint point1    = NSZeroPoint;
        NSPoint point2    = NSZeroPoint;
        NSPoint point3    = NSZeroPoint;
        CGFloat v1, v2;

        switch(_effectiveArrowEdge)
        {
            case OEMinXEdge:
            case OEMaxXEdge:
                if(_effectiveArrowEdge == OEMinXEdge)
                {
                    arrowRect.size     = OEMinXEdgeArrowSize;
                    arrowRect.origin.x = attachedPoint.x +_backgroundEdgeInsets.left - arrowRect.size.width;
                    v1                 = NSMaxX(arrowRect);
                    v2                 = NSMinX(arrowRect);
                }
                else
                {
                    arrowRect.size     = OEMaxXEdgeArrowSize;
                    arrowRect.origin.x = attachedPoint.x - _backgroundEdgeInsets.right;
                    v1                 = NSMinX(arrowRect);
                    v2                 = NSMaxX(arrowRect);
                }
                arrowRect.origin.y = attachedPoint.y - floor((fabs(_backgroundEdgeInsets.top - _backgroundEdgeInsets.bottom) + arrowRect.size.height) / 2.0);

                point1 = NSMakePoint(v1, NSMinY(arrowRect));
                point2 = NSMakePoint(v2, floor(NSMidY(arrowRect)));
                point3 = NSMakePoint(v1, NSMaxY(arrowRect));
                break;
            case OEMinYEdge:
            case OEMaxYEdge:
                if(_effectiveArrowEdge == OEMinYEdge)
                {
                    arrowRect.size     = OEMinYEdgeArrowSize;
                    arrowRect.origin.y = attachedPoint.y + _backgroundEdgeInsets.bottom - arrowRect.size.height;
                    v1 = NSMaxY(arrowRect);
                    v2 = NSMinY(arrowRect);
                }
                else
                {
                    arrowRect.size     = OEMinYEdgeArrowSize;
                    arrowRect.origin.y = attachedPoint.y - _backgroundEdgeInsets.top;
                    v1                 = NSMinY(arrowRect);
                    v2                 = NSMaxY(arrowRect);
                }
                arrowRect.origin.x = attachedPoint.x - floor((fabs(_backgroundEdgeInsets.left - _backgroundEdgeInsets.right) + arrowRect.size.width) / 2.0);

                point1 = NSMakePoint(NSMinX(arrowRect),        v1);
                point2 = NSMakePoint(floor(NSMidX(arrowRect)), v2);
                point3 = NSMakePoint(NSMaxX(arrowRect),        v1);

                break;
            default:
                break;
        }

        _borderPath = [NSBezierPath bezierPath];
        [_borderPath moveToPoint:point1];
        [_borderPath lineToPoint:point2];
        [_borderPath lineToPoint:point3];
        [_borderPath lineToPoint:point1];
        [_borderPath appendBezierPathWithRoundedRect:backgroundBounds xRadius:2 yRadius:2];

        _rectForArrow.origin = NSZeroPoint;
        _rectForArrow.size   = [_arrowImage size];
        switch(_effectiveArrowEdge)
        {
            case OEMaxXEdge:
                _rectForArrow.origin.y = NSMidY(arrowRect) - NSMidY(_rectForArrow);
                _rectForArrow.origin.x = NSMaxX(bounds) - NSWidth(_rectForArrow);
                break;
            case OEMinXEdge:
                _rectForArrow.origin.y = NSMidY(arrowRect) - NSMidY(_rectForArrow);
                break;
            case OEMaxYEdge:
                _rectForArrow.origin.x = NSMidX(arrowRect) - NSMidX(_rectForArrow);
                _rectForArrow.origin.y = NSMaxY(bounds) - NSHeight(_rectForArrow);
                break;
            case OEMinYEdge:
                _rectForArrow.origin.x = NSMidX(arrowRect) - NSMidX(_rectForArrow);
                break;
            default:
                break;
        }
        _rectForArrow = NSIntegralRect(_rectForArrow);
    }

    [_scrollView setFrame:OENSInsetRectWithEdgeInsets(backgroundBounds, OEMenuContentEdgeInsets)];
    [_documentView setFrameSize:NSMakeSize(NSWidth([_scrollView frame]), NSHeight([_documentView frame]))];

    if(NSHeight(bounds) < NSHeight([_documentView frame]))
    {
        NSRect contentFrame = [_scrollView frame];
        NSRect upFrame;
        NSRect downFrame;

        NSDivideRect(contentFrame, &upFrame,   &contentFrame, OEMenuScrollArrowHeight, NSMaxYEdge);
        NSDivideRect(contentFrame, &downFrame, &contentFrame, OEMenuScrollArrowHeight, NSMinYEdge);

        [_scrollUpIndicatorView setFrame:upFrame];
        [_scrollDownIndicatorView setFrame:downFrame];
    }

    [self OE_updateScrollerVisibility];
}

@end

@implementation _OEMenuScroller

+ (BOOL)isCompatibleWithOverlayScrollers
{
    return YES;
}

- (void)drawKnob
{
}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{
}

+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize scrollerStyle:(NSScrollerStyle)scrollerStyle
{
    return 0.0;
}

@end

@implementation _OEMenuScrollIndicatorView
@synthesize arrow = _arrow;

- (void)drawRect:(NSRect)dirtyRect
{
    // TODO: Should be a themed view that draws based on state changes (if various states are provided)
    const NSSize arrowSize = [_arrow size];
    const NSPoint point = { ceil(NSMidX([self bounds]) - (arrowSize.width / 2.0)), ceil(NSMidY([self bounds]) - (arrowSize.height / 2.0))};
    [_arrow drawAtPoint:point fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
}

- (void)setArrow:(NSImage *)arrow
{
    if(_arrow != arrow)
    {
        _arrow = arrow;
        [self setNeedsDisplay:YES];
    }
}

@end
