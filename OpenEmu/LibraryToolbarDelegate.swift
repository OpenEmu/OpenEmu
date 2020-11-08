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

import Cocoa

extension NSToolbarItem.Identifier {
    public static let oeGridSize = Self("OEToolbarGridSizeItem")
    public static let oeViewMode = Self("OEToolbarViewModeItem")
    public static let oeSearch   = Self("OEToolbarSearchItem")
    public static let oeCategory = Self("OEToolbarCategoryItem")
    public static let oeAdd      = Self("OEToolbarAddItem")
}

@objc(OELibraryToolbarDelegate)
class LibraryToolbarDelegate: NSObject, NSToolbarDelegate {
    var itemCache = [NSToolbarItem.Identifier: NSToolbarItem]()
    
    @IBOutlet weak var toolbarOwner: AnyObject?
    
    
    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return [.flexibleSpace,
                .oeAdd,
                .oeViewMode,
                .oeGridSize,
                .oeSearch,
                .oeCategory]
    }
    
    
    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        cacheItems(toolbar: toolbar as! LibraryToolbar)
        return [.oeViewMode,
                .oeGridSize,
                .oeCategory,
                .flexibleSpace,
                .oeSearch]
    }
    
    
    func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return []
    }
    
    
    func cacheItems(toolbar: LibraryToolbar) {
        gridSizeToolbarItem(toolbar: toolbar)
        viewModeToolbarItem(toolbar: toolbar)
        categoryToolbarItem(toolbar: toolbar)
        searchToolbarItem(toolbar: toolbar)
        addToolbarItem(toolbar: toolbar)
    }
    
    
    func toolbar(_ toolbar: NSToolbar, itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier, willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem? {
        
        switch itemIdentifier {
        case .oeGridSize:
            return gridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeViewMode:
            return viewModeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeCategory:
            return categoryToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeSearch:
            return searchToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeAdd:
            return addToolbarItem(toolbar: toolbar as! LibraryToolbar)
        default:
            return NSToolbarItem(itemIdentifier: itemIdentifier)
        }
    }
    
    
    func toolbarWillAddItem(_ notification: Notification) {
        if (notification.userInfo?["item"] as? NSToolbarItem)?.itemIdentifier == .oeAdd {
            UserDefaults.standard.set(true, forKey: GameScannerViewController.OESidebarHideBottomBarKey)
            NotificationCenter.default.post(name: GameScannerViewController.OESidebarBottomBarDidChange, object: nil)
        }
    }
    
    
    func toolbarDidRemoveItem(_ notification: Notification) {
        if (notification.userInfo?["item"] as? NSToolbarItem)?.itemIdentifier == .oeAdd {
            UserDefaults.standard.set(false, forKey: GameScannerViewController.OESidebarHideBottomBarKey)
            NotificationCenter.default.post(name: GameScannerViewController.OESidebarBottomBarDidChange, object: nil)
        }
    }
    
    
    @discardableResult
    func gridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeGridSize] {
            return item
        }
        
        let slider = NSSlider(value: 1.0, minValue: 0.5, maxValue: 2.5, target: toolbarOwner, action: #selector(LibraryController.changeGridSize(_:)))
        slider.controlSize = .small
        slider.widthAnchor.constraint(equalToConstant: 64).isActive = true
        
        let decreaseButton = NSButton(image: NSImage(named: "grid_slider_min")!, target: toolbarOwner, action: #selector(LibraryController.decreaseGridSize(_:)))
        decreaseButton.isBordered = false
        decreaseButton.toolTip = NSLocalizedString("Decrease Grid Size", comment:"Toolbar, tooltip")
        
        let increaseButton = NSButton(image: NSImage(named: "grid_slider_max")!, target: toolbarOwner, action: #selector(LibraryController.increaseGridSize(_:)))
        increaseButton.isBordered = false
        increaseButton.toolTip = NSLocalizedString("Increase Grid Size", comment:"Toolbar, tooltip")
        
        toolbar.gridSizeSlider = slider
        toolbar.decreaseGridSizeButton = decreaseButton
        toolbar.increaseGridSizeButton = increaseButton
        
        let item = NSToolbarItem()
        item.view = slider
        
        let minHint = NSToolbarItem()
        minHint.view = decreaseButton
        
        let maxHint = NSToolbarItem()
        maxHint.view = increaseButton
        
        let group = NSToolbarItemGroup(itemIdentifier: .oeGridSize)
        group.subitems = [minHint, item, maxHint]
        group.label = NSLocalizedString("Grid Size", comment:"Toolbar, grid size slider label")
        
        itemCache[group.itemIdentifier] = group
        return group;
    }
    
    
    @discardableResult
    func viewModeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeViewMode] {
            return item
        }
        
        let images = [NSImage(named: NSImage.iconViewTemplateName)!,
                      NSImage(named: NSImage.listViewTemplateName)!]
        
        let segmControl = NSSegmentedControl(images: images, trackingMode: .selectOne, target: toolbarOwner, action: #selector(LibraryController.switchToView(_:)))
        segmControl.setToolTip(NSLocalizedString("Switch To Grid View", comment: "Toolbar, tooltip"), forSegment: 0)
        segmControl.setToolTip(NSLocalizedString("Switch To List View", comment: "Toolbar, tooltip"), forSegment: 1)
        segmControl.setWidth(26, forSegment: 0)
        segmControl.setWidth(26, forSegment: 1)
        
        toolbar.viewModeSelector = segmControl
        
        let item = NSToolbarItem(itemIdentifier: .oeViewMode)
        item.view = segmControl
        item.visibilityPriority = .low
        item.label = NSLocalizedString("View Mode", comment:"Toolbar, view mode selector label")
        
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    
    @discardableResult
    func categoryToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeCategory] {
            return item
        }
        
        let titles = [NSLocalizedString("Toolbar: Library", value: "Library", comment: "toolbar, category label"),
                      NSLocalizedString("Toolbar: Save States", value: "Save States", comment: "toolbar, category label"),
                      NSLocalizedString("Toolbar: Screenshots", value: "Screenshots", comment: "toolbar, category label"),
                      NSLocalizedString("Toolbar: Homebrew", value: "Homebrew", comment: "toolbar, category label")]
        
        let segmControl = NSSegmentedControl(labels: titles, trackingMode: .selectOne, target: toolbarOwner, action: #selector(LibraryController.switchCategory(_:)))
        
        toolbar.categorySelector = segmControl
        
        let item = NSToolbarItem(itemIdentifier: .oeCategory)
        item.view = segmControl
        item.menuFormRepresentation = categoryMenu
        item.label = NSLocalizedString("Category", comment:"Toolbar, category selector label")
        
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    let categoryMenu: NSMenuItem = {
        
        let library = NSMenuItem()
        library.title = NSLocalizedString("Toolbar: Library", value: "Library", comment: "")
        library.tag = 100
        library.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let savesStates = NSMenuItem()
        savesStates.title = NSLocalizedString("Toolbar: Save States", value: "Save States", comment: "")
        savesStates.tag = 101
        savesStates.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let screenshots = NSMenuItem()
        screenshots.title = NSLocalizedString("Toolbar: Screenshots", value: "Screenshots", comment: "")
        screenshots.tag = 102
        screenshots.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let homebrew = NSMenuItem()
        homebrew.title = NSLocalizedString("Toolbar: Homebrew", value: "Homebrew", comment: "")
        homebrew.tag = 103
        homebrew.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let menu = NSMenu()
        menu.items = [library, savesStates, screenshots, homebrew]
        
        let categoryMenu = NSMenuItem()
        categoryMenu.title = NSLocalizedString("Category", comment:"")
        categoryMenu.submenu = menu
        
        return categoryMenu
    }()
    
    @discardableResult
    func searchToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeSearch] {
            return item
        }
        
        let searchField = OESearchField()
        searchField.lineBreakMode = .byClipping
        searchField.usesSingleLineMode = true
        searchField.cell?.isScrollable = true
        searchField.sendsWholeSearchString = false
        searchField.sendsSearchStringImmediately = true
        searchField.font = .systemFont(ofSize: 13.0)
        searchField.action = #selector(LibraryController.search(_:))
        searchField.target = toolbarOwner
        searchField.widthAnchor.constraint(lessThanOrEqualToConstant: 166).isActive = true
        if #available(macOS 11.0, *) {
            searchField.heightAnchor.constraint(equalToConstant: 28).isActive = true
        } else {
            searchField.heightAnchor.constraint(equalToConstant: 22).isActive = true
        }
        
        toolbar.searchField = searchField
        
        let item = NSToolbarItem(itemIdentifier: .oeSearch)
        item.view = searchField;
        item.label = NSLocalizedString("Search", comment:"Toolbar, search field label")
        
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    
    @discardableResult
    func addToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeAdd] {
            return item
        }
        
        let images = [NSImage(named: NSImage.addTemplateName)!]
        
        let segmControl = NSSegmentedControl(images: images, trackingMode: .momentary, target: toolbar, action: nil)
        segmControl.setWidth(28, forSegment: 0)
        segmControl.setMenu(addMenu, forSegment: 0)
        
        toolbar.addButton = segmControl
        
        let item = NSToolbarItem(itemIdentifier: .oeAdd)
        item.view = segmControl
        item.label = NSLocalizedString("Add", comment:"Toolbar, add button label")
        
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    let addMenu: NSMenu = {
        
        let addToLibrary = NSMenuItem()
        addToLibrary.title = NSLocalizedString("Add to Library…", comment: "")
        addToLibrary.action = #selector(LibraryController.addToLibrary(_:))
        
        let newCollection = NSMenuItem()
        newCollection.title = NSLocalizedString("New Collection", comment: "")
        newCollection.action = #selector(LibraryController.newCollection(_:))
        
        let menu = NSMenu()
        menu.items = [addToLibrary,
                      NSMenuItem.separator(),
                      newCollection]
        
        return menu
    }()
}
