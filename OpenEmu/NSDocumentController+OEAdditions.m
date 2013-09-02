//
//  NSDocumentController+OEAdditions.m
//  OpenEmu
//
//  Created by Remy Demarest on 01/09/2013.
//
//

#import "NSDocumentController+OEAdditions.h"

#import "OEGameDocument.h"

@implementation NSDocumentController (OEAdditions)

- (OEGameDocument *)currentGameDocument;
{
    OEGameDocument *document = [self currentDocument];
    if([document isKindOfClass:[OEGameDocument class]])
        return document;

    for(OEGameDocument *document in [NSApp orderedDocuments])
        if([document isKindOfClass:[OEGameDocument class]])
            return document;

    return nil;
}

- (void)openGameDocumentWithGame:(OEDBGame *)game display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    [self doesNotImplementSelector:_cmd];
}

- (void)openGameDocumentWithRom:(OEDBRom *)rom display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    [self doesNotImplementSelector:_cmd];
}

- (void)openGameDocumentWithSaveState:(OEDBSaveState *)state display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    [self doesNotImplementSelector:_cmd];
}

@end
