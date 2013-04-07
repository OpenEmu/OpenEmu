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

@class OEKeyBindingDescription, OEOrientedKeyGroupBindingDescription;

typedef enum _OEKeyGroupType
{
    OEKeyGroupTypeUnknown,
    OEKeyGroupTypeAxis,
    OEKeyGroupTypeHatSwitch,
} OEKeyGroupType;

extern NSString *NSStringFromOEKeyGroupType(OEKeyGroupType type);

// OEKeyGroupBindingsDescription allows OEKeyBindingsDescription objects to know about their peers, this class is only used by OESystemBindings
@interface OEKeyBindingGroupDescription : NSObject <NSCopying>

@property(readonly)       OEKeyGroupType  type;
@property(readonly, copy) NSArray        *keys;
@property(readonly, copy) NSArray        *keyNames;

- (OEKeyBindingDescription *)oppositeKeyOfKey:(OEKeyBindingDescription *)aKey;

- (OEOrientedKeyGroupBindingDescription *)orientedKeyGroupWithBaseKey:(OEKeyBindingDescription *)aKey;

- (NSUInteger)indexOfKey:(OEKeyBindingDescription *)aKey;

- (void)enumerateKeysFromKey:(OEKeyBindingDescription *)baseKey usingBlock:(void(^)(OEKeyBindingDescription *key, BOOL *stop))block;

@end

// OEOrientedKeyGroupBindingDescription is used to know to which key of the group a certain value was set when saving the bindings to the disk, it's also used by responders the same way
@interface OEOrientedKeyGroupBindingDescription : OEKeyBindingGroupDescription

@property(readonly, weak) OEKeyBindingGroupDescription *parentKeyGroup;
@property(readonly, weak) OEKeyBindingDescription      *baseKey;

- (OEKeyBindingDescription *)oppositeKey;

- (NSUInteger)indexOfBaseKey;
- (void)enumerateKeysFromBaseKeyUsingBlock:(void(^)(OEKeyBindingDescription *key, BOOL *stop))block;

@end
