/*
 Copyright (c) 2009, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
  *Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
  *Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
  *Neither the name of the OpenEmu Team nor the
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

extern NSString *const OEPopoutHasScreenSizeKey;

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
    OENoCoreForSystemError         = -7,
    OENoCoreForSaveStateError      = -8,
    OECouldNotLoadROMError         = -10,
};

@class OEGameViewController;
@class OEDBRom;
@class OEDBGame;
@class OEDBSaveState;
@class OECorePlugin;

@interface OEGameDocument : NSDocument

- (id)initWithRom:(OEDBRom *)rom;
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin*)core;
- (id)initWithRom:(OEDBRom *)rom error:(NSError **)outError;
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin*)core error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin*)core;
- (id)initWithGame:(OEDBGame *)game error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin*)core error:(NSError **)outError;
- (id)initWithSaveState:(OEDBSaveState *)state;
- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError;
- (void)showInSeparateWindow:(id)sender fullScreen:(BOOL)fullScreen;
@property (readonly, strong) NSViewController *viewController;
@property (readonly, strong) OEGameViewController *gameViewController;
@end
