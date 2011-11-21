//
//  OECoreUpdateController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OECoreDownload.h"

typedef void (^Block)();
@class OEHUDAlert;
@interface OECoreUpdater : NSObject <OECoreDownloadDelegate>
{
    BOOL isCheckingForAlertDownload;
}
+ (id)sharedUpdater;

- (void)checkForUpdates;
- (void)checkForNewCores:(NSNumber*)fromModal;

- (void)installCoreWithIdentifier:(NSString*)coreIdentifier coreName:(NSString*)coreName systemName:(NSString*)systemName withCompletionHandler:(Block)handle;
@property (readonly) NSMutableDictionary* coresDict;
@property (readonly) NSArray* coreList;

@property (copy)    Block completionHandler;
@property (retain)  NSString* coreIdentifier;
@property (retain)  OEHUDAlert* alert;
@property (retain)  OECoreDownload* coreDownload;
@end
