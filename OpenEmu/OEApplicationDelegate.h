/*
 Copyright (c) 2011, OpenEmu Team

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

#import "NSDocumentController+OEAdditions.h"

@class OEDeviceManager;
@class OEMainWindowController;
@class OEPreferencesWindowController;

@interface OEApplicationDelegate : NSDocumentController <NSApplicationDelegate, NSMenuDelegate>

- (IBAction)showAboutWindow:(id)sender;
- (IBAction)showPreferencesWindow:(id)sender;
- (IBAction)showOpenEmuWindow:(id)sender;

- (IBAction)openWeblink:(id)sender;

- (void)updateInfoPlist;

- (void)loadDatabase;

@property(unsafe_unretained) IBOutlet NSWindow     *aboutWindow;
@property(unsafe_unretained) IBOutlet NSMenu       *fileMenu;
@property(strong) IBOutlet OEMainWindowController  *mainWindowController;

@property(readonly) OEPreferencesWindowController *preferencesWindowController;

@property(readonly) NSString           *aboutCreditsPath;
@property(readonly) NSString           *appVersion;
@property(readonly) NSString           *buildVersion;
@property(readonly) NSAttributedString *projectURL;
@property(readonly) NSAttributedString *specialThanks;
#pragma mark - Help Menu
- (IBAction)showOEHelp:(id)sender;
- (IBAction)showOEReleaseNotes:(id)sender;
- (IBAction)showOEWebSite:(id)sender;
- (IBAction)showOEIssues:(id)sender;

#pragma mark - Debug
- (IBAction)OEDebug_logResponderChain:(id)sender;

@end
