//
//  OEThreadProxy.h
//  OpenEmu
//
//  Created by Remy Demarest on 21/07/2013.
//
//

#import <Foundation/Foundation.h>

@interface OEThreadProxy : NSProxy

+ (id)threadProxyWithTarget:(id)target;
+ (id)threadProxyWithTarget:(id)target thread:(NSThread *)thread;

- (id)initWithTarget:(id)target thread:(NSThread *)thread;

@property(readonly) id target;
@property(readonly) NSThread *thread;

@end
