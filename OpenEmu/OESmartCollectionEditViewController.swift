/*
 Copyright (c) 2016, OpenEmu Team
 
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

import Foundation

class OESmartCollectionEditViewController : NSViewController
{
    let DefaultSmartCollectionLimit = 25

    @IBOutlet weak var confirmButton: NSButton!
    @IBOutlet weak var limitButton: NSButton!
    @IBOutlet weak var limitInput: NSTextField!
    @IBOutlet weak var limitOrderButton: NSPopUpButton!
    @IBOutlet weak var predicateEditor: NSPredicateEditor!

    override func viewDidLoad() {
        super.viewDidLoad()

        predicateEditor.addRow(self)
        _popuplateGameSelectionButton()
        _updateUIFromCollection()
    }

    func _popuplateGameSelectionButton() {
        let menu = NSMenu()

        menu.addItem(_makeMenuItemWithTitle("Random", key: nil, ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("Game title", key: "gameTitle", ASC: true))
        menu.addItem(_makeMenuItemWithTitle("Name", key: "name", ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("Best rating", key: "rating", ASC: false))
        menu.addItem(_makeMenuItemWithTitle("Worst rating", key: "rating", ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("most recently played", key: "@max.roms.lastPlayed", ASC: false))
        menu.addItem(_makeMenuItemWithTitle("last recently played", key: "@max.roms.lastPlayed", ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("longest play time", key: "@max.roms.playTime", ASC: false))
        menu.addItem(_makeMenuItemWithTitle("shortest play time", key: "@max.roms.playTime", ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("most often played", key: "@sum.roms.playCount", ASC: false))
        menu.addItem(_makeMenuItemWithTitle("least often played", key: "@sum.roms.playCount", ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("most often played", key: "@sum.roms.playCount", ASC: false))
        menu.addItem(_makeMenuItemWithTitle("least often played", key: "@sum.roms.playCount", ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("most recently added", key: "importDate", ASC: false))
        menu.addItem(_makeMenuItemWithTitle("least recently added", key: "importDate", ASC: true))
        menu.addItem(NSMenuItem.separatorItem())
        menu.addItem(_makeMenuItemWithTitle("most recently synced", key: "lastInfoSync", ASC: false))
        menu.addItem(_makeMenuItemWithTitle("least recently synced", key: "lastInfoSync", ASC: true))

        limitOrderButton.menu = menu;
    }

    func _makeMenuItemWithTitle(title: String, key: String?, ASC: Bool) -> NSMenuItem {
        let menuItem = NSMenuItem.init()
        // TODO: localize title
        menuItem.title = title
        menuItem.representedObject = key
        menuItem.tag = ASC ? 0 : 1

        return menuItem
    }

    override var representedObject: AnyObject? {
        get {
            return super.representedObject as? OEDBSmartCollection
        }

        set {
            super.representedObject = newValue as? OEDBSmartCollection
            _updateUIFromCollection()
        }
    }

    func _updateUIFromCollection() {
        if limitButton == nil || self.representedObject == nil {
            return
        }

        let collection = self.representedObject as! OEDBSmartCollection

        limitButton.state = collection.fetchLimit == nil ? NSOffState : NSOnState
        limitInput.integerValue = collection.fetchLimit?.integerValue ?? DefaultSmartCollectionLimit;

        if let predicate = collection.fetchPredicate as? NSCompoundPredicate {
            predicateEditor.objectValue = predicate;
        }

        if let sortKey = collection.fetchSortKey,
            item = limitOrderButton.itemArray.filter({ (menuItem: NSMenuItem) -> Bool in
                return menuItem.representedObject as? String == sortKey && menuItem.tag == Int(collection.fetchSortAscending);
            }).first
        {
            limitOrderButton.selectItem(item)
        } else {
            limitOrderButton.selectItemAtIndex(0);
        }
    }

    // MARK: - UI Callbacks
    @IBAction func confirm (sender :AnyObject) {
        NSApp.stopModalWithCode(NSAlertSecondButtonReturn)
    }

    @IBAction func cancel(sender :AnyObject) {
        NSApp.stopModalWithCode(NSAlertFirstButtonReturn)
    }

    // MARK: - Accessing Configuration -
    func predicate() -> NSPredicate {
        if let predicate = predicateEditor.predicate {
            return predicate
        }

        return NSPredicate.init(value: true);
    }

    func fetchLimit() -> NSNumber? {
        return limitButton.enabled ? NSNumber(integer: limitInput.integerValue) : nil
    }

    func fetchOrder() -> String? {
        if let selectedItem = limitOrderButton.selectedItem,
                object = selectedItem.representedObject
        {
            return object as? String;
        }

        return nil
    }

    func fetchOrderIsAscending() -> Bool {
        return limitOrderButton.selectedItem?.tag == 1
    }
}