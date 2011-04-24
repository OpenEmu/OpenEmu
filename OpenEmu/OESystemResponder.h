//
//  OEGameSystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class    OESystemController;
@protocol OESystemResponderClient;

@interface OESystemResponder : NSResponder
{
    
}

// Designated initializer
- (id)initWithController:(OESystemController *)controller;

@property(nonatomic, readonly) OESystemController *controller;
@property(nonatomic, assign) id<OESystemResponderClient> client;

@end

// Methods that subclasses must override
@interface OESystemResponder (OEGameSystemResponderSubclass)

+ (Protocol *)gameSystemResponderClientProtocol;

- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName;
- (void)keyboardEventWasSet:(id)theEvent forKey:(NSString *)keyName;
- (void)keyboardEventWasRemovedForKey:(NSString *)keyName;

- (void)HIDEventWasSet:(id)theEvent forKey:(NSString *)keyName;
- (void)HIDEventWasRemovedForKey:(NSString *)keyName;

@end
