//
//  OECollectionDebugWindowController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 2/18/13.
//
//

#import "OECollectionDebugWindowController.h"

NSString * const OEDebugCollectionView = @"OEDebugCollectionView";


@interface OECollectionDebugWindowController ()

@end

@implementation OECollectionDebugWindowController

+ (id)sharedController
{
    static OECollectionDebugWindowController *sharedController = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedController = [[OECollectionDebugWindowController alloc] initWithWindowNibName:@"OECollectionDebugWindowController"];
    });
    return sharedController;
}

- (NSString*)windowNibName
{
    return @"OECollectionDebugWindowController";
}

@end
