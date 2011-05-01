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

#import <Cocoa/Cocoa.h>
#import "OETaskWrapper.h"

extern NSString *const OEGameDocumentErrorDomain;

enum _OEGameDocumentErrorCodes 
{
    OENoError                      =  0,
    OEFileDoesNotExistError        = -1,
    OEIncorrectFileError           = -2,
    OEHelperAppNotRunningError     = -3,
    OEConnectionTimedOutError      = -4,
    OEInvalidHelperConnectionError = -5,
    OENilRootProxyObjectError      = -6,
    OECouldNotLoadROMError         = -10,
};

@protocol OEGameCoreHelper;
@class OECorePlugin, OEGameCoreController, OpenEmuHelperApp;

@interface OEGameCoreManager : NSObject
{
    NSString             *romPath;
    OECorePlugin         *plugin;
    OEGameCoreController *owner;
    
    id<OEGameCoreHelper>  rootProxy;
}

@property(readonly, copy)   NSString             *romPath;
@property(readonly, assign) OECorePlugin         *plugin;
@property(readonly, assign) OEGameCoreController *owner;

- (id)initWithROMAtPath:(NSString *)theRomPath corePlugin:(OECorePlugin *)thePlugin owner:(OEGameCoreController *)theOwner error:(NSError **)outError;

- (BOOL)loadROMError:(NSError **)outError;

- (void)stop;

#pragma mark -
#pragma mark Abstract methods, must be overrode in subclasses
@property(readonly, assign) id<OEGameCoreHelper>  rootProxy;
- (BOOL)startHelperProcessError:(NSError **)outError;
- (void)endHelperProcess;

@end

@interface OEGameCoreProcessManager : OEGameCoreManager <OETaskWrapperController>
{
    // IPC from our OEHelper
    OETaskWrapper        *helper;
    NSString             *taskUUIDForDOServer;
    NSConnection         *taskConnection;    
}

@property(readonly,assign) OETaskWrapper *helper;

@end


@interface OEGameCoreThreadManager : OEGameCoreManager
{
    // IPC from our OEHelper
    NSThread             *helper;
    NSString             *taskUUIDForDOServer;
    NSConnection         *taskConnection;
    NSError              *error;
        
    OpenEmuHelperApp     *helperObject;
}

- (void)executionThread:(id)object;

@end

