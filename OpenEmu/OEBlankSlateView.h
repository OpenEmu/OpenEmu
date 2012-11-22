//
//  OEBlankSlateView.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 21.11.12.
//
//

#import <Cocoa/Cocoa.h>
#import "OESystemPlugin.h"

@class OEBlankSlateView;

@protocol OEBlankSlateViewDelegate <NSObject>
@optional
- (NSDragOperation)blankSlateView:(OEBlankSlateView *)gridView validateDrop:(id<NSDraggingInfo>)sender;
- (NSDragOperation)blankSlateView:(OEBlankSlateView *)gridView draggingUpdated:(id<NSDraggingInfo>)sender;
- (BOOL)blankSlateView:(OEBlankSlateView *)gridView acceptDrop:(id<NSDraggingInfo>)sender;
@end

@interface OEBlankSlateView : NSView

@property (nonatomic) NSString* representedCollectionName;
@property (nonatomic) OESystemPlugin* representedSystemPlugin;

@property (assign) id <OEBlankSlateViewDelegate> delegate;
@end
