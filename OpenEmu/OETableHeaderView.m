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

#import "OETableHeaderView.h"
#import "OETableView.h"
#import "OEMenu.h"

@interface OETableHeaderView()
- (void)OE_updateHeaderState:(id)sender;
@end

@implementation OETableHeaderView
- (NSMenu *)menuForEvent:(NSEvent *)event
{
    [[self window] makeFirstResponder:self];
    OETableView *tableView = (OETableView *) [self tableView];

    NSMenu *menu = [[NSMenu alloc] init];
    NSMenuItem *menuItem;
    NSDictionary *headerState = [tableView headerState];
    if(headerState == nil)
        headerState = [tableView defaultHeaderState];

    for(NSTableColumn *column in [[self tableView] tableColumns])
    {
        NSCell *headerCell = [column headerCell];
        if(![[headerCell stringValue] isEqualToString:@""])
        {
            menuItem = [[NSMenuItem alloc] initWithTitle:[headerCell stringValue] action:@selector(OE_updateHeaderState:) keyEquivalent:@""];
            [menuItem setRepresentedObject:column];
        
            if([[headerState valueForKey:[column identifier]] boolValue])
                [menuItem setState:NSOffState];
            else
                [menuItem setState:NSOnState];
        
            [menu addItem:menuItem];
        }
    }

    OEMenuStyle style = OEMenuStyleDark;
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEMenuOptionsStyleKey]) style = OEMenuStyleLight;

    NSDictionary *options = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInteger:style] forKey:OEMenuOptionsStyleKey];
    [OEMenu openMenu:menu withEvent:event forView:self options:options];
    
    return nil;
}

- (void)OE_updateHeaderState:(id)sender
{
    OETableView *tableView = (OETableView *) [self tableView];
    NSTableColumn *column = [sender representedObject];
    NSDictionary *oldHeaderState = [tableView headerState];
    if(oldHeaderState == nil)
        oldHeaderState = [tableView defaultHeaderState];
    
    NSMutableDictionary *newHeaderState = [[NSMutableDictionary alloc] initWithDictionary:oldHeaderState];
    BOOL newState = ![[newHeaderState valueForKey:[column identifier]] boolValue];

    [newHeaderState setValue:[NSNumber numberWithBool:newState] forKey:[column identifier]];

    [tableView setHeaderState:newHeaderState];
}

@end
