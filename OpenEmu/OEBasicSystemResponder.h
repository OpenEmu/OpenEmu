//
//  OEBasicSystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenEmuSystem/OESystemResponder.h>
#import <OpenEmuSystem/OEMap.h>

@interface OEBasicSystemResponder : OESystemResponder
{
@private
    OEMapRef keyMap;
}

- (OEEmulatorKey)emulatorKeyForKey:(NSString *)aKey index:(NSUInteger)index player:(NSUInteger)thePlayer;
- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer;
- (void)pressEmulatorKey:(OEEmulatorKey)aKey;
- (void)releaseEmulatorKey:(OEEmulatorKey)aKey;

@end
