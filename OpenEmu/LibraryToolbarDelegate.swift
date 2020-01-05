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
    final let OEToolbarItemIdentifierViewMode = NSToolbarItem.Identifier("OEToolbarItemIdentifierViewMode")
    final let OEToolbarItemIdentifierSearch = NSToolbarItem.Identifier("OEToolbarItemIdentifierSearch")
    final let OEToolbarItemIdentifierCategory = NSToolbarItem.Identifier("OEToolbarItemIdentifierCategory")
    
    var itemCache = [String: NSToolbarItem]()
    
    @IBOutlet weak var toolbarOwner: AnyObject?
    
    
    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return [.flexibleSpace,
                OEToolbarItemIdentifierGridSize,
                OEToolbarItemIdentifierViewMode,
                OEToolbarItemIdentifierSearch,
                OEToolbarItemIdentifierCategory]
    }
    
    
    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        cacheItems(toolbar: toolbar as! LibraryToolbar)
        return [OEToolbarItemIdentifierViewMode,
                OEToolbarItemIdentifierGridSize,
                .flexibleSpace,
                OEToolbarItemIdentifierCategory,
                .flexibleSpace,
                OEToolbarItemIdentifierSearch]
    }
    
    
    func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return []
    }
    
    
    func cacheItems(toolbar: LibraryToolbar) {
        let _ = gridSizeToolbarItem(toolbar: toolbar)
        let _ = viewModeToolbarItem(toolbar: toolbar)
        let _ = categoryToolbarItem(toolbar: toolbar)
        let _ = searchToolbarItem(toolbar: toolbar)
    }
    
    
    func toolbar(_ toolbar: NSToolbar, itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier, willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem? {
        switch itemIdentifier {
        case OEToolbarItemIdentifierGridSize:
            return gridSizeToolbarItem(toolbar: toolbar as! LibraryToolbar)
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
        let item = ControlToolbarItem(itemIdentifier: OEToolbarItemIdentifierGridSize)
        
        let view = NSSlider(frame: NSRect(x: 20, y: 0, width: 64, height: 15))
        view.controlSize = .small
        view.isContinuous = true
        view.minValue = 0.5
        view.maxValue = 2.5
        view.doubleValue = 1.0
        view.sliderType = .linear
        view.action = #selector(OECollectionViewController.changeGridSize(_:))
        view.target = toolbarOwner
        view.awakeFromNib()
        
        item.view = view
        item.label = NSLocalizedString("Grid Size", comment:"Grid size toolbar button label, main window")
        item.minSize = view.frame.size
        item.maxSize = view.frame.size
        toolbar.gridSizeSlider = view
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func viewModeToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierViewMode.rawValue] {
            return item
        }
        let item = ControlToolbarItem(itemIdentifier: OEToolbarItemIdentifierViewMode)
        
        let view = NSSegmentedControl(frame: NSRect(x: 0, y: 0, width: 54, height: 24))
        view.segmentCount = 2
        view.setImage(NSImage(named: NSImage.iconViewTemplateName), forSegment: 0)
        view.setImage(NSImage(named: NSImage.listViewTemplateName), forSegment: 1)
        view.sizeToFit()
        view.action = #selector(OELibraryController.switchViewMode(_:))
        view.target = toolbarOwner
        
        item.view = view
        item.label = NSLocalizedString("View Mode", comment:"View mode toolbar button label, main window")
        item.minSize = view.frame.size
        item.maxSize = item.minSize
        toolbar.viewSelector = view
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func categoryToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierCategory.rawValue] {
            return item
        }
        let item = NSToolbarItem(itemIdentifier: OEToolbarItemIdentifierCategory)
        
        let view = NSSegmentedControl(frame: NSRect(x: 0, y: 0, width: 50, height: 24))
        view.segmentCount = 4
        view.setLabel(NSLocalizedString("Library", comment: "Main window, Library category button"), forSegment: 0)
        view.setLabel(NSLocalizedString("Save States", comment: "Main window, Save States category button"), forSegment: 1)
        view.setLabel(NSLocalizedString("Screenshots", comment: "Main window, Screenshots category button"), forSegment: 2)
        view.setLabel(NSLocalizedString("Homebrew", comment: "Main window, Homebrew category button"), forSegment: 3)
        view.sizeToFit()
        view.action = #selector(OELibraryController.switchCategory(_:))
        view.target = toolbarOwner
        
        item.view = view
        item.label = NSLocalizedString("Category", comment:"Category selector toolbar label, main window")
        item.minSize = view.frame.size
        item.maxSize = item.minSize
        toolbar.categorySelector = view
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
    
    
    func searchToolbarItem(toolbar: LibraryToolbar) -> NSToolbarItem {
        if let item = itemCache[OEToolbarItemIdentifierSearch.rawValue] {
            return item
        }
        let item = ControlToolbarItem(itemIdentifier: OEToolbarItemIdentifierSearch)
        
        let view = NSSearchField(frame: NSRect(x: 0, y: 0, width: 166, height: 22))
        view.wantsLayer = true
        view.lineBreakMode = .byClipping
        view.usesSingleLineMode = true
        // searchCell.isScrollable = true
        view.sendsWholeSearchString = false
        view.sendsSearchStringImmediately = true
        view.font = NSFont.systemFont(ofSize: 13.0)
        view.textColor = NSColor.controlBackgroundColor
        view.action = #selector(OELibraryController.search(_:))
        view.target = toolbarOwner
        
        item.view = view;
        item.label = NSLocalizedString("Search", comment:"Search field toolbar label, main window")
        item.minSize = view.frame.size
        item.maxSize = item.minSize
        toolbar.searchField = view
        itemCache[item.itemIdentifier.rawValue] = item
        return item;
    }
}

class ControlToolbarItem: NSToolbarItem {
    override func validate() {
        control?.isEnabled = {
            guard
                let action = self.action,
                let validator = NSApp.target(forAction: action, to: self.target, from: self) as AnyObject?
                else { return false }
            
            switch validator {
            case let validator as NSToolbarItemValidation:
                return validator.validateToolbarItem(self)
            case let validator as NSUserInterfaceValidations:
                return validator.validateUserInterfaceItem(self)
            default:
                return true
            }
        }()
    }
    
    final var control: NSControl? {
        return view as? NSControl
    }
}
