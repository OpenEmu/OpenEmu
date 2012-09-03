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
#import "OEROMImporter.h"

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
- (NSString*)localizedStatusMessage
{
    NSMutableString *message = [NSMutableString string];
    switch ([self importState]) {
        case OEImportItemStatusActive:
            [message appendString:[self localizedStepMessage]];
            break;
        case OEImportItemStatusIdle:
            [message appendString:@"Waiting"];
            break;
        case OEImportItemStatusFatalError:
            [message appendString:[[self error] localizedDescription]];
            break;
        case OEImportItemStatusFinished:
            if([[[self error] domain] isEqualTo:OEImportErrorDomainSuccess] && [[self error] code]==OEImportErrorCodeAlreadyInDatabase)
                [message appendString:@"Skipped (already in database)"];
            else
                [message appendString:@"Finished"];
            break;
        case OEImportItemStatusResolvableError:
            [message appendString:[[self error] localizedDescription]];
            break;
    }
    
    return message;
}

- (NSString*)localizedStepMessage
{
    NSMutableString *message = [NSMutableString string];

    switch ([self importStep]) {
        case OEImportStepCheckDirectory:
            [message appendString:@"Check Directory"];
            break;
        case OEImportStepCheckHash:
            [message appendString:@"Check Hash"];
            break;
        case OEImportStepCreateGame:
            [message appendString:@"Create Game"];
            break;
        case OEImportStepCreateRom:
            [message appendString:@"Create ROM"];
            break;
        case OEImportStepDetermineSystem:
            [message appendString:@"Determine System"];
            break;
        case OEImportStepHash:
            [message appendString:@"Calculating Hash"];
            break;
        case OEImportStepOrganize:
            [message appendString:@"Copying File"];
            break;
        case OEImportStepSyncArchive:
            [message appendString:@"Syncing with Archive.vg"];
            break;
            
        default:
            [message appendString:@"Unkown"];
            break;
    }
            return message;
}
@end
