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
    public static let oeGridSize = Self("OEToolbarItemIdentifierGridSize")
    public static let oeDecreaseGridSize = Self("OEToolbarItemIdentifierDecreaseGridSize")
    public static let oeIncreaseGridSize = Self("OEToolbarItemIdentifierIncreaseGridSize")
    public static let oeViewMode = Self("OEToolbarItemIdentifierViewMode")
    public static let oeSearch = Self("OEToolbarItemIdentifierSearch")
    public static let oeCategory = Self("OEToolbarItemIdentifierCategory")
}

@objc(OELibraryToolbarDelegate)
class LibraryToolbarDelegate: NSObject, NSToolbarDelegate {
    var itemCache = [NSToolbarItem.Identifier: NSToolbarItem]()
    
    @IBOutlet weak var toolbarOwner: AnyObject?
    
    
    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return [.flexibleSpace,
                .oeGridSize,
                .oeDecreaseGridSize,
                .oeIncreaseGridSize,
                .oeViewMode,
                .oeSearch,
                .oeCategory]
    }
    
    
    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        cacheItems(toolbar: toolbar as! LibraryToolbar)
        return [.oeViewMode,
                .oeDecreaseGridSize,
                .oeGridSize,
                .oeIncreaseGridSize,
                .oeCategory,
                .flexibleSpace,
                .oeSearch]
    }
    
    
    func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return []
    }
    
    
    func cacheItems(toolbar: LibraryToolbar) {
        gridSizeToolbarItem(toolbar: toolbar)
        decreaseGridSizeToolbarItem(toolbar: toolbar)
        increaseGridSizeToolbarItem(toolbar: toolbar)
        viewModeToolbarItem(toolbar: toolbar)
        categoryToolbarItem(toolbar: toolbar)
        searchToolbarItem(toolbar: toolbar)
    }
    
    
    func toolbar(_ toolbar: NSToolbar, itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier, willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem? {
        
        switch itemIdentifier {
        case .oeGridSize:
            return gridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeDecreaseGridSize:
            return decreaseGridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeIncreaseGridSize:
            return increaseGridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeViewMode:
            return viewModeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeCategory:
            return categoryToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case .oeSearch:
            return searchToolbarItem(toolbar: toolbar as! LibraryToolbar)
        default:
            return NSToolbarItem(itemIdentifier: itemIdentifier)
        }
    }
    
    
    @discardableResult
    func gridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeGridSize] {
            return item
        }
        let item = NSToolbarItem(itemIdentifier: .oeGridSize)
        
        let slider = NSSlider(value: 1.0, minValue: 0.5, maxValue: 2.5, target: toolbarOwner, action: #selector(LibraryController.changeGridSize(_:)))
        slider.isContinuous = true
        if #available(macOS 11.0, *) {
            slider.controlSize = .mini
        } else {
            slider.controlSize = .small
        }
        
        NSLayoutConstraint(item: slider, attribute: .width, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1, constant: 64).isActive = true
        
        item.view = slider
        item.label = NSLocalizedString("Grid Size", comment:"Grid size toolbar button label, main window")
        toolbar.gridSizeSlider = slider
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    
    @discardableResult
    func decreaseGridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeDecreaseGridSize] {
            return item
        }
        let item = NSToolbarItem(itemIdentifier: .oeDecreaseGridSize)
        
        let minHint = NSButton(image: NSImage(named: "grid_slider_min")!, target: toolbarOwner, action: #selector(LibraryController.decreaseGridSize(_:)))
        minHint.isBordered = false
        
        item.view = minHint
        item.label = NSLocalizedString("Decrease Grid Size", comment:"Grid size toolbar button label, main window")
        toolbar.decreaseGridSizeButton = minHint
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    
    @discardableResult
    func increaseGridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeIncreaseGridSize] {
            return item
        }
        let item = NSToolbarItem(itemIdentifier: .oeIncreaseGridSize)
        
        let maxHint = NSButton(image: NSImage(named: "grid_slider_max")!, target: toolbarOwner, action: #selector(LibraryController.increaseGridSize(_:)))
        maxHint.isBordered = false
        
        item.view = maxHint
        item.label = NSLocalizedString("Increase Grid Size", comment:"Grid size toolbar button label, main window")
        toolbar.increaseGridSizeButton = maxHint
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    
    @discardableResult
    func viewModeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeViewMode] {
            return item
        }
        let item = NSToolbarItem(itemIdentifier: .oeViewMode)
        
        let images = [NSImage(named: NSImage.iconViewTemplateName)!,
                      NSImage(named: NSImage.listViewTemplateName)!]
        
        let segmControl = NSSegmentedControl(images: images, trackingMode: .selectOne, target: toolbarOwner, action: #selector(LibraryController.switchToView(_:)))
        segmControl.setToolTip(NSLocalizedString("Switch To Grid View", comment: "Tooltip"), forSegment: 0)
        segmControl.setToolTip(NSLocalizedString("Switch To List View", comment: "Tooltip"), forSegment: 1)
        segmControl.setWidth(26, forSegment: 0)
        segmControl.setWidth(26, forSegment: 1)
        
        item.view = segmControl
        item.visibilityPriority = .low
        item.label = NSLocalizedString("View Mode", comment:"View mode toolbar button label, main window")
        toolbar.viewModeSelector = segmControl
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    
    @discardableResult
    func categoryToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeCategory] {
            return item
        }
        let item = NSToolbarItem(itemIdentifier: .oeCategory)
        
        let titles = [NSLocalizedString("Toolbar: Library", value: "Library", comment: "toolbar, category label"),
                      NSLocalizedString("Toolbar: Save States", value: "Save States", comment: "toolbar, category label"),
                      NSLocalizedString("Toolbar: Screenshots", value: "Screenshots", comment: "toolbar, category label"),
                      NSLocalizedString("Toolbar: Homebrew", value: "Homebrew", comment: "toolbar, category label")]
        
        let segmControl = NSSegmentedControl(labels: titles, trackingMode: .selectOne, target: toolbarOwner, action: #selector(LibraryController.switchCategory(_:)))
        
        item.view = segmControl
        item.label = NSLocalizedString("Category", comment:"Category selector toolbar label, main window")
        toolbar.categorySelector = segmControl
        
        let libraryMenuItem = NSMenuItem()
        libraryMenuItem.title = NSLocalizedString("Toolbar: Library", value: "Library", comment: "")
        libraryMenuItem.target = toolbarOwner
        libraryMenuItem.tag = 100
        libraryMenuItem.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let savesStatesMenuItem = NSMenuItem()
        savesStatesMenuItem.title = NSLocalizedString("Toolbar: Save States", value: "Save States", comment: "")
        savesStatesMenuItem.target = toolbarOwner
        savesStatesMenuItem.tag = 101
        savesStatesMenuItem.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let screenshotsMenuItem = NSMenuItem()
        screenshotsMenuItem.title = NSLocalizedString("Toolbar: Screenshots", value: "Screenshots", comment: "")
        screenshotsMenuItem.target = toolbarOwner
        screenshotsMenuItem.tag = 102
        screenshotsMenuItem.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let homebrewMenuItem = NSMenuItem()
        homebrewMenuItem.title = NSLocalizedString("Toolbar: Homebrew", value: "Homebrew", comment: "")
        homebrewMenuItem.target = toolbarOwner
        homebrewMenuItem.tag = 103
        homebrewMenuItem.action = #selector(LibraryController.switchCategoryFromMenu(_:))
        
        let menu = NSMenu()
        menu.addItem(libraryMenuItem)
        menu.addItem(savesStatesMenuItem)
        menu.addItem(screenshotsMenuItem)
        menu.addItem(homebrewMenuItem)
        
        let menuFormRepresentation = NSMenuItem()
        menuFormRepresentation.title = NSLocalizedString("Category", comment:"")
        menuFormRepresentation.submenu = menu
        
        item.menuFormRepresentation = menuFormRepresentation
        
        itemCache[item.itemIdentifier] = item
        return item;
    }
    
    
    @discardableResult
    func searchToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[.oeSearch] {
            return item
        }
        let item = NSToolbarItem(itemIdentifier: .oeSearch)
        
        let searchField = OESearchField()
        searchField.lineBreakMode = .byClipping
        searchField.usesSingleLineMode = true
        searchField.cell?.isScrollable = true
        searchField.sendsWholeSearchString = false
        searchField.sendsSearchStringImmediately = true
        searchField.font = NSFont.systemFont(ofSize: 13.0)
        searchField.action = #selector(LibraryController.search(_:))
        searchField.target = toolbarOwner
        
        NSLayoutConstraint(item: searchField, attribute: .width, relatedBy: .lessThanOrEqual, toItem: nil, attribute: .notAnAttribute, multiplier: 1, constant: 166).isActive = true
        
        item.view = searchField;
        item.label = NSLocalizedString("Search", comment:"Search field toolbar label, main window")
        toolbar.searchField = searchField
        itemCache[item.itemIdentifier] = item
        return item;
    }
}

