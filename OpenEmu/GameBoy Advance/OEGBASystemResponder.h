//
//  OEGBASystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 08/05/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <OpenEmuSystem/OpenEmuSystem.h>

@protocol OEGBASystemResponderClient;

extern NSString *OEGBAButtonNameTable[];

@interface OEGBASystemResponder : OEBasicSystemResponder

@property(nonatomic, assign) id<OEGBASystemResponderClient> client;

@end
