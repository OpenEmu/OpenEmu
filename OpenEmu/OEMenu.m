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

#import "OEMenu.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OEPopupButton.h"

@interface OEMenu ()
- (BOOL)_isClosing;
- (OEMenuView *)menuView;
- (void)_performcloseMenu;
- (void)_closeByClickingItem:(NSMenuItem *)selectedItem;

- (void)setIsAlternate:(BOOL)flag;
- (CAAnimation*)alphaValueAnimation;
- (void)setAlphaValueAnimation:(CAAnimation *)anim;
@end

@implementation OEMenu
@synthesize edge=_edge;
@dynamic style;
@synthesize menu, supermenu, visible, popupButton, delegate;
@synthesize minSize, maxSize, itemsAboveScroller, itemsBelowScroller;
@synthesize alternate=_alternate;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEMenu class])
        return;
    
    NSImage *menuArrows = [NSImage imageNamed:@"dark_menu_popover_arrow"];
    [menuArrows setName:@"dark_menu_popover_arrow_normal" forSubimageInRect:NSMakeRect(0, menuArrows.size.height/2, menuArrows.size.width, menuArrows.size.height/2)];
    [menuArrows setName:@"dark_menu_popover_arrow_selected" forSubimageInRect:NSMakeRect(0, 0, menuArrows.size.width, menuArrows.size.height/2)];
    
    
    NSImage *scrollArrows = [NSImage imageNamed:@"dark_menu_scroll_arrows"];
    [scrollArrows setName:@"dark_menu_scroll_up" forSubimageInRect:NSMakeRect(0, 0, 9, 15)];
    [scrollArrows setName:@"dark_menu_scroll_down" forSubimageInRect:NSMakeRect(0, 15, 9, 15)];
    
    NSImage* tickMark = [NSImage imageNamed:@"tick_mark"];
    [tickMark setName:@"tick_mark_normal" forSubimageInRect:(NSRect){{0,0},{7,12}}];
    [tickMark setName:@"tick_mark_selected" forSubimageInRect:(NSRect){{7,0},{7,12}}];
}

- (id)init
{
    if((self = [super initWithContentRect:NSZeroRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO]))
    {
        [self setStyle:OEMenuStyleDark];
        
        [self setMaxSize:(NSSize){5000,500}];
        [self setMinSize:(NSSize){82,19*2}];
        
        [self setItemsAboveScroller:0];
        [self setItemsBelowScroller:0];
        
        OEMenuView *view = [[OEMenuView alloc] initWithFrame:NSZeroRect];
        [view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        [[self contentView] addSubview:view];
        [view release];
        
        [self setLevel:NSTornOffMenuWindowLevel];
        [self setBackgroundColor:[NSColor clearColor]];
        [self setHasShadow:NO];
        [self setOpaque:NO];
        
        CAAnimation *anim = [self alphaValueAnimation];
        [anim setDuration:0.075];
        [self setAlphaValueAnimation:anim];
    }
    
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    self.popupButton = nil;
    self.highlightedItem = nil;
    
    self.menu = nil;
    self.submenu = nil;
    self.supermenu = nil;
    
    self.delegate = nil;
    
    if(_localMonitor != nil)
    {
        [NSEvent removeMonitor:_localMonitor];
        [_localMonitor release];
        _localMonitor = nil;
    }
    
    [super dealloc];
}

- (BOOL)isVisible
{
    return visible && [super isVisible] && !closing;
}

#pragma mark -
#pragma mark Opening / Closing the menu

- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow *)win
{
    visible = YES;
    closing = NO;
    _alternate = NO;
    self.alphaValue = 1.0;
    
    if(_localMonitor != nil)
    {
        [NSEvent removeMonitor:_localMonitor];
        [_localMonitor release];
        
        _localMonitor = nil;
        
    }
    
    _localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDownMask | NSRightMouseDownMask | NSOtherMouseDownMask | NSKeyDownMask | NSFlagsChangedMask handler:
                     ^ NSEvent * (NSEvent *incomingEvent)
                     {
                         OEMenuView *view = [[[self contentView] subviews] lastObject];
                         
                         if([incomingEvent type] == NSFlagsChanged)
                         {
                             [self setIsAlternate:([incomingEvent modifierFlags] & NSAlternateKeyMask) != 0];
                             return nil;
                         }
                         
                         if([incomingEvent type] == NSKeyDown)
                         {
                             [view keyDown:incomingEvent];
                             return nil;
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
    
    [_localMonitor retain];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(closeMenuWithoutChanges:) name:NSApplicationWillResignActiveNotification object:NSApp];
    
    [[self menuView] updateAndDisplay:YES];
    
    
    NSPoint origin = p;
    if(![self supermenu] && style == OEMenuStyleLight)
        switch ([self edge]) {
            case NSMaxYEdge:
                origin.x -= self.frame.size.width / 2;
                origin.y -= self.frame.size.height;
                break;
            case NSMinYEdge:
                origin.x -= self.frame.size.width / 2;
                break;
            case NSMinXEdge:
                origin.y -= self.frame.size.height / 2;
                break;
            case NSMaxXEdge:
                origin.y -= self.frame.size.height / 2;
                origin.x -= self.frame.size.width;
                break;
            default:
                NSLog(@"This edge is not supported yet");
                break;
        }
    else 
    {
        if(origin.y<0)
            origin.y = 0;
        else if(origin.y + NSHeight([self frame]) > NSMaxY([[win screen] visibleFrame]))
            origin.y = NSMaxY([[win screen] visibleFrame])-NSHeight([self frame]);
    }
    
    [self setFrameOrigin:origin];
    [win addChildWindow:self ordered:NSWindowAbove];
    
    
    NSPoint windowP = [self convertScreenToBase:[NSEvent mouseLocation]];
    [[self menuView] highlightItemAtPoint:windowP];
    
    if([[self delegate] respondsToSelector:@selector(menuDidShow:)])
        [[self delegate] menuDidShow:self];
}


- (void)openOnEdge:(NSRectEdge)edge atPoint:(NSPoint)p ofWindow:(NSWindow*)win
{
    _edge = edge;
    [self setStyle:OEMenuStyleLight];
    [self openAtPoint:p ofWindow:win];
}

- (void)closeMenuWithoutChanges:(id)sender
{
    closing = YES;
    // make sure the menu does not vanish while being closed
    [self retain];
    
    if([self submenu] != nil) [self.submenu closeMenuWithoutChanges:sender];
    
    [self _performcloseMenu];
    
    if([[self delegate] respondsToSelector:@selector(menuDidCancel:)])
        [[self delegate] menuDidCancel:self];
    
    // now we are ready to be deallocated if needed
    [self release];
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

- (void)setMenu:(NSMenu *)nmenu
{
    [nmenu retain];
    [menu release];
    
    menu = nmenu;
    
    [[self menuView] updateAndDisplay:NO];
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
            [_localMonitor release];
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
    
    [theAnimation setDelegate:nil];
}

#pragma mark -
#pragma mark Setter / getter

- (NSMenuItem *)highlightedItem { return highlightedItem; }
- (void)setHighlightedItem:(NSMenuItem *)value
{
    if(highlightedItem != value)
    {
        [highlightedItem release];
        highlightedItem = [value retain];
        
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
        [[self menuView] updateAndDisplay:NO];
        
        NSRect selectedItemRect = [[self menuView] rectOfItem:self.highlightedItem];
        NSPoint submenuSpawnPoint = [self frame].origin;
        
        submenuSpawnPoint.x += [self frame].size.width;
        submenuSpawnPoint.x -= 9;
        
        if(![self supermenu] && [self style]==OEMenuStyleLight)
            submenuSpawnPoint.x -= 9;
        
        submenuSpawnPoint.y = 8 - selectedItemRect.origin.y + [self frame].origin.y -_submenu.frame.size.height + [self frame].size.height;
        
        _submenu.popupButton = self.popupButton;
        _submenu.supermenu = self;
        [_submenu openAtPoint:submenuSpawnPoint ofWindow:self];
    }
    
    [_submenu retain];
    [submenu release];
    submenu = _submenu;
}

- (OEMenu*)submenu
{
    return submenu;
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
#pragma mark NSMenu wrapping

- (NSArray *)itemArray
{
    return [[self menu] itemArray];
}

#pragma mark -
#pragma mark Private Methods
#define flickerDelay 0.09

- (BOOL)_isClosing
{
    return closing;
}

- (OEMenuView *)menuView
{
    return [[[self contentView] subviews] lastObject];
}

- (void)_performcloseMenu
{
    CAAnimation *anim = [self alphaValueAnimation];
    anim.delegate = self;
    [self setAlphaValueAnimation:anim];
    
    // fade menu window out 
    [[self animator] setAlphaValue:0.0];
}

- (void)_closeByClickingItem:(NSMenuItem *)selectedItem
{    
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
    
    [self _performcloseMenu];
    
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
    if(doAlternateAction && [(OEMenuItem*)selectedItem alternateTarget] && [(OEMenuItem*)selectedItem alternateAction]!=NULL && [[(OEMenuItem*)selectedItem alternateTarget] respondsToSelector:[(OEMenuItem*)selectedItem alternateAction]])
    {
        [[(OEMenuItem*)selectedItem alternateTarget] performSelectorOnMainThread:[(OEMenuItem*)selectedItem alternateAction] withObject:selectedItem waitUntilDone:NO];
    }
    else if(selectedItem && [selectedItem isEnabled] && [selectedItem target] && [selectedItem action]!=NULL && [[selectedItem target] respondsToSelector:[selectedItem action]])
    {
        [[selectedItem target] performSelectorOnMainThread:[selectedItem action] withObject:selectedItem waitUntilDone:NO];
    }
    
    // tell the delegate that the menu selected an item
    if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidSelect:)]) [self.delegate performSelector:@selector(menuDidSelect:) withObject:self];
}

- (void)setIsAlternate:(BOOL)flag
{
    if(closing || flag==_alternate) return;
    
    _alternate = flag;
    if(self.highlightedItem) [self display];
}

@end

#pragma mark -

@implementation NSMenu (OEAdditions)

- (OEMenu *)convertToOEMenu
{
    OEMenu *menu = [[OEMenu alloc] init];
    menu.menu = self;
    return [menu autorelease];
}

@end

#pragma mark -
#pragma mark Dark Style Sizes
#define MenuShadowLeft 5
#define MenuShadowRight 5
#define MenuTickmarkSpace 19
#define MenuImageWidth 15
#define MenuImageTitleSpacing 6
#pragma mark -
#pragma mark Light Style Sizes
#define LightStyleImageTop 21
#define LightStyleImageLeft 20
#define LightStyleImageRight 20
#define LightStyleImageBottom 21

#define LightStyleContentBottom 16
#define LightStyleContentTop 16
#define LightStyleContentLeft 14
#define LightStyleContentRight 14
#pragma mark -
#pragma mark General Sizing
#define menuItemHeight (imageIncluded ? menuItemHeightImage : menuItemHeightNoImage)
#define menuItemSeparatorHeight 7
#define menuItemHeightNoImage 17
#define menuItemHeightImage 20

#pragma mark -
#pragma mark -
#pragma mark Menu Item Sizes + Spacing
#define menuItemSpacingTop 8 + (imageIncluded ? 1 : 0)
#define menuItemSpacingBottom 9 + (imageIncluded ? 1 : 0)
#define menuItemSpacingLeft 0 + (imageIncluded ? 13 : 0)
#define menuItemImageWidth 16
#define menuItemImageTitleSpacing 6
#define menuItemSpacingRight 14 - (imageIncluded ? 1 : 0)


#pragma mark -
#pragma mark OEMenuView

@interface OEMenuView ()
- (void)highlightItemAtPoint:(NSPoint)p;
@end

@implementation OEMenuView

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseMoved|NSTrackingMouseEnteredAndExited|NSTrackingActiveInActiveApp owner:self userInfo:nil];
        [self addTrackingArea:area];
        [area release];
    }
    return self;
}

- (void)dealloc
{
    while([[self trackingAreas] count] != 0)
        [self removeTrackingArea:[[self trackingAreas] lastObject]];
    
    [super dealloc];
}

#pragma mark -
#pragma mark TextAttributes

- (NSDictionary *)itemTextAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
    NSColor *textColor = [[self menu] style]==OEMenuStyleDark?[NSColor whiteColor]:[NSColor blackColor];
    NSMutableParagraphStyle *ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
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
    NSMutableParagraphStyle *ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
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
    NSMutableParagraphStyle *ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
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
    NSMutableParagraphStyle *ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
    [ps setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [dict setObject:font forKey:NSFontAttributeName];
    [dict setObject:textColor forKey:NSForegroundColorAttributeName];
    [dict setObject:ps forKey:NSParagraphStyleAttributeName];
    
    return dict;
}

#pragma mark -
#pragma mark Drawing
- (void)updateAndDisplay:(BOOL)displayFlag
{
    NSArray *items = [[self menu] itemArray];
    
    float width = 0;
    
    imageIncluded = NO;
    
    int normalItems = 0;
    int separatorItems = 0;
    NSDictionary *attributes = [self itemTextAttributes];
    for(NSMenuItem *menuItem in items)
    {
        if([menuItem isSeparatorItem])
        { 
            separatorItems++; continue;
        }
        
        NSAttributedString *attributedTitle = [[NSAttributedString alloc] initWithString:menuItem.title attributes:attributes];
        width = width < attributedTitle.size.width ? attributedTitle.size.width : width;
        [attributedTitle release];
        
        if(menuItem.image!=nil)
        {
            imageIncluded = YES;
        }
        normalItems ++;
    }
    
    float height;
    
    if([[self menu] style] == OEMenuStyleDark || [[self menu] supermenu])
    {
        height = menuItemHeight*normalItems + menuItemSeparatorHeight*separatorItems + menuItemSpacingTop+menuItemSpacingBottom;
        
        width = MenuShadowLeft + MenuTickmarkSpace + width + MenuShadowRight;
        width += imageIncluded ? menuItemImageWidth+menuItemImageTitleSpacing : 0 ;
    }
    else
    {
        height = LightStyleContentTop+ menuItemHeight*normalItems + menuItemSeparatorHeight*separatorItems + LightStyleContentBottom;
        
        width = LightStyleContentLeft + MenuTickmarkSpace + width + LightStyleContentRight+menuItemSpacingRight;
        width += imageIncluded ? menuItemImageWidth+menuItemImageTitleSpacing : 0 ;
    }
    
    width = width < [self menu].minSize.width ? [self menu].minSize.width : width;
    width = width > [self menu].maxSize.width ? [self menu].maxSize.width : width;
    
    height = height < [self menu].minSize.height ? [self menu].minSize.height : height;
    height = height > [self menu].maxSize.height ? [self menu].maxSize.height : height;
    
    NSRect winFrame = [self menu].frame;
    winFrame.size = NSMakeSize(width, height);
    
    [[self menu] setFrame:winFrame display:displayFlag];
}

- (void)drawRect:(NSRect)dirtyRect
{
    OEMenuStyle style = [[self menu] style];
    
    NSColor *startColor = style==OEMenuStyleDark?[NSColor colorWithDeviceWhite:0.91 alpha:0.10]:[NSColor colorWithDeviceWhite:0.85 alpha:0.90];
    NSColor *endColor = style==OEMenuStyleDark?[startColor colorWithAlphaComponent:0.0]:[NSColor colorWithDeviceWhite:0.75 alpha:0.90];
    NSGradient *grad = [[[NSGradient alloc] initWithStartingColor:startColor endingColor:endColor] autorelease];
    
    NSRect backgroundRect;
    if(style == OEMenuStyleDark || [[self menu] supermenu])
    {
        backgroundRect = NSInsetRect([self bounds], 4, 4);
    }
    else
    {
        backgroundRect = NSInsetRect([self bounds], 14, 14);
    }
    
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:backgroundRect xRadius:3 yRadius:3];
    if(style == OEMenuStyleDark)    // Draw Background color
    {
        NSColor *backgroundColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.8];
        [[[[NSGradient alloc] initWithStartingColor:backgroundColor endingColor:backgroundColor] autorelease] drawInBezierPath:path angle:90];
        // draw gradient above
        [grad drawInBezierPath:path angle:90];
    } 
    else if([self superview])
        [grad drawInBezierPath:path angle:90];
    
    // draw background border
    if(style == OEMenuStyleDark || [[self menu] supermenu])
    {
        NSImage *img = [[self menu] supermenu]==nil ? [NSImage imageNamed:@"dark_menu_body"] : [NSImage imageNamed:@"dark_submenu_body"];
        [img drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:17 rightBorder:17 topBorder:19 bottomBorder:19];
    }
    else if(![[self menu] supermenu])
    {
        NSSize cornerSize = (NSSize){20, 21};
        
        // draw open edge
        NSRect edgeSourceRect, edgeTargetRect;
        NSRectEdge edge = [[self menu] edge];
        switch (edge) {
            case NSMinXEdge:
                edgeSourceRect = (NSRect){{0,21},{20, 15}};
                edgeTargetRect = (NSRect){{0, roundf((self.frame.size.height-edgeSourceRect.size.height)/2)},edgeSourceRect.size};
                backgroundRect.size.width += edgeSourceRect.size.width;
                backgroundRect.origin.x -= edgeSourceRect.size.width;                
                break;
            case NSMaxXEdge:
                edgeSourceRect = (NSRect){{34,21},{20, 15}};
                edgeTargetRect = (NSRect){{self.frame.size.width-edgeSourceRect.size.width, roundf((self.frame.size.height-edgeSourceRect.size.height)/2)},edgeSourceRect.size};
                backgroundRect.size.width += edgeSourceRect.size.width;
                break;
            case NSMinYEdge:
                edgeSourceRect = (NSRect){{20,0},{14, 21}};
                edgeTargetRect = (NSRect){{roundf((self.frame.size.width-edgeSourceRect.size.width)/2), self.frame.size.height-edgeSourceRect.size.height},edgeSourceRect.size};
                backgroundRect.size.height += edgeSourceRect.size.height;
                break;
            case NSMaxYEdge:
                edgeSourceRect = (NSRect){{20,36},{14, 21}};
                edgeTargetRect = (NSRect){{roundf((self.frame.size.width-edgeSourceRect.size.width)/2), 0},edgeSourceRect.size};
                backgroundRect.size.height += edgeSourceRect.size.height;
                backgroundRect.origin.y -= edgeSourceRect.size.height;
                break;
            default:
                break;
        }
        NSImage* lightMenuBodyImage = [NSImage imageNamed:@"light_arrows_menu_body"];
        NSImage* lightMenuBodyMaskImage = [NSImage imageNamed:@"light_arrows_menu_body_mask"];
        [[NSColor blackColor] setFill];
        [path fill];
        [lightMenuBodyMaskImage drawInRect:edgeTargetRect fromRect:edgeSourceRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:NoInterpol];
        
        [[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeSourceIn];
        
        [grad drawInRect:backgroundRect angle:90];
        
        [lightMenuBodyImage drawInRect:edgeTargetRect fromRect:edgeSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        [[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeCopy];

        
        // top left
        [lightMenuBodyImage drawInRect:(NSRect){{0,0}, cornerSize} fromRect:(NSRect){{0,lightMenuBodyImage.size.height-cornerSize.height}, cornerSize} operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        // top right
        [lightMenuBodyImage drawInRect:(NSRect){{self.frame.size.width-cornerSize.width,0}, cornerSize} fromRect:(NSRect){{[lightMenuBodyImage size].width-cornerSize.width,lightMenuBodyImage.size.height-cornerSize.height}, cornerSize} operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        // bottom left
        [lightMenuBodyImage drawInRect:(NSRect){{0,self.frame.size.height-cornerSize.height}, cornerSize} fromRect:(NSRect){{0,0}, cornerSize} operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        // bottom right
        [lightMenuBodyImage drawInRect:(NSRect){{self.frame.size.width-cornerSize.width,self.frame.size.height-cornerSize.height}, cornerSize} fromRect:(NSRect){{[lightMenuBodyImage size].width-cornerSize.width,0}, cornerSize} operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        
        
        // draw rest of the frame        
        NSRect leftTargetRect = (NSRect){{0, LightStyleImageTop},{LightStyleImageLeft, self.bounds.size.height-LightStyleImageBottom-LightStyleImageTop}};
        NSRect leftSourceRect = (NSRect){{0,LightStyleImageTop-1}, {LightStyleImageLeft,1}};
        if(edge != NSMinXEdge)
            [lightMenuBodyImage drawInRect:leftTargetRect fromRect:leftSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        else
        {
            float edgeY = roundf((self.frame.size.height-edgeSourceRect.size.height)/2);
            leftTargetRect.origin.y = LightStyleImageTop;
            leftTargetRect.size.height = edgeY-LightStyleImageTop;
            [lightMenuBodyImage drawInRect:leftTargetRect fromRect:leftSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            leftTargetRect.origin.y += leftTargetRect.size.height+edgeSourceRect.size.height;
            [lightMenuBodyImage drawInRect:leftTargetRect fromRect:leftSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        
        NSRect rightTargetRect = (NSRect){{self.frame.size.width-LightStyleImageRight, LightStyleImageTop},{LightStyleImageRight, self.bounds.size.height-LightStyleImageBottom-LightStyleImageTop}};
        NSRect rightSourceRect = (NSRect){{lightMenuBodyImage.size.width-LightStyleImageRight,LightStyleImageTop-1},{LightStyleImageRight, 1}};
        if(edge != NSMaxXEdge)
            [lightMenuBodyImage drawInRect:rightTargetRect fromRect:rightSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        else
        {
            float edgeY = roundf((self.frame.size.height-edgeSourceRect.size.height)/2);
            rightTargetRect.origin.y = LightStyleImageTop;
            rightTargetRect.size.height = edgeY-LightStyleImageTop;
            [lightMenuBodyImage drawInRect:rightTargetRect fromRect:rightSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            rightTargetRect.origin.y += rightTargetRect.size.height+edgeSourceRect.size.height;
            [lightMenuBodyImage drawInRect:rightTargetRect fromRect:rightSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        
        NSRect topTargetRect = (NSRect){{LightStyleImageLeft, 0}, {self.frame.size.width-LightStyleImageLeft-LightStyleImageRight, LightStyleImageTop}};
        NSRect topSourceRect = (NSRect){{LightStyleImageLeft-1, lightMenuBodyImage.size.height-LightStyleImageTop},{1, LightStyleImageTop}};
        if(edge != NSMaxYEdge)
            [lightMenuBodyImage drawInRect:topTargetRect fromRect:topSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        else
        {
            float edgeX = roundf((self.frame.size.width-edgeSourceRect.size.width)/2);
            topTargetRect.origin.x = LightStyleImageLeft;
            topTargetRect.size.width = edgeX - LightStyleImageLeft;
            [lightMenuBodyImage drawInRect:topTargetRect fromRect:topSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];

            topTargetRect.origin.x += topTargetRect.size.width + edgeSourceRect.size.width;
            [lightMenuBodyImage drawInRect:topTargetRect fromRect:topSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        NSRect bottomTargetRect = (NSRect){{LightStyleImageLeft, self.frame.size.height-LightStyleImageBottom}, {self.frame.size.width-LightStyleImageLeft-LightStyleImageRight, LightStyleImageBottom}};
        NSRect bottomSourceRect = (NSRect){{lightMenuBodyImage.size.width-LightStyleImageRight, 0},{1, LightStyleImageBottom}};
        
        if(edge != NSMinYEdge)
            [lightMenuBodyImage drawInRect:bottomTargetRect fromRect:bottomSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        else
        {
            float edgeX = roundf((self.frame.size.width-edgeSourceRect.size.width)/2);
            bottomTargetRect.origin.x = LightStyleImageLeft;
            bottomTargetRect.size.width = edgeX - LightStyleImageLeft;
            [lightMenuBodyImage drawInRect:bottomTargetRect fromRect:bottomSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
            
            bottomTargetRect.origin.x += bottomTargetRect.size.width + edgeSourceRect.size.width;
            [lightMenuBodyImage drawInRect:bottomTargetRect fromRect:bottomSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
    }
    
    NSArray *items = [[self menu] itemArray];
    BOOL darkStyle = style == OEMenuStyleDark  || [[self menu] supermenu];
    
    float y = darkStyle?menuItemSpacingTop:LightStyleContentTop;
    for(NSMenuItem *menuItem in items)
    {
        if([menuItem isSeparatorItem])
        {
            NSRect lineRect;
            if(darkStyle)
            {
                lineRect = (NSRect){{MenuShadowLeft, y}, {[self frame].size.width-MenuShadowLeft-MenuShadowRight, 1}};
                lineRect = NSInsetRect(lineRect, 5, 0);
                
            }
            else
            {
                lineRect = (NSRect){{LightStyleContentLeft, y}, {[self frame].size.width-LightStyleContentLeft-LightStyleContentRight, 1}};
            }
            
            lineRect.origin.y += 2;
            [[NSColor colorWithDeviceWhite:0.19 alpha:1.0] setFill];
            NSRectFill(lineRect);
            
            lineRect.origin.y += 1;
            [[NSColor colorWithDeviceWhite:1.0 alpha:0.5] setFill];
            NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);
            
            y += menuItemSeparatorHeight;
            continue;
        }
        NSRect itemRect, menuItemFrame;
        if(darkStyle)
        {
            itemRect = NSMakeRect(MenuShadowLeft+MenuTickmarkSpace, y, [self frame].size.width-MenuShadowLeft-menuItemSpacingRight-MenuShadowRight-MenuTickmarkSpace, menuItemHeight);
            menuItemFrame = NSMakeRect(MenuShadowLeft, y, [self frame].size.width-MenuShadowLeft-MenuShadowRight, menuItemHeight);
        }
        else
        {
            itemRect = NSMakeRect(LightStyleContentLeft+MenuTickmarkSpace, y, [self frame].size.width-LightStyleContentLeft-LightStyleContentRight-MenuTickmarkSpace-menuItemSpacingRight, menuItemHeight);
            menuItemFrame = NSMakeRect(LightStyleContentLeft, y, [self frame].size.width-LightStyleContentLeft-LightStyleContentRight, menuItemHeight);
        }
        BOOL isHighlighted = [self menu].highlightedItem==menuItem;
        BOOL isDisabled = ![menuItem isEnabled];
        BOOL hasImage = [menuItem image]!=nil;
        BOOL hasSubmenu = [menuItem hasSubmenu];
        
        BOOL drawAlternate = !isDisabled && isHighlighted && [menuItem isKindOfClass:[OEMenuItem class]] && [(OEMenuItem*)menuItem hasAlternate] && [self menu].alternate;
        if(!isDisabled && isHighlighted)
        {
            NSColor *cTop, *cBottom;
            
            if(drawAlternate)
            {
                cTop = [NSColor colorWithDeviceRed:0.71 green:0.07 blue:0.14 alpha:1.0];
                cBottom = [NSColor colorWithDeviceRed:0.48 green:0.02 blue:0.07 alpha:1.0];
            }
            else
            {
                cTop = style==OEMenuStyleDark?[NSColor colorWithDeviceWhite:0.91 alpha:1.0]:[NSColor colorWithDeviceWhite:0.19 alpha:1.0];
                cBottom = style==OEMenuStyleDark?[NSColor colorWithDeviceWhite:0.71 alpha:1.0]:[NSColor colorWithDeviceWhite:0.10 alpha:1.0];
            }
            
            NSGradient *selectionGrad = [[NSGradient alloc] initWithStartingColor:cTop endingColor:cBottom];
            [selectionGrad drawInRect:menuItemFrame angle:90];
            [selectionGrad release];
        }
        
        // Draw menu item image
        if(hasImage)
        {
            NSRect imageRect = itemRect;
            imageRect.origin.y += 2;
            imageRect.size.width = menuItemImageWidth;
            imageRect.size.height = menuItemImageWidth;
            
            [menuItem.image drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
            
            itemRect.origin.x += menuItemImageWidth+menuItemImageTitleSpacing;
            itemRect.size.width -= (menuItemImageWidth+menuItemImageTitleSpacing);
        }
        
        // Draw submenu arrow
        if(hasSubmenu)
        {
            NSImage *arrow = isHighlighted ^ (style==OEMenuStyleLight)? [NSImage imageNamed:@"dark_menu_popover_arrow_selected"] : [NSImage imageNamed:@"dark_menu_popover_arrow_normal"];
            NSRect arrowRect = NSMakeRect(0, 0, 0, 0);
            arrowRect.size = arrow.size;
            arrowRect.origin.x = menuItemFrame.origin.x + menuItemFrame.size.width - 11;
            arrowRect.origin.y = menuItemFrame.origin.y + (menuItemHeight - arrow.size.height)/2;
            [arrow drawInRect:arrowRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
            
            itemRect.size.width -= 11;
        }
        
        // Draw tickmark
        if([[self menu] popupButton] && [[[self menu] popupButton] selectedItem]==menuItem)
        {
            NSImage *tickMarkImage = isHighlighted ? [NSImage imageNamed:@"tick_mark_selected"] : [NSImage imageNamed:@"tick_mark_normal"];
            NSRect tickMarkRect = menuItemFrame;
            tickMarkRect.origin.x += 6;
            tickMarkRect.origin.y += roundf((menuItemFrame.size.height-12)/2)-1;
            tickMarkRect.size = (NSSize){7, 12};
            
            [tickMarkImage drawInRect:tickMarkRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
        }
        
        // Draw Item Title
        NSDictionary *textAttributes = isHighlighted ? drawAlternate ? [self selectedItemAlternateTextAttributes]:[self selectedItemTextAttributes] : [self itemTextAttributes];
        textAttributes = isDisabled ? [self disabledItemTextAttributes] : textAttributes;
        
        NSAttributedString *attrStr = [[NSAttributedString alloc] initWithString:menuItem.title attributes:textAttributes];
        itemRect.origin.y += (menuItemHeight-attrStr.size.height)/2.0;
        [attrStr drawInRect:itemRect];
        [attrStr release];
        
        y += menuItemHeight;
    }
}

#pragma mark -
#pragma mark Interaction

- (void)updateTrackingAreas
{
    NSTrackingArea *area = [[self trackingAreas] objectAtIndex:0];
    [self removeTrackingArea:area];
    area = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseMoved|NSTrackingMouseEnteredAndExited|NSTrackingActiveAlways owner:self userInfo:nil];
    [self addTrackingArea:area];
    [area release];
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
    
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    // if not mouse on subwindow
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

#pragma mark -

- (void)keyDown:(NSEvent *)theEvent
{
    if([[self menu] _isClosing]) return;
    
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
            break;
            
        case 125 : // DOWN
            if([[self menu] highlightedItem])
            {
                NSInteger index = [self.menu.itemArray indexOfObject:self.menu.highlightedItem];
                if(index!=NSNotFound && index < self.menu.itemArray.count-1)
                    [[self menu] setHighlightedItem:[[[self menu] itemArray] objectAtIndex:index + 1]];
            }
            else [[self menu] setHighlightedItem:[[[self menu] itemArray] objectAtIndex:0]];
            break;
        case 123 : // LEFT (exit submenu if any)
            break;
        case 124 : // RIGHT (enter submenu if any)
            break;
        case 53 : // ESC (close without changes)
            [[self menu] closeMenuWithoutChanges:self];
            break;
        case 49 : // SPACE ("click" selected item)
        case 36 : // ENTER (same as space)
            [[self menu] closeMenu];
            break;
        default:
            break;
    }
    
    // a little explanation for this:
    // selecting a separator item should not be possible, so if the selected item is a separator we try to jump over it
    // this will continue until either a normal item was selected or the last (or first depending on direction) item is reached
    // we then check if the selected item is still a separator and if so we select the item we started with
    // this ensures that a valid item will be selected after a key was pressed
    if(([theEvent keyCode] == 126 || [theEvent keyCode] == 125) && [[self menu] highlightedItem] != currentItem && [[[self menu ] highlightedItem] isSeparatorItem])
    {
        [self keyDown:theEvent];
        if([[[self menu] highlightedItem] isSeparatorItem])
            [[self menu] setHighlightedItem:currentItem];
    }
    
    [self setNeedsDisplay:YES];
}

#pragma mark -

- (void)highlightItemAtPoint:(NSPoint)p
{
    NSMenuItem *highlighItem = [self itemAtPoint:p];
    
    if(highlighItem != [self menu].highlightedItem)
    {
        if([highlighItem isSeparatorItem])
            highlighItem = nil;
        
        [[self menu] setHighlightedItem:highlighItem];
        
        [self setNeedsDisplay:YES];
    }
}

- (NSMenuItem *)itemAtPoint:(NSPoint)p
{
    if(p.x <= MenuShadowLeft || p.x >= [self bounds].size.width-MenuShadowRight)
        return nil;
    if(p.y <= menuItemSpacingTop || p.y >= [self bounds].size.height-menuItemSpacingBottom)
        return nil;
    
    float y = menuItemSpacingTop;
    
    for(NSMenuItem *item in [[self menu] itemArray])
    {
        if([item isSeparatorItem])
        {
            y += menuItemSeparatorHeight;
            continue;
        }
        
        y += menuItemHeight;
        if(p.y < y && p.y > y - menuItemHeight)
            return item;
    }
    
    return nil;
}

- (NSRect)rectOfItem:(NSMenuItem *)m
{
    NSArray *itemArray = [[self menu] itemArray];
    NSUInteger pos = [itemArray indexOfObject:m];
    
    float y = menuItemHeight*pos +menuItemSpacingTop;
    NSRect menuItemFrame = NSMakeRect(MenuShadowLeft, y, [self frame].size.width-MenuShadowLeft-MenuShadowRight, menuItemHeight);
    return menuItemFrame;
}

#pragma mark -
#pragma mark View Config Overrides

- (BOOL)acceptsFirstResponder
{
    return YES;
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

- (OEMenu *)menu
{
    return (OEMenu *)[self window];
}

@end
