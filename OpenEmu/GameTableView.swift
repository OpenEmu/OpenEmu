// Copyright (c) 2019, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import Cocoa

protocol GameTableViewMenuSource {
    func tableView(_ tableView: GameTableView, menuForItemsAt indexes: IndexSet) -> NSMenu
}

extension NSUserInterfaceItemIdentifier {
    static let gameTableViewTitleColumn = NSUserInterfaceItemIdentifier(rawValue: "listViewTitle")
}

@objc(OEGameTableView)
class GameTableView: OETableView {
    var headerState: [String: Bool] = [:] {
        didSet {
            for column in tableColumns {
                column.isHidden = headerState[column.identifier.rawValue] ?? false
            }
        }
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
        
        headerState = [String: Bool](uniqueKeysWithValues: tableColumns.map({ ($0.identifier.rawValue, false) }))
        
        for column in tableColumns {
            switch column.identifier {
            case "listViewStatus":
                column.headerCell.title = ""
            case "listViewTitle":
                column.headerCell.title = NSLocalizedString("Name", comment: "Game table, column header")
            case "listViewRating":
                column.headerCell.title = NSLocalizedString("Rating", comment: "Game table, column header")
            case "listViewLastPlayed":
                column.headerCell.title = NSLocalizedString("Last Played", comment: "Game table, column header")
            case "listViewConsoleName":
                column.headerCell.title = NSLocalizedString("System", comment: "Game table, column header")
            case "listViewSaveStateCount":
                column.headerCell.title = NSLocalizedString("Save Games", comment: "Game table, column header")
            case "listViewPlayCount":
                column.headerCell.title = NSLocalizedString("Play Count", comment: "Game table, column header")
            case "listViewPlayTime":
                column.headerCell.title = NSLocalizedString("Play Time", comment: "Game table, column header")
            default:
                break
            }
        }
    }
    
    override func menu(for event: NSEvent) -> NSMenu? {
        window?.makeFirstResponder(self)
        guard let del = dataSource as? GameTableViewMenuSource else { return super.menu(for: event) }
        
        let locInWindow = event.locationInWindow
        let locInView   = convert(locInWindow, from: nil)
        
        let index = row(at: locInView)
        guard index > -1 else { return super.menu(for: event) }
        
        let itemIsSelected = selectedRowIndexes.contains(index)
        let indexes = itemIsSelected ? selectedRowIndexes : IndexSet(arrayLiteral: index)
        if !itemIsSelected {
            selectRowIndexes(indexes, byExtendingSelection: false)
        }
        
        let menu = del.tableView(self, menuForItemsAt: indexes)
        NSMenu.popUpContextMenu(menu, with: event, for: self)
        
        return nil
    }
    
    override func mouseDown(with event: NSEvent) {
        // AppKit posts a control-mouse-down event when the user control-clicks the view and -menuForEvent: returns nil
        // since a nil return normally means there is no contextual menu.
        // However, we do show a menu before returning nil from -menuForEvent:, so we need to ignore control-mouse-down events.
        if !event.modifierFlags.contains(.control) {
            super.mouseDown(with: event)
        }
    }
    
    override func cancelOperation(_ sender: Any?) {
        guard let _ = currentEditor() as? NSTextView else { return }
        abortEditing()
        window?.makeFirstResponder(self)
    }
    
    // MARK: - Context Menu
    
    @objc func beginEditingWithSelectedItem(_ sender: Any?) {
        guard selectedRowIndexes.count == 1 else { return }
        
        let titleColIndex = column(withIdentifier: .gameTableViewTitleColumn)
        precondition(titleColIndex != -1, "The list view must have a column identified by listViewTitle")
        
        editColumn(titleColIndex, row: selectedRowIndexes.first!, with: nil, select: true)
    }
}

@objc(OEGameTableHeaderView)
class GameTableHeaderView: NSTableHeaderView {
    override func menu(for event: NSEvent) -> NSMenu? {
        window?.makeFirstResponder(self)
        
        guard let tableView = tableView as? GameTableView else { return super.menu(for: event) }
        let headerState = tableView.headerState
        
        let menu = NSMenu()
        for column in tableView.tableColumns {
            let cell = column.headerCell
            guard !cell.stringValue.isEmpty else { continue }
            
            let menuItem = NSMenuItem(title: cell.stringValue, action: #selector(updateHeaderState(_:)), keyEquivalent: "")
            menuItem.representedObject = column
            if let v = headerState[column.identifier.rawValue], v == true {
                menuItem.state = .off
            } else {
                menuItem.state = .on
            }
            
            menu.addItem(menuItem)
        }
        
        NSMenu.popUpContextMenu(menu, with: event, for: self)
        
        return nil
    }
    
    @objc private func updateHeaderState(_ sender: NSMenuItem) {
        guard
            let tableView = tableView as? GameTableView,
            let column = sender.representedObject as? NSTableColumn
            else { return }
        
        var newHeaderState = tableView.headerState
        var newState       = newHeaderState[column.identifier.rawValue] ?? false
        newState.toggle()
        newHeaderState[column.identifier.rawValue] = newState
        tableView.headerState = newHeaderState
    }
}
