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
- (void) serverDidEnableBonjour:(OENetServer*)server withName:(NSString*)name;
- (void) server:(OENetServer*)server didNotEnableBonjour:(NSDictionary *)errorDict;
- (void) didAcceptConnectionForServer:(OENetServer*)server inputStream:(NSInputStream *)istr outputStream:(NSOutputStream *)ostr;
@end

@interface OENetServer : NSObject <NSNetServiceDelegate>
{
	uint16_t port;
	AsyncUdpSocket *asyncSocket;
	NSNetService *netService;
	id <OENetServerDelegate> delegate;
}

- (BOOL)start:(NSError **)error;
- (BOOL)stop;
- (BOOL)enableBonjourWithDomain:(NSString*)domain applicationProtocol:(NSString*)protocol name:(NSString*)name; //Pass "nil" for the default local domain - Pass only the application protocol for "protocol" e.g. "myApp"
- (void)disableBonjour;
+ (NSString*) bonjourTypeFromIdentifier:(NSString*)identifier;

@property (assign) id <OENetServerDelegate> delegate;
@end
