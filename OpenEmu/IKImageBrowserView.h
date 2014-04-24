
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
@end
