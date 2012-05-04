/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
  *Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
  *Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
  *Neither the name of the OpenEmu Team nor the
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
#import "NSImage+OEDrawingAdditions.h"
#import "OEPopupButton.h"

#pragma mark Item Spaces
#define ItemTickMarkSpace 19.0
#define ItemImageSpace 22.0
#define ItemSubmenuSpace 10.0

#define ItemHeightWithImage 20.0
#define ItemHeightWithoutImage 17.0
#define ItemSeparatorHeight 7.0

#define ItemImageWidth 16.0
#define ItemImageHeight 16.0
#pragma mark -
#pragma mark Submenu Borders
#define SubmenuBorderLeft 5.0
#define SubmenuBorderRight 5.0
#define SubmenuBorderTop 9.0
#define SubmenuBorderBottom 9.0
#pragma mark -
#pragma mark NoEdge Borders
#define NoEdgeContentBorderLeft 5.0
#define NoEdgeContentBorderRight 5.0
#define NoEdgeContentBorderTop 9.0
#define NoEdgeContentBorderBottom 9.0
#pragma mark -
#pragma mark MaxX Borders
#define MaxXEdgeContentBorderLeft 14.0
#define MaxXEdgeContentBorderRight 5.0
#define MaxXEdgeContentBorderTop 9.0
#define MaxXEdgeContentBorderBottom 9.0
#pragma mark -
#pragma mark MinX Borders
#define MinXEdgeContentBorderLeft 5.0
#define MinXEdgeContentBorderRight 14.0
#define MinXEdgeContentBorderTop 9.0
#define MinXEdgeContentBorderBottom 9.0
#pragma mark -
#pragma mark MinY Borders
#define MinYEdgeContentBorderLeft 5.0
#define MinYEdgeContentBorderRight 5.0
#define MinYEdgeContentBorderTop 18.0
#define MinYEdgeContentBorderBottom 9.0
#pragma mark -
#pragma mark MaxY Borders
#define MaxYEdgeContentBorderLeft 5.0
#define MaxYEdgeContentBorderRight 5.0
#define MaxYEdgeContentBorderTop 9.0
#define MaxYEdgeContentBorderBottom 18.0
#pragma mark -
#pragma mark Background Gradient Sizes
#define BGInsetTopNoEdge 5.0
#define BGInsetLeftNoEdge 5.0
#define BGInsetRightNoEdge 5.0
#define BGInsetBottomNoEdge 5.0
#define BGInsetTopEdge 14.0
#define BGInsetBottomEdge 14.0
#define BGInsetLeftEdge 14.0
#define BGInsetRightEdge 14.0

#pragma mark -
#pragma mark Edge Sizes
#define BGEdgeSizeLeft (NSSize){8.0, 14.0}
#define BGEdgeSizeRight (NSSize){8.0, 14.0}
#define BGEdgeSizeTop (NSSize) {14.0, 8.0}
#define BGEdgeSizeBottom (NSSize) {14.0, 8.0}

#pragma mark -
#pragma mark Background Image Sizes
#define BGImageCornerWidth 10.0
#define BGImageCornerHeight 10.0
#define BGImageCornerEdgeInset 9.0

#pragma mark -
#pragma mark Arrow Image Sources
#define LeftArrowSource (NSRect){{0.0, 18.0},{14.0, 21.0}}
#define RightArrowSource (NSRect){{40.0, 18.0},{14.0, 21.0}}
#define BottomArrowSource (NSRect){{18.0, 0.0},{18.0, 14.0}}
#define TopArrowSource (NSRect){{18.0, 42.0},{19.0, 15.0}}

#pragma mark -
#pragma mark Animation
#define flickerDelay 0.09
#pragma mark -
#pragma mark -

@class OEMenuItemsView, OEMenuContentView, OEMenuScrollerView;
@interface OEMenu ()
- (BOOL)_isClosing;
- (OEMenuContentView *)menuView;
- (void)_performCloseMenu;
- (void)_closeByClickingItem:(NSMenuItem *)selectedItem;
- (void)setIsAlternate:(BOOL)flag;
- (CAAnimation*)alphaValueAnimation;
- (void)setAlphaValueAnimation:(CAAnimation *)anim;

- (void)OE_createEventMonitor;
- (void)OE_removeEventMonitor;

@property NSRect openRect; 

- (void)OE_repositionMenu;
- (NSPoint)OE_originForEdge:(OERectEdge)anEdge ofWindow:(NSWindow*)win;
- (OERectEdge)OE_oppositeRectEdgeForEdge:(OERectEdge)edge;
@end

#pragma mark -
@interface OEMenuContentView : NSView
- (int)OE_submenuPosition;
- (void)highlightItemAtPoint:(NSPoint)p;
- (void)highlightItemWithScrollingAtPoint:(NSPoint)p;
- (void)mouseHighlightItemAtPoint:(NSPoint)p;
- (NSMenuItem *)itemAtPoint:(NSPoint)p;
- (NSRect)rectOfItem:(NSMenuItem *)m;

- (BOOL)menuKeyDown:(NSEvent *)theEvent;

- (void)updateScrollerViews:(BOOL)animated;
- (void)scrollDown;
- (void)scrollUp;
- (void)scrollBy:(float)yDelta;
#pragma mark -
#pragma mark TextAttributes
- (NSDictionary *)itemTextAttributes;
- (NSDictionary *)selectedItemTextAttributes;
- (NSDictionary *)selectedItemAlternateTextAttributes;
- (NSDictionary *)disabledItemTextAttributes;

#pragma mark -
@property(nonatomic, readonly) OEMenu           *menu;
@property(nonatomic, readonly) OEMenuItemsView *menuItemsView;
@property(nonatomic, readonly) NSScrollView     *scrollView;
@property NSPoint cachedContentOffset;
@property NSSize  cachedBorderSize;

@property (nonatomic, readonly) OEMenuScrollerView *scrollDownView;
@property (nonatomic, readonly) OEMenuScrollerView *scrollUpView;
@property NSTimer *scrollTimer;

@property NSPoint lastHighlightPoint;
@end

@interface OEMenuContentView ()
- (NSSize)OE_calculateRequiredViewSize;
@end
#pragma mark -
@interface OEMenuItemsView : NSView
- (NSMenuItem *)itemAtPoint:(NSPoint)p;
- (NSRect)rectOfItem:(NSMenuItem *)m;

- (OEMenu *)menu;
@end
@interface OEMenuItemsView ()
- (NSSize)OE_calculateAndSetRequiredViewSize;
@end
#pragma mark -
@interface OE_MenuScrollView : NSScrollView

- (OEMenu *)menu;
@end
#pragma mark -
@interface OE_MenuScroller : NSScroller
@end
#pragma mark -
@interface OEMenuScrollerView : NSView
@property BOOL up;
- (NSImage*)scrollUpArrowImageForStyle:(OEMenuStyle)style;
- (NSImage*)scrollDownArrowImageForStyle:(OEMenuStyle)style;
@end
#pragma mark -
#pragma mark -
@implementation OEMenu
@synthesize openRect, openEdge=_edge, allowsOppositeEdge, displaysOpenEdge;
@dynamic style;
@synthesize menu, supermenu, visible, popupButton, delegate;
@synthesize minSize, maxSize, itemsAboveScroller, itemsBelowScroller;
@synthesize alternate=_alternate;
@synthesize containsItemWithImage;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEMenu class])
        return;
    
    NSImage *menuArrows = [NSImage imageNamed:@"dark_menu_popover_arrow"];
    [menuArrows setName:@"dark_menu_popover_arrow_normal" forSubimageInRect:NSMakeRect(0, menuArrows.size.height/2, menuArrows.size.width, menuArrows.size.height/2)];
    [menuArrows setName:@"dark_menu_popover_arrow_selected" forSubimageInRect:NSMakeRect(0, 0, menuArrows.size.width, menuArrows.size.height/2)];
        
    NSImage *scrollArrows = [NSImage imageNamed:@"dark_menu_scroll_arrows"];
    [scrollArrows setName:@"dark_menu_scroll_down" forSubimageInRect:NSMakeRect(0, 0, 9, 7)];
    [scrollArrows setName:@"dark_menu_scroll_up" forSubimageInRect:NSMakeRect(9, 0, 9, 7)];
    
    scrollArrows = [NSImage imageNamed:@"light_menu_scroll_arrows"];
    [scrollArrows setName:@"light_menu_scroll_down" forSubimageInRect:NSMakeRect(0, 0, 9, 7)];
    [scrollArrows setName:@"light_menu_scroll_up" forSubimageInRect:NSMakeRect(9, 0, 9, 7)];
    
    NSImage *tickMark = [NSImage imageNamed:@"tick_mark"];
    [tickMark setName:@"tick_mark_normal" forSubimageInRect:(NSRect){{0,0},{7,12}}];
    [tickMark setName:@"tick_mark_selected" forSubimageInRect:(NSRect){{7,0},{7,12}}];
    
    NSImage *mixedState = [NSImage imageNamed:@"mixed_state"];
    [mixedState setName:@"mixed_state_normal" forSubimageInRect:(NSRect){{0,0},{7,12}}];
    [mixedState setName:@"mixed_state_selected" forSubimageInRect:(NSRect){{7,0},{7,12}}];
}

- (id)init
{
    if((self = [super initWithContentRect:NSZeroRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO]))
    {
        [self setStyle:OEMenuStyleDark];
        [self setMinSize:NSZeroSize];
        [self setMaxSize:(NSSize){5000,500}];
        
        [self setItemsAboveScroller:0];
        [self setItemsBelowScroller:0];
        
        [self setAllowsOppositeEdge:YES];
        
        OEMenuContentView *view = [[OEMenuContentView alloc] initWithFrame:(NSRect){{0,0}, {1,1}}];
        [view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        [[self contentView] addSubview:view];
        
        [self setLevel:NSTornOffMenuWindowLevel];
        [self setBackgroundColor:[NSColor clearColor]];
        [self setHasShadow:NO];
        [self setOpaque:NO];
        [self setAlphaValue:0.0];
        
        CAAnimation *anim = [self alphaValueAnimation];
        [anim setDuration:0.075];
        [anim setDelegate:self];
        [self setAlphaValueAnimation:anim];
    }
    return self;
}

- (void)dealloc
{
    [self OE_removeEventMonitor];
}

#pragma mark -
#pragma mark Opening / Closing the menu
- (void)openOnEdge:(OERectEdge)anEdge ofRect:(NSRect)rect ofWindow:(NSWindow*)win
{
    visible = YES;
    closing = NO;
    _alternate = NO;
    
    [self OE_removeEventMonitor];
    
    [self setOpenEdge:anEdge];
    [self setOpenRect:rect];
    [self updateSize];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(closeMenuWithoutChanges:) name:NSApplicationWillResignActiveNotification object:NSApp];

    NSPoint point = [self OE_originForEdge:anEdge ofWindow:win];

    [self setFrameOrigin:point];
    [win addChildWindow:self ordered:NSWindowAbove];    
    
    [self OE_repositionMenu];
    
    NSPoint windowP = [self convertScreenToBase:[NSEvent mouseLocation]];
    [[self menuView] highlightItemWithScrollingAtPoint:windowP];
    
    [self display];
    [self setAlphaValue:1.0];
    
    [self OE_createEventMonitor];
    
    if([[self delegate] respondsToSelector:@selector(menuDidShow:)])
        [[self delegate] menuDidShow:self];
}

- (void)closeMenuWithoutChanges:(id)sender
{
    closing = YES;

    if([self submenu]) [self.submenu closeMenuWithoutChanges:sender];
    
    [self _performCloseMenu];
    
    if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidCancel:)]) [self.delegate performSelector:@selector(menuDidCancel:) withObject:self];

}

- (void)closeMenu
{   
    closing = YES;
    
    OEMenu *superMen = self;
    while([superMen supermenu])
        superMen = [superMen supermenu];
    
    if(superMen != self)
    {
        [superMen closeMenu];
        return;
    }
    
    OEMenu *subMen = self;
    while([subMen submenu])
        subMen = [subMen submenu];
    
    NSMenuItem *selectedItem = [subMen highlightedItem];
    [self _closeByClickingItem:selectedItem];
}
#pragma mark -
#pragma mark Positioning / Sizing
- (void)OE_repositionMenu
{   
    NSRect menuRect         =   [self frame];
    NSRect screenRect       =   [[[self parentWindow] screen] visibleFrame];
    OERectEdge edge         =   [self openEdge];
    NSRect intersectionRect =   NSIntersectionRect(menuRect, screenRect);
    if(NSEqualSizes(menuRect.size, intersectionRect.size))
        return;
    
    // check if x is the problem
    if([self allowsOppositeEdge])
    {
        if((edge == OEMaxXEdge || edge == OEMinXEdge) && (intersectionRect.size.width != menuRect.size.width))
        {
            OERectEdge oppositeEdge = [self OE_oppositeRectEdgeForEdge:edge];
            [self setOpenEdge:oppositeEdge];
            [[self menuView] OE_calculateRequiredViewSize];
            menuRect.origin = [self OE_originForEdge:oppositeEdge ofWindow:[self parentWindow]];
        }
    }
    
    // update intersection rect if old menu rect was completly off-screen
    if(NSHeight(intersectionRect) == 0)
        intersectionRect =   NSIntersectionRect(menuRect, screenRect);
    
    // resize if nescessary    
    if(NSHeight(menuRect) != NSHeight(intersectionRect))
    {
        menuRect.size.height = NSHeight(intersectionRect);
        menuRect.origin.y    = NSMinY(intersectionRect);
    }
    
    if(!NSEqualRects(menuRect, [self frame]))
    {
        [self setFrame:menuRect display:NO];
    }
    
    [[self menuView] updateScrollerViews:NO];
}

- (NSPoint)OE_originForEdge:(OERectEdge)anEdge ofWindow:(NSWindow*)win
{
    NSRect  rect        = [self openRect];
    BOOL    isSubmenu   = [self supermenu] != nil;
    NSPoint point;
    
    if(!NSEqualSizes(rect.size, (NSSize){0,0}))
    {
        switch (anEdge) {
            case OENoEdge:
                point = (NSPoint){NSMidX(rect), NSMidY(rect)};
                point = NSPointSub(point, ((NSPoint){[self frame].size.width/2, [self frame].size.height/2}));
                break;
            case OEMaxXEdge:
                point = (NSPoint){NSMaxX(rect), NSMidY(rect)};
                break;
            case OEMinXEdge:
                point = (NSPoint){NSMinX(rect), NSMidY(rect)};
                point.x -= [self frame].size.width;
                break;
            case OEMinYEdge:
                point = (NSPoint){NSMidX(rect), NSMinY(rect)};
                point.y -= [self frame].size.height;
                break;
            case OEMaxYEdge:
                point = (NSPoint){NSMidX(rect), NSMaxY(rect)};
                break;
            default:
                break;
        }
        
        switch (anEdge) {
            case OEMaxXEdge:
            case OEMinXEdge:
                point.y -= [self frame].size.height/2;
                break;
            case OEMinYEdge:
            case OEMaxYEdge:
                point.x -= [self frame].size.width/2;
                break;
            default:
                break;
        }
        
        if(isSubmenu)
        {
            point.y = NSMaxY([self openRect])-NSHeight([self frame]);
            if (anEdge == OEMaxXEdge)
                point = NSPointSub(point, ((NSPoint){SubmenuBorderLeft-1, -SubmenuBorderTop}));
            else
                point = NSPointSub(point, ((NSPoint){-SubmenuBorderRight+1, -SubmenuBorderTop}));
        }
    }
    else
    {
        point = rect.origin;
        point.y -= [self frame].size.height;
    }
    
    point.x += [win frame].origin.x;
    point.y += [win frame].origin.y;
    
    return point;
}

#pragma mark -
- (void)setMenu:(NSMenu *)nmenu
{
    menu = nmenu;
}

- (BOOL)isVisible
{
    return visible && [super isVisible] && !closing;
}

#pragma mark -
#pragma mark Interaction
- (void)menuMouseDragged:(NSEvent *)theEvent
{
    [[self menuView] mouseDragged:theEvent];
}

- (void)menuMouseUp:(NSEvent*)theEvent
{
    [[self menuView] mouseUp:theEvent];
}

#pragma mark -
#pragma mark Animation Stuff
- (CAAnimation*)alphaValueAnimation
{
    return [[self animator] animationForKey:@"alphaValue"];
}

- (void)setAlphaValueAnimation:(CAAnimation*)anim
{
    [[self animator] setAnimations:[NSDictionary dictionaryWithObject:anim forKey:@"alphaValue"]];
}

- (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag
{
    if(flag)
    {
        [self setHighlightedItem:nil];
        
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        
        // Remove event monitor
        if(_localMonitor != nil)
        {
            [NSEvent removeMonitor:_localMonitor];
            _localMonitor = nil;
        }
        
        // remove window
        if(closing)
        {
            [[self parentWindow] removeChildWindow:self];
            [self orderOut:nil];
        }
        
        // call menuDidHide: on delegate
        if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidHide:)]) [self.delegate performSelector:@selector(menuDidHide:) withObject:self];
        
        visible = NO;
        closing = NO;
        
        // display to make sure next time the menu opens it shows the correct state
        [self display];
    }
    
    if(!self.popupButton)
        [self close];
    
    [theAnimation setDelegate:nil];
}

#pragma mark -
#pragma mark Setter / getter
- (NSMenuItem *)highlightedItem { return highlightedItem; }
- (void)setHighlightedItem:(NSMenuItem *)value
{
    if(highlightedItem != value)
    {
        highlightedItem = value;
        self.submenu = [[highlightedItem submenu] convertToOEMenu];
    }
}

- (void)setSubmenu:(OEMenu *)_submenu
{
    if(submenu)
    {
        [submenu closeMenuWithoutChanges:nil];
    }
    
    if(_submenu)
    {        
        NSRect selectedItemRect = [[self menuView] rectOfItem:self.highlightedItem];
        NSRect selectedItemRectOnWindow = [[self menuView] convertRectToBase:selectedItemRect];
        [_submenu setDisplaysOpenEdge:NO];
        [_submenu setPopupButton:[self popupButton]];
        [_submenu setSupermenu:self];
        
        [_submenu openOnEdge:OEMaxXEdge ofRect:selectedItemRectOnWindow ofWindow:self];
    }
    
    submenu = _submenu;
}

- (OEMenu*)submenu
{
    return submenu;
}

- (void)setIsAlternate:(BOOL)flag
{
    if(closing || flag==_alternate) return;
    
    _alternate = flag;
    if(self.highlightedItem) [self display];
    if(self.submenu) [[self submenu] display];
}

- (BOOL)alternate
{
    return _alternate || [[self supermenu] alternate];
}
#pragma mark -
- (void)setStyle:(OEMenuStyle)aStyle
{
    style = aStyle;
}

- (OEMenuStyle)style
{
    if([self supermenu])
        return [[self supermenu] style];
    
    return style;
}
#pragma mark -
- (void)updateSize
{
    NSSize contentSize = [[self menuView] OE_calculateRequiredViewSize];
    [self setFrame:(NSRect){[self frame].origin, contentSize} display:NO];
}
#pragma mark -
#pragma mark NSMenu wrapping
- (NSArray *)itemArray
{
    return [[self menu] itemArray];
}

#pragma mark -
#pragma mark Private Methods
- (BOOL)_isClosing
{
    return closing;
}

- (OEMenuContentView *)menuView
{
    return [[[self contentView] subviews] lastObject];
}

- (void)_performCloseMenu
{
    CAAnimation *anim = [self alphaValueAnimation];
    anim.delegate = self;
    [self setAlphaValueAnimation:anim];
    
    // fade menu window out 
    [[self animator] setAlphaValue:0.0];
    
    [self OE_removeEventMonitor];
}

- (void)_closeByClickingItem:(NSMenuItem *)selectedItem
{    
    [self OE_removeEventMonitor];
    
    closing = YES;
    if(self.submenu) [self.submenu closeMenuWithoutChanges:nil];
    
    [NSTimer scheduledTimerWithTimeInterval:flickerDelay target:self selector:@selector(_closeTimer:) userInfo:selectedItem repeats:NO];
    self.highlightedItem = nil;
    [self display];
}

- (void)_closeTimer:(NSTimer *)timer
{
    NSMenuItem *selectedItem = [timer userInfo];
    [timer invalidate];
    if(![self _isClosing]) return;
    
    [NSTimer scheduledTimerWithTimeInterval:flickerDelay target:self selector:@selector(_finalClosing:) userInfo:selectedItem repeats:NO];
    
    self.highlightedItem = selectedItem;
    [self display];
}

- (void)_finalClosing:(NSTimer*)timer
{
    NSMenuItem *selectedItem = [timer userInfo];
    [timer invalidate];
    if(![self _isClosing]) return;
        
    BOOL doAlternateAction = selectedItem && [selectedItem isEnabled] && [selectedItem isKindOfClass:[OEMenuItem class]] && [(OEMenuItem*)selectedItem hasAlternate] && self.alternate;
    if(doAlternateAction){
        ((OEMenuItem*)selectedItem).isAlternate = YES;
    }
    
    // if an item is selected and the menu is attached to a popupbutton
    if(selectedItem && [selectedItem isEnabled] && self.popupButton)
    {
        // we tell the popupbutton to select the item
        [self.popupButton selectItem:selectedItem];
    }
    
    // if an item is selected and has a targen + action we call it    
    id target;
    SEL action;
    if(doAlternateAction && [selectedItem respondsToSelector:@selector(alternateAction)] && [(OEMenuItem*)selectedItem alternateAction]!=NULL)
    {
        target = [(OEMenuItem*)selectedItem alternateTarget];
        action = [(OEMenuItem*)selectedItem alternateAction];
    }
    else if([selectedItem isEnabled])
    {
        target = [(OEMenuItem*)selectedItem target];
        action = [(OEMenuItem*)selectedItem action];
    }
    
    [NSApp sendAction:action to:target from:selectedItem];
        
    // tell the delegate that the menu selected an item
    if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidSelect:)]) [self.delegate performSelector:@selector(menuDidSelect:) withObject:self];
    
    [self _performCloseMenu];
}

#pragma mark -
#pragma mark Utilities
- (OERectEdge)OE_oppositeRectEdgeForEdge:(OERectEdge)edge
{
    switch (edge) {
        case OENoEdge:
            return OENoEdge;
            break;
        case OEMaxXEdge:
            return OEMinXEdge;
            break;
        case OEMinXEdge:
            return OEMaxXEdge;
            break;
        case OEMinYEdge:
            return OEMaxYEdge;
            break;
        case OEMaxYEdge:
            return OEMinYEdge;
            break;
        default:
            break;
    }
}
#pragma mark -
- (void)OE_createEventMonitor
{   
    _localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDownMask | NSRightMouseDownMask | NSOtherMouseDownMask | NSKeyDownMask | NSFlagsChangedMask | NSScrollWheelMask handler:
                     ^ NSEvent  *(NSEvent *incomingEvent)
                     {
                         
                         OEMenuContentView *view = [[[self contentView] subviews] lastObject];
                         
                         if([incomingEvent type]==NSScrollWheel)
                         {
                             if([self submenu])
                                 return incomingEvent;
                             
                             [view scrollWheel:incomingEvent];
                             return nil;
                         }
                         
                        if([incomingEvent type] == NSFlagsChanged)
                         {
                             [self setIsAlternate:([incomingEvent modifierFlags] & NSAlternateKeyMask) != 0];
                             return nil;
                         }
                         
                         if([incomingEvent type] == NSKeyDown)
                         {
                             if([view menuKeyDown:incomingEvent])
                                 return nil;
                             return incomingEvent;
                         }
                         
                         if([[incomingEvent window] isKindOfClass:[self class]])// mouse down in window, will be handle by content view
                         {
                             return incomingEvent;
                         }
                         else
                         {
                             // event is outside of window, close menu without changes and remove event
                             [self closeMenuWithoutChanges:nil];
                         }
                         
                         return nil;
                     }];
}

- (void)OE_removeEventMonitor
{
    if(_localMonitor != nil)
    {
        [NSEvent removeMonitor:_localMonitor];
        _localMonitor = nil;
    }
}
@end

#pragma mark -
@implementation NSMenu (OEAdditions)

- (OEMenu *)convertToOEMenu
{
    OEMenu *menu = [[OEMenu alloc] init];
    menu.menu = self;
    return menu;
}

@end

#pragma mark -
@implementation OEMenuContentView
@synthesize scrollDownView, scrollUpView, scrollTimer;

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        frame.origin = (NSPoint){0,0};
        
        OE_MenuScrollView *scrollView = [[OE_MenuScrollView alloc] initWithFrame:frame];
        [scrollView setDrawsBackground:NO];
        [scrollView setBorderType:NSNoBorder];
        [scrollView setHasVerticalScroller:YES];
        [scrollView setVerticalScroller:[[OE_MenuScroller alloc] init]];
        [scrollView setVerticalScrollElasticity:NSScrollElasticityNone];
        [scrollView setDocumentView:[[OEMenuItemsView alloc] initWithFrame:frame]];
        
        [self addSubview:scrollView];
        
        frame.size.height = 13.0;
        
        scrollDownView = [[OEMenuScrollerView alloc] initWithFrame:frame];
        [scrollDownView setAlphaValue:0.0];
        [scrollDownView addObserver:self forKeyPath:@"alphaValue" options:0 context:nil];
        CAAnimation *animation = [scrollDownView animationForKey:@"alphaValue"];
        [animation setDuration:0.02];
        [scrollDownView setAnimations:[NSDictionary dictionaryWithObject:animation forKey:@"alphaValue"]];
        
        scrollUpView = [[OEMenuScrollerView alloc] initWithFrame:frame];
        [scrollUpView setAlphaValue:0.0];
        [scrollUpView addObserver:self forKeyPath:@"alphaValue" options:0 context:nil];
        [scrollUpView setUp:YES];
        animation = [scrollUpView animationForKey:@"alphaValue"];
        [animation setDuration:0.02];
        [scrollUpView setAnimations:[NSDictionary dictionaryWithObject:animation forKey:@"alphaValue"]];
        
        [self addSubview:scrollDownView];
        [self addSubview:scrollUpView];
        
        NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseMoved|NSTrackingMouseEnteredAndExited|NSTrackingActiveInActiveApp owner:self userInfo:nil];
        [self addTrackingArea:area];
    }
    return self;
}


- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    NSView *view = nil;
    if(object == [self scrollDownView] || object == [self scrollUpView])
    {
        view = object;
    }
    if(view)
    {
        [[self menuItemsView] setNeedsDisplayInRect:[[self menuItemsView] convertRect:[view frame] fromView:self]];
    }
}

- (void)dealloc
{    
    [[self scrollUpView] removeObserver:self forKeyPath:@"alphaValue"];
    [[self scrollDownView] removeObserver:self forKeyPath:@"alphaValue"];
    
    while([[self trackingAreas] count] != 0)
        [self removeTrackingArea:[[self trackingAreas] lastObject]];
}
#pragma mark -
#pragma mark Interaction
- (void)updateTrackingAreas
{
    NSArray *trackingAreas = [self trackingAreas];
    NSTrackingArea *area = [trackingAreas objectAtIndex:0];
    
    [self removeTrackingArea:area];
    area = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseMoved|NSTrackingMouseEnteredAndExited|NSTrackingActiveAlways owner:self userInfo:nil];
    [self addTrackingArea:area];
}


- (void)mouseUp:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    // check if on selected item && selected item not disabled
    // perform action, update selected item
    
    if(![[[self menu] highlightedItem] hasSubmenu])
        [[self menu] closeMenu];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    
    NSPoint loc = [theEvent window]==[self window]?[theEvent locationInWindow]:[[self window] convertScreenToBase:[[theEvent window] convertBaseToScreen:[theEvent locationInWindow]]];
    [self highlightItemWithScrollingAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    
    NSPoint loc = [theEvent window]==[self window]?[theEvent locationInWindow]:[[self window] convertScreenToBase:[[theEvent window] convertBaseToScreen:[theEvent locationInWindow]]];
    [self highlightItemWithScrollingAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemWithScrollingAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    // if not mouse on subwindow
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemWithScrollingAtPoint:[self convertPointFromBase:loc]];
}

- (BOOL)menuKeyDown:(NSEvent *)theEvent
{
    BOOL accepted = NO;
    
    if([[self menu] _isClosing]) return accepted;
    
    NSMenuItem *currentItem = [[self menu] highlightedItem];
    
    switch(theEvent.keyCode)
    {
        case 126 : // UP
            if([[self menu] highlightedItem])
            {
                NSInteger index = [[self menu].itemArray indexOfObject:[self menu].highlightedItem];
                if(index!=NSNotFound && index>0)
                    [[self menu] setHighlightedItem:[[[self menu] itemArray] objectAtIndex:index - 1]];
            }
            else [[self menu] setHighlightedItem:[[[self menu] itemArray] lastObject]];
            
            accepted = YES;
            break;
            
        case 125 : // DOWN
            if([[self menu] highlightedItem])
            {
                NSInteger index = [self.menu.itemArray indexOfObject:self.menu.highlightedItem];
                if(index!=NSNotFound && index < self.menu.itemArray.count-1)
                    [[self menu] setHighlightedItem:[[[self menu] itemArray] objectAtIndex:index + 1]];
            }
            else [[self menu] setHighlightedItem:[[[self menu] itemArray] objectAtIndex:0]];
            
            accepted = YES;
            break;
        case 123 : // LEFT (exit submenu if any)
            break;
        case 124 : // RIGHT (enter submenu if any)
            break;
        case 53 : // ESC (close without changes)
            [[self menu] closeMenuWithoutChanges:self];
            accepted = YES;
            break;
        case 49 : // SPACE ("click" selected item)
        case 36 : // ENTER (same as space)
            [[self menu] closeMenu];
            accepted = YES;
        default:
            break;
    }
    
    // a little explanation for this:
    // selecting a separator item should not be possible, so if the selected item is a separator we try to jump over it
    // this will continue until either a normal item was selected or the last (or first depending on direction) item is reached
    // we then check if the selected item is still a separator and if so we select the item we started with.
    // this ensures that a valid item will be selected after a key was pressed
    if(([theEvent keyCode] == 126 || [theEvent keyCode] == 125) && [[self menu] highlightedItem] != currentItem && [[[self menu ] highlightedItem] isSeparatorItem])
    {
        [self menuKeyDown:theEvent];
        if([[[self menu] highlightedItem] isSeparatorItem])
            [[self menu] setHighlightedItem:currentItem];
        
        accepted = YES;
    }
    
    if(accepted)
        [self setNeedsDisplay:YES];
    return accepted;
}

#pragma mark -
#pragma mark Scrolling
#define AutoScrollInterval 0.01
#define ScrollStep 8.0
- (void)scrollUp
{
    [self scrollBy:ScrollStep];
    
    if(![self scrollTimer])
    {
        [self setScrollTimer:[NSTimer scheduledTimerWithTimeInterval:AutoScrollInterval target:self selector:@selector(scrollUp) userInfo:nil repeats:YES]];
    }
}

- (void)scrollDown
{
    [self scrollBy:-ScrollStep];
    
    if(![self scrollTimer])
        [self setScrollTimer:[NSTimer scheduledTimerWithTimeInterval:AutoScrollInterval target:self selector:@selector(scrollDown) userInfo:nil repeats:YES]];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    [self scrollBy:[theEvent scrollingDeltaY]];
}

- (void)scrollBy:(float)yDelta
{
    NSScrollView *scrollView = [self scrollView];
    
    NSPoint point = [[scrollView contentView] bounds].origin;
    point.y -= yDelta;
    
    [[scrollView documentView] scrollPoint:point];

    [self updateScrollerViews:YES];
    [self mouseHighlightItemAtPoint:[self convertPointFromBase:[[self window] convertScreenToBase:[NSEvent mouseLocation]]]];
}

- (void)updateScrollerViews:(BOOL)animated
{
    BOOL isAtBottom = NSMinY([[self menuItemsView] visibleRect]) == 0.0;
    BOOL isAtTop = NSMaxY([[self menuItemsView] visibleRect])==NSHeight([[self menuItemsView] bounds]);

    [animated?[[self scrollDownView] animator]:[self scrollDownView] setAlphaValue:isAtTop?0.0:1.0];
    [animated?[[self scrollUpView] animator]:[self scrollUpView] setAlphaValue:isAtBottom?0.0:1.0];
}

#pragma mark -
#pragma mark Drawing Stlye
- (NSDictionary *)itemTextAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
    NSColor *textColor = [[self menu] style]==OEMenuStyleDark?[NSColor whiteColor]:[NSColor blackColor];
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [dict setObject:font forKey:NSFontAttributeName];
    [dict setObject:textColor forKey:NSForegroundColorAttributeName];
    [dict setObject:ps forKey:NSParagraphStyleAttributeName];
    
    return dict;
}

- (NSDictionary *)selectedItemTextAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
    NSColor *textColor = [[self menu] style]==OEMenuStyleDark?[NSColor blackColor]:[NSColor whiteColor];
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [dict setObject:font forKey:NSFontAttributeName];
    [dict setObject:textColor forKey:NSForegroundColorAttributeName];
    [dict setObject:ps forKey:NSParagraphStyleAttributeName];
    
    return dict;
}

- (NSDictionary *)selectedItemAlternateTextAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
    NSColor *textColor = [NSColor whiteColor];
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [dict setObject:font forKey:NSFontAttributeName];
    [dict setObject:textColor forKey:NSForegroundColorAttributeName];
    [dict setObject:ps forKey:NSParagraphStyleAttributeName];
    
    return dict;
}

- (NSDictionary *)disabledItemTextAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
    NSColor *textColor = [NSColor colorWithDeviceRed:0.49 green:0.49 blue:0.49 alpha:1.0];
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [dict setObject:font forKey:NSFontAttributeName];
    [dict setObject:textColor forKey:NSForegroundColorAttributeName];
    [dict setObject:ps forKey:NSParagraphStyleAttributeName];
    
    return dict;
}

- (NSGradient*)backgroundGradient
{
    OEMenuStyle style = [[self menu] style];
    
    NSColor *startColor = style==OEMenuStyleDark?[NSColor colorWithDeviceWhite:0.91 alpha:0.10]:[NSColor colorWithDeviceWhite:0.85 alpha:0.90];
    NSColor *endColor = style==OEMenuStyleDark?[startColor colorWithAlphaComponent:0.0]:[NSColor colorWithDeviceWhite:0.75 alpha:0.90];
    NSGradient *grad = [[NSGradient alloc] initWithStartingColor:startColor endingColor:endColor];
    
    return grad;
}

- (NSColor*)backgroundColor
{
    OEMenuStyle style = [[self menu] style];
    return style==OEMenuStyleDark?[NSColor colorWithDeviceWhite:0.0 alpha:0.8]:nil;
}

- (NSBezierPath*)backgroundPath
{
    OERectEdge openEdge = [[self menu] openEdge];
    openEdge = [[self menu] displaysOpenEdge] ? openEdge : OENoEdge;
    BOOL isSubmenu = [[self menu] supermenu]!=nil;
    
    NSBezierPath *path;
    NSRect rect;
    if(isSubmenu || openEdge==OENoEdge)
    {
        rect = NSInsetRect([self bounds], 4, 4);
        path = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:isSubmenu?0:3 yRadius:3];
    }
    else 
    {
        rect = NSInsetRect([self bounds], 0, -1);
        path = [NSBezierPath bezierPath];
        
        switch (openEdge) {
            case OEMaxXEdge:
            case OEMinXEdge:
                rect.origin.y += BGInsetTopNoEdge;
                rect.size.height -= BGInsetTopNoEdge + BGInsetBottomNoEdge;
                break;
            case OEMinYEdge:
            case OEMaxYEdge:
                rect.origin.x += BGInsetLeftNoEdge;
                rect.size.width -= BGInsetLeftNoEdge + BGInsetRightNoEdge;
            default:
                break;
        }

        switch(openEdge){
            case OEMaxXEdge:
                rect.origin.x += BGInsetLeftEdge;
                rect.size.width -= BGInsetRightNoEdge + BGInsetLeftEdge;
                
                [path moveToPoint:(NSPoint){ NSMinX(rect), NSMidY(rect) + BGEdgeSizeLeft.height/2 }];
                [path lineToPoint:(NSPoint){ NSMinX(rect) - BGEdgeSizeLeft.width, NSMidY(rect) }];
                [path lineToPoint:(NSPoint){ NSMinX(rect), NSMidY(rect) - BGEdgeSizeLeft.height/2 }];
                [path lineToPoint:(NSPoint){ NSMinX(rect), NSMidY(rect) + BGEdgeSizeLeft.height }];
                break;
            case OEMinXEdge:
                rect.origin.x += BGInsetLeftNoEdge;
                rect.size.width -= BGInsetRightEdge + BGInsetLeftNoEdge;
                
                [path moveToPoint:(NSPoint){ NSMaxX(rect), NSMidY(rect) + BGEdgeSizeRight.height/2 }];
                [path lineToPoint:(NSPoint){ NSMaxX(rect) + BGEdgeSizeRight.width, NSMidY(rect) }];
                [path lineToPoint:(NSPoint){ NSMaxX(rect), NSMidY(rect) - BGEdgeSizeRight.height/2 }];
                [path lineToPoint:(NSPoint){ NSMaxX(rect), NSMidY(rect) + BGEdgeSizeRight.height }];
                break;
            case OEMinYEdge:
                rect.origin.y += BGInsetTopEdge;
                rect.size.height -= BGInsetTopEdge + BGInsetBottomNoEdge;

                [path moveToPoint:(NSPoint){ NSMidX(rect) - BGEdgeSizeTop.width/2, NSMinY(rect) }];
                [path lineToPoint:(NSPoint){ NSMidX(rect), NSMinY(rect) - BGEdgeSizeTop.height}];
                [path lineToPoint:(NSPoint){ NSMidX(rect) + BGEdgeSizeTop.width/2, NSMinY(rect) }];
                [path lineToPoint:(NSPoint){ NSMidX(rect) - BGEdgeSizeTop.width/2, NSMinY(rect)}];
                break;
            case OEMaxYEdge:
                rect.origin.y += BGInsetTopNoEdge;
                rect.size.height -= BGInsetTopNoEdge + BGInsetBottomEdge;
                
                [path moveToPoint:(NSPoint){ NSMidX(rect) - BGEdgeSizeBottom.width/2, NSMaxY(rect) }];
                [path lineToPoint:(NSPoint){ NSMidX(rect), NSMaxY(rect) + BGEdgeSizeBottom.height }];
                [path lineToPoint:(NSPoint){ NSMidX(rect) + BGEdgeSizeBottom.width/2, NSMaxY(rect) }];
                [path lineToPoint:(NSPoint){ NSMidX(rect) - BGEdgeSizeBottom.width/2, NSMaxY(rect) }];
                break;
            default: break;
        }

        [path appendBezierPathWithRoundedRect:rect xRadius:4 yRadius:4];
    }
    
    return path;
}

- (NSImage*)backgroundImage
{    
    BOOL isSubmenu = [[self menu] supermenu]!=nil;
    OEMenuStyle style = [[self menu] style];

    if(style==OEMenuStyleDark)
        return isSubmenu ? [NSImage imageNamed:@"dark_submenu_body"] : [NSImage imageNamed:@"dark_menu_body"];
    else
        return isSubmenu ? [NSImage imageNamed:@"light_submenu_body"] : [NSImage imageNamed:@"light_menu_body"];
    return nil;
}

- (NSImage*)arrowsImage
{
    if([[self menu] supermenu]!=nil)
        return nil;

    OEMenuStyle style = [[self menu] style];
    if(style==OEMenuStyleDark)
        return [NSImage imageNamed:@"dark_arrows_menu_body"];
    else
        return [NSImage imageNamed:@"light_arrows_menu_body"];

    return nil;
}

- (NSGradient*)selectionGradientWithAlternateState:(BOOL)alternateFlag
{
    NSColor *topColor, *bottomColor;
    OEMenuStyle style = [[self menu] style];
    if(alternateFlag)
    {
        topColor = [NSColor colorWithDeviceRed:0.71 green:0.07 blue:0.14 alpha:1.0];
        bottomColor = [NSColor colorWithDeviceRed:0.48 green:0.02 blue:0.07 alpha:1.0];
    }
    else if(style == OEMenuStyleDark)
    {
        topColor = [NSColor colorWithDeviceWhite:0.91 alpha:1.0];
        bottomColor = [NSColor colorWithDeviceWhite:0.71 alpha:1.0];
    } 
    else if(style == OEMenuStyleLight)
    {
        topColor = [NSColor colorWithDeviceWhite:0.19 alpha:1.0];
        bottomColor = [NSColor colorWithDeviceWhite:0.10 alpha:1.0];

    }
    else return nil;
    
    NSGradient *selectionGradient = [[NSGradient alloc] initWithStartingColor:topColor endingColor:bottomColor];
    return selectionGradient;
}

- (NSImage*)submenuImageWithHighlightedState:(BOOL)highlighted
{
    OEMenuStyle style = [[self menu] style];
    return (highlighted ^ (style==OEMenuStyleLight)) ? [NSImage imageNamed:@"dark_menu_popover_arrow_selected"] : [NSImage imageNamed:@"dark_menu_popover_arrow_normal"];
}

- (NSColor*)upperSeparatorColor
{
    OEMenuStyle style = [[self menu] style];
    return style==OEMenuStyleDark ? [NSColor blackColor] : [NSColor colorWithDeviceWhite:0.19 alpha:1.0];
}
- (NSColor*)lowerSeparatorColor
{
    OEMenuStyle style = [[self menu] style];
    return style==OEMenuStyleDark ? [NSColor colorWithDeviceWhite:1.0 alpha:0.12] : [NSColor colorWithDeviceWhite:1.0 alpha:0.5];
}

- (NSImage*)imageForState:(NSInteger)state withStyle:(BOOL)darkStyleFlag
{
    if(state == NSOnState)
        return darkStyleFlag ? [NSImage imageNamed:@"tick_mark_selected"] : [NSImage imageNamed:@"tick_mark_normal"];
    return darkStyleFlag ? [NSImage imageNamed:@"mixed_state_selected"] : [NSImage imageNamed:@"mixed_state_normal"];
}
#pragma mark -
#pragma mark Drawing
- (void)drawRect:(NSRect)dirtyRect
{
    BOOL isSubmenu          = [[self menu] supermenu]!=nil;
    OERectEdge openEdge     = [[self menu] openEdge];
    openEdge = [[self menu] displaysOpenEdge] ? openEdge : OENoEdge;
    
    NSColor      *backgroundColor    = [self backgroundColor];
    NSGradient   *backgroundGradient = [self backgroundGradient];
    NSBezierPath *backgroundPath     = [self backgroundPath];
    NSImage      *backgroundImage    = [self backgroundImage];
    
    if(backgroundColor)
    {
        [backgroundColor setFill];
        [backgroundPath fill];
    }

    [backgroundGradient drawInBezierPath:backgroundPath angle:90];

    if(isSubmenu || openEdge == OENoEdge)
    {
        [backgroundImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:BGImageCornerWidth rightBorder:BGImageCornerWidth topBorder:BGImageCornerHeight bottomBorder:BGImageCornerHeight];
    }
    else
    {
        NSImage *arrowsImage = [self arrowsImage];
        
        // bottom left
        NSRect sourceRect, targetRect;
        targetRect.size = sourceRect.size = (NSSize){BGImageCornerWidth, BGImageCornerHeight};
        
        sourceRect.origin = (NSPoint){0,0};
        targetRect.origin = (NSPoint){
            NSMinX([self bounds]) + (openEdge!=OEMaxXEdge?0:BGImageCornerEdgeInset), 
            NSMaxY([self bounds]) - (openEdge!=OEMaxYEdge?0:BGImageCornerEdgeInset)   -targetRect.size.height
        };
        [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        
        // bottom right
        sourceRect.origin = (NSPoint){[backgroundImage size].width-BGImageCornerWidth,0};
        targetRect.origin = (NSPoint){
            NSMaxX([self bounds]) - (openEdge!=OEMinXEdge?0:BGImageCornerEdgeInset)   -targetRect.size.width, 
            NSMaxY([self bounds]) - (openEdge!=OEMaxYEdge?0:BGImageCornerEdgeInset)   -targetRect.size.height
        };
        [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        
        // top left
        sourceRect.origin = (NSPoint){0, [backgroundImage size].height-BGImageCornerHeight};
        targetRect.origin = (NSPoint){
            NSMinX([self bounds]) + (openEdge!=OEMaxXEdge?0:BGImageCornerEdgeInset), 
            NSMinY([self bounds]) + (openEdge!=OEMinYEdge?0:BGImageCornerEdgeInset)
        };
        [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        
        // top right
        sourceRect.origin = (NSPoint){[backgroundImage size].width-BGImageCornerWidth, [backgroundImage size].height-BGImageCornerHeight};
        targetRect.origin = (NSPoint){
            NSMaxX([self bounds]) - (openEdge!=OEMinXEdge?0:BGImageCornerEdgeInset)   -targetRect.size.width, 
            NSMinY([self bounds]) + (openEdge!=OEMinYEdge?0:BGImageCornerEdgeInset)
        };
        [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        
        // left side
        sourceRect.size = (NSSize){ BGImageCornerWidth, 1 };
        sourceRect.origin = (NSPoint){ 0, BGImageCornerHeight };
        if(openEdge != OEMaxXEdge)
        {
            targetRect.origin = (NSPoint){0, BGImageCornerHeight + (openEdge==OEMinYEdge?BGImageCornerEdgeInset:0.0) };
            targetRect.size = (NSSize){BGImageCornerWidth, NSHeight([self bounds]) - 2*BGImageCornerHeight - (openEdge==OEMaxYEdge || openEdge==OEMinYEdge?BGImageCornerEdgeInset:0)};
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        else
        {
            NSRect arrowsTargetRect = (NSRect){{0.0, (NSHeight([self bounds])-NSHeight(LeftArrowSource))/2}, LeftArrowSource.size};
            [arrowsImage drawInRect:arrowsTargetRect fromRect:LeftArrowSource operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin = (NSPoint){BGImageCornerEdgeInset, BGImageCornerHeight};
            targetRect.size = (NSSize){BGImageCornerWidth, arrowsTargetRect.origin.y-NSHeight(LeftArrowSource)/2+1};
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin.y += targetRect.size.height + NSHeight(LeftArrowSource);
            targetRect.size.height -= 1;
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        
        // right side
        sourceRect.size = (NSSize){ BGImageCornerWidth, 1 };
        sourceRect.origin = (NSPoint){ [backgroundImage size].width-BGImageCornerWidth, BGImageCornerHeight };
        if(openEdge != OEMinXEdge)
        {
            targetRect.origin = (NSPoint){NSMaxX([self bounds])-BGImageCornerWidth, BGImageCornerHeight + (openEdge==OEMinYEdge?BGImageCornerEdgeInset:0.0) };
            targetRect.size = (NSSize){BGImageCornerWidth, NSHeight([self bounds]) - 2*BGImageCornerHeight - (openEdge==OEMaxYEdge || openEdge==OEMinYEdge?BGImageCornerEdgeInset:0)};
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        else
        {
            NSRect arrowsTargetRect = (NSRect){{NSMaxX([self bounds])-NSWidth(RightArrowSource), (NSHeight([self bounds])-NSHeight(RightArrowSource))/2}, RightArrowSource.size};
            [arrowsImage drawInRect:arrowsTargetRect fromRect:RightArrowSource operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin = (NSPoint){NSMaxX([self bounds])-BGImageCornerWidth-BGImageCornerEdgeInset, BGImageCornerHeight};
            targetRect.size = (NSSize){BGImageCornerWidth, arrowsTargetRect.origin.y-NSHeight(RightArrowSource)/2+1};
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin.y += targetRect.size.height + NSHeight(RightArrowSource);
            targetRect.size.height -= 1;
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        
        // top side
        sourceRect.size = (NSSize){ 1, BGImageCornerHeight };
        sourceRect.origin = (NSPoint){ BGImageCornerWidth, [backgroundImage size].height-BGImageCornerHeight };
        if(openEdge != OEMinYEdge)
        {
            targetRect.origin = (NSPoint){ BGImageCornerWidth + (openEdge==OEMaxXEdge?BGImageCornerEdgeInset:0.0), NSMinY([self bounds]) };
            targetRect.size = (NSSize){ NSWidth([self bounds])-2*BGImageCornerWidth - (openEdge==OEMinXEdge || openEdge==OEMaxXEdge?BGImageCornerEdgeInset:0), BGImageCornerHeight };
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        else
        {
            NSRect arrowsTargetRect = (NSRect){{round(NSWidth([self bounds])-NSWidth(TopArrowSource))/2, NSMinY([self bounds])}, TopArrowSource.size};
            [arrowsImage drawInRect:arrowsTargetRect fromRect:TopArrowSource operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin = (NSPoint){ BGImageCornerWidth, NSMinY([self bounds])+BGImageCornerEdgeInset };
            targetRect.size = (NSSize){ NSMinX(arrowsTargetRect)-BGImageCornerWidth, BGImageCornerHeight };
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin.x = NSMaxX(arrowsTargetRect);
            targetRect.size.width = NSWidth([self bounds])-NSMaxX(arrowsTargetRect)-BGImageCornerWidth;
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        
        // bottom side
        sourceRect.size = (NSSize){ 1, BGImageCornerHeight };
        sourceRect.origin = (NSPoint){ BGImageCornerWidth, 0 };
        if(openEdge != OEMaxYEdge)
        {
            targetRect.origin = (NSPoint){ BGImageCornerWidth + (openEdge==OEMaxXEdge?BGImageCornerEdgeInset:0.0), NSMaxY([self bounds])-BGImageCornerHeight };
            targetRect.size = (NSSize){ NSWidth([self bounds])-2*BGImageCornerWidth - (openEdge==OEMinXEdge || openEdge==OEMaxXEdge?BGImageCornerEdgeInset:0), BGImageCornerHeight };
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        else
        {
            NSRect arrowsTargetRect = (NSRect){{(NSWidth([self bounds])-NSWidth(BottomArrowSource))/2, NSMaxY([self bounds])-NSHeight(BottomArrowSource)}, BottomArrowSource.size};
            [arrowsImage drawInRect:arrowsTargetRect fromRect:BottomArrowSource operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin = (NSPoint){ BGImageCornerWidth, NSMaxY([self bounds])-BGImageCornerHeight-BGImageCornerEdgeInset };
            targetRect.size = (NSSize){ NSMinX(arrowsTargetRect)-BGImageCornerWidth, BGImageCornerHeight };
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            targetRect.origin.x = NSMaxX(arrowsTargetRect);
            targetRect.size.width = NSWidth([self bounds])-NSMaxX(arrowsTargetRect)-BGImageCornerWidth;
            [backgroundImage drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
    }
}

#pragma mark -
@synthesize lastHighlightPoint;
#define SubmenuHighlightDelay 0.07
- (void)mouseHighlightItemAtPoint:(NSPoint)p
{       
    if([[self menu] supermenu]) [NSObject cancelPreviousPerformRequestsWithTarget:[[[self menu] supermenu] contentView] selector:@selector(highlightAtLastHighlightPoint) object:nil];
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(highlightAtLastHighlightPoint) object:nil];

    if (![[self menu] submenu]) {
        [self highlightItemAtPoint:p];
        [self setLastHighlightPoint:p];
    } else {
        int submenuPosition = [self OE_submenuPosition];
        float distance = p.x-[self lastHighlightPoint].x;
        if(distance*submenuPosition > -0.7)
        {
            [self highlightItemAtPoint:p];
        }
        else 
        {
            [self performSelector:@selector(highlightAtLastHighlightPoint) withObject:nil afterDelay:SubmenuHighlightDelay];
        }
        
        [self setLastHighlightPoint:p];
    }
}

- (void)highlightAtLastHighlightPoint
{
    [self highlightItemAtPoint:[self lastHighlightPoint]];
}

- (int)OE_submenuPosition
{
    if(![[self menu] submenu]) return 0;
    else if(NSMinX([[[self menu] submenu] frame]) > NSMidX([[self menu] frame])) return -1;
    else if(NSMaxX([[[self menu] submenu] frame]) < NSMidX([[self menu] frame])) return 1;
    else return 0;
}

- (void)highlightItemAtPoint:(NSPoint)p
{
    if([[self menu] supermenu]) [NSObject cancelPreviousPerformRequestsWithTarget:[[[self menu] supermenu] contentView] selector:@selector(highlightAtLastHighlightPoint) object:nil];
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(highlightAtLastHighlightPoint) object:nil];

    NSMenuItem *highlighItem = [self itemAtPoint:p];
    
    if(highlighItem != [self menu].highlightedItem)
    {
        if([[self menu] submenu] && ![highlighItem isSeparatorItem] && highlighItem==nil) // if a submenu is open and we are about to close it
        {
            int submenuPosition = [self OE_submenuPosition];
            if(((submenuPosition == -1) && p.x > NSMinX([self bounds])) || // submenu is on the right + highlight point is on the right, OR
               ((submenuPosition ==  1) && p.x < NSMaxX([self bounds])))   // submenu is on the left + highlight point is on the left
                return;                                                   // skip deselecting the submenu so it stays open
        }
        if([highlighItem isSeparatorItem])
            highlighItem = nil;

        [[self menu] setHighlightedItem:highlighItem];

        [self setNeedsDisplay:YES];
    }
}

- (void)highlightItemWithScrollingAtPoint:(NSPoint)p
{    
    if([[self scrollDownView] alphaValue]!=0.0 && NSPointInRect(p, [[self scrollDownView] frame]))
    {
        [self scrollDown];
    }
    else if([[self scrollUpView] alphaValue]!=0.0 && NSPointInRect(p, [[self scrollUpView] frame]))
    {
        [self scrollUp];
    } else {
        [[self scrollTimer] invalidate];
        [self setScrollTimer:nil];
        
        [self mouseHighlightItemAtPoint:p];
    }
}

- (NSMenuItem *)itemAtPoint:(NSPoint)p
{    
    if(([[self scrollUpView] alphaValue]==1.0 && NSPointInRect(p, [[self scrollUpView] frame])) || 
       ([[self scrollDownView] alphaValue]==1.0 && NSPointInRect(p, [[self scrollDownView] frame])))
        return nil;
    
    NSPoint pointInView = [self convertPoint:p toView:[self menuItemsView]];
    return [[self menuItemsView] itemAtPoint:pointInView];
}

- (NSRect)rectOfItem:(NSMenuItem *)m
{
    OEMenuItemsView *itemsView = [self menuItemsView];
    NSRect           rect       = [itemsView rectOfItem:m];
    return [self convertRect:rect fromView:itemsView];
}

#define ItemViewAdditonalWidth 1
#pragma mark -
#pragma mark View Config Overrides
- (void)setFrameSize:(NSSize)newSize
{    
    [super setFrameSize:newSize];
    
    NSRect newScrollViewRect;
    newScrollViewRect.origin = [self cachedContentOffset];
    newScrollViewRect.size.width = newSize.width-[self cachedBorderSize].width;
    newScrollViewRect.size.height = newSize.height-[self cachedBorderSize].height;
    
    [[self scrollView] setFrame:newScrollViewRect];
    
    NSRect scrollArrowsFrame = (NSRect){{newScrollViewRect.origin.x, 0}, { NSWidth(newScrollViewRect), 19}};
    scrollArrowsFrame.origin.y = NSMaxY(newScrollViewRect) -NSHeight([[self scrollDownView] frame]);
    [[self scrollDownView] setFrame:scrollArrowsFrame];
    scrollArrowsFrame.origin.y = newScrollViewRect.origin.y;
    [[self scrollUpView] setFrame:scrollArrowsFrame];
}

- (BOOL)acceptsFirstResponder
{
    return NO;
}

- (BOOL)isFlipped
{
    return YES;
}

- (BOOL)isOpaque
{
    return NO;
}

#pragma mark -
#pragma mark Sizing
- (NSSize)OE_calculateRequiredViewSize
{
    NSSize      contentSize = [[self menuItemsView] OE_calculateAndSetRequiredViewSize];
    OEMenu      *menu       = [self menu];
    BOOL        isSubmenu   = [menu supermenu]!=nil;
    OERectEdge  openEdge    = [menu openEdge];
    openEdge = [menu displaysOpenEdge] ? openEdge : OENoEdge;
    
    contentSize.width = contentSize.width < [menu minSize].width ? [menu minSize].width : contentSize.width;
    contentSize.height = contentSize.height < [menu minSize].height ? [menu minSize].height : contentSize.height;    
    
    NSSize borderSize       = NSZeroSize;
    NSPoint contentOffset   = NSZeroPoint;
    if(isSubmenu)
    {
        borderSize.width = SubmenuBorderLeft + SubmenuBorderRight;
        borderSize.height = SubmenuBorderTop + SubmenuBorderBottom;
        
        contentOffset.x = SubmenuBorderLeft;
        contentOffset.y = SubmenuBorderTop;
    } 
    else if(openEdge == OENoEdge)
    {
        borderSize.width = NoEdgeContentBorderLeft + NoEdgeContentBorderRight;
        borderSize.height = NoEdgeContentBorderTop + NoEdgeContentBorderBottom;
        
        contentOffset.x = NoEdgeContentBorderLeft;
        contentOffset.y = NoEdgeContentBorderTop;
    }
    else if(openEdge == OEMaxXEdge)
    {
        borderSize.width = MaxXEdgeContentBorderLeft + MaxXEdgeContentBorderRight;
        borderSize.height = MaxXEdgeContentBorderTop + MaxXEdgeContentBorderBottom;
        
        contentOffset.x = MaxXEdgeContentBorderLeft;
        contentOffset.y = MaxXEdgeContentBorderTop;
    }
    else if(openEdge == OEMinXEdge)
    {
        borderSize.width = MinXEdgeContentBorderLeft + MinXEdgeContentBorderRight;
        borderSize.height = MinXEdgeContentBorderTop + MinXEdgeContentBorderBottom;
        
        contentOffset.x = MinXEdgeContentBorderLeft;
        contentOffset.y = MinXEdgeContentBorderTop;
    }
    else if(openEdge == OEMaxYEdge)
    {
        borderSize.width = MaxYEdgeContentBorderLeft + MaxYEdgeContentBorderRight;
        borderSize.height = MaxYEdgeContentBorderTop + MaxYEdgeContentBorderBottom;
        
        contentOffset.x = MaxYEdgeContentBorderLeft;
        contentOffset.y = MaxYEdgeContentBorderTop;
    }
    else if(openEdge == OEMinYEdge)
    {
        borderSize.width = MinYEdgeContentBorderLeft + MinYEdgeContentBorderRight;
        borderSize.height = MinYEdgeContentBorderTop + MinYEdgeContentBorderBottom;
        
        contentOffset.x = MinYEdgeContentBorderLeft;
        contentOffset.y = MinYEdgeContentBorderTop;
    }
    
    [self setCachedContentOffset:contentOffset];
    [self setCachedBorderSize:borderSize];
    
    
    if(!NSEqualPoints([self cachedContentOffset], contentOffset)  || NSEqualSizes([self cachedBorderSize], borderSize))
    {
        [self setFrameSize:[self frame].size];
    }
    
    return NSSizeAdd(borderSize, contentSize);
}

#pragma mark -
- (OEMenu *)menu
{
    return (OEMenu *)[self window];
}

- (OEMenuItemsView *)menuItemsView;
{
    return [[self scrollView] documentView];
}

- (OE_MenuScrollView *)scrollView
{
    return [[self subviews] objectAtIndex:0];
}

@synthesize cachedContentOffset, cachedBorderSize;
@end
#pragma mark -
@implementation OEMenuItemsView

#pragma mark -
#pragma mark View Config Overrides
- (BOOL)isFlipped
{
    return YES;
}

#pragma mark -
#pragma mark Items
- (NSRect)rectOfItem:(NSMenuItem *)m
{
    BOOL menuContainsImage = [[self menu] containsItemWithImage];

    float y = 0;
    float itemHeight = menuContainsImage?ItemHeightWithImage:ItemHeightWithoutImage;
    NSArray *items = [[self menu] itemArray];
    for(NSMenuItem *item in items)
    {
        if(item == m)
        {
            return (NSRect){{0, y}, { NSWidth([self bounds]) ,[item isSeparatorItem]?ItemSeparatorHeight:itemHeight}};
        }
        
        if([item isSeparatorItem])
        {
            y += ItemSeparatorHeight;
            continue;
        }
        
        y += itemHeight;
    }
    
    return NSZeroRect;
}

- (NSMenuItem *)itemAtPoint:(NSPoint)p
{
    BOOL menuContainsImage = [[self menu] containsItemWithImage];
   
    if(p.x < 0 || p.x > NSWidth([self bounds]))
        return nil;
    if(p.y < 0 || p.y > NSHeight([self bounds]))
        return nil;
    
    float y = 0;
    float itemHeight = menuContainsImage?ItemHeightWithImage:ItemHeightWithoutImage;
    NSArray *items = [[self menu] itemArray];
    for(NSMenuItem *item in items)
    {
        if([item isSeparatorItem])
        {
            y += ItemSeparatorHeight;
            continue;
        }
        
        y += itemHeight;
        if(p.y < y && p.y > y - itemHeight)
            return item;
    }
    return nil;
}
#pragma mark -
- (NSSize)OE_calculateAndSetRequiredViewSize
{
    OEMenu          *menu              = [self menu];
    OEMenuContentView     *menuView          = [menu menuView];
    NSArray         *menuItems         = [menu itemArray];
    NSDictionary    *titleAttributes   = [menuView itemTextAttributes];
    
    float __block maxTitleWidth = 0;
    BOOL __block menuContainsImage = NO;
    
    int __block normalItemCount = 0;
    int __block separatorItemCount = 0;
    [menuItems enumerateObjectsUsingBlock:^(NSMenuItem* menuItem, NSUInteger idx, BOOL *stop) 
     {
         if([menuItem isSeparatorItem])
         { 
             separatorItemCount++;
         }
         else
         {
             NSString *title = [menuItem title];
             NSImage *image = [menuItem image];
             NSAttributedString *attributedTitle = [[NSAttributedString alloc] initWithString:title attributes:titleAttributes];
             NSSize titleSize = [attributedTitle size];
             
             if(maxTitleWidth < titleSize.width)
                 maxTitleWidth = titleSize.width;
             
             if(image)
                 menuContainsImage = YES;
             
             normalItemCount ++;
         }
     }];
    
    [menu setContainsItemWithImage:menuContainsImage];
    
    NSSize contentSize;
    contentSize.width = ItemTickMarkSpace + (menuContainsImage? ItemImageSpace : 0 ) + maxTitleWidth + ItemSubmenuSpace;
    contentSize.height = normalItemCount  *(menuContainsImage? ItemHeightWithImage : ItemHeightWithoutImage) + separatorItemCount  *ItemSeparatorHeight; 

    contentSize.width = ceilf(contentSize.width < [menu minSize].width ? [menu minSize].width : contentSize.width)+ItemViewAdditonalWidth;
    
    NSRect frame;
    frame.size      = contentSize;
    frame.origin    = (NSPoint){0, 0};
    [self setFrame:frame];
    
    contentSize.width -= ItemViewAdditonalWidth;
    return contentSize;
}

- (void)drawRect:(NSRect)dirtyRect
{
    // TODO: only draw dirty rect!
    OEMenuContentView *menuView   = (OEMenuContentView *)[[self enclosingScrollView] superview];
    
    OERectEdge openEdge     = [[self menu] openEdge];
    openEdge = [[self menu] displaysOpenEdge] ? openEdge : OENoEdge;
    
    NSBezierPath *clippingPath = [NSBezierPath bezierPathWithRect:[self bounds]];
    [clippingPath setWindingRule:NSEvenOddWindingRule];
        
    if([[menuView scrollUpView] alphaValue] == 1.0)
        [clippingPath appendBezierPathWithRect:[menuView convertRect:[[menuView scrollUpView] frame] toView:self]];
    if([[menuView scrollDownView] alphaValue] == 1.0)
       [clippingPath appendBezierPathWithRect:[menuView convertRect:[[menuView scrollDownView] frame] toView:self]];
    [clippingPath addClip];
   
    
    // Draw Items
    NSArray *items = [[self menu] itemArray];
    BOOL menuContainsImage = [[self menu] containsItemWithImage];
    float y     = 0;
    float baseX = 0;
    
    for(NSMenuItem *menuItem in items)
    {
        float itemWidth = NSWidth([self bounds]);
        if([menuItem isSeparatorItem])
        {
            NSRect lineRect = (NSRect){{baseX, y}, {itemWidth, 1}};
            
            lineRect.origin.y += 2;
            [[menuView upperSeparatorColor] setFill];
            NSRectFill(lineRect);
            
            lineRect.origin.y += 1;
            [[menuView lowerSeparatorColor] setFill];
            NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);
            
            y += ItemSeparatorHeight;
            continue;
        }
        
        NSRect      menuItemFrame       = (NSRect){{baseX, y}, {itemWidth, menuContainsImage?ItemHeightWithImage:ItemHeightWithoutImage}};
        BOOL        itemIsHighlighted   = [self menu].highlightedItem==menuItem;
        NSInteger   state               = [menuItem state];
        BOOL        itemIsDisabled      = ![menuItem isEnabled];
        BOOL        itemHasImage        = [menuItem image]!=nil;
        BOOL        itemHasSubmenu      = [menuItem hasSubmenu];
        BOOL        drawAlternate       = !itemIsDisabled && itemIsHighlighted && [menuItem isKindOfClass:[OEMenuItem class]] && [(OEMenuItem*)menuItem hasAlternate] && [[self menu] alternate];
        
        // Draw Selection
        if(!itemIsDisabled && itemIsHighlighted)
        {
            [[menuView selectionGradientWithAlternateState:drawAlternate] drawInRect:menuItemFrame angle:90];
        }
        
        // Draw Tickmark
        if(state != NSOffState)
        {
            NSImage *tickMarkImage =  [menuView imageForState:state withStyle:itemIsHighlighted^([[self menu] style]==OEMenuStyleLight)];            
            NSRect tickMarkRect = menuItemFrame;
            tickMarkRect.size = [tickMarkImage size];
            
            tickMarkRect.origin.x += (ItemTickMarkSpace-NSWidth(tickMarkRect))/2;
            tickMarkRect.origin.y += (NSHeight(menuItemFrame)-NSHeight(tickMarkRect))/2;
            
            [tickMarkImage drawInRect:tickMarkRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        menuItemFrame.origin.x += ItemTickMarkSpace;
        menuItemFrame.size.width -= ItemTickMarkSpace;
        
        // Draw Image
        if(itemHasImage)
        {
            NSRect imageRect = menuItemFrame;
            imageRect.origin.y += 2;
            imageRect.size.width = ItemImageWidth;
            imageRect.size.height = ItemImageHeight;
            
            [[menuItem image] drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        if(menuContainsImage)
        {
            menuItemFrame.origin.x += ItemImageSpace;
            menuItemFrame.size.width -= ItemImageSpace;
        }
        
        // Draw Submenu Arrow
        if(itemHasSubmenu)
        {
            NSImage *arrowImage = [menuView submenuImageWithHighlightedState:itemIsHighlighted];
            NSRect arrowRect = NSZeroRect;
            
            arrowRect.size = [arrowImage size];
            arrowRect.origin.x = NSMaxX(menuItemFrame) - ItemSubmenuSpace;
            arrowRect.origin.y = menuItemFrame.origin.y + round((NSHeight(menuItemFrame) - NSHeight(arrowRect))/2);
            
            [arrowImage drawInRect:arrowRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        menuItemFrame.size.width -= ItemSubmenuSpace;
        
        // Draw Item Title
        NSDictionary *textAttributes = itemIsHighlighted ? drawAlternate ? [menuView selectedItemAlternateTextAttributes]:[menuView selectedItemTextAttributes] : [menuView itemTextAttributes];
        textAttributes = itemIsDisabled ? [menuView disabledItemTextAttributes] : textAttributes;
        
        NSAttributedString *attrStr = [[NSAttributedString alloc] initWithString:menuItem.title attributes:textAttributes];
        menuItemFrame.origin.y += (NSHeight(menuItemFrame)-attrStr.size.height)/2.0;
        [attrStr drawInRect:menuItemFrame];        
        
        y += menuItemFrame.size.height;
    }
}

#pragma mark -
- (OEMenu *)menu
{
    return (OEMenu *)[self window];
}

@end

#pragma mark -
@implementation OE_MenuScrollView
#pragma mark -
- (OEMenu *)menu
{
    return (OEMenu *)[self window];
}
@end
#pragma mark -
@implementation OE_MenuScroller

+ (BOOL)isCompatibleWithOverlayScrollers
{
    return YES;
}

- (void)drawKnob
{}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{}

+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize scrollerStyle:(NSScrollerStyle)scrollerStyle
{
    return 0.0;
}

+ (CGFloat)scrollerWidth
{
    return 0.0;
}
@end
#pragma mark -
@implementation OEMenuScrollerView 
@synthesize up;

- (NSImage*)scrollUpArrowImageForStyle:(OEMenuStyle)style
{
    return [NSImage imageNamed:style == OEMenuStyleDark ? @"dark_menu_scroll_up" : @"light_menu_scroll_up"];
}

- (NSImage*)scrollDownArrowImageForStyle:(OEMenuStyle)style
{
    return [NSImage imageNamed:style == OEMenuStyleDark ? @"dark_menu_scroll_down" : @"light_menu_scroll_down"];
    
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{        
    OEMenu  *menu        = (OEMenu*)[self window];
    NSImage *scrollArrow = [self up] ? [self scrollUpArrowImageForStyle:[menu style]] : [self scrollDownArrowImageForStyle:[menu style]];
    
    float x = NSMidX([self bounds])-[scrollArrow size].width/2;
    float y = NSMidY([self bounds])-[scrollArrow size].height/2;
    
    NSRect targetRect = (NSRect){{x, y}, scrollArrow.size};
    [scrollArrow drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
}
@end
