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

@objc(OELibraryToolbarDelegate)
class LibraryToolbarDelegate: NSObject, NSToolbarDelegate {
    
    final let OEToolbarItemIdentifierGridSize = NSToolbarItem.Identifier("OEToolbarItemIdentifierGridSize")
    final let OEToolbarItemIdentifierDecreaseGridSize = NSToolbarItem.Identifier("OEToolbarItemIdentifierDecreaseGridSize")
    final let OEToolbarItemIdentifierIncreaseGridSize = NSToolbarItem.Identifier("OEToolbarItemIdentifierIncreaseGridSize")
    final let OEToolbarItemIdentifierViewMode = NSToolbarItem.Identifier("OEToolbarItemIdentifierViewMode")
    final let OEToolbarItemIdentifierSearch = NSToolbarItem.Identifier("OEToolbarItemIdentifierSearch")
    final let OEToolbarItemIdentifierCategory = NSToolbarItem.Identifier("OEToolbarItemIdentifierCategory")
    
    var itemCache = [String: NSToolbarItem]()
    
    @IBOutlet weak var toolbarOwner: AnyObject?
    
    
    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return [.flexibleSpace,
                OEToolbarItemIdentifierGridSize,
                OEToolbarItemIdentifierDecreaseGridSize,
                OEToolbarItemIdentifierIncreaseGridSize,
                OEToolbarItemIdentifierViewMode,
                OEToolbarItemIdentifierSearch,
                OEToolbarItemIdentifierCategory]
    }
    
    
    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        cacheItems(toolbar: toolbar as! LibraryToolbar)
        return [OEToolbarItemIdentifierViewMode,
                OEToolbarItemIdentifierDecreaseGridSize,
                OEToolbarItemIdentifierGridSize,
                OEToolbarItemIdentifierIncreaseGridSize,
                OEToolbarItemIdentifierCategory,
                .flexibleSpace,
                OEToolbarItemIdentifierSearch]
    }
    
    
    func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return []
    }
    
    
    func cacheItems(toolbar: LibraryToolbar) {
        let _ = gridSizeToolbarItem(toolbar: toolbar)
        let _ = decreaseGridSizeToolbarItem(toolbar: toolbar)
        let _ = increaseGridSizeToolbarItem(toolbar: toolbar)
        let _ = viewModeToolbarItem(toolbar: toolbar)
        let _ = categoryToolbarItem(toolbar: toolbar)
        let _ = searchToolbarItem(toolbar: toolbar)
    }
    
    
    func toolbar(_ toolbar: NSToolbar, itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier, willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem? {
        switch itemIdentifier {
        case OEToolbarItemIdentifierGridSize:
            return gridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case OEToolbarItemIdentifierDecreaseGridSize:
            return decreaseGridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case OEToolbarItemIdentifierIncreaseGridSize:
            return increaseGridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case OEToolbarItemIdentifierViewMode:
            return viewModeToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case OEToolbarItemIdentifierCategory:
            return categoryToolbarItem(toolbar: toolbar as! LibraryToolbar)
        case OEToolbarItemIdentifierSearch:
            return searchToolbarItem(toolbar: toolbar as! LibraryToolbar)
        default:
            return NSToolbarItem(itemIdentifier: itemIdentifier)
        }
    }
    
    
    func gridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierGridSize.rawValue] {
            return item
        }
        let item = ValidatingToolbarItem(itemIdentifier: OEToolbarItemIdentifierGridSize)
        
        let slider = NSSlider(value: 1.0, minValue: 0.5, maxValue: 2.5, target: toolbarOwner, action: #selector(OECollectionViewController.changeGridSize(_:)))
        if #available(macOS 11.0, *) {
            slider.controlSize = .mini
        } else {
            slider.controlSize = .small
        }
        
        item.view = slider
        item.label = NSLocalizedString("Grid Size", comment:"Grid size toolbar button label, main window")
        item.maxSize.width = 64
        toolbar.gridSizeSlider = slider
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func decreaseGridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierDecreaseGridSize.rawValue] {
            return item
        }
        let item = ValidatingToolbarItem(itemIdentifier: OEToolbarItemIdentifierDecreaseGridSize)
        
        let minHint = NSButton(image: NSImage(named: "grid_slider_min")!, target: toolbarOwner, action: #selector(OECollectionViewController.decreaseGridSize(_:)))
        minHint.isBordered = false
        
        item.view = minHint
        item.label = NSLocalizedString("Decrease Grid Size", comment:"Grid size toolbar button label, main window")
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func increaseGridSizeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierIncreaseGridSize.rawValue] {
            return item
        }
        let item = ValidatingToolbarItem(itemIdentifier: OEToolbarItemIdentifierIncreaseGridSize)
        
        let maxHint = NSButton(image: NSImage(named: "grid_slider_max")!, target: toolbarOwner, action: #selector(OECollectionViewController.increaseGridSize(_:)))
        maxHint.isBordered = false
        
        item.view = maxHint
        item.label = NSLocalizedString("Increase Grid Size", comment:"Grid size toolbar button label, main window")
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func viewModeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierViewMode.rawValue] {
            return item
        }
        let item = ValidatingToolbarItem(itemIdentifier: OEToolbarItemIdentifierViewMode)
        
        let segmControl = NSSegmentedControl()
        segmControl.segmentCount = 2
        segmControl.setImage(NSImage(named: "NSIconViewTemplate"), forSegment: 0)
        segmControl.setImage(NSImage(named: "NSListViewTemplate"), forSegment: 1)
        segmControl.setToolTip(NSLocalizedString("Switch To Grid View", comment: "Tooltip"), forSegment: 0)
        segmControl.setToolTip(NSLocalizedString("Switch To List View", comment: "Tooltip"), forSegment: 1)
        segmControl.sizeToFit()
        segmControl.action = #selector(OELibraryController.switchToView(_:))
        segmControl.target = toolbarOwner
        
        item.view = segmControl
        item.label = NSLocalizedString("View Mode", comment:"View mode toolbar button label, main window")
        toolbar.viewModeSelector = segmControl
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func categoryToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierCategory.rawValue] {
            return item
        }
        let item = ValidatingToolbarItem(itemIdentifier: OEToolbarItemIdentifierCategory)
        
        let segmControl = NSSegmentedControl()
        segmControl.segmentCount = 4
        segmControl.setLabel(NSLocalizedString("Library", comment: "Main window, Library category button"), forSegment: 0)
        segmControl.setLabel(NSLocalizedString("Save States", comment: "Main window, Save States category button"), forSegment: 1)
        segmControl.setLabel(NSLocalizedString("Screenshots", comment: "Main window, Screenshots category button"), forSegment: 2)
        segmControl.setLabel(NSLocalizedString("Homebrew", comment: "Main window, Homebrew category button"), forSegment: 3)
        segmControl.sizeToFit()
        segmControl.action = #selector(OELibraryController.switchCategory(_:))
        segmControl.target = toolbarOwner
        
        item.view = segmControl
        item.label = NSLocalizedString("Category", comment:"Category selector toolbar label, main window")
        toolbar.categorySelector = segmControl
        
        let libraryMenuItem = NSMenuItem()
        libraryMenuItem.title = NSLocalizedString("Library", comment: "")
        libraryMenuItem.target = toolbarOwner
        libraryMenuItem.tag = 100
        libraryMenuItem.action = #selector(OELibraryController.switchCategoryFromMenu(_:))
        
        let savesStatesMenuItem = NSMenuItem()
        savesStatesMenuItem.title = NSLocalizedString("Save States", comment: "")
        savesStatesMenuItem.target = toolbarOwner
        savesStatesMenuItem.tag = 101
        savesStatesMenuItem.action = #selector(OELibraryController.switchCategoryFromMenu(_:))
        
        let screenshotsMenuItem = NSMenuItem()
        screenshotsMenuItem.title = NSLocalizedString("Screenshots", comment: "")
        screenshotsMenuItem.target = toolbarOwner
        screenshotsMenuItem.tag = 102
        screenshotsMenuItem.action = #selector(OELibraryController.switchCategoryFromMenu(_:))
        
        let homebrewMenuItem = NSMenuItem()
        homebrewMenuItem.title = NSLocalizedString("Homebrew", comment: "")
        homebrewMenuItem.target = toolbarOwner
        homebrewMenuItem.tag = 103
        homebrewMenuItem.action = #selector(OELibraryController.switchCategoryFromMenu(_:))
        
        let menu = NSMenu()
        menu.addItem(libraryMenuItem)
        menu.addItem(savesStatesMenuItem)
        menu.addItem(screenshotsMenuItem)
        menu.addItem(homebrewMenuItem)
        
        let menuFormRepresentation = NSMenuItem()
        menuFormRepresentation.title = NSLocalizedString("Category", comment:"")
        menuFormRepresentation.submenu = menu
        
        item.menuFormRepresentation = menuFormRepresentation
        
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func searchToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierSearch.rawValue] {
            return item
        }
        let item = ValidatingToolbarItem(itemIdentifier: OEToolbarItemIdentifierSearch)
        
        let searchField = OESearchField()
        searchField.lineBreakMode = .byClipping
        searchField.usesSingleLineMode = true
        searchField.cell?.isScrollable = true
        searchField.sendsWholeSearchString = false
        searchField.sendsSearchStringImmediately = true
        searchField.font = NSFont.systemFont(ofSize: 13.0)
        searchField.action = #selector(OELibraryController.search(_:))
        searchField.target = toolbarOwner
        
        item.view = searchField;
        item.label = NSLocalizedString("Search", comment:"Search field toolbar label, main window")
        item.maxSize.width = 166
        toolbar.searchField = searchField
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
}

