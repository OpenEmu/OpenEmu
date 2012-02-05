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

#import "OENewMenu.h"
#import "OEPopupButton.h"
#import "NSImage+OEDrawingAdditions.h"
@interface OENewMenu (Private)
- (void)layoutAndReload;

- (void)_performCloseMenu;
- (void)_closeByClickingItem:(NSMenuItem*)selectedItem;

- (void)_setupWindow;

- (BOOL)_isClosing;
@end
@implementation OENewMenu
#pragma mark -
#pragma mark Sizing
#define menuItemHeightNoImage 17
#define menuItemHeightImage 20
#define menuItemSeparatorHeight 7

#pragma mark -
#pragma mark Life Cycle
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OENewMenu class])
        return;

    NSImage *menuArrows = [NSImage imageNamed:@"dark_menu_popover_arrow"];
    [menuArrows setName:@"dark_menu_popover_arrow_normal" forSubimageInRect:NSMakeRect(0, menuArrows.size.height/2, menuArrows.size.width, menuArrows.size.height/2)];
    [menuArrows setName:@"dark_menu_popover_arrow_selected" forSubimageInRect:NSMakeRect(0, 0, menuArrows.size.width, menuArrows.size.height/2)];
    
    NSImage *scrollArrows = [NSImage imageNamed:@"dark_menu_scroll_arrows"];
    [scrollArrows setName:@"dark_menu_scroll_up" forSubimageInRect:NSMakeRect(0, 0, 9, 15)];
    [scrollArrows setName:@"dark_menu_scroll_down" forSubimageInRect:NSMakeRect(0, 15, 9, 15)];
    
    if(![NSImage imageNamed:@"hud_window_active"])
    {
        NSImage *hudWindow = [NSImage imageNamed:@"hud_window"];
        
        [hudWindow setName:@"hud_window_active" forSubimageInRect:NSMakeRect(0, 0, hudWindow.size.width/2, hudWindow.size.height)];
        [hudWindow setName:@"hud_window_inactive" forSubimageInRect:NSMakeRect(hudWindow.size.width/2, 0, hudWindow.size.width/2, hudWindow.size.height)];
    }
    
    [NSWindow registerWindowClassForCustomThemeFrameDrawing:[self class]];   
}


- (id)init {
    self = [super init];
    if (self) {
        NSLog(@"OENewMenu init");
        
        [self _setupWindow];
        
        [self addObserver:self forKeyPath:@"itemsAboveScroller" options:0 context:nil];
        [self addObserver:self forKeyPath:@"itemsBelowScroller" options:0 context:nil];
    }
    return self;
}

- (void)dealloc {
    NSLog(@"OENewMenu dealloc");
    
    [self removeObserver:self forKeyPath:@"itemsAboveScroller"];
    [self removeObserver:self forKeyPath:@"itemsBelowScroller"];
    
    [upperTableView release];
    upperTableView = nil;
    
    [mainTableView release];
    mainTableView = nil;
    
    [lowerTableView release];
    lowerTableView = nil;
    
    if(_localMonitor)
    {
        [_localMonitor release];
        _localMonitor = nil;
    }
    
    [super dealloc];
}
#pragma mark -
- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow*)win
{
    visible = YES;
    closing = NO;
    [self setAlernate:NO];
    [self setAlphaValue:1.0];
    
    NSAssert(_localMonitor == nil, @"_localMonitor still exists somehow");
    _localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDownMask | NSRightMouseDownMask | NSOtherMouseDownMask | NSKeyDownMask | NSFlagsChangedMask handler:^(NSEvent *incomingEvent) 
                     {
                         OEMenuView *view = [[[self contentView] subviews] lastObject];
                         
                         if([incomingEvent type] == NSFlagsChanged){
                             [self setAlernate:([incomingEvent modifierFlags] & NSAlternateKeyMask) != 0];
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
                         else 
                         {
                             // event is outside of window, close menu without changes and remove event
                             [self closeMenuWithoutChanges:nil];
                         }
                         return (NSEvent *)nil;
                     }];
    [_localMonitor retain];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(closeMenuWithoutChanges:) name:NSApplicationWillResignActiveNotification object:NSApp];
    
    [self setFrameOrigin:p];
    [win addChildWindow:self ordered:NSWindowAbove];
    
    [self layoutAndReload];
    
    NSPoint windowP = [self convertScreenToBase:[NSEvent mouseLocation]];
    [self highlightItemAtPoint:windowP];
    if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidShow:)]) [self.delegate performSelector:@selector(menuDidShow:) withObject:self];
}

- (void)closeMenuWithoutChanges:(id)sender
{
    closing = YES;
    // make sure the menu does not vanish while being closed
    [self retain];
    
    if([self activeSubmenu]) [[self activeSubmenu] closeMenuWithoutChanges:sender];
    
    [self _performCloseMenu];
    
    if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidCancel:)]) [self.delegate performSelector:@selector(menuDidCancel:) withObject:self];
    
    // now we are ready to be deallocated if needed
    [self release];
}

- (void)closeMenu
{   
    closing = YES;
    
    OENewMenu *superMen = self;
    while([self supermenu])
    {
        superMen = superMen.supermenu;
    }
    
    if(superMen != self)
    {
        [superMen closeMenu];
        return;
    }
    
    OENewMenu *subMen = self;
    while([subMen activeSubmenu])
    {
        subMen = [subMen activeSubmenu];
    }
    
    NSMenuItem *selectedItem = subMen.highlightedItem;
    [self _closeByClickingItem:selectedItem];
}

#pragma mark -
- (void)highlightItemAtPoint:(NSPoint)p
{
    NSMenuItem *highlighItem = [self itemAtPoint:p];
    
    if(highlighItem != [self highlightedItem])
    {
        if([highlighItem isSeparatorItem])
        {
            highlighItem = nil;
        }
        
        [self setHighlightedItem:highlighItem];
    }
}

- (id)itemAtPoint:(NSPoint)p
{
    NSInteger row = -1;
    NSTableView *tableView = nil;
    
    if(row != -1) 
    {
        row = [upperTableView rowAtPoint:p];
        tableView = upperTableView;
    }
    if(row != -1) 
    {
        row = [lowerTableView rowAtPoint:p];
        tableView = lowerTableView;
    }
    
    if(row != -1) 
    {
        row = [upperTableView rowAtPoint:p];
        tableView = upperTableView;
    }
    
    if(row == -1)
        return nil;
    
    return [self tableView:tableView objectValueForTableColumn:nil row:row];    
}
@synthesize highlightedItem, alernate;
#pragma mark -
@synthesize itemsAboveScroller, itemsBelowScroller;
@synthesize menu, supermenu, activeSubmenu;
@synthesize delegate;
@synthesize popupButton;

- (BOOL)isSubmenu
{
    return [self supermenu]!=nil;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self layoutAndReload];
}

- (void)layoutAndReload
{
    [upperTableView reloadData];
    NSRect  upperRect = [upperTableView rectOfColumn:0];
    [upperTableView setFrame:upperRect];
    
    [lowerTableView reloadData];
    NSRect lowerRect = [lowerTableView rectOfColumn:0];
    [lowerTableView setFrame:lowerRect];
    
    [mainTableView reloadData];
    NSRect mainRect = [mainTableView rectOfColumn:0];
    [mainTableView setFrame:mainRect];
    
    NSSize size;
    size.height = upperRect.size.height+lowerRect.size.height+mainRect.size.height;
    size.width = upperRect.size.width;
    if(size.width < lowerRect.size.width) size.width = lowerRect.size.width;
    if(size.width < mainRect.size.width) size.width = mainRect.size.width;
    
    NSRect rect;
    rect.origin = [self frame].origin;
    rect.size = size;
    
    rect = NSInsetRect(rect, 5, 5);
    [self setFrame:rect display:YES];
}
- (NSArray *)itemArray
{
    return [[self menu] itemArray]; 
}

#pragma mark -
#pragma mark Animations
#define flickerDelay 0.09
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
    
    [theAnimation setDelegate:nil];
}

#pragma mark -
#pragma mark Private Methods
- (void)_performCloseMenu
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
    if([self activeSubmenu]) [[self activeSubmenu] closeMenuWithoutChanges:nil];
    
    [NSTimer scheduledTimerWithTimeInterval:flickerDelay target:self selector:@selector(_closeTimer:) userInfo:selectedItem repeats:NO];
    self.highlightedItem = nil;
    [self display];
}

- (void)_setupWindow
{
    [self setStyleMask:NSBorderlessWindowMask];
    
    NSTableColumn *column;
    
    upperTableView = [[NSTableView alloc] init];
    [upperTableView setDelegate:self];
    [upperTableView setDataSource:self];
    [upperTableView setHeaderView:nil];
    column = [[NSTableColumn alloc] initWithIdentifier:@"mainColumn"];
    [upperTableView addTableColumn:column];
    [column release];
    [[self contentView] addSubview:upperTableView];
    
    mainTableView = [[NSTableView alloc] init];
    [mainTableView setDelegate:self];
    [mainTableView setDataSource:self];
    [mainTableView setHeaderView:nil];
    column = [[NSTableColumn alloc] initWithIdentifier:@"mainColumn"];
    [mainTableView addTableColumn:column];
    [column release];
    [[self contentView] addSubview:mainTableView];
    
    lowerTableView = [[NSTableView alloc] init];
    [lowerTableView setDelegate:self];
    [lowerTableView setDataSource:self];
    [lowerTableView setHeaderView:nil];
    column = [[NSTableColumn alloc] initWithIdentifier:@"mainColumn"];
    [lowerTableView addTableColumn:column];
    [column release];
    [[self contentView] addSubview:lowerTableView];
    
    CAAnimation *anim = [self alphaValueAnimation];
    [anim setDuration:0.075];
    [self setAplhaValueAnimation:anim];
    
    [self setLevel:NSTornOffMenuWindowLevel];
    [self setHasShadow:NO];
    [self setBackgroundColor:[NSColor clearColor]];
    [self setOpaque:NO];
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
- (BOOL)_isClosing
{
    return closing;
}
#pragma mark -
#pragma mark NSWindow Overrides
- (BOOL)canBecomeKeyWindow
{
    return NO;
}
#pragma mark -
#pragma mark OECustomWindow Implementation
- (BOOL)drawsAboveDefaultThemeFrame
{
    return YES;
}

- (void)drawThemeFrame:(NSValue *)dirtyRectValue
{
    NSRect bounds = [(NSView*)self bounds];
    [[NSColor clearColor] setFill];
    NSRectFill(bounds);
    
    NSColor *startColor = [NSColor colorWithDeviceWhite:0.91 alpha:0.10];
    NSColor *endColor = [startColor colorWithAlphaComponent:0.0];
    NSGradient *backgroundGradient = [[NSGradient alloc] initWithStartingColor:startColor endingColor:endColor];
    
    NSRect backgroundRect = NSInsetRect(bounds, 4, 4);
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:backgroundRect xRadius:3 yRadius:3];
    
    // Draw Background color
    NSColor *backgroundColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.8];
    NSGradient *grad = [[NSGradient alloc] initWithStartingColor:backgroundColor endingColor:backgroundColor];
    [grad drawInBezierPath:path angle:90];
    [grad release];
    
    // draw gradient above
    [backgroundGradient drawInBezierPath:path angle:90];
    [backgroundGradient release];
    
    // draw background border
    NSImage *img = [self.menu supermenu]==nil ? [NSImage imageNamed:@"dark_menu_body"] : [NSImage imageNamed:@"dark_submenu_body"];
    [img drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:17 rightBorder:17 topBorder:19 bottomBorder:19];
}

#pragma mark -
#pragma mark NSTableViewDelegate Implementation
- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return NO;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    OEMenuItem *item = [self tableView:tableView objectValueForTableColumn:nil row:row];
    if([item isSeparatorItem])
        return menuItemSeparatorHeight;
    if([item image]!=nil)
        return menuItemHeightImage;
    
    return menuItemHeightNoImage;
}

#pragma mark -
#pragma mark NSTableViewDataSource Implementation
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    NSInteger allItemCount = [[self itemArray] count];
    if (tableView == lowerTableView)
        return [self itemsBelowScroller]<=allItemCount?[self itemsBelowScroller]:allItemCount;
    else if(tableView == upperTableView)
    {
        NSInteger availableItems = allItemCount-[self itemsBelowScroller];
        return [self itemsAboveScroller]<=availableItems?[self itemsAboveScroller]:availableItems;
    }
    else if(tableView == mainTableView)
    {
        NSInteger availableItems = allItemCount-[self itemsAboveScroller]-[self itemsBelowScroller];
        return availableItems<0?0:availableItems;
    }
    return 0;
}
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSLog(@"Reload for %@ | %@ | %ld", tableView==lowerTableView?@"lowerTableView":(tableView==upperTableView?@"upperTableView":@"mainTableView"), tableView, [self numberOfRowsInTableView:tableView]);
    
    BOOL returnTitle = (tableColumn != nil);
    OEMenuItem *item = nil;
    if(tableView==lowerTableView)
        item = [[self itemArray] objectAtIndex:row];
    else if(tableView == mainTableView)
        item = [[self itemArray] objectAtIndex:row+[self itemsBelowScroller]];
    else if(tableView == upperTableView)
        item = [[self itemArray] objectAtIndex:[[self itemArray] count]-[self itemsAboveScroller]];
    
    return returnTitle?[item title]:item;
}
@end
@implementation NSMenu (OEOENewMenuAdditions)
- (OENewMenu*)convertToOENewMenu
{
    OENewMenu *menu = [[OENewMenu alloc] init];
    menu.menu = self;
    return [menu autorelease];
}
@end