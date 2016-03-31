//
//  OEGameLayerView.h
//  OpenEmu
//
//  Created by Alexander Strange on 4/19/16.
//
//

@import Cocoa;
#import "OEGameView.h"

@interface OEGameLayerView : NSView

@property (nonatomic, weak) id<OEGameViewDelegate> delegate;
@property (nonatomic) NSColor *backgroundColor;

- (void)setAspectSize:(OEIntSize)newAspectSize;
- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;

// TODO shouldn't be an instance method
- (NSSize)correctScreenSize:(OEIntSize)screenSize forAspectSize:(OEIntSize)aspectSize returnVertices:(BOOL)flag;

@end
