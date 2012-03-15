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

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import "OEMenuItem.h"
@class OEPopupButton;
@protocol OEMenuDelegate;
@class OEMenuContentView;

typedef enum _OEMenuStyle {
    OEMenuStyleDark,
    OEMenuStyleLight
} OEMenuStyle;

typedef enum _OERectEdge
{
    OENoEdge,
    OEMinYEdge,
    OEMaxYEdge,
    OEMinXEdge,
    OEMaxXEdge
} OERectEdge;

@interface OEMenu : NSWindow 
{
@private
    NSMenu *menu;
    NSMenuItem *highlightedItem;
    
    OEMenu *submenu;
    OEPopupButton *popupButton;
    
    int itemsAboveScroller, itemsBelowScroller;
    
    id _localMonitor;
    BOOL visible;
    BOOL closing;
    
    OEMenuStyle style;
}

// If edge==OENoEdge and rect.size != {0,0} the menu is centered in the rect
// IF edee==OENoEdge and rect.size == {0,0} the top left corner of the menu is placed on rect.origin
- (void)openOnEdge:(OERectEdge)anedge ofRect:(NSRect)rect ofWindow:(NSWindow*)win;

- (void)closeMenuWithoutChanges:(id)sender;
- (void)closeMenu;

- (void)menuMouseDragged:(NSEvent *)theEvent;
- (void)menuMouseUp:(NSEvent*)theEvent;

- (void)updateSize;
#pragma mark - NSMenu wrapping
- (NSArray *)itemArray;

@property (readonly) BOOL alternate;

@property (readwrite)                    NSSize         minSize, maxSize;
@property (strong)                       OEPopupButton  *popupButton;
@property (nonatomic, strong)            OEMenu         *submenu;
@property (nonatomic, unsafe_unretained) OEMenu         *supermenu;

@property OEMenuStyle   style;
@property OERectEdge    openEdge;
@property BOOL          allowsOppositeEdge;
@property BOOL          displaysOpenEdge;

@property (nonatomic, strong)   NSMenu      *menu;
@property (strong)              NSMenuItem  *highlightedItem;
@property (readonly, getter=isVisible) BOOL visible;

@property int itemsAboveScroller, itemsBelowScroller;
@property (nonatomic, unsafe_unretained) id <OEMenuDelegate> delegate;

@property BOOL containsItemWithImage;
@end

@interface NSMenu (OEAdditions)
- (OEMenu*)convertToOEMenu;
@end

@protocol OEMenuDelegate <NSObject>
@optional
- (void)menuDidShow:(OEMenu*)men;
- (void)menuDidHide:(OEMenu*)men;
- (void)menuDidSelect:(OEMenu*)men;
- (void)menuDidCancel:(OEMenu*)men;
@end
