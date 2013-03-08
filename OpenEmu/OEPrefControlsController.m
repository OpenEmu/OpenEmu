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
    
    @interface OEPrefControlsController ()
        {
        OESystemPlugin *selectedPlugin;
        OEHIDEvent     *readingEvent;
        NSMutableSet   *ignoredEvents;
}

- (void)OE_setCurrentBindingsForEvent:(OEHIDEvent *)anEvent;
- (void)OE_rebuildSystemsMenu;
- (void)OE_setupControllerImageViewWithTransition:(NSString *)transition;
- (void)OE_preparePaneWithNotification:(NSNotification *)notification;
    
    // Only one event can be managed at a time, all events should be ignored until the currently read event went back to its null state
    // All ignored events are stored until they go back to the null state
- (BOOL)OE_shouldRegisterEvent:(OEHIDEvent *)anEvent;
    
    @property(nonatomic, readwrite) OESystemBindings *currentSystemBindings;
    @property(nonatomic, readwrite) OEPlayerBindings *currentPlayerBindings;

@end

@implementation OEPrefControlsController

#pragma mark Properties
@synthesize controllerView, controllerContainerView;
@synthesize consolesPopupButton, playerPopupButton, inputPopupButton;

@synthesize gradientOverlay;
@synthesize controlsContainer;
@synthesize controlsSetupView;
@synthesize selectedPlayer;
@synthesize selectedBindingType;
@synthesize selectedKey;

@synthesize currentSystemBindings;

+ (NSSet *)keyPathsForValuesAffectingCurrentPlayerBindings
{
    return [NSSet setWithObjects:@"currentSystemBindings", @"currentSystemBindings.devicePlayerBindings", @"selectedPlayer", @"selectedBindingType", nil];
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
    callbacks.equal = NULL;
    callbacks.hash  = NULL;
    
    ignoredEvents = (__bridge_transfer NSMutableSet *)CFSetCreateMutable(NULL, 0, &callbacks);
    
    [[self controlsSetupView] setTarget:self];
    [[self controlsSetupView] setAction:@selector(changeInputControl:)];
    
    [[self controlsSetupView] bind:@"bindingsProvider" toObject:self withKeyPath:@"currentPlayerBindings" options:nil];
    
    NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];
    
    NSImage *controlsBackgroundImage = [NSImage imageNamed:@"controls_background"];
    [(OEBackgroundImageView *)[self view] setImage:controlsBackgroundImage];
    
    /** ** ** ** ** ** ** ** **/
    // Setup controls popup console list
    [self OE_rebuildSystemsMenu];
    [self OE_setupInputMenu];
    
    // restore previous state
    NSInteger binding = [sud integerForKey:OELastControlsDeviceTypeKey];
    [[self inputPopupButton] selectItemWithTag:binding];
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
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(systemsChanged) name:OEDBSystemsDidChangeNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_preparePaneWithNotification:) name:OEPreferencesOpenPaneNotificationName object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_preparePaneWithNotification:) name:OEPreferencesSetupPaneNotificationName object:nil];
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
    if(flag)
    {
        [[[self controllerView] layer] setValue:[NSNumber numberWithInt:10.0] forKeyPath:@"filters.pixellate.inputScale"];
    }
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

- (void)OE_setupPlayerMenuForNumberOfPlayers:(NSUInteger)numberOfPlayers;
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
    [[self playerPopupButton] setHidden:(numberOfPlayers == 1)];
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

- (void)OE_setupInputMenu;
{
    NSMenu *inputMenu = [[NSMenu alloc] init];
    NSMenuItem *inputItem = [[NSMenuItem alloc] initWithTitle:@"Keyboard" action:@selector(changeInputDevice:) keyEquivalent:@""];
    [inputItem setTag:0];
    [inputItem setState:NSOnState];
    [inputMenu addItem:inputItem];
    
    // TODO: remove generic Gamepad item, loop through attached HID devices and add to menu, else show nothing
    NSMenuItem *inputItemDevice = [[NSMenuItem alloc] initWithTitle:@"Gamepad" action:@selector(changeInputDevice:) keyEquivalent:@""];
    [inputItemDevice setTag:1];
    [inputMenu addItem:inputItemDevice];
    
    [inputMenu addItem:[NSMenuItem separatorItem]];
    
    NSMenuItem *inputItemWiimote = [[NSMenuItem alloc] initWithTitle:@"Add a Wiimote…" action:@selector(searchForWiimote:) keyEquivalent:@""];
    [inputMenu addItem:inputItemWiimote];
    
    [[self inputPopupButton] setMenu:inputMenu];
    [[self inputPopupButton] selectItemWithTag:[[NSUserDefaults standardUserDefaults] integerForKey:OELastControlsDeviceTypeKey]];
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
    [self OE_setupPlayerMenuForNumberOfPlayers:[systemController numberOfPlayers]];
    
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
    [self OE_setupControllerImageViewWithTransition:(order == NSOrderedDescending ? kCATransitionFromLeft : kCATransitionFromRight)];
}

- (void)OE_setupControllerImageViewWithTransition:(NSString *)transition;
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
    
    if(controllerView != nil)
        [[[self controllerContainerView] animator] replaceSubview:controllerView with:newControllerView];
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
    NSInteger bindingType = 0;
    if(sender && [sender respondsToSelector:@selector(selectedTag)])
        bindingType = [sender selectedTag];
    else if(sender && [sender respondsToSelector:@selector(tag)])
        bindingType = [sender tag];
    
    [self setSelectedBindingType:bindingType];
}

- (IBAction)changeInputControl:(id)sender
{
    if(sender == [self controllerView] || sender == [self controlsSetupView])
        [self setSelectedKey:[sender selectedKey]];
}

- (IBAction)searchForWiimote:(id)sender
{
    [self OE_setupInputMenu];
    
    // TODO: add dialog with additional instructions
    
    // Start WiiRemote support
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEWiimoteSupportEnabled])
        [[OEDeviceManager sharedDeviceManager] startWiimoteSearch];
}

- (void)setSelectedKey:(NSString *)value
{
    if(selectedKey == value) value = nil;
    
    selectedKey = [value copy];
    [CATransaction begin];
    [[self controlsSetupView] setSelectedKey:selectedKey];
    [[self controllerView]    setSelectedKey:selectedKey animated:YES];
    [CATransaction commit];
    
    if(selectedKey != nil) [[[self view] window] makeFirstResponder:[self view]];
}

- (void)setSelectedBindingType:(NSInteger)value
{
    if(selectedBindingType != value)
    {
        selectedBindingType = value;
        [[self inputPopupButton] selectItemWithTag:selectedBindingType];
        [[NSUserDefaults standardUserDefaults] setInteger:selectedBindingType forKey:OELastControlsDeviceTypeKey];
    }
}

- (void)setSelectedPlayer:(NSUInteger)value
{
    if(selectedPlayer != value)
    {
        selectedPlayer = value;
        [[self playerPopupButton] selectItemWithTag:value];
        [[NSUserDefaults standardUserDefaults] setInteger:selectedPlayer forKey:OELastControlsPlayerKey];
    }
}

- (void)OE_setCurrentBindingsForEvent:(OEHIDEvent *)anEvent;
{
    if([anEvent type] == OEHIDEventTypeKeyboard)
        [self setSelectedBindingType:0];
    else
    {
        NSUInteger playerNumber = [[self currentSystemBindings] playerNumberForEvent:anEvent];
        
        [self setSelectedBindingType:1];
        [self setSelectedPlayer:playerNumber];
    }
}

#pragma mark -
#pragma mark Input and bindings management methods

- (BOOL)isKeyboardEventSelected
{
    return selectedBindingType == 0;
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
    if([readingEvent hasOffState] || ([readingEvent cookie] != [anEvent cookie])) readingEvent = nil;
    
    if([self selectedKey] == nil && [self view] == [[[self view] window] firstResponder])
        [[[self view] window] makeFirstResponder:nil];
    
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
    
    // The event is the currently read event,
    // if it's off state, nil the reading event,
    // in either case, this event shouldn't be registered
    if(readingEvent == anEvent)
    {
        if([anEvent hasOffState])
            readingEvent = nil;
        
        return NO;
    }
    
    if(![anEvent hasOffState]) [ignoredEvents addObject:anEvent];
    
    return NO;
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if([self OE_shouldRegisterEvent:anEvent])
    {
        [self setSelectedBindingType:1];
        [self registerEvent:anEvent];
    }
}

- (void)triggerPull:(OEHIDEvent *)anEvent;
{
    if([self OE_shouldRegisterEvent:anEvent])
    {
        [self setSelectedBindingType:1];
        [self registerEvent:anEvent];
    }
}

- (void)triggerRelease:(OEHIDEvent *)anEvent;
{
    [self OE_shouldRegisterEvent:anEvent];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    if([self OE_shouldRegisterEvent:anEvent])
    {
        [self setSelectedBindingType:1];
        [self registerEvent:anEvent];
    }
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    [self OE_shouldRegisterEvent:anEvent];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    if([self OE_shouldRegisterEvent:anEvent])
    {
        [self setSelectedBindingType:1];
        [self registerEvent:anEvent];
    }
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    if([self OE_shouldRegisterEvent:anEvent])
    {
        [self setSelectedBindingType:0];
        [self registerEvent:anEvent];
    }
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
    return NSMakeSize(561, 543);
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
