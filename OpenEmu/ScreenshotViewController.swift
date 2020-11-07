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

@objc
class ScreenshotViewController: ImageCollectionViewController {
    override var representedObject: Any? {
        willSet {
            precondition(newValue == nil || newValue is OEDBScreenshotsMedia, "unexpected object")
        }
    }
    
    var dataSource = ImagesDataSource<OEDBScreenshot>(gameKeyPath: \.rom?.game,
                                                      titleKeyPath: \.name,
                                                      timestampKeyPath: \.timestamp,
                                                      imageURLKeyPath: \.screenshotURL,
                                                      sortDescriptors: [
                                                        NSSortDescriptor(keyPath: \OEDBScreenshot.rom?.game?.name, ascending: true),
                                                        NSSortDescriptor(keyPath: \OEDBScreenshot.timestamp, ascending: true)],
                                                      entityName: OEDBScreenshot.entityName)
    
    override func viewDidLoad() {
        self.representedObject = OEDBScreenshotsMedia.shared
        self.dataSourceDelegate = dataSource
        super.viewDidLoad()
    }
}

extension ScreenshotViewController: CollectionViewExtendedDelegate, NSMenuItemValidation {
    func collectionView(_ collectionView: CollectionView, setTitle title: String, forItemAt indexPath: IndexPath) {
        guard let item = dataSource.item(at: indexPath), !title.isEmpty else { return }
        
        item.name = title
        item.updateFile()
        item.save()
        
        if let itemView = self.collectionView.item(at: indexPath) as? ImageCollectionViewItem {
            dataSource.loadItemView(itemView, at: indexPath)
        }
    }
    
    @objc func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        if let sel = menuItem.action, sel == #selector(showInFinder(_:)) || sel == #selector(delete(_:)) {
            return collectionView.selectionIndexPaths.count > 0
        }
        
        return true
    }
    
    func collectionView(_ collectionView: CollectionView, menuForItemsAt indexPaths: Set<IndexPath>) -> NSMenu? {
        let menu    = NSMenu()
        let subMenu = shareMenu(forItemsAt: indexPaths)
        
        let share = menu.addItem(withTitle: NSLocalizedString("Share", comment: "SaveState View Context menu"),
                                 action: nil,
                                 keyEquivalent: "")
        share.submenu = subMenu
        
        if indexPaths.count == 1 {
            menu.addItem(withTitle: NSLocalizedString("Rename", comment: "SaveState View Context menu"),
                         action: #selector(CollectionView.beginEditingWithSelectedItem(_:)),
                         keyEquivalent: "")
            
            menu.addItem(withTitle: NSLocalizedString("Show in Finder", comment: "SaveState View Context menu"),
                         action: #selector(showInFinder(_:)),
                         keyEquivalent: "")
            
            menu.addItem(withTitle: NSLocalizedString("Delete Screenshot", comment: "Screenshot View Context menu"),
                         action: #selector(deleteSelectedItems(_:)),
                         keyEquivalent: "")
        } else {
            menu.addItem(withTitle: NSLocalizedString("Show in Finder", comment: "SaveState View Context menu"),
                         action: #selector(showInFinder(_:)),
                         keyEquivalent: "")
            
            menu.addItem(withTitle: NSLocalizedString("Delete Screenshots", comment: "Screenshot View Context menu (plural)"),
                         action: #selector(deleteSelectedItems(_:)),
                         keyEquivalent: "")
        }
        
        return menu
    }
    
    func shareMenu(forItemsAt indexPaths: Set<IndexPath>) -> NSMenu {
        let menu = NSMenu()
        
        let urls  = dataSourceDelegate.imageURLs(forItemsAt: indexPaths)
        let sharing = NSSharingService.sharingServices(forItems: urls)
        
        for service in sharing {
            let menuItem = NSMenuItem(title: service.title, action: #selector(shareFromService(_:)), keyEquivalent: "")
            menuItem.image = service.image
            menuItem.representedObject = service
            menu.addItem(menuItem)
        }
        
        return menu
    }
    
    @objc func shareFromService(_ sender: NSMenuItem?) {
        guard let menuItem = sender else { return }
        
        let urls = dataSourceDelegate.imageURLs(forItemsAt: collectionView.selectionIndexPaths)
        if let service = menuItem.representedObject as? NSSharingService {
            service.perform(withItems: urls)
        }
    }
    
    @IBAction override func delete(_ sender: Any?) {
        deleteSelectedItems(sender)
    }
    
    @IBAction func deleteSelectedItems(_ sender: Any?) {
        let items = dataSource.items(at: collectionView.selectionIndexPaths)
        if items.count == 0 {
            return
        }
        
        var alert: OEAlert
        if items.count == 1 {
            alert = .deleteScreenshot(name: items.first!.name!)
        } else {
            alert = .deleteScreenshots(count: items.count)
        }
        
        if alert.runModal() == .alertFirstButtonReturn {
            items.forEach { $0.delete() }
            try? OELibraryDatabase.default?.mainThreadContext.save()
            return
        }
        
        reloadData()
    }
    
    func collectionView(_ collectionView: CollectionView, doubleClickForItemAt indexPath: IndexPath) {
        guard
            let item = dataSource.item(at: indexPath),
            let url  = item.url.absoluteURL as URL?
        else { return }
        NSWorkspace.shared.activateFileViewerSelecting([url])
    }
}

// MARK: - Touch Bar

fileprivate extension NSTouchBar.CustomizationIdentifier {
    static let screenshotsTouchBar = "org.openemu.ScreenshotViewController.screenshotsTouchBar"
}

fileprivate extension NSTouchBarItem.Identifier {
    static let deleteScreenshot = NSTouchBarItem.Identifier("org.openemu.ScreenshotViewController.screenshotsTouchBar.delete")
    static let showScreenshotInFinder = NSTouchBarItem.Identifier("org.openemu.ScreenshotViewController.screenshotsTouchBar.showInFinder")
    static let shareScreenshot = NSTouchBarItem.Identifier("org.openemu.ScreenshotViewController.screenshotsTouchBar.share")
}

extension ScreenshotViewController {
    
    open override func makeTouchBar() -> NSTouchBar? {
        
        let touchBar = NSTouchBar()
        
        touchBar.delegate = self
        
        touchBar.customizationIdentifier = .screenshotsTouchBar
        touchBar.defaultItemIdentifiers = [.deleteScreenshot,
                                           .flexibleSpace,
                                           .showScreenshotInFinder,
                                           .shareScreenshot,
                                           .otherItemsProxy]
        touchBar.customizationAllowedItemIdentifiers = [.deleteScreenshot,
                                                        .showScreenshotInFinder,
                                                        .shareScreenshot]
        touchBar.principalItemIdentifier = .showScreenshotInFinder
        
        return touchBar
    }
    
    override func selectionDidChange() {
        guard let touchBar = self.touchBar else { return }
        let selected = self.collectionView.selectionIndexPaths
        
        if let deleteItem = touchBar.item(forIdentifier: .deleteScreenshot) as? NSCustomTouchBarItem {
            let button = deleteItem.view as! NSButton
            button.isEnabled = !selected.isEmpty
        }
        
        if let showInFinderItem = touchBar.item(forIdentifier: .showScreenshotInFinder) as? NSCustomTouchBarItem {
            let button = showInFinderItem.view as! NSButton
            button.isEnabled = !selected.isEmpty
        }
        
        if let shareItem = touchBar.item(forIdentifier: .shareScreenshot) as? NSSharingServicePickerTouchBarItem {
            shareItem.isEnabled = !selected.isEmpty
        }
    }
}

extension ScreenshotViewController: NSTouchBarDelegate {
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
        
        case .deleteScreenshot:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Delete", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarDeleteTemplateName)!, target: nil, action: #selector(deleteSelectedItems(_:)))
            
            button.isEnabled = !collectionView.selectionIndexPaths.isEmpty
            button.bezelColor = #colorLiteral(red: 0.5665243268, green: 0.2167189717, blue: 0.2198875844, alpha: 1)
            
            item.view = button
            
            return item
            
        case .showScreenshotInFinder:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Show in Finder", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.revealFreestandingTemplateName)!, target: nil, action: #selector(showInFinder(_:)))
            
            button.isEnabled = !collectionView.selectionIndexPaths.isEmpty
            
            item.view = button
            
            return item
            
        case .shareScreenshot:
            
            let item = NSSharingServicePickerTouchBarItem(identifier: identifier)
            
            item.delegate = self
            item.isEnabled = !collectionView.selectionIndexPaths.isEmpty
            
            return item
            
        default:
            return nil
        }
    }
}

extension ScreenshotViewController: NSSharingServicePickerTouchBarItemDelegate {
    public func items(for pickerTouchBarItem: NSSharingServicePickerTouchBarItem) -> [Any] {
        return dataSource.imageURLs(forItemsAt: collectionView.selectionIndexPaths).compactMap { NSImage(contentsOf: $0) }
    }
}
