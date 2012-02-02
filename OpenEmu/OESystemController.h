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
#import <OpenEmuBase/OEPluginController.h>
#import <OpenEmuSystem/OEControlsViewController.h>

@class OESystemResponder;

extern NSString *const OESettingValueKey;
extern NSString *const OEHIDEventValueKey;
extern NSString *const OEKeyboardEventValueKey;
extern NSString *const OEControlsPreferenceKey;
extern NSString *const OESystemPluginName;
extern NSString *const OESystemName;
extern NSString *const OESystemIdentifier;
extern NSString *const OEProjectURLKey;
extern NSString *const OEArchiveIDs;
extern NSString *const OEFileTypes;

@interface OESystemController : NSObject <OEPluginController, OEControlsViewControllerDelegate>
{
@private
    NSBundle            *_bundle;
    NSMutableArray      *_gameSystemResponders;
    NSMutableDictionary *_preferenceViewControllers;
    
    NSString *_systemName;
}

/*
  *The method search for a class associated with aKey and instantiate the controller
  *with the default Nib name provided by the key.
  *
  *For example: if the passed-in key is @"OEControlsPreferenceKey" the default nib name will be
  *@"ControlsPreference" (the two-letter prefix "OE" and three-letter suffix "Key" are removed from
  *the name).
 */
- (id)newPreferenceViewControllerForKey:(NSString *)aKey;

// A dictionary of keys and UIViewController classes, keys are different panels available in the preferences
// Must be overridden by subclasses to provide the appropriate classes
- (NSDictionary *)preferenceViewControllerClasses;


@property(readonly) NSString   *systemIdentifier;
@property(readonly) NSString   *systemName;

@property(readonly) NSUInteger  numberOfPlayers;
@property(readonly) Class       responderClass;
@property(readonly) NSArray    *genericSettingNames;
@property(readonly) NSArray    *genericControlNames;
@property(readonly) NSString   *playerString;

- (NSUInteger)playerNumberInKey:(NSString *)keyName getKeyIndex:(NSUInteger *)idx;

#pragma mark -
#pragma mark Bindings settings

// Dictionary containing the default values to register for the system
@property(readonly) NSDictionary *defaultControls;
- (void)registerDefaultControls;

// Converts anObject, usually representing an event, into a value registerable into the defaults
- (id)registarableValueWithObject:(id)anObject;

// Returns the keyPath associating the key name to the type of the value
- (NSString *)keyPathForKey:(NSString *)keyName withValueType:(NSString *)aType;

// Register an event into the user defaults
// The keypath should be built using -keyPathForKey:withValueType:
- (void)registerValue:(id)aValue forKeyPath:(NSString *)keyPath;
// Remove all bindings (usually there's only one) that could be associated to theEvent value
- (void)removeBindingsToEvent:(id)theEvent withValueType:(NSString *)aType;

#pragma mark -
#pragma mark Game System Responder objects

- (id)newGameSystemResponder;
- (void)registerGameSystemResponder:(OESystemResponder *)responder;
- (void)unregisterGameSystemResponder:(OESystemResponder *)responder;

#pragma mark -
#pragma mark ROM Handling
@property(readonly) NSArray *fileTypes;
@property(readonly) NSArray *archiveIDs;

- (BOOL)canHandleFile:(NSString*)path;
@end
