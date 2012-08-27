//
//  OEImportItem.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 14.08.12.
//
//

#import "OEImportItem.h"

@implementation OEImportItem
+ (id)itemWithURL:(NSURL*)url andCompletionHandler:(OEImportItemCompletionBlock)handler
{
    id item = [[OEImportItem alloc] init];

    // TODO: make sure url has a valid suffix
    
    [item setUrl:url];
    [item setCompletionHandler:handler];
    [item setImportState:OEImportItemStatusIdle];
    [item setImportInfo:[NSMutableDictionary dictionaryWithCapacity:5]];
    
    return item;
}
@end
