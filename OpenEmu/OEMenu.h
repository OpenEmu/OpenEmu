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

@import Cocoa;

@class OEPopUpButton;

#pragma mark -
#pragma mark Enumerations

typedef enum : NSUInteger
{
    OEMenuStyleDark,
    OEMenuStyleLight
} OEMenuStyle;

typedef enum : NSUInteger
{
    OENoEdge,
    OEMinYEdge,
    OEMaxYEdge,
    OEMinXEdge,
    OEMaxXEdge
} OERectEdge;

#pragma mark -
#pragma mark Menu option keys

extern NSString * const OEMenuOptionsStyleKey;           // Defines the menu style (dark or light), OEMenuStyle encapsulated in an -[NSNumber numberWithUnsignedInteger:]
extern NSString * const OEMenuOptionsArrowEdgeKey;       // Defines the edge that the arrow is on, OERectEdge encapsulated in an -[NSNumber numberWithUnsignedInteger:]
extern NSString * const OEMenuOptionsMaximumSizeKey;     // Maximum size of the menu, NSSize encapsulated in an NSValue
extern NSString * const OEMenuOptionsHighlightedItemKey; // Initial item to be highlighted, NSMenuItem
extern NSString * const OEMenuOptionsScreenRectKey;      // Reference screen rect to attach the menu to, NSRect encapsulated in an NSValue

#pragma mark -
#pragma mark Implementation

static inline NSRect OENSInsetRectWithEdgeInsets(NSRect rect, NSEdgeInsets insets)
{
    return NSMakeRect(NSMinX(rect) + insets.left, NSMinY(rect) + insets.bottom, NSWidth(rect) - insets.left - insets.right, NSHeight(rect) - insets.bottom - insets.top);
}

@class OEMenuView;
@class OEMenuDocumentView;

@interface OEMenu : NSWindow
{
@private
    BOOL _cancelTracking; // Event tracking loop canceled
    BOOL _closing;        // Menu is closing

    OEMenuView *_view;    // Menu's content view
    OEMenu     *_submenu; // Used to track opened submenu
}

+ (void)openMenuForPopUpButton:(OEPopUpButton *)button withEvent:(NSEvent *)event options:(NSDictionary *)options;
+ (void)openMenu:(NSMenu *)menu withEvent:(NSEvent *)event forView:(NSView *)view options:(NSDictionary *)options;
+ (NSSize)sizeOfMenu:(NSMenu *)menu forView:(NSView *)view options:(NSDictionary *)options;

- (void)cancelTracking;
- (void)cancelTrackingWithoutAnimation;

@property(nonatomic, readonly) OEMenuStyle style;                        // Menu's theme style
@property(nonatomic, readonly) OERectEdge  arrowEdge;                    // Edge that the arrow should appear on

@property(nonatomic, readonly, getter = isSubmenu) BOOL   submenu;       // Identifies if this menu represents a submenu
@property(nonatomic, readonly)                     NSSize intrinsicSize; // Natural, unrestricted size of the menu
@property(nonatomic, readonly)                     NSSize size;          // A confined representation of the menu's size, this ensures a menu is completely visible on the screen and does not extend beyond the maximum size specified

@property(nonatomic, assign) NSMenuItem *highlightedItem;                // Currently highlighted menu item (can be a primary or alternate menu item)

@end

@interface NSMenu (OEMenuAdditions)
@property (nonatomic, setter=setOEMenu:, assign) OEMenu *oeMenu;
@end
