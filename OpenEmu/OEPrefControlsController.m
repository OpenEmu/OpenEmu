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

#import <Quartz/Quartz.h>

#import "NSViewController+OEAdditions.h"

#import "OEPrefControlsController.h"
#import "OEBackgroundGradientView.h"
#import "OEBackgroundImageView.h"
#import "OELibraryDatabase.h"

#import "OEPlugin.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OESystemController.h"
#import "OEDeviceHandler.h"
#import "OEDeviceDescription.h"

#import "OEControllerImageView.h"
#import "OEControlsButtonSetupView.h"

#import "OEHIDEvent.h"

#import "OEBindingsController.h"
#import "OESystemBindings.h"
#import "OEPlayerBindings.h"
#import "OEKeyBindingGroupDescription.h"

#import "OEDeviceManager.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"

#import "OEPreferencesController.h"

NSString *const OELastControlsPluginIdentifierKey = @"lastControlsPlugin";
NSString *const OELastControlsPlayerKey           = @"lastControlsPlayer";
NSString *const OELastControlsDeviceTypeKey       = @"lastControlsDevice";
NSString *const OEKeyboardBindingsIsSelectedKey   = @"OEKeyboardBindingsIsSelectedKey";

static NSString *const _OEKeyboardMenuItemRepresentedObject = @"org.openemu.Bindings.Keyboard";

@interface OEPrefControlsController ()
{
    OESystemPlugin *selectedPlugin;
    OEHIDEvent     *readingEvent;
    NSMutableSet   *ignoredEvents;
}

- (void)OE_setCurrentBindingsForEvent:(OEHIDEvent *)anEvent;
- (void)OE_rebuildSystemsMenu;
- (void)OE_setUpControllerImageViewWithTransition:(NSString *)transition;
- (void)OE_preparePaneWithNotification:(NSNotification *)notification;

// Only one event can be managed at a time, all events should be ignored until the currently read event went back to its null state
// All ignored events are stored until they go back to the null state
- (BOOL)OE_shouldRegisterEvent:(OEHIDEvent *)anEvent;

@property(nonatomic, readwrite) OESystemBindings *currentSystemBindings;
@property(nonatomic, readwrite) OEPlayerBindings *currentPlayerBindings;

@end

static Boolean _OEHIDEventIsEqualSetCallback(OEHIDEvent *value1, OEHIDEvent *value2)
{
    return [value1 isUsageEqualToEvent:value2];
}

static CFHashCode _OEHIDEventHashSetCallback(OEHIDEvent *value)
{
    return [value controlIdentifier];
}

@implementation OEPrefControlsController

+ (NSSet *)keyPathsForValuesAffectingCurrentPlayerBindings
{
    return [NSSet setWithObjects:@"currentSystemBindings", @"currentSystemBindings.devicePlayerBindings", @"selectedPlayer", nil];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark -
#pragma mark ViewController Overrides

- (void)awakeFromNib
{
    [super awakeFromNib];

    // We're using CFSet here because NSSet is confused by the changing state of OEHIDEvents
    CFSetCallBacks callbacks = kCFTypeSetCallBacks;
    callbacks.equal = (CFSetEqualCallBack)_OEHIDEventIsEqualSetCallback;
    callbacks.hash  = (CFSetHashCallBack)_OEHIDEventHashSetCallback;

    ignoredEvents = (__bridge_transfer NSMutableSet *)CFSetCreateMutable(NULL, 0, &callbacks);

    [[self controlsSetupView] setTarget:self];
    [[self controlsSetupView] setAction:@selector(changeInputControl:)];

    [[self controlsSetupView] bind:@"bindingsProvider" toObject:self withKeyPath:@"currentPlayerBindings" options:nil];

    NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];

    NSImage *controlsBackgroundImage = [NSImage imageNamed:@"controls_background"];
    [(OEBackgroundImageView *)[self view] setImage:controlsBackgroundImage];

    // Setup controls popup console list
    [self OE_rebuildSystemsMenu];
    [self OE_setUpInputMenu];

    // Restore previous state.
    [self changeInputDevice:self];

    NSString *pluginName = [sud stringForKey:OELastControlsPluginIdentifierKey];
    [[self consolesPopupButton] selectItemAtIndex:0];
    for(NSMenuItem *anItem in [[self consolesPopupButton] itemArray])
    {
        if([[anItem representedObject] isEqual:pluginName])
        {
            [[self consolesPopupButton] selectItem:anItem];
            break;
        }
    }

    [CATransaction setDisableActions:YES];
    [self changeSystem:[self consolesPopupButton]];
    [CATransaction commit];

    [self gradientOverlay].topColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.3];
    [self gradientOverlay].bottomColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.0];

    [[self controllerView] setWantsLayer:YES];

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(systemsChanged) name:OEDBSystemsDidChangeNotification object:nil];
    [center addObserver:self selector:@selector(OE_preparePaneWithNotification:) name:OEPreferencesOpenPaneNotificationName object:nil];
    [center addObserver:self selector:@selector(OE_preparePaneWithNotification:) name:OEPreferencesSetupPaneNotificationName object:nil];

    [center addObserver:self selector:@selector(OE_devicesDidUpdateNotification:) name:OEDeviceManagerDidAddDeviceHandlerNotification object:[OEDeviceManager sharedDeviceManager]];
    [center addObserver:self selector:@selector(OE_devicesDidUpdateNotification:) name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:[OEDeviceManager sharedDeviceManager]];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];

    [[OEBindingsController defaultBindingsController] synchronize];
}

- (void)animationDidStart:(CAAnimation *)theAnimation
{
    [[[self controllerView] layer] setValue:[NSNumber numberWithFloat:1.0] forKeyPath:@"filters.pixellate.inputScale"];
}

- (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag
{
    if(flag) [[[self controllerView] layer] setValue:[NSNumber numberWithInt:10.0] forKeyPath:@"filters.pixellate.inputScale"];
}

- (NSString *)nibName
{
    return @"OEPrefControlsController";
}

- (void)systemsChanged
{
    NSMenuItem *menuItem = [[self consolesPopupButton] selectedItem];
    NSString *selectedSystemIdentifier = [menuItem representedObject];

    [self OE_rebuildSystemsMenu];

    [[self consolesPopupButton] selectItemAtIndex:0];
    for(NSMenuItem *anItem in [[self consolesPopupButton] itemArray])
    {
        if([[anItem representedObject] isEqual:selectedSystemIdentifier])
        {
            [[self consolesPopupButton] selectItem:anItem];
            break;
        }
    }

    [CATransaction setDisableActions:YES];
    [self changeSystem:[self consolesPopupButton]];
    [CATransaction commit];
}

- (OESystemController *)currentSystemController;
{
    return [selectedPlugin controller];
}

- (OEPlayerBindings *)currentPlayerBindings
{
    OEPlayerBindings *ret = ([self isKeyboardEventSelected]
                             ? [[self currentSystemBindings] keyboardPlayerBindingsForPlayer:[self selectedPlayer]]
                             : [[self currentSystemBindings] devicePlayerBindingsForPlayer:[self selectedPlayer]]);

    NSAssert(ret == nil || [ret isKindOfClass:[OEPlayerBindings class]], @"Expecting OEPlayerBindingsController instance, got: %@", ret);

    return ret;
}

#pragma mark -
#pragma mark UI Methods

- (void)OE_setUpPlayerMenuForNumberOfPlayers:(NSUInteger)numberOfPlayers;
{
    NSMenu *playerMenu = [[NSMenu alloc] init];
    for(NSUInteger player = 0; player < numberOfPlayers; player++)
    {
        NSString   *playerTitle = [NSString stringWithFormat:NSLocalizedString(@"Player %ld", @""), player + 1];
        NSMenuItem *playerItem  = [[NSMenuItem alloc] initWithTitle:playerTitle action:NULL keyEquivalent:@""];
        [playerItem setTag:player + 1];
        [playerMenu addItem:playerItem];
    }

    [[self playerPopupButton] setMenu:playerMenu];

    // Hide player PopupButton if there is only one player
    //[[self playerPopupButton] setHidden:(numberOfPlayers == 1)];
    [[self playerPopupButton] selectItemWithTag:[[NSUserDefaults standardUserDefaults] integerForKey:OELastControlsPlayerKey]];
}

- (NSMenu *)OE_playerMenuForPlayerCount:(NSUInteger)numberOfPlayers;
{
    NSMenu *playerMenu = [[NSMenu alloc] init];
    for(NSUInteger player = 0; player < numberOfPlayers; player++)
    {
        NSString   *playerTitle = [NSString stringWithFormat:NSLocalizedString(@"Player %ld", @""), player + 1];
        NSMenuItem *playerItem  = [[NSMenuItem alloc] initWithTitle:playerTitle action:NULL keyEquivalent:@""];
        [playerItem setTag:player + 1];
        [playerMenu addItem:playerItem];
    }

    return playerMenu;
}

- (void)OE_devicesDidUpdateNotification:(NSNotification *)notification
{
    [self OE_setUpInputMenu];
}

- (void)OE_setUpInputMenu;
{
    NSMenu *inputMenu = [[NSMenu alloc] init];
    NSMenuItem *inputItem = [inputMenu addItemWithTitle:NSLocalizedString(@"Keyboard", @"Keyboard bindings menu item.") action:NULL keyEquivalent:@""];
    [inputItem setRepresentedObject:_OEKeyboardMenuItemRepresentedObject];

    [inputMenu addItem:[NSMenuItem separatorItem]];

    [self OE_addControllersToInputMenu:inputMenu];

    [inputMenu addItem:[NSMenuItem separatorItem]];

    [inputMenu addItemWithTitle:NSLocalizedString(@"Add a Wiimote…", @"Wiimote bindings menu item.") action:@selector(searchForWiimote:) keyEquivalent:@""];

    [[self inputPopupButton] setMenu:inputMenu];
    [self OE_updateInputPopupButtonSelection];
}

- (void)OE_updateInputPopupButtonSelection
{
    NSPopUpButton *inputButton = [self inputPopupButton];
    BOOL keyboardIsSelected = [[NSUserDefaults standardUserDefaults] boolForKey:OEKeyboardBindingsIsSelectedKey];

    OEDeviceHandler *currentDeviceHandler = [[[self currentSystemBindings] devicePlayerBindingsForPlayer:[self selectedPlayer]] deviceHandler];
    id representedObject = (keyboardIsSelected || currentDeviceHandler == nil ? _OEKeyboardMenuItemRepresentedObject : currentDeviceHandler);

    if(!keyboardIsSelected && currentDeviceHandler == nil)
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:OEKeyboardBindingsIsSelectedKey];

    for(NSMenuItem *item in [inputButton itemArray])
    {
        if([item state] == NSOnState) continue;
        if([item representedObject])
            [item setState:[item representedObject] == currentDeviceHandler ? NSMixedState : NSOffState];
    }

    [inputButton selectItemAtIndex:MAX(0, [inputButton indexOfItemWithRepresentedObject:representedObject])];
}

- (void)OE_addControllersToInputMenu:(NSMenu *)inputMenu
{
    NSArray *controllers = [[OEDeviceManager sharedDeviceManager] controllerDeviceHandlers];
    if([controllers count] == 0)
    {
        [[inputMenu addItemWithTitle:NSLocalizedString(@"No available controllers", @"Menu item indicating that no controllers is plugged in") action:NULL keyEquivalent:@""] setEnabled:NO];
        return;
    }

    for(OEDeviceHandler *handler in controllers)
    {
        NSString *deviceName = [[handler deviceDescription] name];
        NSMenuItem *item = [inputMenu addItemWithTitle:deviceName action:NULL keyEquivalent:@""];
        [item setRepresentedObject:handler];
    }
}

- (IBAction)changeSystem:(id)sender
{
    NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];

    NSMenuItem *menuItem = [[self consolesPopupButton] selectedItem];
    NSString *systemIdentifier = [menuItem representedObject] ? : [sud objectForKey:OELastControlsPluginIdentifierKey];

    NSString *oldPluginName = [selectedPlugin systemName];

    OESystemPlugin *newPlugin = [OESystemPlugin gameSystemPluginForIdentifier:systemIdentifier];
    if(newPlugin == nil)
    {
        NSArray *allPlugins = [OESystemPlugin allPlugins];
        if([allPlugins count] > 0) newPlugin = [allPlugins objectAtIndex:0];
    }

    if(selectedPlugin != nil && newPlugin == selectedPlugin) return;
    selectedPlugin = newPlugin;

    OESystemController *systemController = [self currentSystemController];

    NSAssert(systemController != nil, @"The systemController of the plugin %@ with system identifier %@ is nil for some reason.", selectedPlugin, [selectedPlugin systemIdentifier]);
    [self setCurrentSystemBindings:[[OEBindingsController defaultBindingsController] systemBindingsForSystemController:systemController]];

    //[self setKeyBindings:[[systemController controllerKeyPositions] allKeys]];

    // Rebuild player menu
    [self OE_setUpPlayerMenuForNumberOfPlayers:[systemController numberOfPlayers]];

    OEControlsButtonSetupView *preferenceView = [self controlsSetupView];
    [preferenceView setBindingsProvider:[self currentPlayerBindings]];
    [preferenceView setupWithControlList:[systemController controlPageList]];
    [preferenceView setAutoresizingMask:NSViewMaxXMargin | NSViewMaxYMargin];

    NSRect rect = (NSRect){ .size = { [self controlsSetupView].bounds.size.width, preferenceView.frame.size.height }};
    [preferenceView setFrame:rect];

    NSScrollView *scrollView = [[self controlsSetupView] enclosingScrollView];
    [[self controlsSetupView] setFrameOrigin:(NSPoint){ 0, scrollView.frame.size.height - rect.size.height}];

    if([[self controlsSetupView] frame].size.height <= scrollView.frame.size.height)
        [scrollView setVerticalScrollElasticity:NSScrollElasticityNone];
    else
    {
        [scrollView setVerticalScrollElasticity:NSScrollElasticityAutomatic];
        [scrollView flashScrollers];
    }

    [sud setObject:systemIdentifier forKey:OELastControlsPluginIdentifierKey];

    [self changePlayer:[self playerPopupButton]];
    [self changeInputDevice:[self inputPopupButton]];

    NSComparisonResult order = [oldPluginName compare:[selectedPlugin systemName]];
    [self OE_setUpControllerImageViewWithTransition:(order == NSOrderedDescending ? kCATransitionFromTop : kCATransitionFromBottom)];
}

- (void)OE_setUpControllerImageViewWithTransition:(NSString *)transition;
{
    OESystemController *systemController = [self currentSystemController];

    OEControllerImageView *newControllerView = [[OEControllerImageView alloc] initWithFrame:[[self controllerContainerView] bounds]];
    [newControllerView setImage:[systemController controllerImage]];
    [newControllerView setImageMask:[systemController controllerImageMask]];
    [newControllerView setKeyPositions:[systemController controllerKeyPositions]];
    [newControllerView setTarget:self];
    [newControllerView setAction:@selector(changeInputControl:)];

    // Animation for controller image swapping
    CATransition *controllerTransition = [CATransition animation];
    [controllerTransition setType:kCATransitionPush];
    [controllerTransition setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault]];
    [controllerTransition setDuration:1.0];
    [controllerTransition setSubtype:transition];
    [controllerTransition setRemovedOnCompletion:YES];

    [[self controllerContainerView] setAnimations:[NSDictionary dictionaryWithObject:controllerTransition forKey:@"subviews"]];

    if(_controllerView != nil)
        [[[self controllerContainerView] animator] replaceSubview:_controllerView with:newControllerView];
    else
        [[[self controllerContainerView] animator] addSubview:newControllerView];

    [[self controllerContainerView] setAnimations:[NSDictionary dictionary]];

    [self setControllerView:newControllerView];
}

- (IBAction)changePlayer:(id)sender
{
    NSInteger player = 1;
    if(sender && [sender respondsToSelector:@selector(selectedTag)])
        player = [sender selectedTag];
    else if(sender && [sender respondsToSelector:@selector(tag)])
        player = [sender tag];

    [self setSelectedPlayer:player];
}

- (IBAction)changeInputDevice:(id)sender
{
    id representedObject = [[[self inputPopupButton] selectedItem] representedObject];
    if(representedObject == nil)
    {
        [[self inputPopupButton] selectItemAtIndex:0];
        return;
    }

    [self willChangeValueForKey:@"currentPlayerBindings"];

    BOOL isSelectingKeyboard = [representedObject isEqual:_OEKeyboardMenuItemRepresentedObject];
    [[NSUserDefaults standardUserDefaults] setBool:isSelectingKeyboard forKey:OEKeyboardBindingsIsSelectedKey];

    if(!isSelectingKeyboard)
    {
        NSAssert([representedObject isKindOfClass:[OEDeviceHandler class]], @"Expecting instance of class OEDeviceHandler got: %@", representedObject);

        OEDeviceHandler *currentPlayerHandler = [[[self currentSystemBindings] devicePlayerBindingsForPlayer:[self selectedPlayer]] deviceHandler];
        if(![representedObject isEqual:currentPlayerHandler])
            [[self currentSystemBindings] setDeviceHandler:representedObject forPlayer:[self selectedPlayer]];
    }

    [self OE_updateInputPopupButtonSelection];

    [self didChangeValueForKey:@"currentPlayerBindings"];
}

- (IBAction)changeInputControl:(id)sender
{
    if(sender == [self controllerView] || sender == [self controlsSetupView])
        [self setSelectedKey:[sender selectedKey]];
}

- (IBAction)searchForWiimote:(id)sender
{
    [self OE_setUpInputMenu];

    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    [alert setMessageText:NSLocalizedString(@"If there is a red button on the back battery cover, press it.\nIf not, hold down buttons ①+②.", @"")];
    [alert setDefaultButtonTitle:NSLocalizedString(@"Start Scanning", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    [alert setHeadlineText:NSLocalizedString(@"Make your Wiimote discoverable", @"")];

    if([alert runModal])
    {
        // Start WiiRemote support
        if([[NSUserDefaults standardUserDefaults] boolForKey:OEWiimoteSupportEnabled])
            [[OEDeviceManager sharedDeviceManager] startWiimoteSearch];
    }
}

- (void)setSelectedKey:(NSString *)value
{
    if(_selectedKey == value) value = nil;

    _selectedKey = [value copy];
    [CATransaction begin];
    [[self controlsSetupView] setSelectedKey:_selectedKey];
    [[self controllerView]    setSelectedKey:_selectedKey animated:YES];
    [CATransaction commit];

    if(_selectedKey != nil) [[[self view] window] makeFirstResponder:[self view]];
}

- (void)setSelectedPlayer:(NSUInteger)value
{
    if(_selectedPlayer != value)
    {
        _selectedPlayer = value;
        [[NSUserDefaults standardUserDefaults] setInteger:_selectedPlayer forKey:OELastControlsPlayerKey];
        [[self playerPopupButton] selectItemWithTag:value];
        [self OE_updateInputPopupButtonSelection];
    }
}

- (void)OE_setCurrentBindingsForEvent:(OEHIDEvent *)anEvent;
{
    [self willChangeValueForKey:@"currentPlayerBindings"];
    BOOL isKeyboardEvent = [anEvent type] == OEHIDEventTypeKeyboard;
    [[NSUserDefaults standardUserDefaults] setBool:isKeyboardEvent forKey:OEKeyboardBindingsIsSelectedKey];

    if(!isKeyboardEvent) [self setSelectedPlayer:[[self currentSystemBindings] playerNumberForEvent:anEvent]];

    [self OE_updateInputPopupButtonSelection];
    [self didChangeValueForKey:@"currentPlayerBindings"];
}

#pragma mark -
#pragma mark Input and bindings management methods

- (BOOL)isKeyboardEventSelected
{
    return [[[[self inputPopupButton] selectedItem] representedObject] isEqual:_OEKeyboardMenuItemRepresentedObject];
}

- (void)registerEvent:(OEHIDEvent *)anEvent;
{
    // Ignore any off state events
    if([anEvent hasOffState]) return;

    if([self selectedKey] != nil)
    {
        [self OE_setCurrentBindingsForEvent:anEvent];

        id assignedKey = [[self currentPlayerBindings] assignEvent:anEvent toKeyWithName:[self selectedKey]];

        if([assignedKey isKindOfClass:[OEKeyBindingGroupDescription class]])
            [[self controlsSetupView] selectNextKeyAfterKeys:[assignedKey keyNames]];
        else
            [[self controlsSetupView] selectNextKeyButton];

        [self changeInputControl:[self controlsSetupView]];
    }
}

- (void)keyDown:(NSEvent *)theEvent
{
}

- (void)keyUp:(NSEvent *)theEvent
{
}

- (BOOL)OE_shouldRegisterEvent:(OEHIDEvent *)anEvent;
{
    // The event is the currently read event,
    // if its state is off, nil the reading event,
    // in either case, this event shouldn't be registered.
    if([readingEvent isUsageEqualToEvent:anEvent])
    {
        if([anEvent hasOffState])
            readingEvent = nil;

        return NO;
    }

    if([self selectedKey] == nil && [self view] != [[[self view] window] firstResponder])
        [[[self view] window] makeFirstResponder:[self view]];

    // Check if the event is ignored
    if([ignoredEvents containsObject:anEvent])
    {
        // Ignored events going back to off-state are removed from the ignored events
        if([anEvent hasOffState]) [ignoredEvents removeObject:anEvent];

        return NO;
    }

    // Ignore keyboard events if the user hasn’t explicitly chosen to configure
    // keyboard bindings. See https://github.com/OpenEmu/OpenEmu/issues/403
    if([anEvent type] == OEHIDEventTypeKeyboard && ![self isKeyboardEventSelected])
        return NO;

    // No event currently read, if it's not off state, store it and read it
    if(readingEvent == nil)
    {
        // The event is not ignored but it's off, ignore it anyway
        if([anEvent hasOffState]) return NO;

        readingEvent = anEvent;
        return YES;
    }

    if(![anEvent hasOffState]) [ignoredEvents addObject:anEvent];

    return NO;
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if([self OE_shouldRegisterEvent:anEvent])
        [self registerEvent:anEvent];
}

- (void)triggerPull:(OEHIDEvent *)anEvent;
{
    if([self OE_shouldRegisterEvent:anEvent])
        [self registerEvent:anEvent];
}

- (void)triggerRelease:(OEHIDEvent *)anEvent;
{
    [self OE_shouldRegisterEvent:anEvent];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    if([self OE_shouldRegisterEvent:anEvent])
        [self registerEvent:anEvent];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    [self OE_shouldRegisterEvent:anEvent];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    if([self OE_shouldRegisterEvent:anEvent])
        [self registerEvent:anEvent];
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    if([self OE_shouldRegisterEvent:anEvent])
        [self registerEvent:anEvent];
}

- (void)HIDKeyUp:(OEHIDEvent *)anEvent;
{
    [self OE_shouldRegisterEvent:anEvent];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if([self selectedKey] != nil) [self setSelectedKey:[self selectedKey]];
}

#pragma mark -
#pragma mark OEPreferencePane Protocol

- (NSImage *)icon
{
    return [NSImage imageNamed:@"controls_tab_icon"];
}

- (NSString *)title
{
    return @"Controls";
}

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], @"");
}

- (NSSize)viewSize
{
    return NSMakeSize(740, 450);
}

- (NSColor *)toolbarSeparationColor
{
    return [NSColor colorWithDeviceWhite:0.32 alpha:1.0];
}

#pragma mark -
- (void)OE_preparePaneWithNotification:(NSNotification *)notification
{
    NSDictionary *userInfo = [notification userInfo];
    NSString     *paneName = [userInfo valueForKey:OEPreferencesUserInfoPanelNameKey];

    if([paneName isNotEqualTo:[self title]]) return;

    NSString *systemIdentifier = [userInfo valueForKey:OEPreferencesUserInfoSystemIdentifierKey];
    NSUInteger itemIndex = -1;
    for(NSUInteger i = 0; i < [[[self consolesPopupButton] itemArray] count]; i++)
    {
        NSMenuItem *item = [[[self consolesPopupButton] itemArray] objectAtIndex:i];
        if([[item representedObject] isEqual:systemIdentifier])
        {
            itemIndex = i;
            break;
        }
    }

    if(itemIndex != -1)
    {
        [[self consolesPopupButton] selectItemAtIndex:itemIndex];
        [self changeSystem:nil];
    }
}

- (void)OE_rebuildSystemsMenu
{
    NSMenu *consolesMenu    = [[NSMenu alloc] init];
    NSArray *enabledSystems = [OEDBSystem enabledSystems];

    for(OEDBSystem *system in enabledSystems)
    {
        OESystemPlugin *plugin = [system plugin];
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:[plugin systemName] action:@selector(changeSystem:) keyEquivalent:@""];
        [item setTarget:self];
        [item setRepresentedObject:[plugin systemIdentifier]];
        
        [item setImage:[plugin systemIcon]];
        
        [consolesMenu addItem:item];
    }
    
    [[self consolesPopupButton] setMenu:consolesMenu];
}

@end
