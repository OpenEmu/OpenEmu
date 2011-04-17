//
//  OEGameSystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEGameCoreController.h"

@protocol OEGameSystemResponderClient;

@interface OEGameSystemResponder : NSResponder <OESettingObserver>
{
    
}

+ (Protocol *)gameSystemResponderClientProtocol;

@property(nonatomic, assign) id<OEGameSystemResponderClient> client;

@end

@protocol OEGameSystemResponderClient <NSObject>
@end
