//
//  OEArrayController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 23/12/13.
//
//

#import <Cocoa/Cocoa.h>

@interface OEArrayController : NSArrayController
@property NSInteger limit;
@property NSArray *fetchSortDescriptors;
@end
