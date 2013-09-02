//
//  NSDocumentController+OEAdditions.h
//  OpenEmu
//
//  Created by Remy Demarest on 01/09/2013.
//
//

#import <Cocoa/Cocoa.h>

@class OEDBGame;
@class OEDBRom;
@class OEDBSaveState;
@class OEGameDocument;

@interface NSDocumentController (OEAdditions)
- (OEGameDocument *)currentGameDocument;

- (void)openGameDocumentWithGame:(OEDBGame *)game display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
- (void)openGameDocumentWithRom:(OEDBRom *)rom display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
- (void)openGameDocumentWithSaveState:(OEDBSaveState *)state display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
@end
