//
//  OEImportItem.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 14.08.12.
//
//

#import "OEImportItem.h"
#import "OESystemPlugin.h"
#import "NSURL+OELibraryAdditions.h"
@implementation OEImportItem
+ (id)itemWithURL:(NSURL*)url andCompletionHandler:(OEImportItemCompletionBlock)handler
{
    id item = nil;
    
    NSDictionary *resourceValues = [url resourceValuesForKeys:[NSArray arrayWithObjects:NSURLIsPackageKey, NSURLIsHiddenKey, nil] error:nil];
    if([[resourceValues objectForKey:NSURLIsHiddenKey] boolValue] || [[resourceValues objectForKey:NSURLIsPackageKey] boolValue])
        return nil;    

    NSArray *validExtensions = [OESystemPlugin supportedTypeExtensions];
    NSString *extension = [[url pathExtension] lowercaseString];
    if([extension length] == 0 || [validExtensions containsObject:extension])
    {
        item = [[OEImportItem alloc] init];
        
        [item setURL:url];
        [item setCompletionHandler:handler];
        [item setImportState:OEImportItemStatusIdle];
        [item setImportInfo:[NSMutableDictionary dictionaryWithCapacity:5]];
    }

    return item;
}
@end
