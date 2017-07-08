//
//  OEGameLayerView.h
//  OpenEmu
//
//  Created by Alexander Strange on 4/19/16.
//
//

@import Cocoa;

extern NSString * const OEScreenshotAspectRatioCorrectionDisabled;
extern NSString * const OEDefaultVideoFilterKey;

// SPI USE: Stolen from Chrome.
typedef uint32_t CAContextID;

@class OEGameLayerView;
@class OEEvent;

@protocol OEGameViewDelegate <NSObject>
- (NSString *)systemIdentifier;
- (void)gameView:(OEGameLayerView *)gameView didReceiveMouseEvent:(OEEvent *)event;
@end

@interface OEGameLayerView : NSView

@property (nonatomic, weak) id<OEGameViewDelegate> delegate;
@property (nonatomic) NSColor *backgroundColor;
@property (nonatomic) CAContextID remoteContextID;

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize;

@end
