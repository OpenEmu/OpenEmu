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

public extension NSToolbarItem.Identifier {
    static let oeGridSize = NSToolbarItem.Identifier("OEToolbarGridSizeItem")
    static let oeViewMode = NSToolbarItem.Identifier("OEToolbarViewModeItem")
    static let oeSearch   = NSToolbarItem.Identifier("OEToolbarSearchItem")
    static let oeCategory = NSToolbarItem.Identifier("OEToolbarCategoryItem")
    static let oeAdd      = NSToolbarItem.Identifier("OEToolbarAddItem")
}

final class LibraryToolbarDelegate: NSObject, NSToolbarDelegate {
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
        if #available(macOS 11.0, *) {
            return [.oeViewMode,
                    .oeCategory,
                    .flexibleSpace,
                    .oeSearch]
        } else {
            return [.oeViewMode,
                    .oeGridSize,
                    .oeCategory,
                    .flexibleSpace,
                    .oeSearch]
        }
    }
    
    func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return []
    }
    
    func cacheItems(toolbar: LibraryToolbar) {
        gridSizeToolbarItem(toolbar: toolbar)
        viewModeToolbarItem(toolbar: toolbar)
        categoryToolbarItem(toolbar: toolbar)
        if #available(macOS 11.0, *) {
            searchToolbarItem11(toolbar: toolbar)
        } else {
            searchToolbarItem(toolbar: toolbar)
        }
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
            if #available(macOS 11.0, *) {
                return searchToolbarItem11(toolbar: toolbar as! LibraryToolbar)
            } else {
                return searchToolbarItem(toolbar: toolbar as! LibraryToolbar)
            }
        case .oeAdd:
            return addToolbarItem(toolbar: toolbar as! LibraryToolbar)
        default:
            return NSToolbarItem(itemIdentifier: itemIdentifier)
        }
    }
    
    // MARK: - Grid Size
    
    @discardableResult
    func gridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeGridSize] {
            return item
        }
        
        let slider = GridSizeSlider()
        slider.target = toolbarOwner
        slider.slider.action = #selector(LibraryController.changeGridSize(_:))
        slider.minButton.action = #selector(LibraryController.decreaseGridSize(_:))
        slider.maxButton.action = #selector(LibraryController.increaseGridSize(_:))
        
        slider.minButton.toolTip = NSLocalizedString("Decrease Grid Size", comment:"Toolbar, tooltip")
        slider.maxButton.toolTip = NSLocalizedString("Increase Grid Size", comment:"Toolbar, tooltip")
        
        toolbar.gridSizeSlider = slider.slider
        toolbar.decreaseGridSizeButton = slider.minButton
        toolbar.increaseGridSizeButton = slider.maxButton
        
        let item = NSToolbarItem(itemIdentifier: .oeGridSize)
        item.view = slider
        item.label = NSLocalizedString("Grid Size", comment:"Toolbar, grid size slider label")
        item.menuFormRepresentation = gridSizeMenu
        
        itemCache[item.itemIdentifier] = item
        return item
    }
    
    let gridSizeMenu: NSMenuItem = {
        
        let decrease = NSMenuItem()
        decrease.title = NSLocalizedString("Decrease Grid Size", comment: "")
        decrease.action = #selector(LibraryController.decreaseGridSize(_:))
        
        let increase = NSMenuItem()
        increase.title = NSLocalizedString("Increase Grid Size", comment: "")
        increase.action = #selector(LibraryController.increaseGridSize(_:))
        
        let menu = NSMenu()
        menu.items = [decrease, increase]
        
        let gridSizeMenu = NSMenuItem()
        gridSizeMenu.title = NSLocalizedString("Grid Size", comment:"")
        gridSizeMenu.submenu = menu
        
        return gridSizeMenu
    }()
    
    // MARK: - View Mode
    
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
        item.menuFormRepresentation = viewModeMenu
        item.label = NSLocalizedString("View Mode", comment:"Toolbar, view mode selector label")
        
        itemCache[item.itemIdentifier] = item
        return item
    }
    
    let viewModeMenu: NSMenuItem = {
        
        let gridView = NSMenuItem()
        gridView.title = NSLocalizedString("as Grid", tableName: "MainMenu", comment: "")
        gridView.action = #selector(LibraryController.switchToGridView(_:))
        
        let listView = NSMenuItem()
        listView.title = NSLocalizedString("as List", tableName: "MainMenu", comment: "")
        listView.action = #selector(LibraryController.switchToListView(_:))
        
        let menu = NSMenu()
        menu.items = [gridView, listView]
        
        let viewModeMenu = NSMenuItem()
        viewModeMenu.title = NSLocalizedString("View", tableName: "MainMenu", comment:"")
        viewModeMenu.submenu = menu
        
        return viewModeMenu
    }()
    
    // MARK: - Category
    
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
        item.visibilityPriority = .high
        item.menuFormRepresentation = categoryMenu
        item.label = NSLocalizedString("Category", comment:"Toolbar, category selector label")
        
        itemCache[item.itemIdentifier] = item
        return item
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
    
    // MARK: - Search
    
    lazy var searchField: OESearchField = {
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
        return searchField
    }()
    
    @discardableResult
    func searchToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeSearch] {
            return item
        }
        
        let searchField = self.searchField
        
        toolbar.searchField = searchField
        
        let item = NSToolbarItem(itemIdentifier: .oeSearch)
        item.view = searchField
        item.label = NSLocalizedString("Search", comment:"Toolbar, search field label")
        
        itemCache[item.itemIdentifier] = item
        return item
    }
    
    @available(macOS 11.0, *)
    @discardableResult
    func searchToolbarItem11(toolbar: LibraryToolbar) -> NSSearchToolbarItem {
        if let item = itemCache[.oeSearch] {
            return item as! NSSearchToolbarItem
        }
        
        let searchField = self.searchField
        
        toolbar.searchField = searchField
        
        let item = NSSearchToolbarItem(itemIdentifier: .oeSearch)
        item.searchField = searchField
        item.label = NSLocalizedString("Search", comment:"Toolbar, search field label")
        
        itemCache[item.itemIdentifier] = item
        return item
    }
    
    // MARK: - Add
    
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
        
        let addMenu = NSMenuItem()
        addMenu.title = NSLocalizedString("Add", comment:"")
        addMenu.submenu = self.addMenu
        item.menuFormRepresentation = addMenu
        
        itemCache[item.itemIdentifier] = item
        return item
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
                      .separator(),
                      newCollection]
        
        return menu
    }()
}
