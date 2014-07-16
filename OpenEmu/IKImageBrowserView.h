
#import <Foundation/Foundation.h>

#import "IKImageBrowserLayoutManager.h"
#import "IKImageWrapper.h"
#import "IKRenderer.h"

@interface IKImageBrowserView (ApplePrivate)
// -handleKeyInput:character: is called to allow space in type select
- (BOOL)handleKeyInput:(id)arg1 character:(unsigned short)arg2;

// -allowsTypeSelect is undocumented as of 10.9, original implementation seems to return [self cellsStyleMask]&IKCellsStyleTitled
- (BOOL)allowsTypeSelect;

// -thumbnailImageAtIndex: is used to generate drag image
- (id)snapshotOfItemAtIndex:(unsigned long long)arg1;
- (IKImageWrapper*)thumbnailImageAtIndex:(unsigned long long)arg1;

- (void)drawDragBackground;

// - (void)drawGroupsOverlays overridden to draw custom highlight
- (void)drawDragOverlays;

// - (void)drawGroupsOverlays overridden to draw grid view gradients
- (void)drawGroupsOverlays;

// - (void)drawGroupsOverlays overridden to draw noise and lightning
- (void)drawBackground:(struct CGRect)arg1;

- (id <IKRenderer>)renderer;

// -layoutManager is used to customize layout (general margin, alignment of single rows, etc.)
- (IKImageBrowserLayoutManager*)layoutManager;

// -gridGroupFromDictionary:(id)arg1
- (id)gridGroupFromDictionary:(id)arg1;

// -drawGroupsBackground
- (void)drawGroupsBackground;

// Group rects
- (struct CGRect)rectOfFloatingGroupHeader:(id)arg1;
- (struct CGRect)_rectOfGroup:(id)arg1;
- (struct CGRect)_rectOfGroupTail:(id)arg1;
- (struct CGRect)_rectOfGroupHeader:(id)arg1;

- (void)startSelectionProcess:(NSPoint)arg1;
- (void)endSelectionProcess:(NSPoint)arg1;
- (void)updateSelectionProcess:(NSPoint)arg1;

- (unsigned long long)groupIndexAtViewLocation:(struct CGPoint)arg1 clickableArea:(BOOL)arg2;
- (id)groupAtViewLocation:(struct CGPoint)arg1 clickableArea:(BOOL)arg2;

- (void)reloadCellDataAtIndex:(unsigned long long)arg1;
@end
