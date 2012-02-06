/*
 Copyright (c) 2009, OpenEmu Team
 
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

@protocol OEControlsViewControllerDelegate;

@interface OEControlsViewController : NSViewController
{
    NSUInteger selectedPlayer;
    NSInteger selectedBindingType;
@private
}

@property(unsafe_unretained) id<OEControlsViewControllerDelegate> delegate;

@property(unsafe_unretained) id selectedControl;

- (NSString *)selectedKey;
- (NSUInteger)selectedPlayer;

- (void)selectPlayer:(NSUInteger)_player;
- (NSString *)keyPathForKey:(NSString *)aKey;

- (void)selectBindingType:(NSInteger)newType;
- (NSInteger)selectedBindingType;

- (BOOL)isKeyboardEventSelected;
- (void)registerEvent:(id)anEvent;

- (IBAction)selectInputControl:(id)sender;
- (void)resetKeyBindings;

- (void)resetBindingsWithKeys:(NSArray *)keys;

- (NSImage *)controllerImage;
- (NSImage *)controllerImageMask;
@end

@protocol OEControlsViewControllerDelegate <NSObject>

- (NSArray *)genericSettingNamesInControlsViewController:(OEControlsViewController *)sender;
- (NSArray *)genericControlNamesInControlsViewController:(OEControlsViewController *)sender;
- (NSString *)controlsViewController:(OEControlsViewController *)sender playerKeyForKey:(NSString *)aKey player:(NSUInteger)playerNumber;

// Reading values
- (id)controlsViewController:(OEControlsViewController *)sender settingForKey:(NSString *)keyName;
- (id)controlsViewController:(OEControlsViewController *)sender HIDEventForKey:(NSString *)keyName;
- (id)controlsViewController:(OEControlsViewController *)sender keyboardEventForKey:(NSString *)keyName;

// Writting values
- (void)controlsViewController:(OEControlsViewController *)sender registerSetting:(id)settingValue forKey:(NSString *)keyName;
- (void)controlsViewController:(OEControlsViewController *)sender registerEvent:(id)theEvent forKey:(NSString *)keyName;

@end
