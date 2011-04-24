//
//  OENESSystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 24/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenEmuSystem/OpenEmuSystem.h>

extern NSString *const OENESControlNameTable[];

typedef enum _OENESButton
{
    OENESButtonA,
    OENESButtonB,
    OENESButtonUp,
    OENESButtonDown,
    OENESButtonLeft,
    OENESButtonRight,
    OENESButtonStart,
    OENESButtonSelect,
} OENESButton;

@protocol OENESSystemResponderClient <OESystemResponderClient, NSObject>

- (void)didPushButton:(OENESButton)button forPlayer:(NSUInteger)player;
- (void)didReleaseButton:(OENESButton)button forPlayer:(NSUInteger)player;

@end

@interface OENESSystemResponder : OEBasicSystemResponder

@property(nonatomic, assign) id<OENESSystemResponderClient> client;

@end
