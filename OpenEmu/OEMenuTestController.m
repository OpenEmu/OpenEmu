//
//  OEMenuTestController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 07.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEMenuTestController.h"
#import "OECompositionPlugin.h"

#import "OELibraryDatabase.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"
@implementation OEMenuTestController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    
    [self loadWindow];
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(!self) return nil;
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (NSString *)windowNibName
{
    return @"OEMenuTest";
}

- (void)awakeFromNib
{
    // Setup plugins menu
    NSArray *filterPlugins = [[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    // These filters are loaded and run by GL, and do not rely on QTZs
    NSArray *filterNames = [filterPlugins arrayByAddingObjectsFromArray:OEOpenGLFilterNameArray];
    NSMenu *filterMenu = [[NSMenu alloc] init];
    for(NSString *aName in filterNames)
    {
        [filterMenu addItemWithTitle:aName action:NULL keyEquivalent:@""];
    }
    [[self filterSelection] setMenu:filterMenu];
    [filterMenu release];	
    
    [[self window] setOpaque:NO];
    [[self window] setHasShadow:NO];
    
    NSMenu *consolesMenu = [[NSMenu alloc] init];
    
    NSArray *enabledSystems = [[OELibraryDatabase defaultDatabase] enabledSystems]; 
    
    for(OEDBSystem *system in enabledSystems)
    {
        OESystemPlugin *plugin = [system plugin];
        
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:[plugin systemName] action:@selector(changeSystem:) keyEquivalent:@""];
        [item setTarget:self];
        [item setRepresentedObject:[plugin systemIdentifier]];
        
        [item setImage:[NSImage imageNamed:[item title]]];
        
        [consolesMenu addItem:item];
        [item release];
    }
    [[self consolesPopupButton] setMenu:consolesMenu];
    [consolesMenu release];
}

- (IBAction)buttonAction:(id)sender
{    
    OERectEdge rectEdge;
    if(sender == [self topButton])
        rectEdge = OEMaxYEdge;
    else if(sender == [self bottomButton])
        rectEdge = OEMinYEdge;
    else if(sender == [self leftButton])
        rectEdge = OEMinXEdge;
    else if(sender == [self rightButton])
        rectEdge = OEMaxXEdge;
    else if(sender == [self centerButton])
        rectEdge = OENoEdge;
    
    OEMenu* menu = [[self testMenu] convertToOEMenu];
    [menu setStyle:[[self styleSelection] selectedTag]==1?OEMenuStyleDark:OEMenuStyleLight];
    
    NSRect rect = [[[self window] contentView] convertRect:[[self centerButton] bounds] fromView:[self centerButton]];
    [menu openOnEdge:rectEdge ofRect:rect ofWindow:[self window]];
}
- (IBAction)styleAction:(id)sender
{
}

- (IBAction)toggleWindowBackground:(id)sender
{
    if([[self window] backgroundColor] == [NSColor clearColor])
        [[self window] setBackgroundColor:[NSColor windowBackgroundColor]];
    else
        [[self window] setBackgroundColor:[NSColor clearColor]];
}

@synthesize centerButton, topButton, leftButton, rightButton, bottomButton;
@synthesize styleSelection;
@synthesize testMenu;
@synthesize filterSelection, consolesPopupButton;
@end
