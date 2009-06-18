//
//  OEPluginDocument.m
//  OpenEmu
//
//  Created by Remy Demarest on 17/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEPluginDocument.h"
#import "OEPlugin.h"

NSString *const OENeedsRestartErrorDomain = @"OENeedsRestartErrorDomain";

@implementation OEPluginDocument

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to write your document to data of the specified type. If the given outError != NULL, ensure that you set *outError when returning nil.

    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.

    // For applications targeted for Panther or earlier systems, you should use the deprecated API -dataRepresentationOfType:. In this case you can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.

    return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    BOOL worked = YES;
    NSString *path = [absoluteURL path];
    Class type = [OEPlugin typeForExtension:[path pathExtension]];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSAllDomainsMask, YES);
    if([paths count] > 0)
    {
        NSString *newPath = [[[[paths objectAtIndex:0] stringByAppendingPathComponent:@"OpenEmu"] stringByAppendingPathComponent:[type pluginFolder]] stringByAppendingPathComponent:[path lastPathComponent]];
        NSFileManager *manager = [NSFileManager defaultManager];
        BOOL dir = NO;
        if([manager fileExistsAtPath:newPath isDirectory:&dir])
            worked = [manager removeItemAtPath:newPath error:outError];
        if(worked) worked = [manager moveItemAtPath:path toPath:newPath error:outError];
        
        if(worked)
        {
            worked = [OEPlugin pluginWithBundleAtPath:newPath type:type forceReload:YES] != nil;
            
            if(!worked)
                *outError = [NSError errorWithDomain:NSCocoaErrorDomain code:NSExecutableLoadError
                                            userInfo:
                             [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSString stringWithFormat:@"Couldn't load %@ plugin", path], NSLocalizedDescriptionKey,
                              @"A version of this plugin is already loaded", NSLocalizedFailureReasonErrorKey,
                              @"You need to restart the application to commit the change", NSLocalizedRecoverySuggestionErrorKey,
                              nil]];
        }
    }
    return worked;
}

@end
