//
//  OESMSSystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenEmuSystem/OpenEmuSystem.h>
#import "OESNESSystemResponderClient.h"

extern NSString *OESNESButtonNameTable[];

@interface OESNESSystemResponder : OEBasicSystemResponder

@property(nonatomic, assign) id<OESNESSystemResponderClient> client;

@end
