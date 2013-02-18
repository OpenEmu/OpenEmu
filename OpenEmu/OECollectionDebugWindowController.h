//
//  OECollectionDebugWindowController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 2/18/13.
//
//

#import <Cocoa/Cocoa.h>

extern NSString * const OEDebugCollectionView;

@interface OECollectionDebugWindowController : NSWindowController
+ (id)sharedController;
@property (strong) id representedObject;
@end
