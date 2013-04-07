/*
 Copyright (c) 2012, OpenEmu Team

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

#import <Foundation/Foundation.h>
#import "OEHIDEvent.h"

@class OEControlDescription;
@class OEControlValueDescription;
@class OEDeviceHandler;
@class OEHIDEvent;

@interface OEControllerDescription : NSObject <NSCopying>

// Return YES if the controller is not known to the application database.
@property(readonly) BOOL isGeneric;

@property(readonly) NSString *identifier;
@property(readonly) NSString *name;

@property(readonly) NSArray *devices;

@property(readonly) NSUInteger numberOfControls;
@property(readonly) NSArray *controls;

- (OEControlValueDescription *)controlValueDescriptionForEvent:(OEHIDEvent *)event;
- (OEControlValueDescription *)controlValueDescriptionForIdentifier:(NSString *)controlIdentifier;
- (OEControlValueDescription *)controlValueDescriptionForValueIdentifier:(NSNumber *)controlValueIdentifier;

- (OEControlDescription *)addControlWithIdentifier:(NSString *)identifier name:(NSString *)name event:(OEHIDEvent *)event;
- (OEControlDescription *)addControlWithIdentifier:(NSString *)identifier name:(NSString *)name event:(OEHIDEvent *)event valueRepresentations:(NSDictionary *)valueRepresentations;

@end

OEHIDEventType OEHIDEventTypeFromNSString(NSString *string);
NSUInteger OEUsageFromUsageStringWithType(NSString *usageString, OEHIDEventType type);
