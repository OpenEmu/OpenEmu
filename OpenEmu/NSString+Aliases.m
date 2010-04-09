/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "NSString+Aliases.h"

/*
 Use this with APIs that return an OSStatus. If the function returns
 anything but noErr, then it will log the calling function and an
 error message specified by the caller. It will then use a goto
 (this is one time when goto! is useful) to jump to wherever you
 place an OECatch(), skipping the processing steps that likely
 don't matter if there's an error.
 */

#define OEThrowIfError(__cmd, __err, ...) \
    do{ \
        err = (__cmd); \
        if(err != noErr){ \
            NSLog(@"%s " __err, __PRETTY_FUNCTION__, ##__VA_ARGS__); \
            goto OEFail; \
        } \
    }while(0)

#define OECatch() \
OEFail:

@implementation NSString (OEAliases)

+(NSString *)OE_stringWithPathOfAliasData:(NSData *)aliasData
{
    if(!aliasData) return nil;
    
    AliasHandle handle = NULL;
    NSString *pathString = nil;
    
    // Look mah! I IZ LEGACY
    OSErr err = noErr;
    
    OEThrowIfError(PtrToHand([aliasData bytes], (Handle *)&handle, [aliasData length]), @"PtrToHand");
    
    FSRef fileRef;
    Boolean wasChanged;
    OEThrowIfError(FSResolveAlias(NULL, handle, &fileRef, &wasChanged), @"FSResolveAlias");
    
    char path[1024];
    
    OEThrowIfError(FSRefMakePath(&fileRef, (UInt8 *)path, 1024), @"FSRefMakePath");
    
    pathString = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
    pathString = [pathString stringByStandardizingPath];
    
    OECatch();
    if(handle)
    {
        DisposeHandle((Handle)handle);
        handle = NULL;
    }
        
    return pathString;
}

- (NSString *)  OE_initWithPathOfAliasData:(NSData *)aliasData
{
    return [[[self class] OE_stringWithPathOfAliasData:aliasData] copy];
}

- (NSData *)OE_pathAliasData
{
    AliasHandle handle = NULL;
    Boolean isDirectory;

    OSErr err = noErr;
    NSData *aliasData = nil;
    
    OEThrowIfError(FSNewAliasFromPath( NULL, [self UTF8String], 0, &handle, &isDirectory ), @"FSNewAliasFromPath");
    
    long aliasSize = GetAliasSize(handle);
    aliasData = [NSData dataWithBytes:*handle length:aliasSize];
    
    OECatch();
    if(handle != NULL) DisposeHandle((Handle)handle);
    
    return aliasData;
}

@end
