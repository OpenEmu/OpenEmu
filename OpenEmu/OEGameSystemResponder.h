//
//  OEGameSystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol OEGameSystemResponderClient;

@interface OEGameSystemResponder : NSResponder
{
    
}

@property(nonatomic, assign) id<OEGameSystemResponderClient> client;

@end

// Methods that subclasses must override
@interface OEGameSystemResponder (OEGameSystemResponderSubclass)

+ (Protocol *)gameSystemResponderClientProtocol;

- (void)settingWasSet:(bycopy id)aValue forKey:(bycopy NSString *)keyName;
- (void)keyboardEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName;
- (void)keyboardEventWasRemovedForKey:(bycopy NSString *)keyName;

- (void)HIDEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName;
- (void)HIDEventWasRemovedForKey:(bycopy NSString *)keyName;

@end
