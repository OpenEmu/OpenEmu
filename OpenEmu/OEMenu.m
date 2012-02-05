//
//  OENSMenu.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEMenu.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OEPopupButton.h"
@interface OEMenu (Private)
- (BOOL)_isClosing;
- (OEMenuView*)menuView;
- (void)_performcloseMenu;
- (void)_closeByClickingItem:(NSMenuItem*)selectedItem;

- (void)setIsAlternate:(BOOL)flag;
- (CAAnimation*)alphaValueAnimation;
- (void)setAplhaValueAnimation:(CAAnimation*)anim;
@end

@implementation OEMenu
@synthesize menu, supermenu, visible, popupButton, delegate;
@synthesize minSize, maxSize, itemsAboveScroller, itemsBelowScroller;
@synthesize alternate=_alternate;
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEMenu class])
        return;

    NSImage *menuArrows = [NSImage imageNamed:@"dark_menu_popover_arrow"];
    [menuArrows setName:@"dark_menu_popover_arrow_normal" forSubimageInRect:NSMakeRect(0, menuArrows.size.height/2, menuArrows.size.width, menuArrows.size.height/2)];
    [menuArrows setName:@"dark_menu_popover_arrow_selected" forSubimageInRect:NSMakeRect(0, 0, menuArrows.size.width, menuArrows.size.height/2)];
    
    
    NSImage *scrollArrows = [NSImage imageNamed:@"dark_menu_scroll_arrows"];
    [scrollArrows setName:@"dark_menu_scroll_up" forSubimageInRect:NSMakeRect(0, 0, 9, 15)];
    [scrollArrows setName:@"dark_menu_scroll_down" forSubimageInRect:NSMakeRect(0, 15, 9, 15)];
}

- (id)init
{
    self = [super initWithContentRect:NSZeroRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
    if (self) 
    {
        self.maxSize = NSMakeSize(192, 500);
        self.minSize = NSMakeSize(82, 19*2);
        
        self.itemsAboveScroller = 0;
        self.itemsBelowScroller = 0;
        
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
        [self setAplhaValueAnimation:anim];
        
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
    
    if(_localMonitor){
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
- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow*)win
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
    _localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDownMask | NSRightMouseDownMask | NSOtherMouseDownMask | NSKeyDownMask | NSFlagsChangedMask handler:^(NSEvent *incomingEvent) 
                     {
                         OEMenuView *view = [[[self contentView] subviews] lastObject];
                         
                         if([incomingEvent type] == NSFlagsChanged){
                             [self setIsAlternate:([incomingEvent modifierFlags] & NSAlternateKeyMask) != 0];
                             return (NSEvent *)nil;
                         } 
                         
                         if([incomingEvent type] == NSKeyDown)
                         {
                             
                             [view keyDown:incomingEvent];
                             return (NSEvent *)nil;
                         }
                         
                         if([[incomingEvent window] isKindOfClass:[self class]])// mouse down in window, will be handle by content view
                         { 
                             return incomingEvent;
                         } 
                         else if([self popupButton] && NSPointInRect([[self popupButton] convertPoint:[incomingEvent locationInWindow] fromView:nil], [[self popupButton] bounds])){
                             [(OEPopupButton*)[self popupButton] setDontOpenMenuOnNextMouseUp:YES];
                             [self closeMenuWithoutChanges:nil];
                         } else {
                             // event is outside of window, close menu without changes and remove event
                             [self closeMenuWithoutChanges:nil];
                         }
                         return (NSEvent *)nil;
                     }];
    [_localMonitor retain];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(closeMenuWithoutChanges:) name:NSApplicationWillResignActiveNotification object:NSApp];
    
    [self setFrameOrigin:p];
    [win addChildWindow:self ordered:NSWindowAbove];
    
    [[self menuView] update];
    
    NSPoint windowP = [self convertScreenToBase:[NSEvent mouseLocation]];
    [[self menuView] highlightItemAtPoint:windowP];
    if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidShow:)]) [self.delegate performSelector:@selector(menuDidShow:) withObject:self];}

- (void)closeMenuWithoutChanges:(id)sender
{
    closing = YES;
    // make sure the menu does not vanish while being closed
    [self retain];
    
    if(self.submenu) [self.submenu closeMenuWithoutChanges:sender];
    
    [self _performcloseMenu];
    
    if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidCancel:)]) [self.delegate performSelector:@selector(menuDidCancel:) withObject:self];
    
    // now we are ready to be deallocated if needed
    [self release];
}

- (void)closeMenu
{   
    closing = YES;
    
    OEMenu *superMen = self;
    while(superMen.supermenu)
    {
        superMen = superMen.supermenu;
    }
    
    if(superMen != self)
    {
        [superMen closeMenu];
        return;
    }
    
    OEMenu *subMen = self;
    while(subMen.submenu)
    {
        subMen = subMen.submenu;
    }
    
    NSMenuItem *selectedItem = subMen.highlightedItem;
    [self _closeByClickingItem:selectedItem];
}

- (void)setMenu:(NSMenu *)nmenu
{
    [nmenu retain];
    [menu release];
    
    menu = nmenu;
    
    [[self menuView] update];
}

#pragma mark -
#pragma mark Animation Stuff
- (CAAnimation*)alphaValueAnimation
{
    return [[self animator] animationForKey:@"alphaValue"];
}
- (void)setAplhaValueAnimation:(CAAnimation*)anim
{
    [[self animator] setAnimations:[NSDictionary dictionaryWithObject:anim forKey:@"alphaValue"]];
}
- (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag{
    if(flag)
    {
        self.highlightedItem = nil;
        
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        
        // Remove event monitor
        if(_localMonitor!=nil)
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
    
    theAnimation.delegate = nil;
}
#pragma mark -
#pragma mark Setter / getter
- (void)setHighlightedItem:(NSMenuItem *)_highlightedItem
{
    [_highlightedItem retain];
    [highlightedItem release];
    highlightedItem = _highlightedItem;
    
    self.submenu = [[highlightedItem submenu] convertToOEMenu];
}
- (NSMenuItem*)highlightedItem
{
    return highlightedItem;
}

- (void)setSubmenu:(OEMenu *)_submenu
{
    if(submenu)
    {
        [submenu closeMenuWithoutChanges:nil];
    }
    
    if(_submenu)
    {
        [[self menuView] update];
        
        NSRect selectedItemRect = [[self menuView] rectOfItem:self.highlightedItem];
        NSPoint submenuSpawnPoint = [self frame].origin;
        
        submenuSpawnPoint.x += [self frame].size.width;
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
#pragma mark NSMenu wrapping
- (NSArray *)itemArray
{
    return [self.menu itemArray];
}
#pragma mark -
#pragma mark Private Methods
#define flickerDelay 0.09

- (BOOL)_isClosing
{
    return closing;
}

- (OEMenuView*)menuView
{
    return [[[self contentView] subviews] lastObject];
}

- (void)_performcloseMenu
{
    CAAnimation *anim = [self alphaValueAnimation];
    anim.delegate = self;
    [self setAplhaValueAnimation:anim];
    
    // fade menu window out 
    [[self animator] setAlphaValue:0.0];
}

- (void)_closeByClickingItem:(NSMenuItem*)selectedItem
{    
    closing = YES;
    if(self.submenu) [self.submenu closeMenuWithoutChanges:nil];
    
    [NSTimer scheduledTimerWithTimeInterval:flickerDelay target:self selector:@selector(_closeTimer:) userInfo:selectedItem repeats:NO];
    self.highlightedItem = nil;
    [self display];
}

- (void)_closeTimer:(NSTimer*)timer
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

- (void)setIsAlternate:(BOOL)flag{
    if(closing || flag==_alternate) return;
    
    _alternate = flag;
    if(self.highlightedItem) [self display];
}
@end
#pragma mark -
@implementation NSMenu (OEAdditions)

- (OEMenu*)convertToOEMenu
{
    OEMenu *menu = [[OEMenu alloc] init];
    menu.menu = self;
    return [menu autorelease];
}
@end

#pragma mark -
#pragma mark Menu Item Sizes + Spacing
#define menuItemSpacingTop 8 + (imageIncluded ? 1 : 0)
#define menuItemSpacingBottom 9 + (imageIncluded ? 1 : 0)
#define menuItemSpacingLeft 13 + (imageIncluded ? 13 : 0)
#define menuItemImageWidth 16
#define menuItemImageTitleSpacing 6
#define menuItemSpacingRight 17 - (imageIncluded ? 1 : 0)

#define menuItemHeightNoImage 17
#define menuItemHeightImage 20
#define menuItemSeparatorHeight 7
#define menuItemHeight (imageIncluded ? menuItemHeightImage : menuItemHeightNoImage)

#define menuItemScrollerHeight 15
#pragma mark -
#pragma mark OEMenuView
@interface OEMenuView (Private)
- (void)highlightItemAtPoint:(NSPoint)p;
@end
@implementation OEMenuView

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
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
    {
        [self removeTrackingArea:[[self trackingAreas] lastObject]];
    }
    
    [super dealloc];
}

#pragma mark -
#pragma mark TextAttributes
- (NSDictionary*)itemTextAttributes
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

- (NSDictionary*)selectedItemTextAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
    NSColor *textColor = [NSColor blackColor];
    NSMutableParagraphStyle *ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
    [ps setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [dict setObject:font forKey:NSFontAttributeName];
    [dict setObject:textColor forKey:NSForegroundColorAttributeName];
    [dict setObject:ps forKey:NSParagraphStyleAttributeName];
    
    return dict;
}
- (NSDictionary*)selectedItemAlternateTextAttributes
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
- (NSDictionary*)disabledItemTextAttributes
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
- (void)update
{
    NSArray *items = [self.menu itemArray];
    
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
    
    float height = menuItemHeight*normalItems + menuItemSeparatorHeight*separatorItems + menuItemSpacingTop+menuItemSpacingBottom;
    
    width += menuItemSpacingLeft + menuItemSpacingRight;
    width += imageIncluded ? menuItemImageWidth+menuItemImageTitleSpacing : 0 ;
    
    width = width < self.menu.minSize.width ? self.menu.minSize.width : width;
    width = width > self.menu.maxSize.width ? self.menu.maxSize.width : width;
    
    height = height < self.menu.minSize.height ? self.menu.minSize.height : height;
    height = height > self.menu.maxSize.height ? self.menu.maxSize.height : height;
    
    NSRect winFrame = self.menu.frame;
    winFrame.size = NSMakeSize(width, height);
    
    [self.menu setFrame:winFrame display:YES];
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
    
    NSColor *startColor = [NSColor colorWithDeviceWhite:0.91 alpha:0.10];
    NSColor *endColor = [startColor colorWithAlphaComponent:0.0];
    NSGradient *grad = [[[NSGradient alloc] initWithStartingColor:startColor endingColor:endColor] autorelease];
    
    NSRect backgroundRect = NSInsetRect([self bounds], 4, 4);
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:backgroundRect xRadius:3 yRadius:3];
    
    // Draw Background color
    NSColor *backgroundColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.8];
    [[[[NSGradient alloc] initWithStartingColor:backgroundColor endingColor:backgroundColor] autorelease] drawInBezierPath:path angle:90];
    
    // draw gradient above
    [grad drawInBezierPath:path angle:90];
    
    // draw background border
    NSImage *img = [self.menu supermenu]==nil ? [NSImage imageNamed:@"dark_menu_body"] : [NSImage imageNamed:@"dark_submenu_body"];
    [img drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:17 rightBorder:17 topBorder:19 bottomBorder:19];
    
    
    NSArray *items = [self.menu itemArray];
    float y = menuItemSpacingTop;
    for(NSMenuItem *menuItem in items)
    {
        if([menuItem isSeparatorItem])
        {
            NSRect lineRect = NSMakeRect(0, y, [self frame].size.width, 1);
            lineRect = NSInsetRect(lineRect, 5, 0);
            
            lineRect.origin.y += 2;
            [[NSColor blackColor] setFill];
            NSRectFill(lineRect);
            
            lineRect.origin.y += 1;
            [[NSColor colorWithDeviceWhite:1.0 alpha:0.12] setFill];
            NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);
            
            y += menuItemSeparatorHeight;
            continue;
        }
        NSRect itemRect = NSMakeRect(menuItemSpacingLeft, y, [self frame].size.width-menuItemSpacingLeft-menuItemSpacingRight, menuItemHeight);
        NSRect menuItemFrame = NSMakeRect(5, y, [self frame].size.width-5-5, menuItemHeight);
        
        BOOL isSelected = self.menu.highlightedItem==menuItem;
        BOOL isDisabled = ![menuItem isEnabled];
        BOOL hasImage = [menuItem image]!=nil;
        BOOL hasSubmenu = [menuItem hasSubmenu];
        
        BOOL drawAlternate = !isDisabled && isSelected && [menuItem isKindOfClass:[OEMenuItem class]] && [(OEMenuItem*)menuItem hasAlternate] && self.menu.alternate;
        if(!isDisabled && isSelected)
        {
            NSColor *cTop, *cBottom;
            
            if(drawAlternate)
            {
                cTop = [NSColor colorWithDeviceRed:0.71 green:0.07 blue:0.14 alpha:1.0];
                cBottom = [NSColor colorWithDeviceRed:0.48 green:0.02 blue:0.07 alpha:1.0];
            }
            else
            {
                cTop = [NSColor colorWithDeviceWhite:0.91 alpha:1.0];
                cBottom = [NSColor colorWithDeviceWhite:0.71 alpha:1.0];
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
            NSImage *arrow = isSelected ? [NSImage imageNamed:@"dark_menu_popover_arrow_selected"] : [NSImage imageNamed:@"dark_menu_popover_arrow_normal"];
            NSRect arrowRect = NSMakeRect(0, 0, 0, 0);
            arrowRect.size = arrow.size;
            arrowRect.origin.x = menuItemFrame.origin.x + menuItemFrame.size.width - 11;
            arrowRect.origin.y = menuItemFrame.origin.y + (menuItemHeight - arrow.size.height)/2;
            [arrow drawInRect:arrowRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        }
        
        // Draw Item Title
        NSDictionary *textAttributes = isSelected ? drawAlternate ? [self selectedItemAlternateTextAttributes]:[self selectedItemTextAttributes] : [self itemTextAttributes];
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
    if([self.menu _isClosing]) return;
    // check if on selected item && selected item not disabled
    // perform action, update selected item
    
    if(![[self.menu highlightedItem] hasSubmenu])
    {
        [self.menu closeMenu];
    }
}
- (void)mouseMoved:(NSEvent *)theEvent
{
    if([self.menu _isClosing]) return;
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if([self.menu _isClosing]) return;
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    if([self.menu _isClosing]) return;
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if([self.menu _isClosing]) return;
    // if not mouse on subwindow
    NSPoint loc = [theEvent locationInWindow];
    [self highlightItemAtPoint:[self convertPointFromBase:loc]];
}
#pragma mark -
- (void)keyDown:(NSEvent *)theEvent
{
    if([self.menu _isClosing]) return;
    
    NSMenuItem *currentItem = self.menu.highlightedItem;
    switch (theEvent.keyCode) 
    {
        case 126: // UP
            if(self.menu.highlightedItem)
            {
                NSInteger index = [self.menu.itemArray indexOfObject:self.menu.highlightedItem];
                if(index!=NSNotFound && index>0)
                {
                    self.menu.highlightedItem = [self.menu.itemArray objectAtIndex:index-1];
                }
            }
            else 
            {
                self.menu.highlightedItem = self.menu.itemArray.lastObject;
            }
            break;
        case 125: // DOWN
            if(self.menu.highlightedItem)
            {
                NSInteger index = [self.menu.itemArray indexOfObject:self.menu.highlightedItem];
                if(index!=NSNotFound && index < self.menu.itemArray.count-1)
                {
                    self.menu.highlightedItem = [self.menu.itemArray objectAtIndex:index+1];
                }
            } 
            else 
            {
                self.menu.highlightedItem = [self.menu.itemArray objectAtIndex:0];
            }
            break;
        case 123: // LEFT (exit submenu if any)
            break;
        case 124: // RIGHT (enter submenu if any)
            break;
        case 53: // ESC (close without changes)
            [self.menu closeMenuWithoutChanges:self];
            break;
        case 49: // SPACE ("click" selected item)
        case 36: // ENTER (same as space)
            [self.menu closeMenu];
            break;
        default:
            break;
    }
    
    // a little explanation for this:
    // selecting a separator item should not be possible, so if the selected item is a separator we try to jump over it
    // this will continue until either a normal item was selected or the last (or first depending on direction) item is reached
    // we then check if the selected item is still a separator and if so we select the item we started with
    // this ensures that a valid item will be selected after a key was pressed
    if((theEvent.keyCode == 126 || theEvent.keyCode==125) && self.menu.highlightedItem!=currentItem && [self.menu.highlightedItem isSeparatorItem])
    {
        [self keyDown:theEvent];
        if([self.menu.highlightedItem isSeparatorItem])
            self.menu.highlightedItem = currentItem;
    }
    [self setNeedsDisplay:YES];    
}
#pragma mark -
- (void)highlightItemAtPoint:(NSPoint)p
{
    NSMenuItem *highlighItem = [self itemAtPoint:p];
    
    if(highlighItem != self.menu.highlightedItem)
    {
        if([highlighItem isSeparatorItem])
        {
            highlighItem = nil;
        }
        
        self.menu.highlightedItem = highlighItem;
        
        [self setNeedsDisplay:YES];
    }
}

- (NSMenuItem*)itemAtPoint:(NSPoint)p
{
    if(p.x <= 5 || p.x >= [self bounds].size.width)
    {
        return nil;
    }
    if(p.y <= menuItemSpacingTop || p.y >= [self bounds].size.height-menuItemSpacingBottom)
    {
        return nil;
    }
    
    
    float y=menuItemSpacingTop;
    for(NSMenuItem *item in [self.menu itemArray])
    {
        if([item isSeparatorItem])
        {
            y += menuItemSeparatorHeight; continue;
        }
        y+= menuItemHeight;
        if(p.y < y && p.y > y-menuItemHeight)
            return item;
    }
    
    return nil;
}

- (NSRect)rectOfItem:(NSMenuItem*)m
{
    NSArray *itemArray = [self.menu itemArray];
    NSUInteger pos = [itemArray indexOfObject:m];
    
    float y = menuItemHeight*pos +menuItemSpacingTop;
    NSRect menuItemFrame = NSMakeRect(5, y, [self frame].size.width-5-5, menuItemHeight);
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
- (OEMenu*)menu
{
    return (OEMenu*)[self window];
}

@end
