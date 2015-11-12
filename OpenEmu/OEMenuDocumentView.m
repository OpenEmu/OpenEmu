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

#import "OEMenuDocumentView.h"
#import "OEMenuDocumentView+OEMenuView.h"
#import "OEMenu.h"
#import "OEMenuView.h"
#import "OEMenu+OEMenuViewAdditions.h"
#import "NSMenuItem+OEMenuItemExtraDataAdditions.h"
#import "OETheme.h"
#import "OEThemeImage.h"
#import "OEThemeGradient.h"
#import "OEThemeTextAttributes.h"
#import "OEMenuItemExtraData.h"

#pragma mark -
#pragma mark Menu Item Spacing

const CGFloat        OEMenuItemTickMarkWidth      = 19.0; // Value is extern, used by OEMenu.m to calculate popup button menu placement
const CGFloat        OEMenuItemImageWidth         = 22.0; // Value is extern, used by OEMenu.m to calculate popup button menu placement
static const CGFloat OEMenuItemSubmenuArrowWidth  = 10.0;
static const CGFloat OEMenuItemHeightWithImage    = 20.0;
static const CGFloat OEMenuItemHeightWithoutImage = 17.0;
static const CGFloat OEMenuItemSeparatorHeight    =  7.0;
static const CGFloat OEMenuItemSeparatorOffset    =  3.0; // Shorthand for ceil(OEMenuItemSeparatorHeight / 2.0)

#pragma mark -
#pragma mark Menu Item Insets

const NSEdgeInsets OEMenuItemInsets = { 0.0, 5.0, 0.0, 5.0 }; // Value is extern, used by OEMenu.m to calculate menu placement

#pragma mark -
#pragma mark Menu Item Default Mask

static const OEThemeState OEMenuItemStateMask = OEThemeStateDefault & ~OEThemeStateAnyWindowActivity & ~OEThemeStateAnyMouse;

#pragma mark -

@implementation OEMenuDocumentView
@synthesize containImages = _containImages;
@synthesize itemArray = _itemArray;
@synthesize style = _style;

- (id)initWithFrame:(NSRect)frame
{
    if ((self = [super initWithFrame:frame]))
    {
        [self OE_updateTheme];
    }

    return self;
}

- (void)dealloc
{
    // -setItemArray: is responsible for attaching an instance of OEMenuItemExtraData when the menu is first set, following operation makes sure we clear out that extra data
    [self setItemArray:nil];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self OE_layoutIfNeeded];

    const NSUInteger count = [_itemArray count];
    if(count == 0) return;

    // Constrain the clipped path to that of the dirtyRect and the OEMenuInlineView's clippingRect
    NSView *menuView = [[self enclosingScrollView] superview];
    dirtyRect        = NSIntersectionRect(dirtyRect, [menuView convertRect:[(OEMenuView *)menuView clippingRect] toView:self]);

    // If there is nothing to draw then return
    if(NSIsEmptyRect(dirtyRect)) return;
    NSRectClip(dirtyRect);

    // Setup positioning frames
    NSRect tickMarkFrame = NSMakeRect(0.0, 0.0, NSWidth([self bounds]), (_containImages ? OEMenuItemHeightWithImage : OEMenuItemHeightWithoutImage));
    NSRect imageFrame;
    NSRect textFrame;
    NSRect submenuArrowFrame;

    NSDivideRect(tickMarkFrame, &tickMarkFrame,     &imageFrame, OEMenuItemTickMarkWidth,                     NSMinXEdge);
    NSDivideRect(imageFrame,    &imageFrame,        &textFrame,  (_containImages ? OEMenuItemImageWidth : 0), NSMinXEdge);
    NSDivideRect(textFrame,     &submenuArrowFrame, &textFrame,  OEMenuItemSubmenuArrowWidth,                 NSMaxXEdge);

    // Cache for future use
    const NSSize separatorSize = [_separatorImage size];

    // Render the list of items
    for(NSUInteger i = 0; i < count; i++)
    {
        // Figure out if an alternate item should be rendered
        NSMenuItem          *item          = [_itemArray objectAtIndex:i];
        OEMenuItemExtraData *extraData     = [item extraData];
        NSRect               menuItemFrame = [extraData frame];
        
        // Skip over any alternate items
        item  = [extraData itemWithModifierMask:_lastKeyModifierMask];
        i    += [[extraData alternateItems] count];

        // Continue to the next iteration if the menu item is hidden or it's frame is above the clipping rect
        if([item isHidden] || (NSMinY(menuItemFrame) > NSMaxY(dirtyRect))) continue;

        // Break the loop if the menu item's frame is beyond the clipping rect
        if(NSMaxY(menuItemFrame) < NSMinY(dirtyRect)) break;

        // This item should be within the clipping rect, lets draw it
        if([item isSeparatorItem])
        {
            menuItemFrame.origin.y    = NSMaxY(menuItemFrame) - OEMenuItemSeparatorOffset;
            menuItemFrame.size.height = separatorSize.height;
            [_separatorImage drawInRect:menuItemFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        }
        else
        {
            // Retrieve theme based attributes
            OEThemeState  menuItemState     = [self OE_currentStateFromMenuItem:item];
            NSDictionary *textAttributes    = [self OE_textAttributesForState:menuItemState];
            NSImage      *tickMarkImage     = [_tickImage imageForState:menuItemState];
            NSImage      *submenuArrowImage = [_submenuArrowImage imageForState:menuItemState];

            // Retrieve the static attributes
            NSImage  *menuItemImage = [item image];
            NSString *title         = [item title];

            // Lets put it all together
            [[_backgroundGradient gradientForState:menuItemState] drawInRect:menuItemFrame];

            if(tickMarkImage)
            {
                // TODO: We should scale the tick mark if it's too wide
                NSRect tickMarkRect   = { .size = [tickMarkImage size] };
                tickMarkRect.origin.x = tickMarkFrame.origin.x + ((NSWidth(tickMarkFrame) - NSWidth(tickMarkRect)) / 2.0);
                tickMarkRect.origin.x += [item indentationLevel]*OEMenuItemIndentation;
                tickMarkRect.origin.y = menuItemFrame.origin.y + ((NSHeight(tickMarkFrame) - NSHeight(tickMarkRect)) / 2.0);

                [tickMarkImage drawInRect:NSIntegralRect(tickMarkRect) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
            }

            if(menuItemImage)
            {
                // TODO: We should scale the item's image if it's too wide
                NSRect imageRect   = { .size = [menuItemImage size] };
                imageRect.origin.x = imageFrame.origin.x + 2.0;
                imageRect.origin.x += [item indentationLevel]*OEMenuItemIndentation;

                imageRect.origin.y = menuItemFrame.origin.y + ((NSHeight(imageFrame) - NSHeight(imageRect)) / 2.0);

                [menuItemImage drawInRect:NSIntegralRect(imageRect) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
            }

            if([item hasSubmenu] && submenuArrowImage)
            {
                // TODO: We should scale the submenuArrowImage image if it's too wide
                NSRect arrowRect   = { .size = [submenuArrowImage size] };
                arrowRect.origin.x = submenuArrowFrame.origin.x;

                arrowRect.origin.y = menuItemFrame.origin.y + ((NSHeight(submenuArrowFrame) - NSHeight(arrowRect)) / 2.0);

                [submenuArrowImage drawInRect:NSIntegralRect(arrowRect) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
            }

            NSRect textRect   = { .size = [title sizeWithAttributes:textAttributes] };
            textRect.origin.x = textFrame.origin.x;
            textRect.origin.x += [item indentationLevel]*OEMenuItemIndentation;
            textRect.origin.y = menuItemFrame.origin.y + ((NSHeight(textFrame) - NSHeight(textRect)) / 2.0);
            [title drawInRect:textRect withAttributes:textAttributes];
        }
    }
}

- (NSMenu *)menuForEvent:(NSEvent *)event
{
    // This call prevents the right click / context menu from appearing
    return nil;
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    // Forward the event only if we are not in the middle of closing the menu
    OEMenu *menu = (OEMenu *)[self window];
    if([menu OE_closing]) return;
    [super scrollWheel:theEvent];
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    // Figure out if any of the modifier flags that we are interested have changed
    NSUInteger modifierFlags = [theEvent modifierFlags] & _keyModifierMask;
    if(_lastKeyModifierMask != modifierFlags)
    {
        // A redraw will change the menu items, we should probably just redraw the items that need to be redrawn -- but figuring this out may be more expensive than what it is worth
        _lastKeyModifierMask = modifierFlags;
        [self setNeedsDisplay:YES];
    }
}

// Returns an OEThemeState with the window and mouse activity excluded (see OEMenuItemStateMask)
- (OEThemeState)OE_currentStateFromMenuItem:(NSMenuItem *)item
{
    return [OEThemeObject themeStateWithWindowActive:NO buttonState:[item state] selected:([(OEMenu *)[self window] highlightedItem] == item) enabled:[item isEnabled] focused:YES houseHover:NO modifierMask:[item isAlternate]?[NSEvent modifierFlags]:0] & OEMenuItemStateMask;
}

// Returns an attributes dictionary (for NSAttributedString or -[NSString drawInRect:withAttributes:]) based on the specified state
- (NSDictionary *)OE_textAttributesForState:(OEThemeState)state
{
    if(!_textAttributes) return nil;

    static NSParagraphStyle *defaultParagraphStyle = nil;
    static dispatch_once_t   onceToken;

    dispatch_once(&onceToken, ^{
        // Establish our own default paragraph style that tuncates the tail of the text rendered
        NSMutableParagraphStyle *ps = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [ps setLineBreakMode:NSLineBreakByTruncatingTail];
        defaultParagraphStyle = [ps copy];
    });

    // Implicitly set the paragraph style if it's not explicitly set
    NSDictionary *attributes = [_textAttributes textAttributesForState:state];
    if(![attributes objectForKey:NSParagraphStyleAttributeName])
    {
        NSMutableDictionary *newAttributes = [attributes mutableCopy];
        [newAttributes setValue:defaultParagraphStyle forKey:NSParagraphStyleAttributeName];
        attributes = [newAttributes copy];
    }

    return attributes;
}

// Invalidates the menu items' frames
- (void)OE_setNeedsLayout
{
    _needsLayout = YES;
    [self setNeedsDisplay:YES];
}

// Updates the frames for the menu items
- (void)OE_layout
{
    _needsLayout = NO;

    if([_itemArray count] == 0) return;

    const NSRect   bounds      = [self bounds];
    const NSRect   contentRect = OENSInsetRectWithEdgeInsets(bounds, OEMenuItemInsets);
    NSDictionary  *attributes  = [_textAttributes textAttributesForState:OEThemeStateDefault];
    const CGFloat  itemHeight  = _containImages ? OEMenuItemHeightWithImage : OEMenuItemHeightWithoutImage;

    __block CGFloat y     = 0.0;
    __block CGFloat width = 0.0;

    [_itemArray enumerateObjectsUsingBlock:
     ^(NSMenuItem *item, NSUInteger idx, BOOL *stop)
     {
         if(![item isHidden])
         {
             OEMenuItemExtraData *extraData = [item extraData];
             if([extraData primaryItem])
             {
                 // Set frame to primary item's frame if this item represents a valid alternate item
                 [extraData setFrame:[[[extraData primaryItem] extraData] frame]];
             }
             else
             {
                 const CGFloat height = ([item isSeparatorItem] ? OEMenuItemSeparatorHeight : itemHeight);
                 [extraData setFrame:NSMakeRect(NSMinX(bounds), NSMaxY(contentRect) - y - height, NSWidth(bounds), height)];

                 y     += height;
                 width  = MAX(width, [[item title] sizeWithAttributes:attributes].width);
             }
         }
     }];

    const CGFloat minimumWidthPadding  = OEMenuItemTickMarkWidth + (_containImages ? OEMenuItemImageWidth : 0) + OEMenuItemSubmenuArrowWidth + OEMenuItemInsets.left + OEMenuItemInsets.right;
    const CGFloat minimumHeightPadding = OEMenuItemInsets.top + OEMenuItemInsets.bottom;

    _intrinsicSize = NSMakeSize(ceil(width + minimumWidthPadding), ceil(y + minimumHeightPadding));
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    [self OE_setNeedsLayout];
}

- (void)setItemArray:(NSArray *)itemArray
{
    if(_itemArray != itemArray)
    {
        // Remove extra data from the old menu items
        [_itemArray makeObjectsPerformSelector:@selector(setExtraData:) withObject:nil];
        _itemArray = itemArray;

        // Identify the alternate and primary items for each menu item
        __block NSMenuItem *lastValidItem   = nil;
        __block NSUInteger  keyModifierMask = 0;
        __block BOOL        containsImages  = NO;

        [_itemArray enumerateObjectsUsingBlock:
         ^ (NSMenuItem *item, NSUInteger idx, BOOL *stop)
         {
             if(![item isHidden])
             {
                 keyModifierMask |= [item keyEquivalentModifierMask];
                 containsImages   = containsImages || ([item image] != nil);
                 if([item isAlternate] && [[lastValidItem keyEquivalent] isEqualToString:[item keyEquivalent]]) [[lastValidItem extraData] addAlternateItem:item];
                 else                                                                                           lastValidItem = item;
             }
         }];

        _keyModifierMask = keyModifierMask;
        _containImages   = containsImages;
        
        // update the lastKeyModifierMask in case keyModifierMask changed
        _lastKeyModifierMask = [[NSApp currentEvent] modifierFlags] & _keyModifierMask;

        [self OE_setNeedsLayout];
        [self setFrameSize:[self intrinsicSize]];
    }
}

- (void)setStyle:(OEMenuStyle)style
{
    if(_style != style)
    {
        _style = style;
        [self OE_updateTheme];
    }
}

- (NSSize)intrinsicSize
{
    [self OE_layoutIfNeeded];
    return _intrinsicSize;
}

- (void)OE_updateTheme
{
    NSString *styleKeyPrefix = (_style == OEMenuStyleDark ? @"dark_menu_" : @"light_menu_");

    _separatorImage     = [[OETheme sharedTheme] imageForKey:[styleKeyPrefix stringByAppendingString:@"separator_item"] forState:OEThemeStateDefault];
    _backgroundGradient = [[OETheme sharedTheme] themeGradientForKey:[styleKeyPrefix stringByAppendingString:@"item_background"]];
    _tickImage          = [[OETheme sharedTheme] themeImageForKey:[styleKeyPrefix stringByAppendingString:@"item_tick"]];
    _textAttributes     = [[OETheme sharedTheme] themeTextAttributesForKey:[styleKeyPrefix stringByAppendingString:@"item"]];
    _submenuArrowImage  = [[OETheme sharedTheme] themeImageForKey:[styleKeyPrefix stringByAppendingString:@"submenu_arrow"]];

    [self setNeedsDisplay:YES];
}

@end

@implementation OEMenuDocumentView (OEMenuView)

- (void)OE_layoutIfNeeded
{
    if(!_needsLayout) return;
    [self OE_layout];
}

- (NSMenuItem *)OE_itemAtPoint:(NSPoint)point
{
    for(NSMenuItem *item in _itemArray)
    {
        if(NSPointInRect(point, [[item extraData] frame]))
        {
            if([item isSeparatorItem]) return nil;

            NSMenuItem *alternateItem = [[item extraData] itemWithModifierMask:_lastKeyModifierMask];
            if(alternateItem) return alternateItem;

            return ([[item extraData] primaryItem] ?: item);
        }
    }

    return nil;
}

@end
