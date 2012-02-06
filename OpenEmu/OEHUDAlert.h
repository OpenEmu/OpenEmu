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

#import <Foundation/Foundation.h>

@class OEHUDAlert;
@class OEHUDProgressbar;

typedef void (^OEAlertCompletionHandler)(OEHUDAlert *alert, NSUInteger result);
@interface OEHUDAlert : NSObject{
    NSWindow *_window;
    NSUInteger result;
    
    OEAlertCompletionHandler callbackHandler;
}

+ (id)saveGameAlertWithProposedName:(NSString*)name;
+ (id)autoSaveGameAlert;
+ (id)deleteGameAlertWithStateName:(NSString*)stateName;

+ (id)alertWithError:(NSError*)error;
+ (id)alertWithMessageText:(NSString *)msgText defaultButton:(NSString*)defaultButtonLabel alternateButton:(NSString*)alternateButtonLabel;
#pragma mark -
- (NSUInteger)runModal;
- (void)closeWithResult:(NSInteger)res;
@property (readonly) NSUInteger result;

@property (retain) NSWindow *window;
#pragma mark -
#pragma mark Sizing
@property float height, width;

@property (readonly, retain) NSView *boxView;
#pragma mark -
#pragma mark Buttons
@property (readonly, retain) NSButton *defaultButton;
@property (readonly, retain) NSButton *alternateButton;

@property (readonly, retain) NSTextView *messageTextView;
@property (readonly, retain) NSTextField *headlineLabelField;

@property (assign) NSString *stringValue, *inputLabelText;
@property (assign) NSString *defaultButtonTitle, *alternateButtonTitle, *title, *messageText, *headlineLabelText;

#pragma mark -
#pragma mark Input Field
@property (readonly, retain) NSTextField *inputField,  *inputLabelField;
@property BOOL showsInputField;
@property int inputLimit;

#pragma mark -
#pragma mark Progress Bar
@property BOOL showsProgressbar;
@property (readonly) OEHUDProgressbar *progressbar;
- (void)setProgress:(float)progress;
- (float)progress;

#pragma mark -
#pragma mark Button Actions
- (void)setDefaultButtonAction:(SEL)sel andTarget:(id)target;
- (void)setAlternateButtonAction:(SEL)sel andTarget:(id)target;

#pragma mark -
#pragma mark Callbacks
@property (assign) id target;
@property SEL callback;
@property (copy) OEAlertCompletionHandler callbackHandler;

#pragma mark -
#pragma mark Suppression Button
- (void)showSuppressionButtonForUDKey:(NSString*)key;
@property BOOL showsSuppressionButton;
@property (readonly) NSButton *suppressionButton;
@property (copy) NSString *suppressionUDKey;
@property (copy) NSString *suppressionLabelText; // default is "Do not ask me again", can be changed (e.g. if alert is too small)
@end




