//
//  OENetServer.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/8/10.
//  Copyright 2010 Roundarch Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AsyncUdpSocket.h"

@class OENetServer;

@protocol OENetServerDelegate <NSObject>
@optional
- (void)server:(OENetServer *)server didEnableBonjourWithName:(NSString *)name;
- (void)server:(OENetServer *)server didNotEnableBonjour:(NSDictionary *)errorDict;
- (void)server:(OENetServer *)server didAcceptConnectionWithInputStream:(NSInputStream *)istr outputStream:(NSOutputStream *)ostr;
@end

@interface OENetServer : NSObject <NSNetServiceDelegate>
{
    id<OENetServerDelegate>  delegate;
    AsyncUdpSocket          *asyncSocket;
    NSNetService            *netService;
    uint16_t                 port;
}

- (BOOL)start:(NSError **)error;
- (BOOL)stop;

//Pass "nil" for the default local domain - Pass only the application protocol for "protocol" e.g. "myApp"
- (BOOL)enableBonjourWithDomain:(NSString*)domain applicationProtocol:(NSString*)protocol name:(NSString*)name;
- (void)disableBonjour;
+ (NSString*)bonjourTypeFromIdentifier:(NSString*)identifier;

@property(assign) id<OENetServerDelegate> delegate;
@end
