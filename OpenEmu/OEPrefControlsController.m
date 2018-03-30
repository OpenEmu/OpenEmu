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

#import "OEPrefControlsController.h"
#import "OEBackgroundImageView.h"
#import "OELibraryDatabase.h"

#import "OEPlugin.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"

#import "OEControllerImageView.h"
#import "OEControlsButtonSetupView.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

@import Quartz;

#import "OpenEmu-Swift.h"

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
    id _eventMonitor;
}

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

#pragma mark - ViewController Overrides

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

    NSAppearance *aquaAppearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
    [self.controlsContainer.enclosingScrollView setAppearance:aquaAppearance];

    [[self controllerView] setWantsLayer:YES];

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(systemsChanged) name:OEDBSystemAvailabilityDidChangeNotification object:nil];

    [center addObserver:self selector:@selector(OE_devicesDidUpdateNotification:) name:OEDeviceManagerDidAddDeviceHandlerNotification object:[OEDeviceManager sharedDeviceManager]];
    [center addObserver:self selector:@selector(OE_devicesDidUpdateNotification:) name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:[OEDeviceManager sharedDeviceManager]];
    
    [center addObserver:self selector:@selector(OE_scrollerStyleDidChange) name:NSPreferredScrollerStyleDidChangeNotification object:nil];
}

- (void)OE_scrollerStyleDidChange
{
    [self.controlsSetupView layoutSubviews:NO];
}

- (void)viewDidLayout
{
    [super viewDidLayout];
    
    // Fixes an issue where, if the controls pane isn't already the default selected pane on launch and the user manually selects the controls pane, the "Gameplay Buttons" OEControlsSectionTitleView has a visible "highlight" artifact until the scroll view gets scrolled.
    [[self controlsSetupView] layoutSubviews:NO];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    if([[[self view] window] isKeyWindow])
        [self OE_setUpEventMonitor];

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:nil];
    [nc addObserver:self selector:@selector(windowDidResignKey:) name:NSWindowDidResignKeyNotification object:nil];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    self.selectedKey = nil;

    [[OEBindingsController defaultBindingsController] synchronize];

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc removeObserver:self name:NSWindowDidBecomeKeyNotification object:self.view.window];
    [nc removeObserver:self name:NSWindowDidResignKeyNotification object:self.view.window];

    [self OE_tearDownEventMonitor];
}

- (void)OE_setUpEventMonitor
{
    if(_eventMonitor != nil) return;

    _eventMonitor =
    [[OEDeviceManager sharedDeviceManager] addGlobalEventMonitorHandler:
     ^ BOOL (OEDeviceHandler *handler, OEHIDEvent *event)
     {
         [self OE_registerEventIfNeeded:event];
         return NO;
     }];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    if (notification.object == self.view.window) {
        [self OE_setUpEventMonitor];
    }
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    if (notification.object == self.view.window) {
        self.selectedKey = nil;
        [self OE_tearDownEventMonitor];
    }
}

- (void)OE_tearDownEventMonitor
{
    if(_eventMonitor == nil) return;

    [[OEDeviceManager sharedDeviceManager] removeMonitor:_eventMonitor];
    _eventMonitor = nil;
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

#pragma mark - UI Methods

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

    [[inputMenu addItemWithTitle:NSLocalizedString(@"Add a Wiimote…", @"Wiimote bindings menu item.") action:@selector(searchForWiimote:) keyEquivalent:@""] setTarget:self];

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

    NSSortDescriptor *sort = [NSSortDescriptor sortDescriptorWithKey:@"deviceDescription.name" ascending:YES];
    controllers = [controllers sortedArrayUsingDescriptors:@[sort]];

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

    OESystemPlugin *newPlugin = [OESystemPlugin systemPluginForIdentifier:systemIdentifier];
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

    // Make sure no key is selected before switching the system
    [self setSelectedKey:nil];

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

    [self OE_setUpControllerImageView];    
}

- (void)OE_setUpControllerImageView
{
    OESystemController *systemController = [self currentSystemController];
    CALayer            *imageViewLayer   = [[self controllerContainerView] layer];

    OEControllerImageView *newControllerView = [[OEControllerImageView alloc] initWithFrame:[[self controllerContainerView] bounds]];
    [newControllerView setImage:[systemController controllerImage]];
    [newControllerView setImageMask:[systemController controllerImageMask]];
    [newControllerView setKeyPositions:[systemController controllerKeyPositions]];
    [newControllerView setTarget:self];
    [newControllerView setAction:@selector(changeInputControl:)];

    // Setup animation that transitions the old controller image out
    CGMutablePathRef pathTransitionOut = CGPathCreateMutable();
    CGPathMoveToPoint(pathTransitionOut, NULL, 0.0f, 0.0f);
    CGPathAddLineToPoint(pathTransitionOut, NULL, 0.0f, 450.0f);

    CAKeyframeAnimation *outTransition = [CAKeyframeAnimation animationWithKeyPath:@"position"];
    outTransition.path = pathTransitionOut;
    outTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    outTransition.duration = 0.35;

    CFRelease(pathTransitionOut);

    // Setup animation that transitions the new controller image in
    CGMutablePathRef pathTransitionIn = CGPathCreateMutable();
    CGPathMoveToPoint(pathTransitionIn, NULL, 0.0f, 450.0f);
    CGPathAddLineToPoint(pathTransitionIn, NULL, 0.0f, 0.0f);

    CAKeyframeAnimation *inTransition = [CAKeyframeAnimation animationWithKeyPath:@"position"];
    inTransition.path = pathTransitionIn;
    inTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    inTransition.duration = 0.35;

    CFRelease(pathTransitionIn);

    [CATransaction begin];
    [CATransaction setCompletionBlock:^{
        if(self->_controllerView != nil)
            [[self controllerContainerView] replaceSubview:self->_controllerView with:newControllerView];
        else
            [[self controllerContainerView] addSubview:newControllerView];
        [self setControllerView:newControllerView];

        [[self controllerContainerView] setFrameOrigin:NSZeroPoint];
        [imageViewLayer addAnimation:inTransition forKey:@"animatePosition"];
    }];

    [[self controllerContainerView] setFrameOrigin:NSMakePoint(0, 450)];
    [imageViewLayer addAnimation:outTransition forKey:@"animatePosition"];

    [CATransaction commit];

    [[self controlsSetupView] layoutSubviews:NO];
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
    [self OE_updateInputPopupButtonSelection];

    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    if([[OEDeviceManager sharedDeviceManager] isBluetoothEnabled])
    {
        [alert setMessageText:NSLocalizedString(@"If there is a red button on the back battery cover, press it.\nIf not, hold down buttons ①+②.", @"")];
        [alert setDefaultButtonTitle:NSLocalizedString(@"Start Scanning", @"")];
        [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
        [alert setOtherButtonTitle:NSLocalizedString(@"Learn More", @"")];
        [alert setHeadlineText:NSLocalizedString(@"Make your Wiimote discoverable", @"")];

        NSUInteger result = [alert runModal];
        if(result == NSAlertFirstButtonReturn)
        {
            // Start WiiRemote support
            if([[NSUserDefaults standardUserDefaults] boolForKey:OEWiimoteSupportEnabled])
                [[OEDeviceManager sharedDeviceManager] startWiimoteSearch];
        }
        else if(result == NSAlertThirdButtonReturn)
        {
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-Wiimote-and-Wii-U-Pro-pairing"]];
        }
    }
    else
    {
        [alert setMessageText:NSLocalizedString(@"Bluetooth must be enabled to pair a Wii controller.", @"")];
        [alert setDefaultButtonTitle:NSLocalizedString(@"OK", @"")];
        [alert setHeadlineText:NSLocalizedString(@"Bluetooth Not Enabled", @"")];
        [alert runModal];
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

#pragma mark - Input and bindings management methods

- (BOOL)isKeyboardEventSelected
{
    return [[[[self inputPopupButton] selectedItem] representedObject] isEqual:_OEKeyboardMenuItemRepresentedObject];
}

- (void)registerEvent:(OEHIDEvent *)anEvent;
{
    // Ignore any off state events
    if([anEvent hasOffState] || [self selectedKey] == nil) return;

    [self OE_setCurrentBindingsForEvent:anEvent];

    id assignedKey = [[self currentPlayerBindings] assignEvent:anEvent toKeyWithName:[self selectedKey]];

    if([assignedKey isKindOfClass:[OEKeyBindingGroupDescription class]])
        [[self controlsSetupView] selectNextKeyAfterKeys:[assignedKey keyNames]];
    else
        [[self controlsSetupView] selectNextKeyButton];

    [self changeInputControl:[self controlsSetupView]];
}

- (void)keyDown:(NSEvent *)theEvent
{
    if ([self selectedKey] == nil)
        return;
    
    if ([theEvent keyCode] == kVK_Escape)
        [[self currentPlayerBindings] removeEventForKeyWithName:[self selectedKey]];
}

- (void)keyUp:(NSEvent *)theEvent
{
}

- (void)OE_registerEventIfNeeded:(OEHIDEvent *)anEvent;
{
    if([self OE_shouldRegisterEvent:anEvent])
        [self registerEvent:anEvent];
}

// Only one event can be managed at a time, all events should be ignored until the currently read event went back to its null state
// All ignored events are stored until they go back to the null state
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

    // Esc-key events are handled through NSEvent
    if ([anEvent isEscapeKeyEvent])
        return NO;

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
    return NSLocalizedString([self title], @"Preferences: Controls Toolbar Item");
}

- (NSSize)viewSize
{
    return NSMakeSize(755, 450);
}

#pragma mark -

- (void)preparePaneWithNotification:(NSNotification *)notification
{
    NSDictionary *userInfo = [notification userInfo];
    NSString     *paneName = [userInfo valueForKey:[OEPreferencesWindowController userInfoPanelNameKey]];

    if([paneName isNotEqualTo:[self title]]) return;

    NSString *systemIdentifier = [userInfo valueForKey:[OEPreferencesWindowController userInfoSystemIdentifierKey]];
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
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSArray *enabledSystems = [OEDBSystem enabledSystemsinContext:context];

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
