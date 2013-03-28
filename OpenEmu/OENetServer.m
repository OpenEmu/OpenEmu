/*
 Copyright (c) 2010, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OENetServer.h"
#import "OEHIDEvent.h"
#import "NSApplication+OEHIDAdditions.h"

@implementation OENetServer
@synthesize delegate;

- (BOOL)start:(NSError **)error
{
    if(asyncSocket != nil) return NO;
    
    NSError *bindError = nil;
    asyncSocket = [[AsyncUdpSocket alloc] initWithDelegate:self];
    if(![asyncSocket bindToPort:61337 error:&bindError])
    {
        NSLog(@"************** %@", bindError);
        return NO;
    }

    port = [asyncSocket localPort];
    NSLog(@"%d", port);
    [asyncSocket receiveWithTimeout:-1 tag:1];
    return YES;
}

- (BOOL)stop
{
    [self disableBonjour];
    //kill socket
    asyncSocket = nil;
    return YES;
}

- (BOOL)enableBonjourWithDomain:(NSString *)domain applicationProtocol:(NSString *)protocol name:(NSString *)name
{
    //Will use default Bonjour registration doamins, typically just ".local"
    if(domain == nil) domain = @"";
    
    //Will use default Bonjour name, e.g. the name assigned to the device in iTunes
    if(name   == nil) name   = @"";
    
    if([protocol length] == 0 || asyncSocket == nil) return NO;
    
    netService = [[NSNetService alloc] initWithDomain:domain type:protocol name:name port:port];
    
    if(netService == nil) return NO;
    
    [netService scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    [netService publish];
    [netService setDelegate:self];
    
    return YES;
}

- (BOOL)onUdpSocket:(AsyncUdpSocket *)sock didReceiveData:(NSData *)data withTag:(long)tag fromHost:(NSString *)host port:(UInt16)port
{
    const uint8_t *bytes = [data bytes];
    uint8_t        b     = bytes[0];

    FIXME("Create virtual interfaces for this.");
    OEHIDEvent *ret = [OEHIDEvent buttonEventWithDeviceHandler:nil
                                                     timestamp:[NSDate timeIntervalSinceReferenceDate]
                                                  buttonNumber:b & 0x7F
                                                         state:b & 0x80 ? NSOnState : NSOffState
                                                        cookie:0];
    
    [[NSApplication sharedApplication] postHIDEvent:ret];
    [asyncSocket receiveWithTimeout:-1 tag:1];
    
    return YES;
}

- (void)onUdpSocket:(AsyncUdpSocket *)sock didNotReceiveDataWithTag:(long)tag dueToError:(NSError *)error
{
    NSLog(@"Didn't recieve: %@", error);
}

/*
 Bonjour will not allow conflicting service instance names (in the same domain), and may have automatically renamed
 the service if there was a conflict.  We pass the name back to the delegate so that the name can be displayed to
 the user.
 See http://developer.apple.com/networking/bonjour/faq.html for more information.
 */

- (void)netServiceDidPublish:(NSNetService *)sender
{
    if([self delegate] != nil && [self.delegate respondsToSelector:@selector(server:didEnableBonjourWithName:)])
        [[self delegate] server:self didEnableBonjourWithName:[sender name]];
}

- (void)netService:(NSNetService *)sender didNotPublish:(NSDictionary *)errorDict
{
    if(self.delegate && [self.delegate respondsToSelector:@selector(server:didNotEnableBonjour:)])
        [self.delegate server:self didNotEnableBonjour:errorDict];
}

- (void)disableBonjour
{
    if(netService)
    {
        [netService stop];
        [netService removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        netService = nil;
    }
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ | port %d | netService = %@>", [super description], port, netService];
}

+ (NSString *) bonjourTypeFromIdentifier:(NSString *)identifier
{
    if(![identifier length]) return nil;
    
    return [NSString stringWithFormat:@"_%@._udp.", identifier];
}

@end
