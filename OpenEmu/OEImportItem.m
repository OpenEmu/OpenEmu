/*
 Copyright (c) 2012, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEImportItem.h"
#import "OESystemPlugin.h"
#import "NSURL+OELibraryAdditions.h"
#import "OEROMImporter.h"

@implementation OEImportItem
+ (id)itemWithURL:(NSURL*)url andCompletionHandler:(OEImportItemCompletionBlock)handler
{
    id item = nil;
    
    NSDictionary *resourceValues = [url resourceValuesForKeys:[NSArray arrayWithObjects:NSURLIsPackageKey, NSURLIsHiddenKey, nil] error:nil];
    DLog(@"%d | %d", [[resourceValues objectForKey:NSURLIsHiddenKey] boolValue], [[resourceValues objectForKey:NSURLIsPackageKey] boolValue]);
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
        case OEImportItemStatusCanceld:
            [message appendString:@"Canceld"];
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
