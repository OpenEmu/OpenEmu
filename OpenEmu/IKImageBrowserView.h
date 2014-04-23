//
//  IKImageBrowserView.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 23/04/14.
//
//
#import <Foundation/Foundation.h>

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
@end
