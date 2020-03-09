// Copyright (c) 2020, OpenEmu Team
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

extension NSPasteboard.PasteboardType {
    static let game = NSPasteboard.PasteboardType("org.openemu.game")
}

class SidebarController: NSViewController {
    
    @IBOutlet var sidebarView: NSOutlineView!
    
    var database: OELibraryDatabase? {
        didSet {
            reloadData()
            
            guard
                !lastSidebarSelection.isEmpty,
                let item = lastSidebarSelectionItem
                else { return }
            
            selectItem(item: item)
        }
    }
    
    @UserDefault(.lastSidebarSelection, defaultValue: "")
    var lastSidebarSelection: String
    
    var lastSidebarSelectionItem: OESidebarItem? {
        systems.first(where: { $0.sidebarID == lastSidebarSelection }) ?? collections.first(where: { $0.sidebarID == lastSidebarSelection})
    }
    
    var groups = [
        SidebarGroupItem(name: NSLocalizedString("Consoles", comment: ""),
                    autosaveName: .sidebarConsolesItem),
        SidebarGroupItem(name: NSLocalizedString("Collections", comment: ""),
                    autosaveName: .sidebarCollectionsItem)
    ]
    
    var systems: [OESidebarItem] = []
    var collections: [OESidebarItem] = []
    
    var selectedSidebarItem: OESidebarItem? {
        let item = sidebarView.item(atRow: sidebarView.selectedRow)
        precondition(item == nil || item is OESidebarItem, "All sidebar items must confirm to OESidebarItem")
        return item as? OESidebarItem
    }
    
    private var token: NSObjectProtocol?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        sidebarView.register(NSNib(nibNamed: "SidebarHeaderView", bundle: nil), forIdentifier: Self.headerViewIdentifier)
        sidebarView.register(NSNib(nibNamed: "SidebarItemView", bundle: nil), forIdentifier: Self.itemViewIdentifier)
        
        sidebarView.registerForDraggedTypes([.fileURL, .game])
        sidebarView.expandItem(nil, expandChildren: true)
        database = OELibraryDatabase.default
        
        let menu = NSMenu()
        menu.addItem(NSMenuItem(title: "Item One", action: nil, keyEquivalent: ""))
        menu.addItem(NSMenuItem(title: "Item Two", action: nil, keyEquivalent: ""))
        menu.delegate = self
        sidebarView.menu = menu
        
        token = NotificationCenter.default.addObserver(forName: .OEDBSystemAvailabilityDidChange, object: nil, queue: .main) { [weak self] _ in
            guard let self = self else { return }
            
            self.reloadDataAndPreserveSelection()
        }
    }
    
    func reloadDataAndPreserveSelection() {
        let possibleItem = self.selectedSidebarItem
        let previousRow  = self.sidebarView.selectedRow
        
        reloadData()
        
        guard let previousItem = possibleItem else { return }
        
        var rowToSelect = previousRow
        let reloadedRowForPreviousItem = sidebarView.row(forItem: previousItem)
        
        if reloadedRowForPreviousItem == -1 {
            func isSelectable(i: Int) -> Bool {
                guard let item = sidebarView.item(atRow: i) else { return false }
                return outlineView(sidebarView, shouldSelectItem: item)
            }
            
            rowToSelect =
                // find a row before the previously selected one
                (0...rowToSelect).reversed().first(where: isSelectable) ??
                
                // find a row after the previously selected one
                (rowToSelect..<sidebarView.numberOfRows).first(where: isSelectable) ??
                
                // or nothing
                NSNotFound
            
        } else if reloadedRowForPreviousItem != previousRow {
            rowToSelect = reloadedRowForPreviousItem
        }
        
        guard rowToSelect != NSNotFound else { return }
        
        sidebarView.selectRowIndexes([rowToSelect], byExtendingSelection: false)
        outlineViewSelectionDidChange(Notification(name: NSOutlineView.selectionDidChangeNotification))
    }
    
    func reloadData() {
        guard let database = database else { return }
        
        systems     = OEDBSystem.enabledSystemsinContext(database.mainThreadContext) ?? []
        collections = database.collections
        sidebarView.reloadData()
    }
    
    func selectItem(item: OESidebarItem) {
        guard item.isSelectableInSidebar else { return }
        
        let index = sidebarView.row(forItem: item)
        guard index > -1 else { return }
        
        if sidebarView.selectedRow != index {
            sidebarView.selectRowIndexes([index], byExtendingSelection: false)
        }
    }
}

// MARK: - Delegate

extension SidebarController: NSOutlineViewDelegate {
    func outlineViewSelectionDidChange(_ notification: Notification) {
        print("\(#function)")
    }
    
    func outlineView(_ outlineView: NSOutlineView, shouldCollapseItem item: Any) -> Bool {
        false
    }
    
    func outlineView(_ outlineView: NSOutlineView, shouldExpandItem item: Any) -> Bool {
        true
    }
}

// MARK: - DataSource

extension NSUserInterfaceItemIdentifier: ExpressibleByStringLiteral {
    public init(stringLiteral: String) {
        self.init(stringLiteral)
    }
}

extension SidebarController: NSOutlineViewDataSource {
    static let headerViewIdentifier: NSUserInterfaceItemIdentifier = "SidebarHeaderView"
    static let itemViewIdentifier: NSUserInterfaceItemIdentifier = "SidebarItemView"
    static let rowViewIdentifier: NSUserInterfaceItemIdentifier = "SidebarRowView"
    
    func outlineView(_ outlineView: NSOutlineView, child index: Int, ofItem item: Any?) -> Any {
        switch item {
        case let group as SidebarGroupItem where group.autosaveName == .sidebarConsolesItem:
            return systems[index]
            
        case let group as SidebarGroupItem where group.autosaveName == .sidebarCollectionsItem:
            return collections[index]
            
        default:
            return groups[index]
        }
    }
    
    func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
        (item as? OESidebarItem)?.isGroupHeaderInSidebar ?? false
    }
    
    func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int {
        if item == nil {
            return groups.count
        }

        guard
            let _ = database,
            let item = item as? SidebarGroupItem
            else { return 0 }
        
        
        switch item.autosaveName {
        case .sidebarConsolesItem:
            return systems.count
        case .sidebarCollectionsItem:
            return collections.count
        }
    }
    
    func outlineView(_ outlineView: NSOutlineView, objectValueFor tableColumn: NSTableColumn?, byItem item: Any?) -> Any? {
        (item as? OESidebarItem)?.sidebarName
    }
    
    func outlineView(_ outlineView: NSOutlineView, viewFor tableColumn: NSTableColumn?, item: Any) -> NSView? {
        guard let item = item as? OESidebarItem else { return nil }
        
        var view: NSTableCellView?
        
        if item.isGroupHeaderInSidebar {
            view = outlineView.makeView(withIdentifier: Self.headerViewIdentifier, owner: self) as? NSTableCellView
            
        } else {
            view = outlineView.makeView(withIdentifier: Self.itemViewIdentifier, owner: self) as? NSTableCellView
            view?.imageView?.image = item.sidebarIcon
            view?.textField?.isSelectable = false
            view?.textField?.isEditable = item.isEditableInSidebar
        }
        
        view?.textField?.stringValue = item.sidebarName
        
        return view
    }
    
    func outlineView(_ outlineView: NSOutlineView, shouldShowOutlineCellForItem item: Any) -> Bool {
        false
    }
    
    func outlineView(_ outlineView: NSOutlineView, heightOfRowByItem item: Any) -> CGFloat {
        switch item {
        case is SidebarGroupItem:
            return 26
        default:
            return 20
        }
    }
    
    func outlineView(_ outlineView: NSOutlineView, isGroupItem item: Any) -> Bool {
        item is SidebarGroupItem
    }
    
    func outlineView(_ outlineView: NSOutlineView, shouldSelectItem item: Any) -> Bool {
        !(item is SidebarGroupItem)
    }
    
    @IBAction func selectCores(_ sender: Any?) {
        guard let v = sender as? NSView else { return }
        
        let po = NSPopover()
        po.behavior = .transient
        po.contentViewController = AvailableLibrariesViewController()
        po.show(relativeTo: v.frame, of: sidebarView, preferredEdge: .maxX)
    }
}

// MARK: - Drag & Drop
extension SidebarController {
    
    func outlineView(_ outlineView: NSOutlineView, acceptDrop info: NSDraggingInfo, item: Any?, childIndex index: Int) -> Bool {
        false
    }
    
    func outlineView(_ outlineView: NSOutlineView, validateDrop info: NSDraggingInfo, proposedItem item: Any?, proposedChildIndex index: Int) -> NSDragOperation {
        .copy
    }
}

// MARK: - NSMenuDelegate

extension SidebarController: NSMenuDelegate {
    func menuNeedsUpdate(_ menu: NSMenu) {
        
    }
    
    func menu(_ menu: NSMenu, update item: NSMenuItem, at index: Int, shouldCancel: Bool) -> Bool {
        return true
    }
}

class SidebarGroupItem: NSObject, OESidebarItem {
    
    enum AutosaveName: String {
        case sidebarConsolesItem, sidebarCollectionsItem
    }
    
    var name: String
    var autosaveName: AutosaveName
    
    init(name: String, autosaveName: AutosaveName) {
        self.name = name
        self.autosaveName = autosaveName
    }
    
    var sidebarIcon: NSImage?
    var sidebarName: String {
        return self.name
    }
    var sidebarID: String?
    var viewControllerClassName: String?
    var isSelectableInSidebar: Bool = false
    var isEditableInSidebar: Bool = false
    var isGroupHeaderInSidebar: Bool = true
    var hasSubCollections: Bool = false
    
}

extension Key {
    static let lastSidebarSelection: Key = "lastSidebarSelection"
}
