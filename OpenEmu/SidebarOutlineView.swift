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

import Cocoa

@objc(OESidebarOutlineView)
class SidebarOutlineView: NSOutlineView {
    
    // This should only be used for drawing the highlight
    private var highlightedRow = -1
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        if highlightedRow != -1 {
            drawDropHighlight(onRow: highlightedRow)
        }
    }
    
    // MARK: - Input Handling
    
    override func mouseDown(with event: NSEvent) {
        // ignore control-clicks, those will go directly to -menuForEvent:
        // before eventually arriving here
        if !event.modifierFlags.contains(.control) {
            super.mouseDown(with: event)
        }
    }
    
    override func keyDown(with event: NSEvent) {
        if event.keyCode == 51 || event.keyCode == 117 {
            NSApp.sendAction(#selector(OESidebarController.removeSelectedItems(of:)), to: dataSource, from: self)
        // ignore left and right arrow keys; pressing them causes flickering
        } else if event.keyCode == 123 || event.keyCode == 124 {
            return
        } else {
            super.keyDown(with: event)
        }
    }
    
    // MARK: - Menu
    
    override func menu(for event: NSEvent) -> NSMenu? {
        
        window?.makeFirstResponder(self)
        
        let mouseLocationInWindow = event.locationInWindow
        let mouseLocationInView = convert(mouseLocationInWindow, from: nil)
        let index = row(at: mouseLocationInView)
        
        guard index != -1 else { return nil }
        let item = self.item(atRow: index) as! OESidebarItem
        
        highlightedRow = index
        needsDisplay = true
        
        let menu = NSMenu()
        var menuItem: NSMenuItem
        
        if item.isGroupHeaderInSidebar {
            if index == 0 {
                if let systems = OEDBSystem.allSystems(in: OELibraryDatabase.default!.mainThreadContext) {
                    
                    for system in systems {
                        menuItem = NSMenuItem()
                        menuItem.title = system.name
                        menuItem.action = #selector(toggleSystem(for:))
                        menuItem.representedObject = system
                        menuItem.state = system.enabled?.boolValue ?? false ? .on : .off
                        menu.addItem(menuItem)
                    }
                }
            }
        }
        else if item is OEDBSystem {
            
            if let cores = OECorePlugin.corePlugins(forSystemIdentifier: (item as! OEDBSystem).systemIdentifier),
               cores.count > 1 {
                
                let systemIdentifier = (item as? OEDBSystem)?.systemIdentifier
                let defaultCoreKey = "defaultCore.\(systemIdentifier ?? "")"
                let defaultCoreIdentifier = UserDefaults.standard.object(forKey: defaultCoreKey) as? String
                
                let coreItem = NSMenuItem()
                coreItem.title = NSLocalizedString("Default Core", comment: "Sidebar context menu item to pick default core for a system")
                let submenu = NSMenu()
                
                for core in cores {
                    let coreName = core.displayName
                    let systemIdentifier = (item as? OEDBSystem)?.systemIdentifier
                    let coreIdentifier = core.bundleIdentifier
                    
                    let item = NSMenuItem()
                    item.title = coreName ?? ""
                    item.action = #selector(OESidebarController.changeDefaultCore(_:))
                    item.state = (coreIdentifier == defaultCoreIdentifier) ? .on : .off
                    item.representedObject = ["core": coreIdentifier,
                                            "system": systemIdentifier]
                    submenu.addItem(item)
                }
                coreItem.submenu = submenu
                menu.addItem(coreItem)
            }
            
            menuItem = NSMenuItem()
            menuItem.title = .localizedStringWithFormat(NSLocalizedString("Hide \"%@\"", comment: ""), (item as? OEDBSystem)?.name ?? "")
            menuItem.action = #selector(toggleSystem(for:))
            menuItem.representedObject = item
            menu.addItem(menuItem)
        }
        else {
            if item.isEditableInSidebar {
                
                menuItem = NSMenuItem()
                menuItem.title = .localizedStringWithFormat(NSLocalizedString("Rename \"%@\"", comment: "Rename collection sidebar context menu item"), item.sidebarName)
                menuItem.action = #selector(renameRow(for:))
                menuItem.tag = index
                menu.addItem(menuItem)
                
                menuItem = NSMenuItem()
                menuItem.title = NSLocalizedString("Duplicate Collection", comment: "")
                menuItem.action = #selector(duplicateCollection(for:))
                menuItem.representedObject = item
                menu.addItem(menuItem)
                
                menuItem = NSMenuItem()
                menuItem.title = NSLocalizedString("Delete Collection", comment: "")
                menuItem.action = #selector(removeRow(for:))
                menuItem.tag = index
                menu.addItem(menuItem)
                
                menu.addItem(NSMenuItem.separator())
            }
            
            menuItem = NSMenuItem()
            menuItem.title = NSLocalizedString("New Collection", comment: "")
            menuItem.action = #selector(LibraryController.newCollection(_:))
            menu.addItem(menuItem)
        }
        
        if menu.items.count != 0 {
            NSMenu.popUpContextMenu(menu, with: event, for: self)
        }
        
        highlightedRow = -1
        needsDisplay = true
        
        return nil
    }
    
    @objc private func renameRow(for menuItem: NSMenuItem) {
        NSApp.sendAction(#selector(OESidebarController.renameItem(for:)), to: dataSource, from: menuItem)
    }
    
    @objc private func removeRow(for menuItem: NSMenuItem) {
        NSApp.sendAction(#selector(OESidebarController.removeItem(for:)), to: dataSource, from: menuItem)
    }
    
    @objc private func toggleSystem(for menuItem: NSMenuItem) {
        let system = menuItem.representedObject as? OEDBSystem
        system?.toggleEnabledAndPresentError()
    }
    
    @objc private func duplicateCollection(for menuItem: NSMenuItem) {
        NSApp.sendAction(#selector(OESidebarController.duplicateCollection(_:)), to: dataSource, from: menuItem.representedObject)
    }
    
    // MARK: - Calculating Rects
    
    override func frameOfOutlineCell(atRow row: Int) -> NSRect {
        return .zero
    }
    
    // MARK: - Drop Highlight
    
    @objc func _drawDropHighlightOnEntireTableView() {
        drawDropHighlight(onRow: -1)
    }
    
    private func drawDropHighlight(onRow row: Int) {
        
        var rect: NSRect
        
        if row == -1 {
            let rawVisibleRect = convert(visibleRect, to: nil)
            let contentRect = window?.contentLayoutRect ?? .zero
            let realVisibleRect = rawVisibleRect.intersection(contentRect)
            rect = convert(realVisibleRect, from: nil).insetBy(dx: 0, dy: 1)
        } else if row == 0 {
            return
        } else {
            rect = self.rect(ofRow: row)
        }
        rect = rect.insetBy(dx: 3, dy: 1)
        
        var dropCornerRadius: CGFloat
        if #available(macOS 11.0, *) {
            dropCornerRadius = 5
        } else {
            dropCornerRadius = 6
        }
        
        let path = NSBezierPath(roundedRect: rect, xRadius: dropCornerRadius, yRadius: dropCornerRadius)
        path.lineWidth = 2
        
        NSColor.controlAccentColor.setStroke()
        path.stroke()
    }
}
