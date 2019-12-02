/*
Copyright (c) 2012, OpenEmu Team

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

#import "OECheatsSettingViewController.h"
#import "OEGameDocument.h"
#import "OEDBRom.h"
#import "OEDBSaveCheat.h"
#import "OpenEmu-Swift.h"


static NSString *const OECheatsSettingTableColumnIdentifierEnabled = @"OECheatsSettingTableColumnIdentifierEnabled";
static NSString *const OECheatsSettingTableColumnIdentifierDescription = @"OECheatsSettingTableColumnIdentifierDescription";
static NSString *const OECheatsSettingTableColumnIdentifierType = @"OECheatsSettingTableColumnIdentifierType";

@interface _CheatsTextViewWithPlaceHolder: NSTextView
@property (copy) NSString *placeHolderString;
@end

@implementation _CheatsTextViewWithPlaceHolder

- (BOOL)becomeFirstResponder {
  [self setNeedsDisplay:YES];
  return [super becomeFirstResponder];
}

- (void)drawRect:(NSRect)rect {
  [super drawRect:rect];
    if ([[self string] isEqualToString:@""] && self != [[self window] firstResponder] && _placeHolderString != nil && _placeHolderString.length > 0) {
        NSEdgeInsets edge = NSEdgeInsetsMake(0, 5, 0, 5);
        NSRect resultRect = NSMakeRect(rect.origin.x + edge.left, rect.origin.y + edge.bottom, rect.size.width - edge.left - edge.right, rect.size.height - edge.top - edge.bottom);
        [[[NSAttributedString alloc] initWithString:_placeHolderString attributes:@{NSForegroundColorAttributeName: [NSColor grayColor]}] drawInRect:resultRect];
    }
}

- (BOOL)resignFirstResponder {
   [self setNeedsDisplay:YES];
   return [super resignFirstResponder];
}

@end


@interface OECheatsSettingViewController ()<NSTableViewDelegate, NSTableViewDataSource, NSTextViewDelegate, NSTextFieldDelegate, NSMenuDelegate>

@property (strong) IBOutlet NSScrollView *tableViewBorderedScrollView;
@property (strong) IBOutlet NSClipView *tableViewClipView;
@property (strong) IBOutlet NSTableView *tableView;
@property (strong) IBOutlet NSTableColumn *enabledColumn;
@property (strong) IBOutlet NSTableColumn *descriptionColumn;
@property (strong) IBOutlet NSTableColumn *typeColumn;
@property (strong) IBOutlet NSButton *closeButton;
@property (strong) IBOutlet NSButton *addButton;
@property (strong) IBOutlet NSButton *removeButton;
@property (strong) IBOutlet NSTextField *descriptionTextField;
@property (strong) IBOutlet _CheatsTextViewWithPlaceHolder *codeTextView;
@property (strong) IBOutlet _CheatsTextViewWithPlaceHolder *notesTextView;
@property (strong) IBOutlet NSMenu *typeColumnMenu;

@property (copy) NSMutableArray<OEDBSaveCheat *> *cheats;
@property (copy) NSArray<NSString *> *availableTypes;

@end

@implementation OECheatsSettingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = NSLocalizedString(@"Cheat Setting", @"");
    
    _availableTypes = @[@"Unknown"];
    
    _tableView.delegate = self;
    _tableView.dataSource = self;
    _tableView.focusRingType = NSFocusRingTypeNone;
    
    _descriptionTextField.delegate = self;
    _codeTextView.delegate = self;
    _notesTextView.delegate = self;
    
    _enabledColumn.identifier = OECheatsSettingTableColumnIdentifierEnabled;
    _descriptionColumn.identifier = OECheatsSettingTableColumnIdentifierDescription;
    _typeColumn.identifier = OECheatsSettingTableColumnIdentifierType;
    
    [_availableTypes enumerateObjectsUsingBlock:^(NSString * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [_typeColumnMenu addItemWithTitle:obj action:nil keyEquivalent:@""];
    }];
    
    NSMenu *menu = [[NSMenu alloc] init];
    menu.delegate = self;
    _tableView.menu = menu;
}

- (void)viewWillAppear {
    [super viewWillAppear];
    _cheats = self.document.rom.saveCheats.allObjects.mutableCopy;
    [_tableView reloadData];
}

#pragma mark - Actions
- (IBAction)addCheat:(id)sender {
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
    OEDBSaveCheat *cheat = [OEDBSaveCheat createSaveCheatIdentifier:[NSUUID UUID]
                                                        description:@""
                                                               type:_availableTypes.firstObject
                                                               code:@""
                                                              notes:@""
                                                            enabled:NO
                                                             forRom:self.document.rom
                                                          inContext:context];
    [_cheats addObject:cheat];
    [_tableView reloadData];
    [_tableView selectRowIndexes:[NSIndexSet indexSetWithIndex:_cheats.count - 1] byExtendingSelection:NO];
}

- (IBAction)removeCheat:(id)sender {
    [self removeCheatAtIndex:_tableView.selectedRow];
}

- (void)removeCheatByMenu {
    [self removeCheatAtIndex:_tableView.clickedRow];
}

- (void)removeCheatAtIndex:(NSInteger)index {
    if (index >= 0 && index < _cheats.count) {
        OEDBSaveCheat *cheat = _cheats[index];
        [_cheats removeObjectAtIndex:index];
        [_tableView reloadData];
        [cheat delete];
        if (cheat.enabled) {
            [self.document setCheat:cheat.code withType:cheat.type enabled:NO];
        }
    }
}

- (IBAction)closeWindow:(id)sender {
    [self.view.window close];
}


#pragma mark - NSTableViewDelegate & NSTableViewDataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [_cheats count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    if (row < 0 || row >= _cheats.count) { return nil; }
    NSString *columnIdentifier = tableColumn.identifier;
    OEDBSaveCheat *cheat = _cheats[row];
    if (columnIdentifier == OECheatsSettingTableColumnIdentifierEnabled) {
        return @(cheat.enabled);
    } else if (columnIdentifier == OECheatsSettingTableColumnIdentifierDescription) {
        return cheat.codeDescription;
    } else if (columnIdentifier == OECheatsSettingTableColumnIdentifierType) {
        return @([_availableTypes indexOfObject:cheat.type]);
    } else {
        return nil;
    }
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    if (row < 0 || row >= _cheats.count) { return; }
    NSString *columnIdentifier = tableColumn.identifier;
    OEDBSaveCheat *cheat = _cheats[row];
    if (columnIdentifier == OECheatsSettingTableColumnIdentifierEnabled) {
        if ([object isKindOfClass:NSNumber.class]) {
            cheat.enabled = [object boolValue];
            [cheat save];
            [self.document setCheat:cheat.code withType:cheat.type enabled:cheat.enabled];
        }
    } else if (columnIdentifier == OECheatsSettingTableColumnIdentifierDescription) {
        if ([object isKindOfClass:NSString.class]) {
            cheat.codeDescription = object;
            [cheat save];
        }
    } else if (columnIdentifier == OECheatsSettingTableColumnIdentifierType) {
        if ([object isKindOfClass:NSNumber.class]) {
            NSString *type = _availableTypes[[object integerValue]];
            if (cheat.enabled) {
                [self.document setCheat:cheat.code withType:cheat.type enabled:NO];
                [self.document setCheat:cheat.code withType:type enabled:YES];
            }
            cheat.type = type;
            [cheat save];
        }
    }
}


- (BOOL)tableView:(NSTableView *)tableView shouldSelectTableColumn:(NSTableColumn *)tableColumn {
    return NO;
}

- (void)tableViewSelectionIsChanging:(NSNotification *)notification {
    [self tableviewSelectChangeAction];
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
    [self tableviewSelectChangeAction];
}

- (void)tableviewSelectChangeAction {
    BOOL enable = _tableView.selectedRow >= 0 && _tableView.selectedRow < _cheats.count;
    _codeTextView.editable = enable;
    _codeTextView.selectable = enable;
    _notesTextView.editable = enable;
    _notesTextView.selectable = enable;
    _descriptionTextField.editable = enable;
    _descriptionTextField.selectable = enable;
    OEDBSaveCheat *cheat = enable ? _cheats[_tableView.selectedRow] : nil;
    _codeTextView.string = cheat.code ?: @"";
    _notesTextView.string = cheat.notes ?: @"";
    _descriptionTextField.stringValue = cheat.codeDescription ?: @"";
    _descriptionTextField.placeholderString = enable ? NSLocalizedString(@"Cheat Description", @"") : @"";
    _codeTextView.placeHolderString = enable ? NSLocalizedString(@"Join multi-line cheats with '+' e.g. 000-000+111-111", @"") : @"";
    _notesTextView.placeHolderString = enable ? NSLocalizedString(@"Notes", @"") : @"";
}

#pragma mark - NSTextViewDelegate
- (void)textDidChange:(NSNotification *)notification {
    if (_tableView.selectedRow >= 0 && _tableView.selectedRow < _cheats.count) {
        OEDBSaveCheat *cheat = _cheats[_tableView.selectedRow];
        if (notification.object == _codeTextView) {
            cheat.code = _codeTextView.string;
        } else if (notification.object == _notesTextView) {
            cheat.notes = _notesTextView.string;
        }
    }
}

- (void)textDidEndEditing:(NSNotification *)notification {
    if (notification.object != _codeTextView) return;
    if (_tableView.selectedRow >= 0 && _tableView.selectedRow < _cheats.count) {
        OEDBSaveCheat *cheat = _cheats[_tableView.selectedRow];
        [cheat save];
        if (cheat.enabled) {
            [self.document setCheat:cheat.code withType:cheat.type enabled:cheat.enabled];
        }
    }
}

#pragma mark - NSTextFieldDelegate
- (void)controlTextDidChange:(NSNotification *)notification {
    if (_tableView.selectedRow >= 0 && _tableView.selectedRow < _cheats.count) {
        if (notification.object == _descriptionTextField) {
            OEDBSaveCheat *cheat = _cheats[_tableView.selectedRow];
            cheat.codeDescription = _descriptionTextField.stringValue;
            [cheat save];
            [_tableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:_tableView.selectedRow]
                                  columnIndexes:[NSIndexSet indexSetWithIndex:1]];
        }
    }
}

#pragma mark - NSMenuDelegate
- (void)menuNeedsUpdate:(NSMenu *)menu {
    [menu removeAllItems];
    if (_tableView.clickedRow >= 0) {
        [menu addItem:[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Delete", @"") action:@selector(removeCheatByMenu) keyEquivalent:@""]];
    }
}

#pragma mark - Updating UI States
@end
