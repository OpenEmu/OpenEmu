//
//  WiimoteHandler.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 15.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <WiiRemote/WiimoteFramework.h>
@interface OEWiimoteHandler : NSObject <WiimoteBrowserDelegate, WiimoteDelegate>
+ (void)search;
+ (id)sharedHandler;
@property (readonly) NSArray *connectedWiiRemotes;
@end
